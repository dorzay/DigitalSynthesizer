#pragma once

#include "../../Common.h"
#include <JuceHeader.h>

/**
 * @class Envelope
 * @brief Contains parameter specifications and DSP logic for an ADSR envelope.
 */
class Envelope
{
public:
    /**
     * @brief Enum representing the four ADSR stages.
     */
    enum class ADSR
    {
        Attack,
        Decay,
        Sustain,
        Release,
        Count
    };

    /**
     * @brief Envelope playback mode for note behavior.
     */
    enum class Mode
    {
        Normal,         ///< Standard ADSR, waits for note-off before release.
        AutoRelease     ///< Triggers full shape on note-on, ignores note-off.
    };

    /**
     * @class Envelope::EnvelopeADSR
     * @brief A custom ADSR subclass that supports Auto Release behavior.
     *
     * Extends juce::ADSR with auto-release logic that triggers release
     * without an external note-off in Auto-Release mode.
     */
    class EnvelopeADSR : public juce::ADSR
    {
    public:
        /**
         * @brief Sets the envelope's playback mode.
         * @param m The desired playback mode (Normal or Auto Release).
         */
        void setMode(Envelope::Mode m) noexcept;

        /**
         * @brief Triggers the start of the envelope.
         */
        void noteOn() noexcept;

        /**
         * @brief Computes the next envelope value and applies auto-release logic if needed.
         * @return The next sample value of the envelope, between 0.0 and 1.0.
         */
        float getNextSample() noexcept;

        /**
         * @brief Returns the last computed envelope output without advancing state.
         */
        float getCurrentValue() const noexcept;

        float lastValue = 0.0f; ///< Last envelope output sample.

    private:
        Envelope::Mode mode = Envelope::Mode::Normal;  ///< Current playback mode.
        bool attackEnded = false;                      ///< Attack phase completed flag.
        bool releaseTriggered = false;                 ///< Auto-release triggered flag.
    };

    /**
     * @brief Constructs an Envelope instance with a specific index.
     * @param index Zero-based envelope index.
     * @param apvts Reference to the AudioProcessorValueTreeState.
     */
    Envelope(int index, juce::AudioProcessorValueTreeState& apvts);

    /**
     * @brief Adds all parameter definitions for this envelope to the APVTS layout.
     */
    static void addParameters(int index, juce::AudioProcessorValueTreeState::ParameterLayout& layout);

    using KnobParamSpecs = ::KnobParamSpecs;         ///< ADSR knob parameter specs.
    using ComboBoxParamSpecs = ::ComboBoxParamSpecs; ///< ComboBox parameter specs.

    /**
     * @brief Gets the display name of the envelope.
     * @return The envelope's display name.
     */
    juce::String getName() const;

    /**
     * @brief Get ADSR parameter specifications for a given envelope index.
     * @param index Envelope index.
     * @return Vector containing ADSR parameter specs.
     */
    static std::vector<KnobParamSpecs> getParamSpecs(int index);

    /**
     * @brief Returns parameter spec for the envelope mode at given index.
     * @param index Zero-based envelope index.
     * @return ComboBox parameter spec for mode.
     */
    static ComboBoxParamSpecs getEnvelopeModeParamSpecs(int index);

    /**
     * @brief Returns parameter spec for the envelope link target at given index.
     * @param index Zero-based envelope index.
     * @return ComboBox parameter spec for link target.
     */
    static ComboBoxParamSpecs getEnvelopeLinkParamSpecs(int index);

    /**
     * @brief Sets ADSR parameters (normalized 0.0–1.0) and scales internally.
     * @param attack Normalized attack value.
     * @param decay Normalized decay value.
     * @param sustain Normalized sustain level.
     * @param release Normalized release value.
     */
    void setParameters(float attack, float decay, float sustain, float release);

    /** @brief Updates mode and ADSR parameters from APVTS. */
    void updateFromParameters();

    /**
     * @brief Sets the sample rate for internal ADSR instances.
     * @param newSampleRate Audio sample rate (e.g., 44100.0 Hz).
     */
    void setSampleRate(double newSampleRate);

