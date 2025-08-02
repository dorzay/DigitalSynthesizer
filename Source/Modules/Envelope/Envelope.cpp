#include "Envelope.h"
#include "../../Modules/Linkable/LinkableUtils.h"

Envelope::Envelope(int index, juce::AudioProcessorValueTreeState& apvts)
    : envelopeIndex(index), apvts(apvts)
{
    name = "Envelope " + juce::String(index + 1);
    setParameters(attackNorm, decayNorm, sustainNorm, releaseNorm);
}

void Envelope::addParameters(int index, juce::AudioProcessorValueTreeState::ParameterLayout& layout)
{
    // Envelope mode
    const auto modeSpec = Envelope::getEnvelopeModeParamSpecs(index);
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        modeSpec.paramID, modeSpec.label, modeSpec.choices, modeSpec.defaultIndex));

    // Link target
    const auto linkSpec = Envelope::getEnvelopeLinkParamSpecs(index);
    juce::StringArray linkChoices = getDefaultLinkableTargetNames();
    linkChoices.insert(0, "-");

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        linkSpec.paramID, linkSpec.label, linkChoices, linkSpec.defaultIndex));

    // ADSR knobs
    for (const auto& spec : Envelope::getParamSpecs(index))
    {
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            spec.id, spec.name,
            juce::NormalisableRange<float>(spec.minValue, spec.maxValue, spec.stepSize),
            spec.defaultValue));
    }
}

juce::String Envelope::getName() const
{
    return name;
}

std::vector<Envelope::KnobParamSpecs> Envelope::getParamSpecs(int index)
{
    const int ID = index + 1;
    const juce::String prefix = "ENV" + juce::String(ID) + "_";

    std::vector<Envelope::KnobParamSpecs> specs(static_cast<int>(ADSR::Count));

    specs[static_cast<int>(ADSR::Attack)] = { prefix + "ATTACK", "A", 0.0f, 1.0f, 0.001f, 0.0f };
    specs[static_cast<int>(ADSR::Decay)] = { prefix + "DECAY", "D", 0.0f, 1.0f, 0.001f, 0.0f };
    specs[static_cast<int>(ADSR::Sustain)] = { prefix + "SUSTAIN", "S", 0.0f, 1.0f, 0.01f, 1.0f };
    specs[static_cast<int>(ADSR::Release)] = { prefix + "RELEASE", "R", 0.0f, 1.0f, 0.001f, 0.0f };

    return specs;
}

Envelope::ComboBoxParamSpecs Envelope::getEnvelopeModeParamSpecs(int index)
{
    ComboBoxParamSpecs spec;

    spec.paramID = "ENV" + juce::String(index + 1) + "_MODE";
    spec.label = "Mode";

    const auto& modeList = getModeList();
    for (const auto& [mode, name] : modeList)
        spec.choices.add(name);

    spec.defaultIndex = 0;

    return spec;
}

Envelope::ComboBoxParamSpecs Envelope::getEnvelopeLinkParamSpecs(int index)
{
    ComboBoxParamSpecs spec;

    spec.paramID = "ENV" + juce::String(index + 1) + "_LINK";
    spec.label = "Link";
    spec.defaultIndex = index + 1;

    return spec;
}

void Envelope::setParameters(float attack, float decay, float sustain, float release)
{
    attackNorm = attack;
    decayNorm = decay;
    sustainNorm = sustain;
    releaseNorm = release;

    juce::ADSR::Parameters newParams = mapToADSRParams(attack, decay, sustain, release);

    for (auto& voice : voiceEnvelopes)
    {
        voice.params = newParams;
        voice.adsr.setParameters(newParams);
    }
}

void Envelope::updateFromParameters()
{
    const auto modeSpec = getEnvelopeModeParamSpecs(envelopeIndex);
    int modeIndex = static_cast<int>(apvts.getRawParameterValue(modeSpec.paramID)->load());
    setMode(static_cast<Mode>(modeIndex));

    const auto& specs = getParamSpecs(envelopeIndex);
    float a = apvts.getRawParameterValue(specs[0].id)->load();
    float d = apvts.getRawParameterValue(specs[1].id)->load();
    float s = apvts.getRawParameterValue(specs[2].id)->load();
    float r = apvts.getRawParameterValue(specs[3].id)->load();
    setParameters(a, d, s, r);
}

void Envelope::setSampleRate(double newRate)
{
    sampleRate = newRate;

    for (auto& voice : voiceEnvelopes)
    {
        voice.adsr.setSampleRate(sampleRate);
    }
}

void Envelope::noteOn(int midiNote)
{
    for (auto& voice : voiceEnvelopes)
    {
        // If this voice is already handling the same note, reset it before reusing
        if (voice.active && voice.midiNote == midiNote)
        {
            voice.adsr.reset();
            voice.active = false;
            voice.midiNote = -1;
        }

        // Use the first inactive voice
        if (!voice.active)
        {
            voice.midiNote = midiNote;
            voice.active = true;

            voice.params = mapToADSRParams(attackNorm, decayNorm, sustainNorm, releaseNorm);
            voice.adsr.setSampleRate(sampleRate);
            voice.adsr.setParameters(voice.params);
            voice.adsr.noteOn();

            return;
        }
    }
}

