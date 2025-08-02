#pragma once

#include "../../Common.h"
#include <JuceHeader.h>

 /**
  * @class LFO
  * @brief Represents an LFO instance for parameter modulation.
  */
class LFO
{
public:
    /**
     * @enum ParamID
     * @brief Identifiers for all LFO parameters.
     */ 
    enum class ParamID
    {
        Freq,       ///< LFO frequency in Hz
        Shape,      ///< Morph parameter
        Steps,      ///< Number of steps (Steps mode only)
        Type,       ///< LFO waveform profile
        Mode,       ///< Free-running or Retriggered
        Bypass,     ///< Bypass toggle
        Count
    };

    /// @brief Enumerates the available waveform types for an LFO.
    enum class Type
    {
        Sine,       ///< Smooth sine wave
        Triangle,   ///< Skewable triangle and ramp wavesLFOType
        Square,     ///< Pulse waveform with variable duty cycle
        Steps,      ///< Step-based sequencer
        Count
    };

    /// @brief Enumerates the possible phase modes for LFO playback.
    enum class Mode
    {
        Free,       ///< Continuous running
        Retrigger,  ///< Restart phase on trigger
        Count
    };

    /**
     * @brief Centralized default values for each LFO parameter.
     */
    struct Default
    {
        static constexpr float freq  = 1.0f;       ///< Default frequency in Hz
        static constexpr float shape = 0.5f;       ///< Default shape/morph
        static constexpr int steps   = 4;          ///< Default number of steps
        static constexpr Type type   = Type::Sine; ///< Default waveform type
        static constexpr Mode mode   = Mode::Free; ///< Default trigger mode
        static constexpr bool bypass = false;      ///< Default bypass state
    };

    using KnobParamSpecs = ::KnobParamSpecs;           ///< Knob parameter spec alias
    using ComboBoxParamSpecs = ::ComboBoxParamSpecs;   ///< ComboBox parameter spec alias

    /**
     * @brief Returns knob parameter specifications for a given LFO parameter.
     * @param id Parameter ID to inspect.
     * @param lfoIndex Index of the LFO (0-based).
     * @return KnobParamSpecs structure.
     */
    static KnobParamSpecs getKnobParamSpecs(ParamID id, int lfoIndex);

    /**
     * @brief Returns combo box parameter specifications for a given LFO parameter.
     * @param id Parameter ID to inspect.
     * @param lfoIndex Index of the LFO (0-based).
     * @return ComboBoxParamSpecs structure.
     */
    static ComboBoxParamSpecs getComboBoxParamSpecs(ParamID id, int lfoIndex);

    /**
     * @brief Returns toggle parameter specifications.
     * @param id Parameter ID to inspect.
     * @param lfoIndex Index of the LFO (0-based).
     * @return Pair of paramID and label string.
     */
    static std::pair<juce::String, juce::String> getToggleParamSpecs(ParamID id, int lfoIndex);

    /**
     * @brief Adds APVTS parameters for a single LFO instance.
     * @param lfoIndex Index of the LFO (0-based).
     * @param layout Reference to the APVTS layout to append parameters to.
     */
    static void addParameters(int lfoIndex, juce::AudioProcessorValueTreeState::ParameterLayout& layout);

    /**
     * @brief Constructs an LFO instance with a given index.
     * @param index The 0-based LFO index.
     */
    LFO(int index);

    /**
     * @brief Returns the display name of the LFO.
     * @return Reference to the internal string name.
     */
    const std::string& getName() const;

    /**
     * @brief Sets the frequency in Hz.
     * @param hz Frequency value.
     */
    void setFrequency(float hz);

    /**
     * @brief Sets the waveform type.
     * @param newType LFO waveform type.
     */
    void setType(Type newType);

    /**
     * @brief Sets the phase mode (Free or Retrigger).
     * @param newMode LFO mode.
     */
    void setMode(Mode newMode);

    /**
     * @brief Sets the shape/morph parameter (0.0 – 1.0).
     * @param newShape Shape value (normalized).
     */
    void setShape(float newShape);

    /**
     * @brief Sets the number of steps for Steps mode.
     * @param newNumSteps Integer step count.
     */
    void setNumSteps(int newNumSteps);