    /**
     * @brief Trigger note-on for a specific MIDI note.
     * @param midiNote MIDI note number to activate.
     */
    void noteOn(int midiNote);

    /**
     * @brief Trigger note-off for a specific MIDI note.
     * @param midiNote MIDI note number to release.
     */
    void noteOff(int midiNote);

    /**
     * @brief Resets all active voices' ADSR envelopes.
     */
    void resetAllVoices();

    /**
     * @brief Checks if a MIDI note is still active.
     * @param midiNote MIDI note number.
     * @return True if the envelope for this note is active.
     */
    bool isNoteActive(int midiNote) const;

    /**
     * @brief Returns true if any voice is currently active.
     * @return True if any envelope voice is active or releasing.
     */
    bool isActive() const;

    /**
     * @brief Gets next envelope sample for a given MIDI note.
     * @param midiNote MIDI note number.
     * @return Next envelope output sample.
     */
    float getNextSampleForNote(int midiNote);

    /**
     * @brief Computes mixed output from all active voices for modulation.
     * @return Normalized modulation level [0.0, 1.0].
     */
    float getModulationValue() const;

    /**
     * @brief Advances all active voices by one sample.
     */
    void tick();

    /**
     * @brief Returns list of supported envelope modes and display names.
     * @return Vector of {Mode, display name} pairs.
     */
    static const std::vector<std::pair<Mode, juce::String>>& getModeList();

    /**
     * @brief Converts an envelope mode to a display string.
     * @param mode Mode to convert.
     * @return Display string.
     */
    static juce::String modeToString(Mode mode);

    /**
     * @brief Converts a display string to an envelope mode.
     * @param label Display string.
     * @return Corresponding Mode (default Normal).
     */
    static Mode stringToMode(const juce::String& label);

    /**
     * @brief Sets the envelope playback mode.
     * @param newMode Mode to use.
     */
    void setMode(Mode newMode);

    /**
     * @brief Returns the current envelope mode.
     * @return Currently set Mode.
     */
    Mode getMode() const;

    static constexpr float MIN_ADSR_TIME_MS = 1.0f;    ///< Minimum ADSR time in milliseconds.
    static constexpr float MAX_ADSR_TIME_MS = 5000.0f; ///< Maximum ADSR time in milliseconds.

private:
    juce::AudioProcessorValueTreeState& apvts; ///< Reference to the global APVTS
    juce::String name;                         ///< Display name of the envelope
    Mode mode = Mode::Normal;                  ///< Current playback mode
    int envelopeIndex;                         ///< Index identifying this envelope instance
    double sampleRate{ 44100.0 };              ///< Current sample rate used to scale time-based ADSR parameters (Hz)
    float attackNorm{ 0.0f };                  ///< Current normalized attack parameter
    float decayNorm{ 0.0f };                   ///< Current normalized decay parameter
    float sustainNorm{ 1.0f };                 ///< Current normalized sustain parameter
    float releaseNorm{ 0.0f };                 ///< Current normalized release parameter

    struct VoiceEnvelope 
    { ///< Represents a per-note envelope voice
        int midiNote = -1;             ///< MIDI note assigned to this voice
        bool active = false;           ///< True if voice is in use
        EnvelopeADSR adsr;             ///< Custom ADSR supporting AutoRelease
        juce::ADSR::Parameters params; ///< Cached ADSR parameters
    };

    static constexpr int maxPolyphony = 16;                 ///< Number of simultaneous voices supported
    std::array<VoiceEnvelope, maxPolyphony> voiceEnvelopes; ///< Fixed pool of envelope voices

    /**
     * @brief Converts normalized ADSR parameters into a JUCE ADSR parameters struct.
     * @param attack Normalized attack time.
     * @param decay Normalized decay time.
     * @param sustain Normalized sustain level.
     * @param release Normalized release time.
     * @return A fully populated juce::ADSR::Parameters instance.
     */
    juce::ADSR::Parameters mapToADSRParams(float attack, float decay, float sustain, float release) const;
};