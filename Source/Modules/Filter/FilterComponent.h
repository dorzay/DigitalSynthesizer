#pragma once

#include "Filter.h"
#include "TalkboxFilter.h"
#include "FilterGraph.h"
#include "../../Common.h"
#include "../Knob/Knob.h"
#include "../ComboBox/ComboBox.h"
#include "../../PluginProcessor.h"
#include <JuceHeader.h>

/**
 * @class FilterComponent
 * @brief GUI for controlling a single filter.
 */
class FilterComponent : public juce::Component
{
public:
    /**
     * @brief Constructs a FilterComponent.
     * @param apvts Reference to the AudioProcessorValueTreeState.
     * @param processor Reference to the owning processor.
     * @param name Display name for the filter.
     * @param filterIndex Zero-based index for this filter.
     * @param targets Map of linkable target names to Linkable* objects.
     */
    FilterComponent(juce::AudioProcessorValueTreeState& apvts, DigitalSynthesizerAudioProcessor& processor, const juce::String& name, int filterIndex, const std::unordered_map<std::string, Linkable*>& targets);

    /** @brief Destructor. Resets all APVTS attachments and cleans up knobs. */
    ~FilterComponent() override;

    /**
     * @brief Registers modulation parameters for filter knobs.
     * @param index Filter index.
     * @param layout Reference to the APVTS parameter layout.
     */
    static void registerModulationParameters(int index, juce::AudioProcessorValueTreeState::ParameterLayout& layout);

    /**
     * @brief Returns the total width.
     * @return Width in pixels.
     */
    static int getTotalWidth();

    /**
     * @brief Renders the filter component visuals.
     * @param g Graphics context used for drawing.
     */
    void paint(juce::Graphics& g) override;

    /**
     * @brief Resizes and lays out all child components.
     */
    void resized() override;

    /** 
     * @brief Applies the current color theme to all subcomponents. 
     */
    void updateTheme();

    /**
     * @brief Updates the cutoff knob's text format based on filter type.
     */
    void updateCutoffKnobFormat();

    /**
     * @brief Shows or hides controls based on selected filter type.
     */
    void updateViewForFilterType();
 
    /**
     * @brief Sets available linkable targets for this filter.
     * @param targets Map of name->Linkable* target objects.
     */
    void setLinkableTargets(const std::unordered_map<std::string, Linkable*>& targets);

    /**
     * @brief Unlinks this filter from the given Linkable target.
     * @param target The Linkable to unlink from.
     */
    void unlinkTarget(Linkable* target);

private:
    const int filterIndex;                        ///< Zero-based filter index
    DigitalSynthesizerAudioProcessor& processor;  ///< Reference to processor

    static constexpr int titleHeight = 40;        ///< Height of title label row
    static constexpr int selectorHeight = 50;     ///< Height of selector row
    static constexpr int comboRowHeight = 30;     ///< Height of combo row
    static constexpr int knobRowHeight = 120;     ///< Height of knob row
    static constexpr int totalWidth = 610;        ///< Total component width
    static constexpr int rowPadding = 5;          ///< Padding around rows
    static constexpr int knobSpacing = 10;        ///< Spacing between knobs
    static constexpr int selectorLabelWidth = 60; ///< Width of comboBox labels
    static constexpr int comboBoxWidth = 100;     ///< Width of combo boxes
    static constexpr int selectorOffsetX = -20;   ///< X-offset for selectors
    static constexpr int textBoxHeight = 20;      ///< Height of knob text box
    static constexpr int wideTextBoxWidth = 180;  ///< Width of wide text box
    static constexpr int narrowTextBoxWidth = 50; ///< Width of narrow text box
    static constexpr int graphLeftMargin = 33;    ///< Left margin for graph
    static constexpr int graphReduceX = 10;       ///< X reduction for graph
    static constexpr int graphReduceY = 20;       ///< Y reduction for graph
    static constexpr int graphTranslateX = -10;   ///< X translation for graph
    static constexpr int graphTranslateY = -12;   ///< Y translation for graph

    juce::Label titleLabel;          ///< Displays filter name
    juce::Label typeLabel;           ///< Label for filter type
    ComboBox typeSelector;           ///< Filter type selector
    juce::ToggleButton bypassToggle; ///< Bypass on/off toggle
    juce::Label linkLabel;           ///< Label for link selector
    ComboBox linkSelector;           ///< Oscillator linking selector
    juce::Label slopeLabel;          ///< Label for slope selector
    ComboBox slopeSelector;          ///< Slope selector
    juce::Label vowelLabel;          ///< Label for vowel selector
    ComboBox vowelSelector;          ///< Vowel selector (Talkbox only)
    Knob cutoffKnob;                 ///< Cutoff frequency knob
    Knob resonanceKnob;              ///< Resonance knob
    Knob driveKnob;                  ///< Drive level knob
    Knob mixKnob;                    ///< Mix amount knob
    Knob morphKnob;                  ///< Talkbox morph knob
    Knob factorKnob;                 ///< Talkbox factor knob

    juce::Rectangle<int> graphArea;            ///< Bounds for FilterGraph
    juce::Rectangle<float> lastGraphArea;      ///< Last graph bounds
    FilterGraph filterGraph;                   ///< Visual filter response graph

    std::unordered_map<std::string, Linkable*> linkableTargets; ///< Available link targets
    Linkable* currentlyLinkedTarget = nullptr;                  ///< Active link target

    /**
     * @brief Initializes and binds a knob to its parameter.
     */
    void setupKnob(juce::AudioProcessorValueTreeState& apvts, Knob& knob, const Filter::KnobParamSpecs& spec);

    /**
     * @brief Updates the FilterGraph parameters when any knob changes.
     */
    void updateGraphFromKnobs();

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> typeAttachment;   ///< APVTS attachment for type
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> linkAttachment;   ///< APVTS attachment for link
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>   bypassAttachment; ///< APVTS attachment for bypass
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> slopeAttachment;  ///< APVTS attachment for slope
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> vowelAttachment;  ///< APVTS attachment for vowel

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FilterComponent)
};