    /**
     * @brief Generates new random values for Steps mode.
     */
    void randomizeSteps();

    /**
     * @brief Enables or disables the LFO output.
     * @param shouldBypass True to bypass, false to enable.
     */
    void setBypassed(bool shouldBypass);

    /**
     * @brief Notifies the LFO that a retrigger event has occurred.
     */
    void onTrigger();

    /**
     * @brief Computes and fills the internal modulation buffer.
     * @param samplesPerBlock Number of audio samples per block.
     * @param sampleRate Host sample rate.
     */
    void advance(int samplesPerBlock, float sampleRate);

    /**
     * @brief Returns the next LFO value from the modulation buffer.
     * @return Normalized modulation value in [0.0, 1.0].
     */
    float getNextValue();

    /**
     * @brief Computes a single modulation value at a given phase.
     * @param phase The normalized phase [0.0, 1.0].
     * @return The shaped waveform value [0.0, 1.0].
     */
    float getValueAtPhase(float phase) const;

    /**
     * @brief Resets the internal phase to 0.
     */
    void resetPhase();

    /**
     * @brief Returns true if the LFO is currently bypassed.
     * @return True if bypassed; false otherwise.
     */
    bool isBypassed() const;

    /**
     * @brief Called on MIDI note-on. Triggers the LFO.
     */
    void noteOn();

    /**
     * @brief Resets the LFO’s active state (e.g., on transport stop).
     */
    void resetTrigger();

    /**
     * @brief Returns true if the LFO is currently active (was triggered).
     * @return True if active; false otherwise.
     */
    bool isActive() const;

    /**
     * @brief Updates the LFO’s internal parameters from the APVTS.
     * This is used to reapply the latest parameter values before calling advance().
     * Ensures that modulation reflects GUI knob changes immediately.
     * @param apvts Reference to the processor's AudioProcessorValueTreeState.
     */
    void updateFromAPVTS(juce::AudioProcessorValueTreeState& apvts);

    /**
     * @brief Returns true if the LFO should currently affect modulation output.
     * @return True if active, false if modulation should be suppressed.
     */
    bool isModulationActive() const;

    /**
     * @brief Sets whether the LFO should affect modulation output.
     * @param shouldBeActive True if LFO should affect modulated values, false to suppress output.
     */
    void setModulationActive(bool shouldBeActive);

private:
    int index = 0;                                     ///< LFO index.
    std::string name;                                  ///< LFO name.
    bool bypassed = false;                             ///< Whether the LFO is bypassed.
    bool needsRetrigger = false;                       ///< Flag indicating that the LFO phase should be reset on the next advance.
    bool isTriggered = false;                          ///< True if the LFO has been triggered via MIDI (Free or Retrigger mode).
    bool modulationActive = true;                      ///< True if the LFO modulation should be applied (note still held or envelope active).
    float frequencyHz = FormattingUtils::lfoFreqMinHz; ///< Current LFO frequency in Hz.
    float shape = 0.5f;                                ///< Morph parameter (0.0 to 1.0), interpreted per profile.
    int numSteps = 4;                                  ///< Number of steps used when in Steps mode.
    static constexpr int minSteps = 2;                 ///< Minimum allowed number of steps in Steps mode.
    static constexpr int maxSteps = 16;                ///< Maximum allowed number of steps in Steps mode.
    float phase = 0.0f;                                ///< Internal phase accumulator.
    Type type = Type::Sine;                            ///< Current selected waveform type.
    Mode mode = Mode::Free;                            ///< Current phase handling mode.
    std::vector<float> stepValues;                     ///< Precomputed step values used in Steps mode.
    std::vector<float> modulationBuffer;               ///< Cached output values per block.
    size_t bufferIndex = 0;                            ///< Read index into the modulation buffer.

    /**
     * @brief Remaps a phase value to emphasize specific waveform regions.
     * @param phase Normalized input phase [0.0, 1.0]
     * @param shape Morphing parameter [0.0, 1.0]
     * @return Warped phase value [0.0, 1.0]
     */
    static float warpPhase(float phase, float shape);
};