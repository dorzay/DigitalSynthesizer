#include "Oscillator.h"

Oscillator::Oscillator(double sampleRate, int i, juce::AudioProcessorValueTreeState& apvtsRef)
    : sampleRate(sampleRate), index(i), apvts(&apvtsRef)
{
    latestParams.waveform = Waveform::Sine;
    name = getDefaultLinkableName(index);
}

KnobParamSpecs Oscillator::getKnobParamSpecs(ParamID id, int oscIndex)
{
    const juce::String prefix = "OSC" + juce::String(oscIndex + 1) + "_";

    switch (id)
    {
    case ParamID::Volume:
        return {
            prefix + "VOLUME", "Volume",
            0.0f, 1.0f, 0.01f, 0.7f,
            FormattingUtils::FormatType::Normal
        };

    case ParamID::Pan:
        return {
            prefix + "PAN", "Pan",
            0.0f, 1.0f, 0.01f, 0.5f,
            FormattingUtils::FormatType::Pan
        };

    case ParamID::Voices:
        return {
            prefix + "VOICES", "Voices",
            1.0f, 8.0f, 1.0f, 1.0f,
            FormattingUtils::FormatType::Discrete,
            true  // isDiscrete
        };

    case ParamID::Detune:
        return {
            prefix + "DETUNE", "Detune",
            0.0f, 1.0f, 0.01f, 0.0f,
            FormattingUtils::FormatType::Normal
        };

    default:
        jassertfalse;
        return {};
    }
}

ComboBoxParamSpecs Oscillator::getComboBoxParamSpecs(ParamID id, int oscIndex)
{
    const juce::String prefix = "OSC" + juce::String(oscIndex + 1) + "_";

    ComboBoxParamSpecs spec;

    switch (id)
    {
    case ParamID::Waveform:
        spec.paramID = prefix + "WAVEFORM";
        spec.label = "Waveform";
        spec.choices = { "Sine", "Square", "Triangle", "Sawtooth", "White Noise" };
        spec.defaultIndex = 0;
        break;

    case ParamID::Octave:
        spec.paramID = prefix + "OCTAVE";
        spec.label = "Octave";
        spec.choices = { "-2", "-1", "0", "+1", "+2" };
        spec.defaultIndex = 2;  // "0" is at index 2
        break;

    default:
        jassertfalse;
        break;
    }

    return spec;
}

std::pair<juce::String, juce::String> Oscillator::getToggleParamSpecs(ParamID id, int oscIndex)
{
    const juce::String prefix = "OSC" + juce::String(oscIndex + 1) + "_";

    switch (id)
    {
    case ParamID::Bypass:
        return { prefix + "BYPASS", "Bypass" };
    default:
        jassertfalse;
        return {};
    }
}

void Oscillator::addParameters(int oscIndex, juce::AudioProcessorValueTreeState::ParameterLayout& layout)
{
    // Waveform ComboBox 
    const auto waveformSpec = getComboBoxParamSpecs(ParamID::Waveform, oscIndex);
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        waveformSpec.paramID, waveformSpec.label, waveformSpec.choices, waveformSpec.defaultIndex));

    // Volume 
    const auto volume = getKnobParamSpecs(ParamID::Volume, oscIndex);
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        volume.id, volume.name,
        juce::NormalisableRange<float>(volume.minValue, volume.maxValue, volume.stepSize),
        volume.defaultValue));

    // Pan 
    const auto pan = getKnobParamSpecs(ParamID::Pan, oscIndex);
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        pan.id, pan.name,
        juce::NormalisableRange<float>(pan.minValue, pan.maxValue, pan.stepSize),
        pan.defaultValue));

    // Octave ComboBox 
    const auto octaveSpec = getComboBoxParamSpecs(ParamID::Octave, oscIndex);
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        octaveSpec.paramID, octaveSpec.label, octaveSpec.choices, octaveSpec.defaultIndex));

    // Voices 
    const auto voices = getKnobParamSpecs(ParamID::Voices, oscIndex);
    layout.add(std::make_unique<juce::AudioParameterInt>(
        voices.id, voices.name,
        static_cast<int>(voices.minValue),
        static_cast<int>(voices.maxValue),
        static_cast<int>(voices.defaultValue)));

    // Detune 
    const auto detune = getKnobParamSpecs(ParamID::Detune, oscIndex);
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        detune.id, detune.name,
        juce::NormalisableRange<float>(detune.minValue, detune.maxValue, detune.stepSize),
        detune.defaultValue));

    // Bypass 
    const auto bypass = getToggleParamSpecs(ParamID::Bypass, oscIndex);
    layout.add(std::make_unique<juce::AudioParameterBool>(bypass.first, bypass.second, false));
}

