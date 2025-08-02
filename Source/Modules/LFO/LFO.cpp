#include "LFO.h"

KnobParamSpecs LFO::getKnobParamSpecs(ParamID id, int lfoIndex)
{
    const juce::String prefix = "LFO" + juce::String(lfoIndex + 1) + "_";

    switch (id)
    {
    case ParamID::Freq:
        return {
            prefix + "FREQ", "Freq",
            0.0f, 1.0f, 0.001f,
            FormattingUtils::valueToNormalized(
                Default::freq,
                FormattingUtils::FormatType::LFOFrequency,
                FormattingUtils::lfoFreqMinHz,
                FormattingUtils::lfoFreqMaxHz
            ),
            FormattingUtils::FormatType::LFOFrequency
        };

    case ParamID::Shape:
        return {
            prefix + "SHAPE", "Shape",
            0.0f, 1.0f, 0.01f,
            Default::shape,
            FormattingUtils::FormatType::Percent
        };

    case ParamID::Steps:
        return {
            prefix + "STEPS", "Steps",
            static_cast<float>(minSteps),
            static_cast<float>(maxSteps),
            1.0f,
            static_cast<float>(Default::steps),
            FormattingUtils::FormatType::Discrete,
            true  // isDiscrete
        };

    default:
        jassertfalse;
        return {};
    }
}

ComboBoxParamSpecs LFO::getComboBoxParamSpecs(ParamID id, int lfoIndex)
{
    const juce::String prefix = "LFO" + juce::String(lfoIndex + 1) + "_";
    ComboBoxParamSpecs spec;

    switch (id)
    {
    case ParamID::Type:
        spec.paramID = prefix + "TYPE";
        spec.label = "Type";
        spec.choices = { "Sine", "Triangle", "Square", "Steps" };
        spec.defaultIndex = static_cast<int>(static_cast<Type>(lfoIndex % static_cast<int>(Type::Count)));
        break;

    case ParamID::Mode:
        spec.paramID = prefix + "MODE";
        spec.label = "Mode";
        spec.choices = { "Free", "Retrigger" };
        spec.defaultIndex = static_cast<int>(Default::mode);
        break;

    default:
        jassertfalse;
        break;
    }

    return spec;
}

std::pair<juce::String, juce::String> LFO::getToggleParamSpecs(ParamID id, int lfoIndex)
{
    const juce::String prefix = "LFO" + juce::String(lfoIndex + 1) + "_";

    switch (id)
    {
    case ParamID::Bypass:
        return { prefix + "BYPASS", "Bypass" };

    default:
        jassertfalse;
        return { "", "" };
    }
}

void LFO::addParameters(int lfoIndex, juce::AudioProcessorValueTreeState::ParameterLayout& layout)
{
    // Knob Parameters
    for (ParamID id : { ParamID::Freq, ParamID::Shape, ParamID::Steps })
    {
        const auto spec = getKnobParamSpecs(id, lfoIndex);
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            spec.id, spec.name,
            juce::NormalisableRange<float>(spec.minValue, spec.maxValue, spec.stepSize),
            spec.defaultValue
        ));
    }

    // ComboBoxes
    for (ParamID id : { ParamID::Type, ParamID::Mode })
    {
        const auto spec = getComboBoxParamSpecs(id, lfoIndex);
        layout.add(std::make_unique<juce::AudioParameterChoice>(
            spec.paramID, spec.label, spec.choices, spec.defaultIndex
        ));
    }

    // Toggle
    const auto [bypassID, bypassLabel] = getToggleParamSpecs(ParamID::Bypass, lfoIndex);
    layout.add(std::make_unique<juce::AudioParameterBool>(bypassID, bypassLabel, Default::bypass));
}

LFO::LFO(int index)
    : index(index)
{
    name = "LFO " + std::to_string(index + 1);
}

const std::string& LFO::getName() const
{
    return name;
}

void LFO::setFrequency(float hz)
{
    frequencyHz = hz;
}

void LFO::setType(Type newType)
{
    type = newType;
}

void LFO::setMode(Mode newMode)
{
    mode = newMode;
}

void LFO::setShape(float newShape)
{
    shape = newShape;
}

void LFO::setNumSteps(int newNumSteps)
{
    if (newNumSteps != numSteps)
    {
        numSteps = newNumSteps;

        if (type == Type::Steps)
            randomizeSteps();
    }
}

void LFO::randomizeSteps()
{
    stepValues.clear();
    stepValues.reserve(numSteps);

    for (int i = 0; i < numSteps; ++i)
    {
        float value = juce::Random::getSystemRandom().nextFloat(); // [0.0, 1.0]
        stepValues.push_back(value);
    }
}

void LFO::setBypassed(bool shouldBypass)
{
    bypassed = shouldBypass;
}

void LFO::onTrigger()
{
    if (mode == Mode::Retrigger)
        needsRetrigger = true;
}

void LFO::advance(int samplesPerBlock, float sampleRate)
{
    if (needsRetrigger)
    {
        resetPhase();
        needsRetrigger = false;
    }

    modulationBuffer.resize(samplesPerBlock);
    bufferIndex = 0;

    float phaseDelta = frequencyHz / sampleRate;

    for (int i = 0; i < samplesPerBlock; ++i)
    {
        modulationBuffer[i] = getValueAtPhase(phase);
        phase += phaseDelta;

        if (phase >= 1.0f)
            phase -= 1.0f;
    }
}