void Envelope::noteOff(int midiNote)
{
    for (auto& voice : voiceEnvelopes)
    {
        if (voice.active && voice.midiNote == midiNote)
        {
            voice.adsr.setSampleRate(sampleRate);
            voice.adsr.setParameters(mapToADSRParams(attackNorm, decayNorm, sustainNorm, releaseNorm));
            voice.adsr.noteOff();
        }
    }
}

void Envelope::resetAllVoices()
{
    for (auto& voice : voiceEnvelopes)
    {
        voice.adsr.reset();
        voice.active = false;
    }
}

bool Envelope::isNoteActive(int midiNote) const
{
    for (const auto& voice : voiceEnvelopes)
    {
        if (voice.active && voice.midiNote == midiNote)
            return true;
    }
    return false;
}

bool Envelope::isActive() const
{
    for (const auto& voice : voiceEnvelopes)
    {
        if (voice.active)
            return true;
    }
    return false;
}

float Envelope::getNextSampleForNote(int midiNote)
{
    float mixedSample = 0.0f;

    for (auto& voice : voiceEnvelopes)
    {
        if (voice.active && voice.midiNote == midiNote)
        {
            float sample = voice.adsr.getNextSample();

            if (!voice.adsr.isActive())
            {
                voice.active = false;
                voice.midiNote = -1;
            }

            mixedSample += (sample < 0.0f) ? 0.0f : sample;
        }
    }

    return mixedSample;
}

float Envelope::getModulationValue() const
{
    float sum = 0.0f;
    int count = 0;

    for (const auto& voice : voiceEnvelopes)
    {
        if (voice.active)
        {
            float level = voice.adsr.getCurrentValue();
            sum += juce::jlimit(0.0f, 1.0f, level);
            ++count;
        }
    }

    return (count > 0) ? (sum / count) : 0.0f;
}

void Envelope::tick()
{
    for (auto& voice : voiceEnvelopes)
    {
        if (voice.active)
        {
            voice.adsr.getNextSample();

            if (!voice.adsr.isActive())
            {
                voice.active = false;
                voice.midiNote = -1;
            }
        }
    }
}

const std::vector<std::pair<Envelope::Mode, juce::String>>& Envelope::getModeList()
{
    static const std::vector<std::pair<Mode, juce::String>> modeList =
    {
        { Mode::Normal, "Normal" },
        { Mode::AutoRelease, "Auto Release" }
    };
    return modeList;
}

juce::String Envelope::modeToString(Mode mode)
{
    for (const auto& [m, name] : getModeList())
        if (m == mode)
            return name;

    return "Unknown";
}

Envelope::Mode Envelope::stringToMode(const juce::String& label)
{
    for (const auto& [m, name] : getModeList())
        if (label == name)
            return m;

    return Mode::Normal;
}

void Envelope::setMode(Mode newMode)
{
    mode = newMode;

    for (auto& voice : voiceEnvelopes)
        voice.adsr.setMode(mode);
}

Envelope::Mode Envelope::getMode() const
{
    return mode;
}

juce::ADSR::Parameters Envelope::mapToADSRParams(float attack, float decay, float sustain, float release) const
{
    juce::ADSR::Parameters params;
    params.attack = FormattingUtils::normalizedToValue(attack, FormattingUtils::FormatType::Time, MIN_ADSR_TIME_MS, MAX_ADSR_TIME_MS) / 1000.0f;
    params.decay = FormattingUtils::normalizedToValue(decay, FormattingUtils::FormatType::Time, MIN_ADSR_TIME_MS, MAX_ADSR_TIME_MS) / 1000.0f;
    params.sustain = std::clamp(sustain, 0.0f, 1.0f);
    params.release = FormattingUtils::normalizedToValue(release, FormattingUtils::FormatType::Time, MIN_ADSR_TIME_MS, MAX_ADSR_TIME_MS) / 1000.0f;
    return params;
}

//==============================================================================
// Envelope::EnvelopeADSR

void Envelope::EnvelopeADSR::setMode(Envelope::Mode m) noexcept
{
    mode = m;
}

void Envelope::EnvelopeADSR::noteOn() noexcept
{
    juce::ADSR::noteOn();

    attackEnded = false;
    releaseTriggered = false;

    if (mode == Envelope::Mode::AutoRelease)
    {
        const auto& p = getParameters();

        const bool isInstantStart = p.attack <= 0.0001f;
        const bool isInstantEnd = p.release <= 0.0001f;

        if (isInstantStart)
            attackEnded = true;

        if (isInstantStart && isInstantEnd)
        {
            // Set release to a very small non-zero value to allow decay
            auto adjustedParams = p;
            adjustedParams.release = 0.05f;
            juce::ADSR::setParameters(adjustedParams);

            // Jump directly to release phase
            juce::ADSR::noteOff();
            releaseTriggered = true;
        }
    }
}

float Envelope::EnvelopeADSR::getNextSample() noexcept
{
    float value = juce::ADSR::getNextSample();

    if (mode == Envelope::Mode::AutoRelease && !releaseTriggered)
    {
        const auto& p = getParameters();

        if (!attackEnded)
        {
            if (value >= 0.99f)
                attackEnded = true;
        }
        else
        {
            if (value <= p.sustain)
            {
                juce::ADSR::noteOff();
                releaseTriggered = true;
            }
        }
    }

    lastValue = value;
    return value;
}

float Envelope::EnvelopeADSR::getCurrentValue() const noexcept
{
    return lastValue;
}


