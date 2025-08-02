#pragma once

#include "../../Common.h"
#include <JuceHeader.h>

/**
 * @class TalkboxFilter
 * @brief Implements a 3-band formant filter simulating vowel-like filtering.
 *
 * The TalkboxFilter class processes audio using three parallel band-pass filters,
 * each corresponding to one of the formant frequencies of a selected vowel.
 * Morphing shifts all formant frequencies exponentially.
 */
class TalkboxFilter
{
public:
    static constexpr int numFormants = 3; ///< Number of formant bands used in the filter.

    /**
     * @enum Vowel
     * @brief Enumerates supported vowel presets with pre-defined formant frequencies.
     */
    enum class Vowel
    {
        A,     ///< Vowel A
        E,     ///< Vowel E
        I,     ///< Vowel I
        O,     ///< Vowel O
        U,     ///< Vowel U
        Count
    };

    /**
     * @enum ParamID
     * @brief Identifiers for TalkboxFilter parameters.
     */
    enum class ParamID
    {
        Morph,   ///< Morphing amount (moves formants)
        Factor,  ///< Resonance / Q of band-pass filters
        Vowel,   ///< Selected vowel
        Count    ///< Number of parameters
    };

    /**
     * @struct FormantBand
     * @brief Holds frequency, Q factor, and gain for graphing purposes.
     */
    struct FormantBand
    {
        float frequency;  ///< Center frequency of the formant
        float q;          ///< Q factor of the filter
        float gain;       ///< Gain applied to the formant
    };

    using KnobParamSpecs = ::KnobParamSpecs;          ///< Alias for knob specification
    using ComboBoxParamSpecs = ::ComboBoxParamSpecs;  ///< Alias for combo-box specification

    /**
     * @brief Returns the knob specification for a given parameter.
     * @param param       Parameter ID.
     * @param filterIndex Filter index for parameter naming.
     * @return KnobParamSpecs containing range and identifiers.
     */
    static KnobParamSpecs getKnobParamSpecs(ParamID param, int filterIndex);

    /**
     * @brief Returns the combo-box specification for a given parameter.
     * @param param       Parameter ID.
     * @param filterIndex Filter index for parameter naming.
     * @return ComboBoxParamSpecs with choices and defaults.
     */
    static ComboBoxParamSpecs getComboBoxParamSpecs(ParamID param, int filterIndex);

    /**
     * @brief Constructs a new TalkboxFilter instance.
     */
    TalkboxFilter();

    /**
     * @brief Prepares the filter with processing settings.
     * @param spec DSP process specification.
     */
    void prepare(const juce::dsp::ProcessSpec& spec);

    /**
     * @brief Resets the internal filter state.
     */
    void reset();

    /**
     * @brief Processes an audio block through formant filters.
     * @param block Audio block to process.
     */
    void process(juce::dsp::AudioBlock<float>& block);

    /**
     * @brief Returns current formant bands for graphing.
     * @return Array of FormantBand structs.
     */
    std::array<FormantBand, numFormants> getFormantBandsForGraph() const;

    /**
     * @brief Sets the vowel preset.
     * @param newVowel Vowel enum to use.
     */
    void setVowel(Vowel newVowel);

    /**
     * @brief Sets the Q factor for all formant filters.
     * @param q Resonance value.
     */
    void setQFactor(float q);

    /**
     * @brief Sets the morph amount for formant shifting.
     * @param morph Normalized morph value in range [-1.0, 1.0].
     */
    void setMorph(float morph);

    /**
     * @brief Gets the currently selected vowel preset.
     * @return Active Vowel enum.
     */
    Vowel getVowel() const;

    /**
     * @brief Gets current morphed formant frequencies.
     * @return Array of morphed frequencies in Hz.
     */
    std::array<float, numFormants> getMorphedFrequencies() const;

private:
    Vowel currentVowel = Vowel::A;            ///< Currently selected vowel preset.
    float qFactor = 5.0f;                     ///< Resonance factor (Q scaling multiplier).
    float morphAmount = 0.0f;                 ///< Morph amount for exponential shifting.
    static constexpr float morphScale = 1.0f; ///< Exponential morph scaling factor.
    double sampleRate = 44100.0;              ///< Sample rate for filter processing.
    bool isPrepared = false;                  ///< Indicates if the filter has been prepared.

    static const std::map<Vowel, std::array<float, numFormants>> baseFormantMap; ///< Map of base formant frequencies per vowel.
    static const std::map<Vowel, std::array<float, numFormants>> baseGainDbMap;  ///< Map of formant gain values (dB) per vowel.

    std::array<float, numFormants> qFactorBase = { 1.0f, 1.75f, 3.0f };            ///< Base Q ratios per formant (relative weighting).
    std::array<float, numFormants> gains{};                                        ///< Linear gain factors derived from dB mapping.
    std::array<float, numFormants> gainCompensation = { 1.0f, 1.0f, 1.0f };        ///< Gain compensation values per formant.
    std::array<std::array<juce::dsp::IIR::Filter<float>, 2>, numFormants> filters; ///< Band-pass filters for each formant and stereo channel.
    std::array<float, numFormants> morphedFormants{};                              ///< Morphed formant frequencies in Hz.

    /**
     * @brief Updates internal filter coefficients based on current parameters.
     */
    void updateFilters();
};