void Oscillator::processBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    if (isBypassed())
        return;

    const int numChannels = outputBuffer.getNumChannels();
    const int numVoices = latestParams.voices;

    // Ensure cached buffers match current voice count
    cachedDetuneCents.resize(numVoices, 0.0);
    cachedLeftGains.resize(numVoices, 1.0f);
    cachedRightGains.resize(numVoices, 1.0f);

    // Compute unison detune and stereo pan gains per voice
    if (numVoices > 1)
    {
        float detuneValue = latestParams.detune.getNextValue();

        for (int voice = 0; voice < numVoices; ++voice)
        {
            // Spread voices symmetrically around center
            cachedDetuneCents[voice] = (voice - (numVoices - 1) / 2.0f) * detuneValue * detuneScale;

            // Compute stereo pan gains using sinusoidal spacing
            float panNorm = static_cast<float>(voice) / static_cast<float>(numVoices - 1);
            float panAngle = std::sin(panNorm * juce::MathConstants<float>::halfPi);
            cachedLeftGains[voice] = std::cos(panAngle * juce::MathConstants<float>::halfPi);
            cachedRightGains[voice] = std::sin(panAngle * juce::MathConstants<float>::halfPi);
        }
    }
    else
    {
        // Single voice: center pan, no detune
        cachedDetuneCents[0] = 0.0;
        cachedLeftGains[0] = cachedRightGains[0] = 1.0f;
    }

    if (linkedFilter == nullptr)
    {
        // Write generated signal directly into output buffer
        for (int i = 0; i < numSamples; ++i)
        {
            auto [left, right] = getNextSample();

            if (numChannels > 0)
                outputBuffer.addSample(0, startSample + i, left);
            if (numChannels > 1)
                outputBuffer.addSample(1, startSample + i, right);
        }

        return;
    }

    // Filtered path: render into a temp buffer first
    juce::AudioBuffer<float> tempBuffer(2, numSamples);
    tempBuffer.clear();

    for (int i = 0; i < numSamples; ++i)
    {
        auto [left, right] = getNextSample();
        tempBuffer.setSample(0, i, left);
        tempBuffer.setSample(1, i, right);
    }

    // Apply the linked filter
    juce::dsp::AudioBlock<float> block(tempBuffer);
    auto context = juce::dsp::ProcessContextReplacing<float>(block);
    linkedFilter->process(context);

    // Mix filtered samples into output buffer
    for (int channel = 0; channel < numChannels; ++channel)
    {
        outputBuffer.addFrom(channel, startSample, tempBuffer, channel, 0, numSamples);
    }
}

