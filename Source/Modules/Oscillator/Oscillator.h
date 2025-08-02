#pragma once

#include "../../Common.h"
#include "../Envelope/Envelope.h"
#include "../Filter/Filter.h"
#include "../Linkable/Linkable.h"
#include <JuceHeader.h>

/**
 * @class Oscillator
 * @brief A digital oscillator capable of generating audio waveforms.
 */
class Oscillator : public Linkable
{
public:
    /**
     * @enum Waveform
     * @brief Supported waveform shapes for the oscillator.
     */
    enum class Waveform
    {
        Sine,         ///< Sine wave
        Square,       ///< Square wave
        Triangle,     ///< Triangle wave
        Sawtooth,     ///< Sawtooth wave
        White_Noise   ///< White noise
    };

    /**
     * @struct Pan
     * @brief Left-right stereo panning values with smoothing.
     */
    struct Pan
    {
        juce::SmoothedValue<float> left { 0.5f }; ///< left gain
        juce::SmoothedValue<float> right{ 0.5f }; ///< right gain
    };

    /**
     * @enum ParamID
     * @brief Identifiers for UI-controllable oscillator parameters.
     */
    enum class ParamID
    {
        Volume,   ///< Output gain
        Pan,      ///< Stereo pan
        Voices,   ///< Number of voices
        Detune,   ///< Detune amount
        Waveform, ///< Selected waveform
        Octave,   ///< Octave offset
        Bypass    ///< Bypass toggle
    };

    /**
     * @struct Params
     * @brief Holds values for all oscillator parameters.
     */
    struct Params
    {
        float volume = 1.0f;                      ///< Output gain
        Pan pan;                                  ///< Stereo panning
        int voices = 1;                           ///< Number of voices
        juce::SmoothedValue<float> detune = 0.0f; ///< Detune value
        int octave = 0;                           ///< Octave offset
        Waveform waveform = Waveform::Sine;       ///< Waveform shape
        bool bypass = false;                      ///< Whether bypassed
    };

    /**
     * @brief Constructs a new Oscillator.
     * @param sampleRate The current audio sample rate.
     * @param index Oscillator index (used for parameter ID suffixes).
     * @param apvtsRef Reference to the AudioProcessorValueTreeState.
     */
    Oscillator(double sampleRate, int index, juce::AudioProcessorValueTreeState& apvtsRef);

    /**
     * @brief Returns parameter spec for a given knob parameter.
     * @param id The parameter ID.
     * @param oscIndex The oscillator index.
     * @return The knob parameter specification.
     */
    static KnobParamSpecs getKnobParamSpecs(ParamID id, int oscIndex);

    /**
     * @brief Returns parameter spec for a combo box.
     * @param id The parameter ID.
     * @param oscIndex The oscillator index.
     * @return The combo box parameter specification.
     */
    static ComboBoxParamSpecs getComboBoxParamSpecs(ParamID id, int oscIndex);

    /**
     * @brief Returns parameter spec for a toggle button.
     * @param id The parameter ID.
     * @param oscIndex The oscillator index.
     * @return A pair of parameter ID and label.
     */
    static std::pair<juce::String, juce::String> getToggleParamSpecs(ParamID id, int oscIndex);

    /**
     * @brief Adds all oscillator parameters to the APVTS layout.
     * @param oscIndex The oscillator index.
     * @param layout The parameter layout to append to.
     */
    static void addParameters(int oscIndex, juce::AudioProcessorValueTreeState::ParameterLayout& layout);

    /**
     * @brief Processes audio and writes into the buffer.
     * @param buffer The audio buffer to fill.
     * @param startSample Start index for writing samples.
     * @param numSamples Number of samples to write.
     */
    void processBlock(juce::AudioBuffer<float>& buffer, int startSample, int numSamples);

    /**
     * @brief Updates internal parameters from the APVTS.
     */
    void updateFromParameters();

    /**
     * @brief Returns the oscillator index.
     * @return The 0-based index.
     */
    int getIndex() const;

    /**
     * @brief Returns the default sample rate.
     * @return The default rate in Hz.
     */
    static double getDefaultSampleRate();

    /**
     * @brief Returns default linkable display name for the oscillator.
     * @param index The oscillator index.
     * @return Oscillator's name.
     */
    static juce::String getDefaultLinkableName(int index);

    /**
     * @brief Returns the current oscillator's name for linking.
     * @return The linkable name string.
     */
    juce::String getLinkableName() const override;