float LFO::getNextValue()
{
    // If LFO is not currently modulation-active, return zero
    if (!isModulationActive())
        return 0.0f;

    if (modulationBuffer.empty())
        return 0.5f; // fallback

    float value = modulationBuffer[bufferIndex];
    bufferIndex = (bufferIndex + 1) % modulationBuffer.size();
    return value;
}

float LFO::getValueAtPhase(float phase) const
{
    if (type == Type::Steps)
    {
        int stepIndex = static_cast<int>(phase * numSteps);
        stepIndex = juce::jlimit(0, numSteps - 1, stepIndex);

        float rampValue = static_cast<float>(stepIndex) / static_cast<float>(numSteps - 1);
        float randomValue = (stepIndex < static_cast<int>(stepValues.size())) ? stepValues[stepIndex] : 0.0f;

        // Interpolation between ramp (shape = 0) and random (shape = 1)
        return juce::jmap(shape, 0.0f, 1.0f, rampValue, randomValue);
    }

    float warped = warpPhase(phase, shape);
    switch (type)
    {
    case Type::Sine:
    {
        float duty = juce::jlimit(0.01f, 0.99f, shape);  // prevent divide-by-zero

        float angle = 0.0f;

        if (phase < duty)
        {
            float localPhase = phase / duty;
            angle = juce::jmap(localPhase, 0.0f, 1.0f, 0.0f, juce::MathConstants<float>::pi);  // upper half
        }
        else
        {
            float localPhase = (phase - duty) / (1.0f - duty);
            angle = juce::jmap(localPhase, 0.0f, 1.0f, juce::MathConstants<float>::pi, juce::MathConstants<float>::twoPi);  // lower half
        }

        return 0.5f + 0.5f * std::sin(angle);
    }


    case Type::Triangle:
    {
        float skew = juce::jlimit(0.001f, 0.999f, shape);  // avoid divide-by-zero

        if (phase < skew)
            return phase / skew;
        else
            return (1.0f - phase) / (1.0f - skew);
    }


    case Type::Square:
    {
        float dutyCycle = shape;
        return phase < dutyCycle ? 1.0f : 0.0f;
    }

    default:
        return 0.5f;
    }
}

void LFO::resetPhase()
{
    phase = 0.0f;
}

bool LFO::isBypassed() const
{
    return bypassed;
}

void LFO::noteOn()
{
    isTriggered = true;
    if (mode == Mode::Retrigger)
        resetPhase();
}

void LFO::resetTrigger()
{
    isTriggered = false;
}

bool LFO::isActive() const
{
    return isTriggered;
}

void LFO::updateFromAPVTS(juce::AudioProcessorValueTreeState& apvts)
{
    const auto freqID = getKnobParamSpecs(ParamID::Freq, index).id;
    const auto shapeID = getKnobParamSpecs(ParamID::Shape, index).id;
    const auto stepsID = getKnobParamSpecs(ParamID::Steps, index).id;
    const auto typeID = getComboBoxParamSpecs(ParamID::Type, index).paramID;
    const auto modeID = getComboBoxParamSpecs(ParamID::Mode, index).paramID;
    const auto bypassID = getToggleParamSpecs(ParamID::Bypass, index).first;

    const float freqNorm = apvts.getRawParameterValue(freqID)->load();
    const float freqHz = FormattingUtils::normalizedToValue(
        freqNorm,
        FormattingUtils::FormatType::LFOFrequency,
        FormattingUtils::lfoFreqMinHz,
        FormattingUtils::lfoFreqMaxHz
    );
    setFrequency(freqHz);

    setType(static_cast<Type>(static_cast<int>(*apvts.getRawParameterValue(typeID))));
    setShape(*apvts.getRawParameterValue(shapeID));

    const float stepsFloat = *apvts.getRawParameterValue(stepsID);
    const int steps = static_cast<int>(stepsFloat);
    setNumSteps(steps);

    setMode(static_cast<Mode>(static_cast<int>(*apvts.getRawParameterValue(modeID))));
    setBypassed(apvts.getRawParameterValue(bypassID)->load() > 0.5f);

    if (type == Type::Steps && static_cast<int>(stepValues.size()) != numSteps)
        randomizeSteps();
}

bool LFO::isModulationActive() const
{
    return modulationActive;
}

void LFO::setModulationActive(bool shouldBeActive)
{
    modulationActive = shouldBeActive;
}

float LFO::warpPhase(float phase, float shape)
{
    if (shape <= 0.0f)
        return juce::jmap(phase, 0.0f, 1.0f, 0.5f, 1.0f);  // only right half

    if (shape >= 1.0f)
        return juce::jmap(phase, 0.0f, 1.0f, 0.0f, 0.5f);  // only left half

    if (shape == 0.5f)
        return phase;

    if (shape < 0.5f)
    {
        float start = juce::jmap(shape, 0.0f, 0.5f, 0.5f, 0.0f);
        return juce::jmap(phase, 0.0f, 1.0f, start, 1.0f);
    }
    else // shape > 0.5f
    {
        float end = juce::jmap(shape, 0.5f, 1.0f, 1.0f, 0.5f);
        return juce::jmap(phase, 0.0f, 1.0f, 0.0f, end);
    }
}