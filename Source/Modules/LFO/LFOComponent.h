#pragma once

#include "../../Common.h"
#include "LFO.h"
#include "LFOGraph.h"
#include "../../PluginProcessor.h"
#include "./../Knob/Knob.h"
#include "./../Combobox/Combobox.h"
#include <JuceHeader.h>

/**
 * @class LFOComponent
 * @brief UI component for controlling a single LFO instance.
 */
class LFOComponent : public juce::Component, private juce::AudioProcessorValueTreeState::Listener
{
public:
    /**
     * @brief Constructs the LFOComponent.
     * @param apvts Reference to the AudioProcessorValueTreeState.
     * @param processor Reference to the audio processor.
     * @param lfoName Display name for the LFO.
     * @param lfoIndex Index of the LFO instance.
     */
    LFOComponent(juce::AudioProcessorValueTreeState& apvts,
        DigitalSynthesizerAudioProcessor& processor,
        const juce::String& lfoName,
        int lfoIndex);

    /**
     * @brief Destructor. Resets all APVTS attachments to prevent GUI crashes.
     */
    ~LFOComponent() override;

    /**
     * @brief Lays out all child components.
     */
    void resized() override;

    /**
     * @brief Paints visual elements (background, outlines, etc.).
     * @param g Graphics context used for painting.
     */
    void paint(juce::Graphics& g) override;

    /**
     * @brief Returns the total layout width of the LFOComponent.
     * @return Width in pixels.
     */
    static int getTotalWidth() { return totalWidth; }

    /**
     * @brief Returns the total height of the LFOComponent for layout.
     * @return Height in pixels.
     */
    static int getTotalHeight() { return totalHeight; }

    /**
     * @brief Updates UI visuals (colors, styles) to match current theme.
     */
    void updateTheme();

    /**
     * @brief Registers modulation parameters for all LFO knobs at the given index.
     * @param index LFO index.
     * @param layout Reference to the APVTS parameter layout.
     */
    static void registerModulationParameters(int index,
        juce::AudioProcessorValueTreeState::ParameterLayout& layout);


private:
    juce::AudioProcessorValueTreeState& apvtsRef;   ///< Reference to the plugin's APVTS.
    DigitalSynthesizerAudioProcessor& processorRef; ///< Reference to the main processor.

    const int index;                 ///< Index of the LFO (0-based).
    const juce::String name;         ///< Display name of the LFO (e.g., "LFO 1").
    juce::Label titleLabel;          ///< Label displaying the LFO name/title.
    juce::ToggleButton bypassButton; ///< Toggle to bypass the LFO’s output.
    juce::Label modeLabel;           ///< Label for the trigger mode selector.
    juce::Label typeLabel;           ///< Label for the waveform type selector.
    ComboBox modeSelector;           ///< Dropdown to select the trigger mode (Free / Retrigger).
    ComboBox typeSelector;           ///< Dropdown to select LFO waveform type.

    Knob freqKnob;  ///< Knob for adjusting frequency.
    Knob shapeKnob; ///< Knob for adjusting shape/morph.
    Knob stepsKnob; ///< Knob for step count (visible only in Steps mode).

    juce::TextButton randomizeButton{ "Random" }; ///< Button to randomize step values (Steps mode only).

    LFOGraph graph; ///< Graphical display of the LFO waveform.
    juce::Rectangle<int> graphBounds; ///< Last used graph bounds for painting.

    static constexpr int totalWidth = 400;             ///< Component width.
    static constexpr int totalHeight = 240;            ///< Component height.
    static constexpr int rowPadding = 5;               ///< Row spacing.
    static constexpr int selectorHeight = 40;          ///< Selector height.
    static constexpr int knobHeight = 110;             ///< Knob row height.
    static constexpr int labelWidth = 50;              ///< Label width.
    static constexpr int comboBoxWidth = 110;          ///< Combo box width.
    static constexpr int graphWidthRatioPct = 75;      ///< Graph width (%).
    static constexpr int randomButtonPadding = 10;     ///< Random button padding.
    static constexpr int randomButtonExtraW = 12;      ///< Random button extra width.
    static constexpr int randomButtonExtraH = 6;       ///< Random button extra height.
    static constexpr int textBoxWidthFreq = 60;        ///< Frequency textbox width.
    static constexpr int textBoxHeightFreq = 20;       ///< Frequency textbox height.
    static constexpr int selectorSpacing = rowPadding; ///< Selector spacing.
    static constexpr int selectorYOffset = 10;         ///< Selector Y offset.
    static constexpr int graphTranslateX = -5;         ///< Graph X shift.
    static constexpr int graphTranslateY = -10;        ///< Graph Y shift.

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modeAttachment; ///< Attachment for mode combobox.
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> typeAttachment; ///< Attachment for type combobox.
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment; ///< Attachment for bypass toggle.

    /**
     * @brief Updates control visibility based on selected LFO type.
     */
    void updateDynamicVisibility();

    /**
     * @brief Initializes all control properties and layout.
     */
    void initializeUI();

    /**
     * @brief Initializes and attaches a Knob using the given parameter spec.
     * @param knob The Knob instance to configure.
     * @param spec The parameter specification from LFO::getKnobParamSpecs().
     */
    void setupKnob(Knob& knob, const LFO::KnobParamSpecs& spec);

    /**
     * @brief Attaches all controls to the APVTS.
     */
    void setupAttachments();

    /**
     * @brief Updates the LFOGraph to reflect the current knob and selector values.
     */
    void updateLFOGraph();

    /**
     * @brief Draws the waveform curve and axis grid lines for the LFO graph.
     * @param g Graphics context to draw into.
     */
    void drawLFOGraph(juce::Graphics& g);

    void parameterChanged(const juce::String& parameterID, float newValue) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LFOComponent)
};