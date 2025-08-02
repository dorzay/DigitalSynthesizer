#include "Filter.h"
#include "../../Modules/Linkable/LinkableUtils.h"

Filter::Filter(int index)
{
    name = "Filter " + juce::String(index + 1);
}

Filter::KnobParamSpecs Filter::getKnobParamSpecs(ParamID id, int filterIndex)
{
    const juce::String prefix = "FILTER" + juce::String(filterIndex + 1) + "_";

    switch (id)
    {
    case ParamID::Cutoff:
    {
        FormattingUtils::FormatType cutoffFormat = FormattingUtils::FormatType::FrequencyLowPass;

        if (Parameters::Default::FilterType == Type::HighPass)
            cutoffFormat = FormattingUtils::FormatType::FrequencyHighPass;

        return { prefix + "CUTOFF", "Cutoff",
                 0.0f, 1.0f, 0.001f,
                 FormattingUtils::valueToNormalized(
                     Parameters::Default::Cutoff,
                     cutoffFormat,
                     FormattingUtils::freqMinHz,
                     FormattingUtils::freqMaxHz),
                 cutoffFormat };
    }

    case ParamID::Resonance:
    return { prefix + "RES", "Res",
             0.0f, 1.0f, 0.001f,
             Parameters::Default::Resonance,
             FormattingUtils::FormatType::Resonance };

    case ParamID::Drive:
        return { prefix + "DRIVE", "Drive",
                 0.0f, 1.0f, 0.01f,
                 Parameters::Default::Drive };

    case ParamID::Mix:
        return { prefix + "MIX", "Mix",
                 0.0f, 1.0f, 0.01f,
                 Parameters::Default::Mix };

    default:
        jassertfalse;
        return {};
    }
}

Filter::ComboBoxParamSpecs Filter::getComboBoxParamSpecs(ParamID id, int filterIndex)
{
    const juce::String prefix = "FILTER" + juce::String(filterIndex + 1) + "_";

    ComboBoxParamSpecs spec;

    switch (id)
    {
    case ParamID::Type:
        spec.paramID = prefix + "TYPE";
        spec.label = "Type";
        spec.choices = { "Low Pass", "High Pass", "Band Pass", "Talkbox"};
        spec.defaultIndex = static_cast<int>(Parameters::Default::FilterType);
        break;

    case ParamID::Link:
        spec.paramID = prefix + "LINK";
        spec.label = "Link";
        spec.defaultIndex = 0;
        // Choices populated dynamically later
        break;

    case ParamID::Slope:
    {
        spec.paramID = prefix + "SLOPE";
        spec.label = "Slope";

        const int slopeCount = static_cast<int>(Slope::Count);
        for (int i = 0; i < slopeCount; ++i)
        {
            float normalized = (slopeCount > 1) ? static_cast<float>(i) / static_cast<float>(slopeCount - 1) : 0.0f;
            spec.choices.add(FormattingUtils::formatValue(
                normalized,
                FormattingUtils::FormatType::Slope,
                0.0f, 1.0f, slopeCount));
        }
        spec.defaultIndex = static_cast<int>(Parameters::Default::Slope);
        break;
    }

    default:
        jassertfalse;
        break;
    }

    return spec;
}

std::pair<juce::String, juce::String> Filter::getToggleParamSpecs(ParamID id, int filterIndex)
{
    const juce::String prefix = "FILTER" + juce::String(filterIndex + 1) + "_";

    switch (id)
    {
    case ParamID::Bypass:
        return { prefix + "BYPASS", "Bypass" };

    default:
        jassertfalse;
        return { "", "" };
    }
}

