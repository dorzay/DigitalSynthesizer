#pragma once

#include "Common.h"
#include "Modules/Linkable/Linkable.h"
#include "Modules/PresetManager/PresetManager.h"
#include "Modules/Oscillator/Oscillator.h"
#include "Modules/Knob/Knob.h"
#include "Modules/Knob/KnobModulation.h"
#include "Modules/Knob/ModulationTarget.h"
#include "Modules/Envelope/Envelope.h"
#include "Modules/Filter/Filter.h"
#include "Modules/LFO/LFO.h"
#include "Modules/VolumeMeter/VolumeMeter.h"
#include <JuceHeader.h>

/**
 * @class DigitalSynthesizerAudioProcessor
 * @brief The main audio processing unit for the digital synthesizer plugin.
 */
class DigitalSynthesizerAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    /** @name Construction / Destruction */
    //==============================================================================

    /**
     * @brief Constructs the `DigitalSynthesizerAudioProcessor` instance.
     * Initializes the audio processor and configures default parameters.
     */
    DigitalSynthesizerAudioProcessor();

    /**
     * @brief Destructor for cleaning up allocated resources.
     */
    ~DigitalSynthesizerAudioProcessor() override;

    //==============================================================================
    /** @name JUCE Plugin data */
    //==============================================================================

    /**
     * @brief Retrieves the name of the plugin.
     * @return The name of the plugin.
     */
    const juce::String getName() const override;

    /**
     * @brief Checks if the plugin accepts MIDI input.
     * @return True if the plugin supports MIDI input, false otherwise.
     */
    bool acceptsMidi() const override;

    /**
     * @brief Checks if the plugin produces MIDI output.
     * @return True if the plugin generates MIDI messages, false otherwise.
     */
    bool producesMidi() const override;

    /**
     * @brief Determines if the plugin functions purely as a MIDI effect.
     * @return True if the plugin is a MIDI effect, false otherwise.
     */
    bool isMidiEffect() const override;

    /**
     * @brief Retrieves the plugin's tail length in seconds.
     * @return The decay time after the sound stops.
     */
    double getTailLengthSeconds() const override;

    //==============================================================================
    /** @name Preset & State Management */
    //==============================================================================

    /**
     * @brief Retrieves the number of programs available.
     * @return The number of available programs.
     */
    int getNumPrograms() override;

    /**
     * @brief Retrieves the current program index.
     * @return The index of the currently selected program.
     */
    int getCurrentProgram() override;

    /**
     * @brief Sets the current program index.
     * @param index The index of the program to select.
     */
    void setCurrentProgram(int index) override;

    /**
     * @brief Retrieves the name of a specific program.
     * @param index The program index.
     * @return The name of the program as a `juce::String`.
     */
    const juce::String getProgramName(int index) override;

    /**
     * @brief Renames a specific program.
     * @param index The program index.
     * @param newName The new name to assign to the program.
     */
    void changeProgramName(int index, const juce::String& newName) override;

    /**
     * @brief Saves the plugin state to a memory block.
     * @param destData A reference to the memory block where state data will be stored.
     */
    void getStateInformation(juce::MemoryBlock& destData) override;

    /**
     * @brief Restores the plugin state from a memory block.
     * @param data Pointer to the memory block containing the saved state.
     * @param sizeInBytes The size of the memory block in bytes.
     */
    void setStateInformation(const void* data, int sizeInBytes) override;

    /**
     * @brief Retrieves the sample rate set during preparation.
     * This is the sample rate used by the audio engine, initialized in prepareToPlay().
     * @return The processor's current sample rate in Hz.
     */
    double getSampleRate() const;

    /**
     * @brief Provides access to the preset manager.
     * Useful for UI components or editors that need to load, save, or browse presets.
     * @return A pointer to the PresetManager instance.
     */
    PresetManager* getPresetManager();

    //==============================================================================
    /** @name Editor & UI */
    //==============================================================================

    /**
     * @brief Creates the user interface editor for the plugin.
     * @return A pointer to the created editor instance.
     */
    juce::AudioProcessorEditor* createEditor() override;

    /**
     * @brief Checks if the plugin provides a graphical user interface.
     * @return True if an editor is available, false otherwise.
     */
    bool hasEditor() const override;

    //==============================================================================
    /** @name Audio Lifecycle */
    //==============================================================================

    /**
     * @brief Prepares the audio processor before playback starts.
     * @param sampleRate The sample rate of the audio engine in Hz.
     * @param samplesPerBlock The number of samples per processing block.
     */
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;

    /**
     * @brief Releases any allocated resources when playback stops.
     */
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    /**
     * @brief Checks if the given channel layout is supported.
     * @param layouts The requested `BusesLayout`.
     * @return True if the requested layout is supported, false otherwise.
     */
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    //==============================================================================
    /** @name Audio + MIDI Processing */
    //==============================================================================

    /**
     * @brief Main per-block audio and MIDI processing entry point.
     * Clears the buffer, updates parameters, renders MIDI by events,
     * generates audio, moves envelopes and LFOs forward, and removes finished notes.
     * @param buffer The audio buffer to fill.
     * @param midiMessages The MIDI events for this block.
     */
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

    //==============================================================================
    /** @name Oscillator & Envelope */
    //==============================================================================

    /** @brief Retrieves a pointer to the oscillator at the given index. */
    Oscillator* getOscillator(int index);

    /** @brief Returns a pointer to the envelope at the given index. */
    Envelope* getEnvelope(int index);

    /** @brief Returns a pointer to the filter at the given index. */
    Filter* getFilter(int index);

    /** @brief Returns a pointer to the LFO at the given index. */
    LFO* getLFO(int index);

    /**
    * @brief Checks whether the given envelope is linked to any oscillator.
    * @param envelopeIndex The index of the envelope.
    * @return True if the envelope is used by at least one oscillator.
    */
    bool isEnvelopeLinkedToOscillator(int envelopeIndex) const;

    //==============================================================================
    /** @name MIDI Controller / Knobs */
    //==============================================================================

    /**
     * @brief Registers a UI knob with the processor for MIDI automation.
     * @param knob Pointer to the `Knob` instance to register.
     */
    void registerKnob(Knob* knob);

    /**
     * @brief Handles incoming MIDI control change messages.
     * @param message The incoming MIDI message.
     */
    void handleControllerMessage(const juce::MidiMessage& message);

    //==============================================================================
    /** @name Linkable Modulation System */
    //==============================================================================

    /**
     * @brief Registers a Linkable object using its display name.
     * @param target Pointer to the Linkable object to register.
     */
    void registerLinkableTarget(Linkable* target);

    /**
     * @brief Returns a const reference to all registered Linkable objects.
     * @return A map of display name strings to Linkable pointers.
     */
    const std::unordered_map<std::string, Linkable*>& getLinkableTargets() const;

    /**
     * @brief Registers ownership of a target Linkable by an EnvelopeComponent.
     * @param target The Linkable being linked.
     * @param newOwner The EnvelopeComponent requesting the link.
     * @return True if the link was successful.
     */
    bool registerEnvelopeLinkOwnership(Linkable* target, class EnvelopeComponent* newOwner);

    /**
     * @brief Registers ownership of a target Linkable by a FilterComponent.
     * @return True if the link was successful.
     */
    bool registerFilterLinkOwnership(Linkable* target, class FilterComponent* newOwner);

    /**
     * @brief Unregisters an EnvelopeComponent’s link to a target when it’s destroyed.
     * @param target The Linkable whose envelope link is to be removed.
     * @param owner The EnvelopeComponent being destroyed.
     */
    void unregisterEnvelopeLink(Linkable* target, class EnvelopeComponent* owner);

    /**
     * @brief Unregisters a FilterComponent’s link to a target when it’s destroyed.
     * @param target The Linkable whose filter link is to be removed.
     * @param owner The FilterComponent being destroyed.
     */
    void unregisterFilterLink(Linkable* target, class FilterComponent* owner);

    /**
     * @brief Clears all Envelopes and Filters link ownerships (when editor closes).
     */
    void clearLinkOwnerships();

    /** @brief Returns a reference to the registered knob pointers. */
    const std::vector<Knob*>& getKnobs() const { return knobs; }

    //==============================================================================
    // Modulation System
    //==============================================================================

    /**
     * @brief Returns a list of available modulation sources of the given type.
     *
     * This includes Envelope and LFO modulators and is used by the UI to build
     * right-click modulation menus for knobs.
     *
     * @param type The modulation source type (Envelope, LFO).
     * @return A vector of (ModulationSourceID, name) pairs.
     */
    std::vector<std::pair<ModulationSourceID, juce::String>> getAvailableModulationSources(ModulationSourceType type) const;

    /**
     * @brief Provides access to the ModulationRouter instance.
     *
     * Used by UI components (e.g., Knob) to connect/disconnect from modulation sources.
     *
     * @return Reference to the internal ModulationRouter.
     */
    ModulationRouter& getModulationRouter();

    /**
     * @brief Re-establishes modulation connections for all registered knobs
     *        based on their saved MOD_SOURCE and MOD_INDEX APVTS values.
     *
     * Called after loading a preset to restore correct runtime routing.
     */
    void restoreModulationRouting();

    /** @brief Unregister all knobs (called when editor is closed). */
    void clearAllKnobs();

    //==============================================================================
    /** @name Volume Metering */
    //==============================================================================

    /** @brief Set the master volume (0.0 to 1.0). */
    void setMasterVolume(float newVolume);

    /**
     * @brief Gets the current peak volume (in dB) of the left output channel.
     * @return The left channel volume in decibels.
     */
    float getMasterVolumeL() const noexcept;

    /**
     * @brief Gets the current peak volume (in dB) of the right output channel.
     * @return The right channel volume in decibels.
     */
    float getMasterVolumeR() const noexcept;

    /**
     * @brief Measures the output peak levels (in dB) across stereo channels and stores them.
     *
     * This method scans the given buffer region and computes the peak amplitude
     * per channel, converting the result to decibels.
     *
     * @param buffer The audio buffer to scan.
     * @param startSample The starting sample index.
     * @param numSamples The number of samples to inspect.
     */
    void updateOutputPeakLevels(const juce::AudioBuffer<float>& buffer, int startSample, int numSamples);

    //==============================================================================
    /** @name APVTS Parameter Layout */
    //==============================================================================

    /**
     * @brief Provides access to the AudioProcessorValueTreeState (APVTS).
     * @return Reference to the internal APVTS instance used for parameter linking.
     */
    juce::AudioProcessorValueTreeState& getAPVTS();

    /**
     * @brief Creates the parameter layout for APVTS.
     * @return A `ParameterLayout` object containing all parameter definitions.
     */
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    //==============================================================================
    /** @name Internal Modulation Proxy System */
    //@{
    /**
    * @brief Instantiates and connects invisible ModulationTarget proxies
    *        for all base parameters that support modulation.
    *
    * This ensures that envelope- and LFO- driven modulation continues
    * to apply even when no UI knobs are alive.
    *
    * Called once in the constructor.
    */
    void initializeModulationTargets();
    //@}

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DigitalSynthesizerAudioProcessor)

    /** @brief The sample rate of the processor, initialized during prepareToPlay. */
    double processorSampleRate{};

    //==============================================================================
    /** @name Parameter System (APVTS) */
    //==============================================================================

    /** @brief AudioProcessorValueTreeState (APVTS) for managing plugin parameters. */
    juce::AudioProcessorValueTreeState apvts;

    //==============================================================================
    /** @name Preset & State Management */
    //==============================================================================

    /**
     * @brief Manages presets for the synthesizer, including loading, saving, and initialization.
     */
    std::unique_ptr<PresetManager> presetManager;

    //==============================================================================
    /** @name Synthesizer's Elements */
    //==============================================================================

    /**
     * @brief Collection of oscillators used in the synthesizer.
     *
     * Each oscillator is managed by a unique_ptr for proper memory handling.
     */
    std::vector<std::unique_ptr<Oscillator>> oscillators;

    /**
     * @brief Collection of envelopes (ADSR) used for modulation.
     *
     * Each envelope is owned by a unique_ptr and modulates audio components like oscillators.
     */
    std::vector<std::unique_ptr<Envelope>> envelopes;

    /**
     * @brief Filter modules responsible for post-oscillator subtractive processing.
     *
     * Each Filter is monophonic and controlled via APVTS parameters,
     * including cutoff, resonance, drive, mix, slope, key tracking, and more.
     * Parameters are initialized via Filter::getKnobParamSpec and getComboBoxParamSpec.
     */
    std::vector<std::unique_ptr<Filter>> filters;

    /**
     * @brief Collection of LFO (Low-Frequency Oscillator) modules used for modulation.
     *
     * Each LFO is managed by a unique_ptr and is responsible for generating
     * low-frequency control signals that can modulate parameters such as
     * filter cutoff, pitch, or amplitude. LFOs are fully configurable through
     * the APVTS and linked into the modulation system.
     */
    std::vector<std::unique_ptr<LFO>> lfos;

    //==============================================================================
    /** @name Audio + MIDI Processing */
    //==============================================================================

    /**
     * @brief Refreshes all synthesizer parameters.
     */
    void updateParameters();

    /**
     * @brief Processes MIDI and renders audio.
     * @param buffer The audio buffer to fill with synthesized samples.
     * @param midiMessages The MIDI events to process for this block.
     */
    void handleMidiAndRender(juce::AudioBuffer<float>& buffer, const juce::MidiBuffer& midiMessages);

    /**
     * @brief Renders a contiguous block of audio samples.
     * @param buffer The audio buffer to write into.
     * @param startSample The index of the first sample to render.
     * @param numSamples The number of samples to process.
     */
    void renderAudioSegment(juce::AudioBuffer<float>& buffer, int startSample, int numSamples);

    /**
     * @brief Advances all ADSR envelopes.
     */
    void tickEnvelopes();

    /**
     * @brief Routes envelope outputs into the modulation system.
     */
    void pushEnvelopeModulation();

    /**
     * @brief Removes finished notes and disables idle modulation.
     */
    void finalizeNotes();

    //==============================================================================
    /** @name Linkable Modulation System */
    //==============================================================================
    /** @brief Registry of all linkable audio modules by display name. */
    std::unordered_map<std::string, Linkable*> linkableTargets;

    /**
    * @brief Tracks which EnvelopeComponent currently owns each Linkable target.
    */
    std::unordered_map<Linkable*, class EnvelopeComponent*> envelopeOwners;

    /**
    * @brief Tracks which filterComponent currently owns each Linkable target.
    */
    std::unordered_map<Linkable*, class FilterComponent*> filterOwners;

    /** @brief Routes modulation values from sources (e.g., Envelopes) to registered knobs. */
    ModulationRouter modulationRouter;

    //==============================================================================
    /** @name LFOs handlers
    //==============================================================================
    /**
     * @brief Triggers all LFOs when a MIDI note-on is received.
     */
    void handleNoteOnLfos();

    /**
     * @brief Resets all LFO triggers (e.g., on playback stop).
     */
    void resetAllLfos();

    /**
     * @brief Advances and pushes modulation values for all triggered LFOs.
     */
    void renderAllLFOs(int blockSize);

    //==============================================================================
    /** @name Knob / MIDI Control System */
    //==============================================================================

    /**
     * @brief Registered knob components for real-time control and MIDI mapping.
     */
    std::vector<Knob*> knobs;

    /**
    * @brief Invisible proxies for each base parameter that support modulation.
    *
    * These remain alive when the editor’s knobs are destroyed, so that
    * ModulationRouter can continue driving parameter changes.
    */
    std::vector<std::unique_ptr<ModulationTarget>> modulationTargets;

    //==============================================================================
    /** @name Volume Control & Metering */
    //==============================================================================

    /** @brief Smoothed master volume to prevent clicks when adjusting output gain. */
    juce::SmoothedValue<float> masterVolume{ 1.0f };

    /** @brief Last measured left channel peak volume in dB. */
    float masterVolumeLDb = VolumeMeter::initialVolumeDb;

    /** @brief Last measured right channel peak volume in dB. */
    float masterVolumeRDb = VolumeMeter::initialVolumeDb;

    /** @brief Global headroom factor for volume control. */
    static constexpr float headroomFactor = 0.7f;
};