    /**
     * @brief Checks whether the oscillator is bypassed.
     * @return True if bypassed.
     */
    bool isBypassed() const;

    /**
     * @brief Links an envelope module to the oscillator.
     * @param newEnvelope Pointer to the envelope.
     */
    void setEnvelope(Envelope* newEnvelope) override;

    /**
     * @brief Returns the linked envelope module.
     * @return A pointer to the envelope.
     */
    Envelope* getEnvelope() const;

    /**
     * @brief Links a filter to the oscillator.
     * @param filter Pointer to the filter module.
     */
    void setFilter(Filter* filter);

    /**
     * @brief Returns the linked filter module.
     * @return Pointer to the filter, or nullptr if none.
     */
    Filter* getFilter() const;

    /**
     * @brief Converts a waveform enum to integer index.
     * @param wf The waveform type.
     * @return Integer index.
     */
    static int waveformToIndex(Waveform wf);

    /**
     * @brief Converts an integer index to waveform enum.
     * @param index Integer waveform index.
     * @return Corresponding Waveform.
     */
    static Waveform indexToWaveform(int index);

    /**
     * @brief Applies octave offset to a MIDI note number.
     * @param midiNoteNumber Raw MIDI note.
     * @return Transposed MIDI note.
     */
    int calculateMidiNoteWithOctaveOffset(int midiNoteNumber) const;

    /**
     * @brief Handles a MIDI note-on message.
     * @param message The MIDI message.
     */
    void noteOn(const juce::MidiMessage& message);

    /**
     * @brief Handles a MIDI note-off message.
     * @param message The MIDI message.
     */
    void noteOff(const juce::MidiMessage& message);

    /**
     * @brief Generates the next stereo output sample.
     * @return A pair of (left, right) samples.
     */
    std::pair<float, float> getNextSample();

    /**
     * @brief Checks whether the oscillator is currently active.
     * @return True if any notes are active.
     */
    bool isPlaying() const;

    /**
     * @brief Removes notes for which the predicate returns true.
     * @param shouldRemove A callback taking a MIDI note number.
     */
    void removeReleasedNotesIf(std::function<bool(int midiNote)> shouldRemove);

private:
    static constexpr int maxVoices = 8;                  ///< Maximum number of voices supported
    static constexpr int minOctaveOffset = -2;           ///< Minimum octave shift
    static constexpr int maxOctaveOffset = 2;            ///< Maximum octave shift
    static constexpr float detuneScale = 20.0f;          ///< Detune scaling factor in cents
    static constexpr float defaultAmplitude = 1.0f;      ///< Maximum allowed output amplitude
    static constexpr double defaultSampleRate = 44100.0; ///< Fallback sample rate in Hz

    const juce::AudioProcessorValueTreeState* apvts = nullptr; ///< Pointer to APVTS
    double sampleRate;                                         ///< Sample rate in Hz
    int index;                                                 ///< Oscillator index
    juce::String name;                                         ///< Linkable name
    Envelope* envelope = nullptr;                              ///< Linked envelope
    Filter* linkedFilter = nullptr;                            ///< Linked filter
    Params latestParams;                                       ///< Cached parameters

    /**
     * @struct NoteData
     * @brief Internal state for each MIDI note.
     */
    struct NoteData
    {
        double frequency = 0.0;              ///< Frequency of the note in Hz
        float velocity = 0.0f;               ///< Normalized MIDI velocity [0, 1]
        std::vector<double> phases;          ///< Phase value per voice
        bool isReleasing = false;            ///< Whether the note is in release phase
        float lastSample = 0.0f;             ///< Last sample used for zero-crossing
        bool pendingNoteOff = false;         ///< True if noteOff is queued for zero-crossing
    };

    std::unordered_map<int, NoteData> notes; ///< Active note map
    NoteData* pLastNote = nullptr;           ///< Last played note

    /**
     * @brief Generates a single waveform sample for a frequency/phase.
     * @param frequency Frequency in Hz.
     * @param phase Phase (in radians), passed by reference and updated.
     * @return The sample value.
     */
    float generateWaveSample(double frequency, double& phase);

    std::vector<double> cachedDetuneCents; ///< Cached Unison State detune offsets per voice
    std::vector<float> cachedLeftGains;    ///< Cached Unison State left gain per voice
    std::vector<float> cachedRightGains;   ///< Cached Unison State right gain per voice
};
