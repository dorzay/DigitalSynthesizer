#pragma once

#include "../../Common.h"
#include "../../PluginProcessor.h"
#include "../Knob/Knob.h"
#include "../ComboBox/ComboBox.h"
#include "Oscillator.h"
#include <JuceHeader.h>

/**
 * @class OscillatorComponent
 * @brief GUI for controlling a single oscillator.
 */
class OscillatorComponent : public juce::Component
{
public:
    /**
     * @brief Constructs an OscillatorComponent.
     * @param apvts Reference to the AudioProcessorValueTreeState.
     * @param processor Reference to the owning processor.
     * @param name Display name for the oscillator.
     * @param index The oscillator index.
     */
    OscillatorComponent(juce::AudioProcessorValueTreeState& apvts, DigitalSynthesizerAudioProcessor& processor, const juce::String& name, int index);

    /** @brief Destructor. Resets APVTS attachments. */
    ~OscillatorComponent() override;

    /**
     * @brief Registers modulation parameters for all knobs.
     * @param index Oscillator index.
     * @param layout Reference to the APVTS parameter layout.
     */
    static void registerModulationParameters(int index, juce::AudioProcessorValueTreeState::ParameterLayout& layout);

    /**
     * @brief Returns the total height required by the component.
     * @return Height in pixels.
     */
    static int getTotalHeight();

    /**
     * @brief Returns the total width required by the component.
     * @return Width in pixels.
     */
    static int getTotalWidth();

    /**
     * @brief Renders the oscillator component visuals.
     * @param g Graphics context used for drawing.
     */
    void paint(juce::Graphics& g) override;

    /**
     * @brief Resizes and lays out all child components.
     */
    void resized() override;

    /**
     * @brief Applies current color theme to all subcomponents.
     */
    void updateTheme();

private:
    /**
     * @brief Initializes the waveform selector with icons.
     */
    void createWaveformSelector();

    /**
     * @brief Initializes and adds a knob with the given spec.
     * @param knob Reference to the Knob object.
     * @param id The Oscillator ParamID.
     */
    void setupKnob(Knob& knob, Oscillator::ParamID id);

    juce::AudioProcessorValueTreeState& apvtsRef;   ///< Reference to parameter state
    DigitalSynthesizerAudioProcessor& processorRef; ///< Reference to owning processor
    int index;                                      ///< Oscillator index

    static constexpr int totalWidth = 400;          ///< Total fixed width
    static constexpr int rowPadding = 5;            ///< Padding between rows
    static constexpr int knobSpacing = 10;          ///< Spacing between knobs

    static constexpr int titleHeight = 40;          ///< Height of the title label row
    static constexpr int titleWidth = 180;          ///< Width of the title label
    static constexpr int bypassWidth = 80;          ///< Width of the bypass toggle

    static constexpr int selectorHeight = 50;       ///< Height of the waveform/octave selector row
    static constexpr int selectorWidth = 90;        ///< Width of each selector label

    static constexpr int knobRowHeight = 120;       ///< Height of the knob row
    static constexpr int numKnobs = 4;              ///< Number of knobs in layout

    juce::Label titleLabel;                         ///< Title label
    juce::ToggleButton bypassButton;                ///< Bypass toggle button
    juce::Label waveformLabel;                      ///< Label for waveform selector
    ComboBox waveformSelector;                      ///< Waveform image ComboBox
    juce::Label octaveLabel;                        ///< Label for octave selector
    ComboBox octaveSelector;                        ///< Octave selector (text mode)

    Knob volumeKnob;                                ///< Output volume knob
    Knob panKnob;                                   ///< Stereo pan knob
    Knob voicesKnob;                                ///< Polyphony count knob
    Knob detuneKnob;                                ///< Unison detune knob

    std::vector<std::unique_ptr<juce::Drawable>> waveformImages; ///< Cached waveform icons

    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;     ///< APVTS attachment for bypass
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> waveformAttachment; ///< APVTS attachment for waveform
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> octaveAttachment;   ///< APVTS attachment for octaves
};
