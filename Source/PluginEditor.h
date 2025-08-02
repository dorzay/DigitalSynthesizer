#pragma once

#include "Common.h"
#include "Modules/MenuBar/MenuBar.h"
#include "Modules/Oscillator/OscillatorComponent.h"
#include "Modules/Envelope/EnvelopeComponent.h"
#include "Modules/VolumeMeter/VolumeMeter.h"
#include "Modules/Filter/FilterComponent.h"
#include "Modules/LFO/LFOComponent.h"
#include <JuceHeader.h>

/**
 * @class DigitalSynthesizerAudioProcessorEditor
 * @brief The graphical user interface (GUI) editor for the digital synthesizer plugin.
 */
class DigitalSynthesizerAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    /**
     * @brief Constructs the plugin editor with a reference to the audio processor.
     * @param processor Reference to the `DigitalSynthesizerAudioProcessor` that processes the audio.
     */
    DigitalSynthesizerAudioProcessorEditor(DigitalSynthesizerAudioProcessor& processor);

    /**
     * @brief Destructor for the plugin editor.
     */
    ~DigitalSynthesizerAudioProcessorEditor() override;

    /**
     * @brief Custom background rendering for the plugin editor.
     * @param g Graphics context used for drawing the UI.
     */
    void paint(juce::Graphics&) override;

    /**
     * @brief Handles layout adjustments when the editor window is resized.
     */
    void resized() override;

    /**
     * @brief Recalculates and sets the plugin editor window size based on component layout.
     */
    void updateEditorSize();

    static constexpr int defaultWidth = 400;     ///< Default plugin window width in pixels.
    static constexpr int defaultHeight = 300;    ///< Default plugin window height in pixels.
    static constexpr int marginSize = 10;        ///< Margin size used for layout adjustments.
    static constexpr int labelFontSize = 15;     ///< Default font size for text labels in the UI.

private:
    DigitalSynthesizerAudioProcessor& audioProcessor; ///< Reference to the processor.

    std::unique_ptr<juce::Component> contentComponent; ///< Container component that holds all visible UI components and gets uniformly scaled.

    static constexpr int margin = 10; ///< UI layout margin.

    std::unique_ptr<MenuBar> menuBar; ///< The menu bar component for selecting themes.
    VolumeMeter volumeMeter;          ///< Visual stereo volume meter for displaying the master output signal levels.

    std::vector<std::unique_ptr<OscillatorComponent>> oscillators; ///< UI components for controlling oscillators.
    std::vector<std::unique_ptr<EnvelopeComponent>> envelopes;     ///< UI components for envelope shaping (ADSR).
    std::vector<std::unique_ptr<FilterComponent>> filters;         ///< UI components for filters.
    std::vector<std::unique_ptr<LFOComponent>> lfos;               ///< UI components for LFOs.

    /**
     * @brief Lays out all subcomponents inside the contentComponent using fixed coordinates.
     */
    void layoutContentComponents();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DigitalSynthesizerAudioProcessorEditor)
};
