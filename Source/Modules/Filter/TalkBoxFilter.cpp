#include "TalkboxFilter.h"

TalkboxFilter::TalkboxFilter() = default;
constexpr float morphScale = 1.0f;

const std::map<TalkboxFilter::Vowel, std::array<float, TalkboxFilter::numFormants>> TalkboxFilter::baseFormantMap =
{
    { Vowel::A, { 730.0f, 1090.0f, 2440.0f } },
    { Vowel::E, { 530.0f, 1840.0f, 2480.0f } },
    { Vowel::I, { 270.0f, 2290.0f, 3010.0f } },
    { Vowel::O, { 570.0f,  840.0f, 2410.0f } },
    { Vowel::U, { 300.0f,  870.0f, 2240.0f } }
};

const std::map<TalkboxFilter::Vowel, std::array<float, TalkboxFilter::numFormants>> TalkboxFilter::baseGainDbMap =
{
    { TalkboxFilter::Vowel::A, { -1.0f,  -5.0f,  -28.0f } },
    { TalkboxFilter::Vowel::E, { -2.0f, -17.0f,  -24.0f } },
    { TalkboxFilter::Vowel::I, { -4.0f, -24.0f,  -28.0f } },
    { TalkboxFilter::Vowel::O, { -1.0f, -12.0f,  -22.0f } },
    { TalkboxFilter::Vowel::U, { -5.0f, -15.0f,  -20.0f } }
};

namespace
{
    const std::map<TalkboxFilter::Vowel, juce::String> vowelDisplayNames = {
        { TalkboxFilter::Vowel::A, " A" },
        { TalkboxFilter::Vowel::E, " E" },
        { TalkboxFilter::Vowel::I, " I" },
        { TalkboxFilter::Vowel::O, " O" },
        { TalkboxFilter::Vowel::U, " U" }
    };
}

TalkboxFilter::KnobParamSpecs TalkboxFilter::getKnobParamSpecs(ParamID id, int filterIndex)
{
    const juce::String prefix = "FILTER" + juce::String(filterIndex + 1) + "_";

    switch (id)
    {
    case ParamID::Morph:
        return { prefix + "MORPH", "Morph", 0.0f, 1.0f, 0.001f,
            FormattingUtils::valueToNormalized(1000.0f, FormattingUtils::FormatType::VowelCenterFrequency, FormattingUtils::vowelMorphMinHz, FormattingUtils::vowelMorphMaxHz),
            FormattingUtils::FormatType::VowelCenterFrequency
        };

    case ParamID::Factor:
        return { prefix + "FACTOR", "Factor", 0.0f, 1.0f, 0.001f,
            FormattingUtils::valueToNormalized((FormattingUtils::resonanceMax / 2), FormattingUtils::FormatType::Resonance, FormattingUtils::resonanceMin, FormattingUtils::resonanceMax),
            FormattingUtils::FormatType::Resonance
        };

    default:
        return {};
    }
}

TalkboxFilter::ComboBoxParamSpecs TalkboxFilter::getComboBoxParamSpecs(ParamID id, int filterIndex)
{
    const juce::String prefix = "FILTER" + juce::String(filterIndex + 1) + "_";

    ComboBoxParamSpecs spec;

    switch (id)
    {
    case ParamID::Vowel:
        spec.paramID = prefix + "VOWEL";
        spec.label = "Vowel";
        spec.choices = { "A", "E", "I", "O", "U" };
        spec.defaultIndex = static_cast<int>(Vowel::A);
        break;

    default:
        break;
    }

    return spec;
}

void TalkboxFilter::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;

    for (auto& filterBand : filters)
        for (auto& f : filterBand)
            f.prepare(spec);

    isPrepared = true;
    updateFilters();
}

void TalkboxFilter::reset()
{
    for (auto& filterBand : filters)
        for (auto& f : filterBand)
            f.reset();
}

void TalkboxFilter::process(juce::dsp::AudioBlock<float>& block)
{
    if (!isPrepared)
        return;

    const int numChannels = static_cast<int>(block.getNumChannels());
    const int numSamples = static_cast<int>(block.getNumSamples());

    juce::AudioBuffer<float> tempBuffer(numChannels, numSamples);
    tempBuffer.clear();

    for (int i = 0; i < numFormants; ++i)
    {
        juce::AudioBuffer<float> formantBuffer(numChannels, numSamples);

        for (int ch = 0; ch < numChannels; ++ch)
            formantBuffer.copyFrom(ch, 0, block.getChannelPointer(ch), numSamples);

        juce::dsp::AudioBlock<float> formantBlock(formantBuffer);

        for (int ch = 0; ch < numChannels; ++ch)
            filters[i][ch].process(juce::dsp::ProcessContextReplacing<float>(formantBlock.getSingleChannelBlock(ch)));

        for (int ch = 0; ch < numChannels; ++ch)
        {
            const float compensatedGain = gains[i] * gainCompensation[i];
            tempBuffer.addFrom(ch, 0, formantBuffer, ch, 0, numSamples, compensatedGain);
        }
    }

    for (int ch = 0; ch < numChannels; ++ch)
        juce::FloatVectorOperations::copy(block.getChannelPointer(ch), tempBuffer.getReadPointer(ch), numSamples);
}

std::array<TalkboxFilter::FormantBand, TalkboxFilter::numFormants> TalkboxFilter::getFormantBandsForGraph() const
{
    std::array<FormantBand, numFormants> bands;

    for (int i = 0; i < numFormants; ++i)
    {
        bands[i].frequency = morphedFormants[i];
        bands[i].q = qFactorBase[i] * qFactor;
        bands[i].gain = gains[i];
    }

    return bands;
}

void TalkboxFilter::setVowel(Vowel newVowel)
{
    if (newVowel != currentVowel)
    {
        currentVowel = newVowel;
        updateFilters();
    }
}

void TalkboxFilter::setQFactor(float q)
{
    qFactor = q;
    updateFilters();
}

void TalkboxFilter::setMorph(float morph)
{
    morphAmount = juce::jlimit(0.0f, 1.0f, morph);
    updateFilters();
}

TalkboxFilter::Vowel TalkboxFilter::getVowel() const
{
    return currentVowel;
}

std::array<float, TalkboxFilter::numFormants> TalkboxFilter::getMorphedFrequencies() const
{
    return morphedFormants;
}

void TalkboxFilter::updateFilters()
{
    if (!isPrepared)
        return;

    // Get base formants (ratios)
    const auto& baseFormants = baseFormantMap.at(currentVowel);
    const auto& dbGains = baseGainDbMap.at(currentVowel);
    for (int i = 0; i < numFormants; ++i)
        gains[i] = std::pow(10.0f, dbGains[i] / 20.0f);

    // Get morph center frequency using normalized morph
    const float centerFreq = FormattingUtils::normalizedToValue(morphAmount, FormattingUtils::FormatType::VowelCenterFrequency, FormattingUtils::vowelMorphMinHz, FormattingUtils::vowelMorphMaxHz, 0);

    for (int i = 0; i < numFormants; ++i)
    {
        // Compute ratio of each formant to vowel's middle formant
        const float ratio = baseFormants[i] / baseFormants[1];

        // Morph frequency = center * ratio
        const float morphedFreq = centerFreq * ratio;
        const float scaledQ = qFactorBase[i] * qFactor;

        auto coeffs = juce::dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, morphedFreq, scaledQ);

        for (int ch = 0; ch < 2; ++ch)
            filters[i][ch].coefficients = coeffs;

        morphedFormants[i] = morphedFreq;

        // Update gain compensation
        gainCompensation[i] = std::sqrt(scaledQ);
    }
}