void Oscillator::updateFromParameters()
{
    jassert(apvts != nullptr);

    // Waveform
    if (auto* param = apvts->getRawParameterValue(
        getComboBoxParamSpecs(ParamID::Waveform, index).paramID))
    {
        latestParams.waveform = indexToWaveform(static_cast<int>(param->load()));
    }

    // Volume
    if (auto* param = apvts->getRawParameterValue(
        getKnobParamSpecs(ParamID::Volume, index).id))
    {
        float newAmplitude = juce::jlimit(0.0f, defaultAmplitude, param->load());
        latestParams.volume = newAmplitude;
    }

    // Pan
    if (auto* param = apvts->getRawParameterValue(
        getKnobParamSpecs(ParamID::Pan, index).id))
    {
        float panValue = juce::jlimit(0.0f, 1.0f, param->load());
        latestParams.pan.left.setTargetValue(1.0f - panValue);
        latestParams.pan.right.setTargetValue(panValue);
    }

    // Voices
    if (auto* param = apvts->getRawParameterValue(
        getKnobParamSpecs(ParamID::Voices, index).id))
    {
        int newVoiceCount = juce::jlimit(1, maxVoices, static_cast<int>(param->load()));
        if (newVoiceCount != latestParams.voices)
        {
            // Safely migrate phase data to new size
            for (auto& [_, note] : notes)
            {
                std::vector<double> newPhases(newVoiceCount, 0.0);
                for (int i = 0; i < std::min<int>(newVoiceCount, note.phases.size()); ++i)
                    newPhases[i] = note.phases[i];
                note.phases = std::move(newPhases);
            }

            latestParams.voices = newVoiceCount;
            cachedDetuneCents.resize(newVoiceCount, 0.0);
            cachedLeftGains.resize(newVoiceCount, 1.0f);
            cachedRightGains.resize(newVoiceCount, 1.0f);
        }
    }

    // Detune
    if (auto* param = apvts->getRawParameterValue(
        getKnobParamSpecs(ParamID::Detune, index).id))
    {
        float detune = juce::jlimit(0.0f, 1.0f, param->load());
        latestParams.detune.setTargetValue(detune);
    }

    // Octave
    if (auto* param = apvts->getRawParameterValue(
        getComboBoxParamSpecs(ParamID::Octave, index).paramID))
    {
        int octave = static_cast<int>(param->load()) - 2; // UI index 2 = octave 0
        if (octave != latestParams.octave)
        {
            // Update cached octave
            latestParams.octave = juce::jlimit(minOctaveOffset,
                maxOctaveOffset,
                octave);

            // Gracefully release all currently active notes
            if (envelope != nullptr)
            {
                for (auto& [midiNote, noteData] : notes)
                    envelope->noteOff(midiNote);
            }

            // Clear stored note data and reset last-note pointer
            notes.clear();
            pLastNote = nullptr;
        }
    }

    // Bypass
    if (auto* param = apvts->getRawParameterValue(
        getToggleParamSpecs(ParamID::Bypass, index).first))
    {
        latestParams.bypass = (param->load() > 0.5f);
    }
}

int Oscillator::getIndex() const
{
    return index;
}

double Oscillator::getDefaultSampleRate()
{
    return defaultSampleRate;
}

juce::String Oscillator::getDefaultLinkableName(int index)
{
    return "Oscillator " + juce::String(index + 1);
}

juce::String Oscillator::getLinkableName() const
{
    return name;
}

bool Oscillator::isBypassed() const
{
    return latestParams.bypass;
}

void Oscillator::setEnvelope(Envelope* newEnvelope)
{
    envelope = newEnvelope;
}

Envelope* Oscillator::getEnvelope() const
{
    return envelope;
}

void Oscillator::setFilter(Filter* filter)
{
    linkedFilter = filter;
}

Filter* Oscillator::getFilter() const
{
    return linkedFilter;
}

int Oscillator::waveformToIndex(Waveform wf)
{
    return static_cast<int>(wf);
}

Oscillator::Waveform Oscillator::indexToWaveform(int index)
{
    return static_cast<Waveform>(index);
}

int Oscillator::calculateMidiNoteWithOctaveOffset(int midiNoteNumber) const
{
    return juce::jlimit(0, 127, midiNoteNumber + (latestParams.octave * 12));
}

void Oscillator::noteOn(const juce::MidiMessage& message)
{
    if (!message.isNoteOn() || envelope == nullptr)
        return;

    // apply octave shift
    int midiNote = calculateMidiNoteWithOctaveOffset(message.getNoteNumber());
    // normalize velocity 0–1
    float velocity = message.getVelocity() / 127.0f;
    // convert to Hz
    double frequency = juce::MidiMessage::getMidiNoteInHertz(midiNote);

    NoteData noteData;
    noteData.frequency = frequency;
    noteData.velocity = velocity;
    noteData.isReleasing = false;

    // Phase continuity logic
    // reuse last note phase if matching voice count
    if (pLastNote && pLastNote->phases.size() == latestParams.voices)
    {
        noteData.phases = pLastNote->phases;
    }
    else
    {
        noteData.phases.resize(latestParams.voices, 0.0);
    }

    notes[midiNote] = noteData;
    pLastNote = &notes[midiNote];
}

