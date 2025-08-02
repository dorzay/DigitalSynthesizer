#pragma once

#include <vector>
#include <map>
#include <set>
#include <string>
#include <cmath>
#include <unordered_map>
#include <utility>
#include <JuceHeader.h>

/**
 * @brief Returns the absolute path to the project root directory.
 * @return juce::File pointing at the main project folder.
 */
juce::File getProjectPath();

inline const juce::URL projectUrl{ "https://github.com/dorzay/DigitalSynthesizer" }; ///< Repository's URL.

#define NUM_OF_OSCILLATORS 2 ///< Number of oscillators in the synth.
#define NUM_OF_ENVELOPES   2 ///< Number of ADSR envelopes.
#define NUM_OF_FILTERS     2 ///< Number of DSP filters.
#define NUM_OF_LFOS        4 ///< Number of LFO modules.

/**
 * @namespace UI
 * @brief User interface settings.
 */
namespace UI
{
    /**
     * @namespace Colors
     * @brief UI Colors and theme application.
     */
    namespace Colors
    {
        inline juce::Colour MainText;           ///< Primary text colour
        inline juce::Colour MainBackground;     ///< Main window background

        inline juce::Colour MenuBarText;                  ///< Menu text colour
        inline juce::Colour MenuBarHeaderText;            ///< Header text in menu bar
        inline juce::Colour MenuBarHighlightedText;       ///< Highlighted menu text
        inline juce::Colour MenuBarBackground;            ///< Menu bar background
        inline juce::Colour MenuBarHighlightedBackground; ///< Highlighted menu background

        inline juce::Colour OscillatorText;       ///< Oscillator labels
        inline juce::Colour OscillatorBackground; ///< Oscillator panel background

        inline juce::Colour EnvelopeText;           ///< Envelope labels
        inline juce::Colour EnvelopeBackground;     ///< Envelope background
        inline juce::Colour EnvelopeGraphCurve;     ///< Envelope curve colour
        inline juce::Colour EnvelopeGraphGridLines; ///< Envelope grid lines
        inline juce::Colour EnvelopeGraphGridText;  ///< Envelope grid labels
        inline juce::Colour EnvelopeGraphStroke;    ///< Envelope graph stroke

        inline juce::Colour FilterText;           ///< Filter labels
        inline juce::Colour FilterBackground;     ///< Filter background
        inline juce::Colour FilterGraphCurve;     ///< Filter curve
        inline juce::Colour FilterGraphGridLines; ///< Filter grid lines
        inline juce::Colour FilterGraphGridText;  ///< Filter grid labels
        inline juce::Colour FilterGraphStroke;    ///< Filter stroke
        inline juce::Colour FilterGraphFlood;     ///< Filter flood fill colour

        inline juce::Colour LFOText;           ///< LFO labels
        inline juce::Colour LFOBackground;     ///< LFO background
        inline juce::Colour LFOGraphCurve;     ///< LFO curve
        inline juce::Colour LFOGraphGridLines; ///< LFO grid lines
        inline juce::Colour LFOGraphGridText;  ///< LFO grid labels
        inline juce::Colour LFOGraphStroke;    ///< LFO stroke

        inline juce::Colour ComboBoxData;                ///< ComboBox text
        inline juce::Colour ComboBoxArrow;               ///< ComboBox arrow
        inline juce::Colour ComboBoxOutline;             ///< ComboBox outline
        inline juce::Colour ComboBoxBackground;          ///< ComboBox background
        inline juce::Colour ComboBoxHighlightBackground; ///< ComboBox highlight

        inline juce::Colour KnobThumb;          ///< Knob thumb colour
        inline juce::Colour KnobTextBoxText;    ///< Knob textbox text
        inline juce::Colour KnobTextBoxOutline; ///< Knob textbox outline
        inline juce::Colour KnobSliderFill;     ///< Knob slider fill
        inline juce::Colour KnobSliderOutline;  ///< Knob slider outline

        inline juce::Colour VolumeMeterText;           ///< Meter text
        inline juce::Colour VolumeMeterBackground;     ///< Meter background
        inline juce::Colour VolumeMeterBarBackground;  ///< Meter bar background

        inline juce::Colour MidiLearning = juce::Colours::orange;                  ///< MIDI learn highlight
        inline juce::Colour MidiConnected = juce::Colours::green;                  ///< MIDI connected indicator
        inline juce::Colour ModulationRing = juce::Colour::fromRGB(120, 200, 210); ///< Modulation ring colour

        /**
         * @brief Apply a named theme to all UI colour variables.
         * @param name Theme name.
         */
        void applyTheme(const juce::String& name);

        /**
         * @brief Apply a theme by numeric ID.
         * @param id Theme ID from getAvailableThemeNames().
         */
        void applyThemeByID(int id);

        /**
         * @brief Retrieve available theme IDs and names.
         * @return Const reference to vector of (ID, name) pairs.
         */
        const std::vector<std::pair<int, juce::String>>& getAvailableThemeNames();

