#include "ModulationTarget.h"
#include "../../Common.h"
#include "../Oscillator/Oscillator.h"
#include "../Envelope/Envelope.h"
#include "../Filter/Filter.h"
#include "../Filter/TalkboxFilter.h"
#include "../LFO/LFO.h"

const std::vector<juce::String>& ModulationTarget::getAllBaseParameterIDs()
{
    static std::vector<juce::String> ids;
    if (ids.empty())
    {
        // Oscillators 
        for (int i = 0; i < NUM_OF_OSCILLATORS; ++i)
        {
            using P = Oscillator::ParamID;
            for (auto idEnum : { P::Volume, P::Pan, P::Voices, P::Detune })
                ids.push_back(Oscillator::getKnobParamSpecs(idEnum, i).id);
        }

        // Envelopes 
        for (int i = 0; i < NUM_OF_ENVELOPES; ++i)
            for (auto& spec : Envelope::getParamSpecs(i))
                ids.push_back(spec.id);

        // Filters 
        for (int i = 0; i < NUM_OF_FILTERS; ++i)
        {
            using FP = Filter::ParamID;
            for (auto idEnum : { FP::Cutoff, FP::Resonance, FP::Drive, FP::Mix })
                ids.push_back(Filter::getKnobParamSpecs(idEnum, i).id);

            ids.push_back(TalkboxFilter::getKnobParamSpecs(
                TalkboxFilter::ParamID::Morph, i).id);
            ids.push_back(TalkboxFilter::getKnobParamSpecs(
                TalkboxFilter::ParamID::Factor, i).id);
        }

        // LFOs 
        for (int i = 0; i < NUM_OF_LFOS; ++i)
        {
            using LP = LFO::ParamID;
            for (auto idEnum : { LP::Freq, LP::Shape, LP::Steps })
                ids.push_back(LFO::getKnobParamSpecs(idEnum, i).id);
        }
    }
    return ids;
}

ModulationTarget::ModulationTarget(juce::AudioProcessorValueTreeState& apvtsIn,
    ModulationRouter& router,
    const juce::String& baseParamID)
    : apvts(apvtsIn),
    modulationRouter(router),
    sourceParamID(baseParamID + "_MOD_SOURCE"),
    indexParamID(baseParamID + "_MOD_INDEX"),
    minParamID(baseParamID + "_MOD_MIN"),
    maxParamID(baseParamID + "_MOD_MAX")
{
    // Look up the core parameters
    baseParam = dynamic_cast<juce::RangedAudioParameter*>(
        apvts.getParameter(baseParamID));
    jassert(baseParam != nullptr);

    minParam = dynamic_cast<juce::AudioParameterFloat*>(
        apvts.getParameter(minParamID));
    jassert(minParam != nullptr);

    maxParam = dynamic_cast<juce::AudioParameterFloat*>(
        apvts.getParameter(maxParamID));
    jassert(maxParam != nullptr);

    // Listen for runtime changes
    apvts.addParameterListener(sourceParamID, this);
    apvts.addParameterListener(indexParamID, this);
    apvts.addParameterListener(minParamID, this);
    apvts.addParameterListener(maxParamID, this);

    // Initialize from saved state
    int    modeInt = static_cast<int>(apvts.getRawParameterValue(sourceParamID)->load());
    currentMode = static_cast<ModulationMode>(modeInt);
    currentSourceIndex = static_cast<int>(apvts.getRawParameterValue(indexParamID)->load());
    currentRange.first = apvts.getRawParameterValue(minParamID)->load();
    currentRange.second = apvts.getRawParameterValue(maxParamID)->load();

    setModulationMode(currentMode);
    setModulationRange(currentRange.first, currentRange.second);

    // Perform initial connect if needed
    if (currentMode == ModulationMode::Envelope)
        modulationRouter.connect({ ModulationSourceType::Envelope, currentSourceIndex }, this);
    else if (currentMode == ModulationMode::LFO)
        modulationRouter.connect({ ModulationSourceType::LFO,      currentSourceIndex }, this);
}

ModulationTarget::~ModulationTarget()
{
    // Tear down connections and listeners
    modulationRouter.disconnect(this);
    apvts.removeParameterListener(sourceParamID, this);
    apvts.removeParameterListener(indexParamID, this);
    apvts.removeParameterListener(minParamID, this);
    apvts.removeParameterListener(maxParamID, this);
}

void ModulationTarget::parameterChanged(const juce::String& parameterID,
    float newValue)
{
    if (parameterID == sourceParamID)
    {
        // Source mode changed: rewire
        auto newMode = static_cast<ModulationMode>(static_cast<int>(newValue));
        if (newMode != currentMode)
        {
            modulationRouter.disconnect(this);
            currentMode = newMode;

            if (currentMode == ModulationMode::Envelope)
                modulationRouter.connect({ ModulationSourceType::Envelope, currentSourceIndex }, this);
            else if (currentMode == ModulationMode::LFO)
                modulationRouter.connect({ ModulationSourceType::LFO,      currentSourceIndex }, this);
        }
    }
    else if (parameterID == indexParamID)
    {
        // Source index changed: rewire if in an active mode
        int newIndex = static_cast<int>(newValue);
        if (newIndex != currentSourceIndex)
        {
            if (currentMode == ModulationMode::Envelope ||
                currentMode == ModulationMode::LFO)
            {
                modulationRouter.disconnect(this);
                auto type = (currentMode == ModulationMode::Envelope
                    ? ModulationSourceType::Envelope
                    : ModulationSourceType::LFO);
                modulationRouter.connect({ type, newIndex }, this);
            }
            currentSourceIndex = newIndex;
        }
    }
    else if (parameterID == minParamID || parameterID == maxParamID)
    {
        // Range changed: update cached bounds
        float minVal = apvts.getRawParameterValue(minParamID)->load();
        float maxVal = apvts.getRawParameterValue(maxParamID)->load();
        setModulationRange(minVal, maxVal);
    }
}

void ModulationTarget::setModulationValue(float normalizedValue)
{
    if (!baseParam)
        return;

    float remapped = juce::jmap(normalizedValue,
        currentRange.first,
        currentRange.second);
    baseParam->setValueNotifyingHost(remapped);
}

void ModulationTarget::setModulationRange(float minNormalized,
    float maxNormalized)
{
    currentRange.first = minNormalized;
    currentRange.second = maxNormalized;
}

std::pair<float, float> ModulationTarget::getModulationRange() const
{
    return currentRange;
}

void ModulationTarget::setModulationMode(ModulationMode newMode)
{
    currentMode = newMode;
}

ModulationMode ModulationTarget::getModulationMode() const
{
    return currentMode;
}

void ModulationTarget::clearModulation()
{
    currentMode = ModulationMode::Manual;
    currentRange = { 0.0f, 1.0f };
}