void Oscillator::noteOff(const juce::MidiMessage& message)
{
    if (!message.isNoteOff() || envelope == nullptr)
        return;

    int midiNote = calculateMidiNoteWithOctaveOffset(message.getNoteNumber());

    auto noteIter = notes.find(midiNote);
    if (noteIter != notes.end())
    {
        // wait for zero-cross to release
        noteIter->second.pendingNoteOff = true;

        // clear last note pointer
        if (&noteIter->second == pLastNote)
            pLastNote = nullptr;
    }
}

std::pair<float, float> Oscillator::getNextSample()
{
    // Nothing to play
    if (notes.empty() || envelope == nullptr)
        return { 0.0f, 0.0f };

    float leftSum = 0.0f;
    float rightSum = 0.0f;
    const int numVoices = latestParams.voices;

    for (auto& [midiNote, note] : notes)
    {
        if (note.phases.size() < numVoices)
            continue;

        // Get current envelope value for this note
        float env = envelope->getNextSampleForNote(midiNote);

        float totalGain = 0.0f;
        float voiceLeft = 0.0f;
        float voiceRight = 0.0f;
        float sumSample = 0.0f;

        for (int voice = 0; voice < numVoices; ++voice)
        {
            // Apply detune in cents (if needed)
            double freq = note.frequency;
            if (numVoices > 1)
                freq *= std::pow(2.0, cachedDetuneCents[voice] / 1200.0);

            // Generate waveform and apply velocity and envelope
            float s = generateWaveSample(freq, note.phases[voice]) * note.velocity * env;

            sumSample += s;
            voiceLeft += s * cachedLeftGains[voice];
            voiceRight += s * cachedRightGains[voice];

            totalGain += cachedLeftGains[voice] * cachedLeftGains[voice]
                + cachedRightGains[voice] * cachedRightGains[voice];
        }

        // Trigger noteOff at zero-crossing
        if (note.pendingNoteOff && note.lastSample * sumSample < 0.0f)
        {
            envelope->noteOff(midiNote);
            note.pendingNoteOff = false;
        }

        note.lastSample = sumSample;

        // Normalize summed signal and add to output
        float gain = (totalGain > 0.0f)
            ? latestParams.volume / std::sqrt(totalGain)
            : 0.0f;

        leftSum += voiceLeft * gain;
        rightSum += voiceRight * gain;
    }

    // Apply smoothed pan gain
    return {
        leftSum * latestParams.pan.left.getNextValue(),
        rightSum * latestParams.pan.right.getNextValue()
    };
}

bool Oscillator::isPlaying() const
{
    return !notes.empty();
}

void Oscillator::removeReleasedNotesIf(std::function<bool(int midiNote)> shouldRemove)
{
    for (auto it = notes.begin(); it != notes.end(); )
    {
        const auto& [midiNote, data] = *it;

        if (data.isReleasing && shouldRemove(midiNote))
        {
            it = notes.erase(it); // Remove this note
        }
        else
        {
            ++it;
        }
    }
}

float Oscillator::generateWaveSample(double frequency, double& phase)
{
    float sample = 0.0f;
    // select waveform shape
    switch (latestParams.waveform)
    {
    case Waveform::Sine:
        sample = static_cast<float>(std::sin(phase));
        break;
    case Waveform::Square:
        sample = static_cast<float>((std::sin(phase) >= 0.0) ? 1.0f : -1.0f);
        break;
    case Waveform::Triangle:
        sample = static_cast<float>(2.0f * std::abs(2.0f * (phase / juce::MathConstants<double>::twoPi - std::floor(phase / juce::MathConstants<double>::twoPi + 0.5))) - 1.0f);
        break;
    case Waveform::Sawtooth:
        sample = static_cast<float>(2.0f * (phase / juce::MathConstants<double>::twoPi - std::floor(phase / juce::MathConstants<double>::twoPi + 0.5)));
        break;
    case Waveform::White_Noise:
        sample = juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f;
        break;
    }

    // advance phase
    phase += (frequency / sampleRate) * juce::MathConstants<double>::twoPi;
    // wrap phase at 2pi
    if (phase >= juce::MathConstants<double>::twoPi)
        phase -= juce::MathConstants<double>::twoPi;

    return sample;
}