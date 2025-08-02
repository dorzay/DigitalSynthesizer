#pragma once

#include "../../Common.h"
#include <JuceHeader.h>

class DigitalSynthesizerAudioProcessor;

/**
 * @class PresetManager
 * @brief Manages synthesizer preset functionality including init, load, and save operations.
 */
class PresetManager
{
public:
    /**
     * @brief Constructs a PresetManager with access to the APVTS and processor.
     * @param apvts Reference to the processor's AudioProcessorValueTreeState.
     * @param processor Reference to the owning processor (used for restoring routing).
     */
    PresetManager(juce::AudioProcessorValueTreeState& apvts, DigitalSynthesizerAudioProcessor& processor);

    /**
     * @brief Returns the default preset folder path, ensuring it exists.
     * @return A juce::File object representing the default preset folder.
     */
    juce::File getDefaultPresetFolder();

    /**
     * @brief Resets the synth parameters to their initial/default state.
     */
    void initPreset();

    /**
     * @brief Saves the current plugin state to the specified file.
     * @param presetFile File where the preset will be saved.
     * @return true if saved successfully, false otherwise.
     */
    bool savePreset(const juce::File& presetFile);

    /**
     * @brief Loads a preset from the specified file into the plugin state.
     * @param presetFile File to load the preset from.
     * @return true if loaded successfully, false otherwise.
     */
    bool loadPreset(const juce::File& presetFile);

    /**
     * @brief Displays a file dialog to let the user choose a preset file to load.
     */
    void showLoadDialogBox();

    /**
     * @brief Displays a file dialog to let the user choose a location to save the current preset.
     */
    void showSaveDialogBox();

private:
    juce::AudioProcessorValueTreeState& apvts;   ///< Reference to the AudioProcessorValueTreeState.
    DigitalSynthesizerAudioProcessor& processor; ///< Reference to the processor.

    static constexpr int dialogBoxHeight = 400; ///< Default dialog height in pixels.
    static constexpr int dialogBoxWidth = 800;  ///< Default dialog width in pixels.

    /**
     * @brief Sets the bounds of the file dialog to a fixed width and height, centered on screen.
     * @param dialog Pointer to the FileChooserDialogBox to be positioned.
     */
    void setDialogBoundsWithAspectRatio(juce::FileChooserDialogBox* dialog);
};