        /**
         * @namespace Presets
         * @brief Predefined theme data.
         */
        namespace Presets
        {
            extern const std::map<juce::String, std::map<juce::String, juce::Colour>> themes;
        }
    }

    /**
     * @namespace Fonts
     * @brief Font sizes and default typeface for UI.
     */
    namespace Fonts
    {
        inline constexpr int defaultFontSize = 18; ///< Default text font size.
        inline constexpr int headerFontSize = 25;  ///< Header font size.
        extern juce::Font defaultFont;             ///< Default UI font.
    }
}


/**
 * @namespace FormattingUtils
 * @brief Value mapping and formatting for parameters.
 */
namespace FormattingUtils
{

    inline constexpr float envelopeTimeExponent = 3.0f;
    inline constexpr float freqMinHz            = 20.0f;
    inline constexpr float freqMaxHz            = 20000.0f;
    inline const     float logFreqMin           = std::log10(freqMinHz);
    inline const     float logFreqMax           = std::log10(freqMaxHz);
    inline constexpr float vowelMorphMinHz      = 100.0f;
    inline constexpr float vowelMorphMaxHz      = 5000.0f;
    inline const     float logVowelMorphMin     = std::log(vowelMorphMinHz);
    inline const     float logVowelMorphMax     = std::log(vowelMorphMaxHz);
    inline constexpr float resonanceMin         = 0.7071f;
    inline constexpr float resonanceMax         = 10.0f;
    inline constexpr float lfoFreqMinHz         = 0.05f;
    inline constexpr float lfoFreqMaxHz         = 20.0f;

    /**
     * @enum FormatType
     * @brief Types of parameter formatting.
     */
    enum class FormatType
    {
        Normal,               ///< Linear 0.0–1.0
        Discrete,             ///< Integer steps
        Pan,                  ///< Stereo pan
        Time,                 ///< Time in ms or s
        Percent,              ///< Percent display
        FrequencyLowPass,     ///< Lowpass cutoff (sqrt warp)
        FrequencyHighPass,    ///< Highpass cutoff (square warp)
        Slope,                ///< Discrete slope steps
        Resonance,            ///< Resonance/Q mapping
        VowelCenterFrequency, ///< Talkbox vowel center
        LFOFrequency          ///< LFO frequency mapping
    };

    /**
     * @brief Map normalized [0–1] to actual value.
     * @param normalizedValue Value in [0–1].
     * @param formatType Type of mapping.
     * @param minValue Minimum of range.
     * @param maxValue Maximum of range.
     * @param enumCount Optional enum count for discrete types.
     * @return Scaled value.
     */
    float normalizedToValue(float normalizedValue, FormatType formatType, float minValue, float maxValue, int enumCount = 0);

    /**
     * @brief Map actual value to normalized [0–1].
     * @param value Value to normalize.
     * @param formatType Type of mapping.
     * @param minValue Minimum of range.
     * @param maxValue Maximum of range.
     * @param enumCount Optional enum count.
     * @return Normalized value.
     */
    float valueToNormalized(float value, FormatType formatType, float minValue, float maxValue, int enumCount = 0);

    /**
     * @brief Format normalized value to string.
     * @param normalized Normalized in [0–1].
     * @param formatType Type of formatting.
     * @param minValue Minimum of range.
     * @param maxValue Maximum of range.
     * @param enumCount Optional enum count.
     * @return Formatted string.
     */
    juce::String formatValue(float normalized, FormatType formatType, float minValue = 0.0f, float maxValue = 1.0f, int enumCount = 0);
}

/**
 * @struct KnobParamSpecs
 * @brief APVTS specification for continuous (rotary) parameters.
 */
struct KnobParamSpecs
{
    juce::String id;         ///< APVTS parameter ID
    juce::String name;       ///< Display name
    float minValue;          ///< Minimum parameter value
    float maxValue;          ///< Maximum parameter value
    float stepSize;          ///< Increment step size
    float defaultValue;      ///< Default value
    FormattingUtils::FormatType formatType = FormattingUtils::FormatType::Normal; ///< Format mapping
    bool isDiscrete = false; ///< Discrete flag
};

/**
 * @struct ComboBoxParamSpecs
 * @brief APVTS specification for discrete ComboBox parameters.
 */
struct ComboBoxParamSpecs
{
    juce::String paramID;      ///< APVTS parameter ID
    juce::String label;        ///< UI label
    juce::StringArray choices; ///< Choice list
    int defaultIndex;          ///< Default selection index
};

/**
 * @namespace MidiController
 * @brief Mapping of MIDI CC numbers to synth controls (Arturia MiniLab).
 */
namespace MidiController
{
    extern const std::map<int, int> ccToKnobIndex; ///< Map from CC number to knob index.
    extern const std::set<int> assignedKnobs;      ///< Set of assigned CCs.
}