void Filter::addParameters(int filterIndex, juce::AudioProcessorValueTreeState::ParameterLayout& layout)
{
    using ParamID = Filter::ParamID;

    // Static ComboBoxes: Type, Slope
    for (auto id : { ParamID::Type, ParamID::Slope })
    {
        auto spec = getComboBoxParamSpecs(id, filterIndex);
        layout.add(std::make_unique<juce::AudioParameterChoice>(
            spec.paramID, spec.label, spec.choices, spec.defaultIndex));
    }

    // Link ComboBox (with dynamic choices)
    {
        auto spec = getComboBoxParamSpecs(ParamID::Link, filterIndex);
        juce::StringArray linkChoices = getDefaultLinkableTargetNames();
        linkChoices.insert(0, "-");
        layout.add(std::make_unique<juce::AudioParameterChoice>(
            spec.paramID, spec.label, linkChoices, spec.defaultIndex));
    }

    // Standard Filter Knobs
    for (auto id : { ParamID::Cutoff, ParamID::Resonance, ParamID::Drive, ParamID::Mix })
    {
        auto spec = getKnobParamSpecs(id, filterIndex);
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            spec.id, spec.name,
            juce::NormalisableRange<float>(spec.minValue, spec.maxValue, spec.stepSize),
            spec.defaultValue));
    }

    // Talkbox-specific Parameters
    {
        using TB = TalkboxFilter;

        auto morphSpec = TB::getKnobParamSpecs(TB::ParamID::Morph, filterIndex);
        auto factorSpec = TB::getKnobParamSpecs(TB::ParamID::Factor, filterIndex);
        auto vowelSpec = TB::getComboBoxParamSpecs(TB::ParamID::Vowel, filterIndex);

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            morphSpec.id, morphSpec.name,
            juce::NormalisableRange<float>(morphSpec.minValue, morphSpec.maxValue, morphSpec.stepSize),
            morphSpec.defaultValue));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            factorSpec.id, factorSpec.name,
            juce::NormalisableRange<float>(factorSpec.minValue, factorSpec.maxValue, factorSpec.stepSize),
            factorSpec.defaultValue));

        layout.add(std::make_unique<juce::AudioParameterChoice>(
            vowelSpec.paramID, vowelSpec.label, vowelSpec.choices, vowelSpec.defaultIndex));
    }

    // Bypass Toggle
    {
        auto [bypassID, bypassLabel] = getToggleParamSpecs(ParamID::Bypass, filterIndex);
        layout.add(std::make_unique<juce::AudioParameterBool>(bypassID, bypassLabel, false));
    }
}

juce::String Filter::getName() const
{
    return name;
}

bool Filter::isTalkboxMode() const
{
    return currentParams.type == Type::Talkbox;
}

TalkboxFilter& Filter::getTalkboxFilter()
{
    return talkboxFilter;
}

void Filter::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize = static_cast<juce::uint32>(samplesPerBlock);

    juce::dsp::ProcessSpec spec{ currentSampleRate, currentBlockSize, 2 };
    ladderFilter.prepare(spec);
    talkboxFilter.prepare(spec);
}

void Filter::reset()
{
    ladderFilter.reset();
    talkboxFilter.reset();
    needsUpdate = true;
}

void Filter::process(juce::dsp::ProcessContextReplacing<float> context)
{
    if (currentParams.bypass)
        return;

    auto& block = context.getOutputBlock();
    const bool needsDryWet = currentParams.mix < 1.0f;

    juce::AudioBuffer<float> dryCopy;
    if (needsDryWet)
    {
        dryCopy.setSize((int)block.getNumChannels(), (int)block.getNumSamples());
        for (size_t ch = 0; ch < block.getNumChannels(); ++ch)
            dryCopy.copyFrom((int)ch, 0, block.getChannelPointer(ch), (int)block.getNumSamples());
    }

    if (needsUpdate)
    {
        updateFilter();
        needsUpdate = false;
    }

    if (currentParams.drive > 0.0f)
        applyDrive(block);

    if (currentParams.type == Type::Talkbox)
        talkboxFilter.process(block);
    else
        ladderFilter.process(context);

    if (needsDryWet)
    {
        for (size_t ch = 0; ch < block.getNumChannels(); ++ch)
        {
            float* wet = block.getChannelPointer(ch);
            const float* dry = dryCopy.getReadPointer((int)ch);
            for (size_t i = 0; i < block.getNumSamples(); ++i)
                wet[i] = (1.0f - currentParams.mix) * dry[i] + currentParams.mix * wet[i];
        }
    }
}

void Filter::updateParametersIfNeeded()
{
    if (needsUpdate)
    {
        updateFilter();
        needsUpdate = false;
    }
}

