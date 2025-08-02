#pragma once

#include "TalkboxFilter.h"
#include "../../Common.h"
#include <JuceHeader.h>

/**
 * @class Filter
 * @brief A filter module of basic Filters and Talkbox.
 */
class Filter
{
public:
    /**
     * @enum ParamID
     * @brief Identifiers for all filter parameters.
     */
    enum class ParamID
    {
        Cutoff,     ///< Cutoff frequency (log scale)
        Resonance,  ///< Filter Q/resonance
        Drive,      ///< Post-filter drive
        Mix,        ///< Dry/wet blend
        Slope,      ///< Filter slope (12/24 dB)
        Type,       ///< Filter type (LP, HP, BP)
        Bypass,     ///< Bypass toggle
        Link,       ///< Oscillator linking target
        Count
    };

    /**
     * @enum Type
     * @brief Filter mode types used to select processing algorithm.
     */
    enum class Type
    {
        LowPass,   ///< Standard 12/24 dB low-pass filter
        HighPass,  ///< Standard 12/24 dB high-pass filter
        BandPass,  ///< Standard 12/24 dB band-pass filter
        Talkbox,   ///< Custom Talkbox filter
        Count
    };

    /**
     * @enum Slope
     * @brief Supported filter slope levels in dB/oct.
     */
    enum class Slope
    {
        dB12 = 0,   ///< 12 dB/octave
        dB24 = 1,   ///< 24 dB/octave
        Count
    };

    /**
     * @struct Parameters
     * @brief Holds all filter values
     */
    struct Parameters
    {
        float cutoffHz;    ///< Cutoff frequency in Hz
        float resonance;   ///< Filter resonance (Q)
        float drive;       ///< Post-filter drive amount
        float mix;         ///< Dry/wet mix (0.0 dry, 1.0 wet)
        Slope slope;       ///< Filter slope
        bool  bypass;      ///< Bypass toggle
        Type  type;        ///< Filter type

        /**
         * @struct Default
         * @brief Default values for each filter parameter.
         */
        struct Default
        {
            static constexpr float Cutoff = 1000.0f;           ///< Default cutoff frequency
            static constexpr float Resonance = 0.0f;           ///< Default resonance
            static constexpr float Drive = 0.0f;               ///< Default drive
            static constexpr float Mix = 1.0f;                 ///< Default wet mix
            static constexpr Slope Slope = Slope::dB12;        ///< Default slope: 12 dB/oct
            static constexpr bool  Bypass = false;             ///< Default bypass state
            static constexpr Type  FilterType = Type::LowPass; ///< Default filter type
        };
    };

    /**
     * @brief Constructs a Filter instance with a specific index.
     * @param index Filter index (used for parameter ID suffixes).
     */
    Filter(int index);

    using KnobParamSpecs = ::KnobParamSpecs;           ///< Knob parameter spec alias
    using ComboBoxParamSpecs = ::ComboBoxParamSpecs;   ///< ComboBox parameter spec alias

    /**
     * @brief Retrieves the specification of a knob-type parameter.
     * @param id Parameter identifier.
     * @param filterIndex Filter index.
     * @return Knob parameter specification.
     */
    static KnobParamSpecs getKnobParamSpecs(ParamID id, int filterIndex);

    /**
     * @brief Retrieves the specification of a ComboBox-type parameter.
     * @param id Parameter identifier.
     * @param filterIndex Filter index.
     * @return ComboBox parameter specification.
     */
    static ComboBoxParamSpecs getComboBoxParamSpecs(ParamID id, int filterIndex);

    /**
     * @brief Returns the toggle parameter spec (e.g., for bypass).
     * @param id Parameter identifier.
     * @param filterIndex Filter index.
     * @return Pair of parameter ID and label.
     */
    static std::pair<juce::String, juce::String> getToggleParamSpecs(ParamID id, int filterIndex);

    /**
     * @brief Adds all APVTS parameters for a given filter index.
     * @param filterIndex Filter index.
     * @param layout Parameter layout to append to.
     */
    static void addParameters(int filterIndex, juce::AudioProcessorValueTreeState::ParameterLayout& layout);

    /**
     * @brief Gets the display name of the filter.
     * @return Filter name string.
     */
    juce::String getName() const;

    /**
     * @brief Returns true if the current filter mode is Talkbox.
     * @return True if Talkbox mode.
     */
    bool isTalkboxMode() const;

    /**
     * @brief Returns a reference to the internal TalkboxFilter instance.
     * @return TalkboxFilter reference.
     */
    TalkboxFilter& getTalkboxFilter();

    /**
     * @brief Prepares the DSP modules for playback.
     * @param sampleRate Current audio sample rate.
     * @param samplesPerBlock Expected block size.
     */
    void prepareToPlay(double sampleRate, int samplesPerBlock);

    /**
     * @brief Resets internal DSP state.
     */
    void reset();

    /**
     * @brief Processes an audio block through the filter.
     * @param context The processing context replacing float.
     */
    void process(juce::dsp::ProcessContextReplacing<float> context);

    /**
     * @brief Updates the internal filter coefficients if needed.
     */
    void updateParametersIfNeeded();

    /**
     * @brief Updates parameter values from the APVTS tree.
     * @param apvts Reference to APVTS.
     * @param filterIndex Filter index.
     */
    void updateFromParameters(const juce::AudioProcessorValueTreeState& apvts, int filterIndex);
    ///@}

private:
    juce::String name;                           ///< Display name of the filter
    Parameters currentParams;                    ///< The live parameter state
    double currentSampleRate = 44100.0;          ///< Cached sample rate in Hz
    juce::uint32 currentBlockSize = 512;         ///< Cached block size
    TalkboxFilter talkboxFilter;                 ///< Talkbox filter instance
    bool needsUpdate = true;                     ///< Flag indicating parameter change
    juce::dsp::LadderFilter<float> ladderFilter; ///< Core Ladder filter processor

    /**
     * @brief Applies cutoff, resonance, drive, and slope to the Ladder filter.
     */
    void updateFilter();

    /**
     * @brief Applies filter's drive stage.
     * @param block Audio block to apply drive on.
     */
    void applyDrive(juce::dsp::AudioBlock<float>& block);
};