void Filter::updateFromParameters(const juce::AudioProcessorValueTreeState& apvts, int filterIndex)
{
    const juce::String prefix = "FILTER" + juce::String(filterIndex + 1) + "_";

    currentParams.cutoffHz = FormattingUtils::normalizedToValue(
        apvts.getRawParameterValue(prefix + "CUTOFF")->load(),
        FormattingUtils::FormatType::FrequencyLowPass,
        FormattingUtils::freqMinHz,
        FormattingUtils::freqMaxHz);

    currentParams.resonance = apvts.getRawParameterValue(prefix + "RES")->load();
    currentParams.drive = apvts.getRawParameterValue(prefix + "DRIVE")->load();
    currentParams.mix = apvts.getRawParameterValue(prefix + "MIX")->load();

    const float slopeNorm = apvts.getRawParameterValue(prefix + "SLOPE")->load();
    int slopeIndex = static_cast<int>(juce::jmap(slopeNorm, 0.0f, 1.0f, 0.0f, static_cast<float>(static_cast<int>(Slope::Count) - 1)) + 0.5f);
    slopeIndex = std::clamp(slopeIndex, 0, static_cast<int>(Slope::Count) - 1);
    currentParams.slope = static_cast<Slope>(slopeIndex);

    currentParams.bypass = (apvts.getRawParameterValue(prefix + "BYPASS")->load() > 0.5f);

    auto typeIdx = static_cast<int>(apvts.getRawParameterValue(prefix + "TYPE")->load());
    currentParams.type = static_cast<Type>(juce::jlimit(0, static_cast<int>(Type::Count) - 1, typeIdx));

    // Talkbox Logic
    if (currentParams.type == Type::Talkbox)
    {
        const float morphNorm = apvts.getRawParameterValue(prefix + "MORPH")->load();
        const float morphValue = morphNorm;

        const float factorNorm = apvts.getRawParameterValue(prefix + "FACTOR")->load();
        const float factorValue = FormattingUtils::normalizedToValue(factorNorm, FormattingUtils::FormatType::Resonance, FormattingUtils::resonanceMin, FormattingUtils::resonanceMax);

        const int vowelIdx = static_cast<int>(apvts.getRawParameterValue(prefix + "VOWEL")->load());
        const auto vowel = static_cast<TalkboxFilter::Vowel>(juce::jlimit(0, static_cast<int>(TalkboxFilter::Vowel::Count) - 1, vowelIdx));

        talkboxFilter.setVowel(vowel);
        talkboxFilter.setMorph(morphValue);
        talkboxFilter.setQFactor(factorValue);
    }

    needsUpdate = true;
}

void Filter::updateFilter()
{
    juce::dsp::LadderFilterMode ladderMode = juce::dsp::LadderFilterMode::LPF24;

    switch (currentParams.type)
    {
    case Type::LowPass:
        ladderMode = (currentParams.slope == Slope::dB12) ? juce::dsp::LadderFilterMode::LPF12 : juce::dsp::LadderFilterMode::LPF24;
        break;
    case Type::HighPass:
        ladderMode = (currentParams.slope == Slope::dB12) ? juce::dsp::LadderFilterMode::HPF12 : juce::dsp::LadderFilterMode::HPF24;
        break;
    case Type::BandPass:
        ladderMode = (currentParams.slope == Slope::dB12) ? juce::dsp::LadderFilterMode::BPF12 : juce::dsp::LadderFilterMode::BPF24;
        break;
    default:
        ladderMode = juce::dsp::LadderFilterMode::LPF24;
        break;
    }

    ladderFilter.setMode(ladderMode);
    ladderFilter.setCutoffFrequencyHz(currentParams.cutoffHz);
    ladderFilter.setResonance(currentParams.resonance);
    float shapedDrive = std::pow(currentParams.drive, 1.5f);
    ladderFilter.setDrive(1.0f + shapedDrive * 3.0f);
}

void Filter::applyDrive(juce::dsp::AudioBlock<float>& block)
{
    const float drive = currentParams.drive;
    if (drive <= 0.0f)
        return;

    const int numChannels = static_cast<int>(block.getNumChannels());
    const int numSamples = static_cast<int>(block.getNumSamples());

    if (currentParams.type == Type::Talkbox)
    {
        // Symmetric drive (more harmonic, vowel-preserving)
        const float gain = 1.0f + drive * 4.0f;

        for (int ch = 0; ch < numChannels; ++ch)
        {
            float* data = block.getChannelPointer(ch);
            for (int i = 0; i < numSamples; ++i)
            {
                float x = data[i] * gain;
                data[i] = std::atan(x); // Smooth harmonics
            }
        }
    }
    else
    {
        // Asymmetric tanh drive
        float shapedDrive = std::pow(drive, 2.0f);
        float preGainPos = 1.0f + shapedDrive * 5.0f;
        float preGainNeg = 1.0f + shapedDrive * 4.0f;

        float normPos = 1.0f / std::tanh(preGainPos);
        float normNeg = 1.0f / std::tanh(preGainNeg);

        for (int ch = 0; ch < numChannels; ++ch)
        {
            float* data = block.getChannelPointer(ch);
            for (int i = 0; i < numSamples; ++i)
            {
                float sample = data[i];

                if (sample >= 0.0f)
                {
                    sample *= preGainPos;
                    data[i] = std::tanh(sample) * normPos;
                }
                else
                {
                    sample *= preGainNeg;
                    data[i] = std::tanh(sample) * normNeg;
                }
            }
        }
    }
}
