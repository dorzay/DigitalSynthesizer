#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Modules/Linkable/LinkableUtils.h"

DigitalSynthesizerAudioProcessor::DigitalSynthesizerAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
    apvts(*this, nullptr, "PARAMETERS", createParameterLayout())
#endif
{
    presetManager = std::make_unique<PresetManager>(apvts, *this);

    oscillators.reserve(NUM_OF_OSCILLATORS);
    for (int i = 0; i < NUM_OF_OSCILLATORS; ++i)
    {
        oscillators.push_back(std::make_unique<Oscillator>(Oscillator::getDefaultSampleRate(), i, apvts));
        registerLinkableTarget(oscillators[i].get());
    }

    envelopes.reserve(NUM_OF_ENVELOPES);
    for (int i = 0; i < NUM_OF_ENVELOPES; ++i)
    {
        envelopes.push_back(std::make_unique<Envelope>(i, apvts));
    }

    filters.reserve(NUM_OF_FILTERS);
    for (int i = 0; i < NUM_OF_FILTERS; ++i)
    {
        filters.push_back(std::make_unique<Filter>(i));
    }

    lfos.reserve(NUM_OF_LFOS);
    for (int i = 0; i < NUM_OF_LFOS; ++i)
        lfos.push_back(std::make_unique<LFO>(i));

    initializeModulationTargets();
}

DigitalSynthesizerAudioProcessor::~DigitalSynthesizerAudioProcessor()
{
    knobs.clear();
}

const juce::String DigitalSynthesizerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DigitalSynthesizerAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool DigitalSynthesizerAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool DigitalSynthesizerAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double DigitalSynthesizerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int DigitalSynthesizerAudioProcessor::getNumPrograms()
{
    return 1;
}

int DigitalSynthesizerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DigitalSynthesizerAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String DigitalSynthesizerAudioProcessor::getProgramName(int index)
{
    return {};
}

void DigitalSynthesizerAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

void DigitalSynthesizerAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    if (auto state = apvts.copyState().createXml())
    {
        copyXmlToBinary(*state, destData);
    }
}

void DigitalSynthesizerAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto state = getXmlFromBinary(data, sizeInBytes))
    {
        apvts.replaceState(juce::ValueTree::fromXml(*state));

        restoreModulationRouting();
    }
}

double DigitalSynthesizerAudioProcessor::getSampleRate() const
{
    return processorSampleRate;
}

PresetManager* DigitalSynthesizerAudioProcessor::getPresetManager()
{
    return presetManager.get();
}

juce::AudioProcessorEditor* DigitalSynthesizerAudioProcessor::createEditor()
{
    return new DigitalSynthesizerAudioProcessorEditor(*this);
}

bool DigitalSynthesizerAudioProcessor::hasEditor() const
{
    return true;
}

void DigitalSynthesizerAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    processorSampleRate = sampleRate;

    juce::ignoreUnused(samplesPerBlock);
    masterVolume.reset(sampleRate, 0.01);

    for (auto& env : envelopes)
        env->setSampleRate(sampleRate);

    for (auto& filter : filters)
        filter->prepareToPlay(sampleRate, samplesPerBlock);

    resetAllLfos();
}

void DigitalSynthesizerAudioProcessor::releaseResources()
{
    resetAllLfos();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DigitalSynthesizerAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

void DigitalSynthesizerAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    if (buffer.getNumSamples() == 0)
        return;

    // Clear the output buffer
    buffer.clear();

    // Refresh all synth parameters from the APVTS
    updateParameters();

    // Handle incoming MIDI and render audio between events
    handleMidiAndRender(buffer, midiMessages);

    // Advance all ADSR envelopes by one block
    tickEnvelopes();

    // Push each envelope’s output into the modulation router
    pushEnvelopeModulation();

    // Advance and route all LFOs for this block
    renderAllLFOs(buffer.getNumSamples());

    // Remove finished notes and disable LFOs if idle
    finalizeNotes();
}

void DigitalSynthesizerAudioProcessor::updateParameters()
{
    for (int i = 0; i < NUM_OF_OSCILLATORS; ++i)
    {
        oscillators[i]->updateFromParameters();
    }

    for (int i = 0; i < NUM_OF_ENVELOPES; ++i)
    {
        envelopes[i]->updateFromParameters();
    }

    for (int i = 0; i < NUM_OF_FILTERS; ++i)
    {
        filters[i]->updateFromParameters(apvts, i);
        filters[i]->updateParametersIfNeeded();
    }
}

void DigitalSynthesizerAudioProcessor::handleMidiAndRender(juce::AudioBuffer<float>& buffer, const juce::MidiBuffer& midiMessages)
{
    const int totalSamples = buffer.getNumSamples();
    int currentSample = 0;

    for (const auto metadata : midiMessages)
    {
        const int eventSample = metadata.samplePosition;
        const auto message = metadata.getMessage();

        // Render audio from currentSample up to the MIDI event
        if (eventSample > currentSample)
            renderAudioSegment(buffer, currentSample, eventSample - currentSample);

        // Apply MIDI event
        currentSample = eventSample;

        if (message.isNoteOn())
        {
            for (auto& osc : oscillators)
            {
                auto* env = osc->getEnvelope();
                if (env != nullptr)
                {
                    const int midiNote = osc->calculateMidiNoteWithOctaveOffset(message.getNoteNumber());
                    env->noteOn(midiNote);
                }

                osc->noteOn(message);
            }
            handleNoteOnLfos();
        }
        else if (message.isNoteOff())
        {
            for (auto& osc : oscillators)
            {
                auto* env = osc->getEnvelope();
                if (env != nullptr)
                {
                    const int midiNote = osc->calculateMidiNoteWithOctaveOffset(message.getNoteNumber());
                    env->noteOff(midiNote);
                }

                osc->noteOff(message);
            }
        }

        if (message.isController())
            handleControllerMessage(message);
    }

    // Render any remaining audio
    if (currentSample < totalSamples)
        renderAudioSegment(buffer, currentSample, totalSamples - currentSample);
}

void DigitalSynthesizerAudioProcessor::renderAudioSegment(juce::AudioBuffer<float>& buffer, int startSample, int numSamples)
{
    const int numChannels = getTotalNumOutputChannels();
    const float normalization = headroomFactor / NUM_OF_OSCILLATORS;

    // Step 1: Clear the segment we are going to write into
    for (int ch = 0; ch < numChannels; ++ch)
        buffer.clear(ch, startSample, numSamples);

    // Step 2: Each oscillator sums into the buffer
    for (auto& osc : oscillators)
    {
        osc->processBlock(buffer, startSample, numSamples);
    }

    // Step 3: Apply normalization and master gain
    for (int ch = 0; ch < numChannels; ++ch)
    {
        float* channelData = buffer.getWritePointer(ch, startSample);
        for (int i = 0; i < numSamples; ++i)
        {
            float masterGain = masterVolume.getNextValue();
            channelData[i] *= normalization * masterGain;
        }
    }

    // Step 4: Update meters
    updateOutputPeakLevels(buffer, startSample, numSamples);
}

void DigitalSynthesizerAudioProcessor::tickEnvelopes()
{
    for (auto& env : envelopes)
        env->tick();
}

void DigitalSynthesizerAudioProcessor::pushEnvelopeModulation()
{
    for (int i = 0; i < static_cast<int>(envelopes.size()); ++i)
    {
        float envOut = envelopes[i]->getModulationValue();
        modulationRouter.pushModulationValue({ ModulationSourceType::Envelope, i }, envOut);
    }
}

void DigitalSynthesizerAudioProcessor::finalizeNotes()
{
    // Remove any released notes
    for (auto& osc : oscillators)
    {
        osc->removeReleasedNotesIf([this, &osc](int midiNote)
            {
                auto* env = osc->getEnvelope();
                return env ? !env->isNoteActive(midiNote) : true;
            });
    }

    // If no envelopes remain active, disable all LFO modulation
    bool anyActive = std::any_of(envelopes.begin(), envelopes.end(),
        [](const std::unique_ptr<Envelope>& env)
        {
            return env->isActive();
        });

    if (!anyActive)
    {
        for (auto& lfo : lfos)
            lfo->setModulationActive(false);
    }
}

Oscillator* DigitalSynthesizerAudioProcessor::getOscillator(int index)
{
    if (index < 0 || index >= oscillators.size())
        return nullptr;
    return oscillators[index].get();
}

Envelope* DigitalSynthesizerAudioProcessor::getEnvelope(int index)
{
    if (index < 0 || index >= envelopes.size())
        return nullptr;
    return envelopes[index].get();
}

Filter* DigitalSynthesizerAudioProcessor::getFilter(int index)
{
    if (index < 0 || index >= filters.size())
        return nullptr;
    return filters[index].get();
}

LFO* DigitalSynthesizerAudioProcessor::getLFO(int index)
{
    if (index < 0 || index >= static_cast<int>(lfos.size()))
        return nullptr;
    return lfos[index].get();
}

bool DigitalSynthesizerAudioProcessor::isEnvelopeLinkedToOscillator(int envelopeIndex) const
{
    if (envelopeIndex < 0 || envelopeIndex >= NUM_OF_OSCILLATORS)
        return false;

    for (const auto& osc : oscillators)
    {
        const Envelope* linkedEnv = osc->getEnvelope();
        if (linkedEnv == envelopes[envelopeIndex].get())
            return true;
    }

    return false;
}

void DigitalSynthesizerAudioProcessor::registerKnob(Knob* knob)
{
    if (knob != nullptr)
    {
        knobs.push_back(knob);
    }
}

void DigitalSynthesizerAudioProcessor::handleControllerMessage(const juce::MidiMessage& message)
{
    int ccNumber = message.getControllerNumber();
    float ccValue = message.getControllerValue() / 127.0f;

    if (MidiController::assignedKnobs.find(ccNumber) == MidiController::assignedKnobs.end())
        return;


    if (getActiveEditor() == nullptr)
        return;

    for (auto* knob : knobs)
    {
        if (!knob)
            continue;

        if (knob->isLearning())
        {
            juce::MessageManager::callAsync([knob, ccNumber]() {
                knob->assignMidiCC(ccNumber);
                });
            return;
        }

        if (knob->getAssignedMidiCC() == ccNumber)
        {
            juce::MessageManager::callAsync([knob, ccValue]() {
                knob->setSliderValue(ccValue);
                });
        }
    }
}

void DigitalSynthesizerAudioProcessor::registerLinkableTarget(Linkable* target)
{
    linkableTargets[target->getLinkableName().toStdString()] = target;
}

const std::unordered_map<std::string, Linkable*>& DigitalSynthesizerAudioProcessor::getLinkableTargets() const
{
    return linkableTargets;
}

bool DigitalSynthesizerAudioProcessor::registerEnvelopeLinkOwnership(Linkable* target, EnvelopeComponent* newOwner)
{
    auto it = envelopeOwners.find(target);
    if (it != envelopeOwners.end() && it->second != newOwner)
    {
        it->second->unlinkTarget(target);
    }

    envelopeOwners[target] = newOwner;
    return true;
}

bool DigitalSynthesizerAudioProcessor::registerFilterLinkOwnership(Linkable* target, FilterComponent* newOwner)
{
    auto it = filterOwners.find(target);
    if (it != filterOwners.end() && it->second != newOwner)
    {
        it->second->unlinkTarget(target);
    }

    filterOwners[target] = newOwner;
    return true;
}

void DigitalSynthesizerAudioProcessor::unregisterEnvelopeLink(Linkable* target, EnvelopeComponent* owner)
{
    auto it = envelopeOwners.find(target);
    if (it != envelopeOwners.end() && it->second == owner)
        envelopeOwners.erase(it);
}

void DigitalSynthesizerAudioProcessor::unregisterFilterLink(Linkable* target, FilterComponent* owner)
{
    auto it = filterOwners.find(target);
    if (it != filterOwners.end() && it->second == owner)
        filterOwners.erase(it);
}

void DigitalSynthesizerAudioProcessor::clearLinkOwnerships()
{
    envelopeOwners.clear();
    filterOwners.clear();
}

std::vector<std::pair<ModulationSourceID, juce::String>> DigitalSynthesizerAudioProcessor::getAvailableModulationSources(ModulationSourceType type) const
{
    std::vector<std::pair<ModulationSourceID, juce::String>> sources;

    switch (type)
    {
    case ModulationSourceType::Envelope:
        for (int i = 0; i < NUM_OF_ENVELOPES; ++i)
        {
            ModulationSourceID id{ ModulationSourceType::Envelope, i };
            juce::String label = envelopes[i]->getName();
            sources.emplace_back(id, label);
        }
        break;

    case ModulationSourceType::LFO:
        for (int i = 0; i < NUM_OF_LFOS; ++i)
        {
            ModulationSourceID id{ ModulationSourceType::LFO, i };
            juce::String label = lfos[i]->getName();
            sources.emplace_back(id, label);
        }
        break;
    }

    return sources;
}

ModulationRouter& DigitalSynthesizerAudioProcessor::getModulationRouter()
{
    return modulationRouter;
}

void DigitalSynthesizerAudioProcessor::restoreModulationRouting()
{
    for (auto* knob : knobs)
    {
        const auto ids = KnobModulationEngine::getParameterIDsFor(knob->getParamID());

        auto* modeParam = apvts.getRawParameterValue(ids[0]);
        auto* indexParam = apvts.getRawParameterValue(ids[1]);

        if (modeParam && indexParam)
        {
            auto mode = static_cast<ModulationMode>(static_cast<int>(modeParam->load()));
            auto index = static_cast<int>(indexParam->load());

            switch (mode)
            {
            case ModulationMode::Envelope:
                modulationRouter.connect({ ModulationSourceType::Envelope, index }, knob);
                break;

            case ModulationMode::LFO:
                modulationRouter.connect({ ModulationSourceType::LFO, index }, knob);
                break;

            default:
                break;
            }
        }
    }
}

void DigitalSynthesizerAudioProcessor::clearAllKnobs()
{
    for (auto* knob : knobs)
    {
        if (knob)
            modulationRouter.disconnect(knob);
    }
    knobs.clear();
}
void DigitalSynthesizerAudioProcessor::handleNoteOnLfos()
{
    for (auto& lfo : lfos)
    {
        lfo->noteOn();
        lfo->setModulationActive(true);
    }
}

void DigitalSynthesizerAudioProcessor::resetAllLfos()
{
    for (auto& lfo : lfos)
        lfo->resetTrigger();
}

void DigitalSynthesizerAudioProcessor::renderAllLFOs(int blockSize)
{
    for (int i = 0; i < static_cast<int>(lfos.size()); ++i)
    {
        auto& lfo = lfos[i];

        lfo->updateFromAPVTS(apvts);

        if (lfo->isBypassed())
        {
            modulationRouter.disconnectAllTargetsUsing({ ModulationSourceType::LFO, i });
            continue;
        }

        if (!lfo->isActive())
            continue;

        lfo->advance(blockSize, static_cast<float>(getSampleRate()));
        float val = lfo->getNextValue();
        modulationRouter.pushModulationValue({ ModulationSourceType::LFO, i }, val);
    }
}

void DigitalSynthesizerAudioProcessor::setMasterVolume(float newVolume)
{
    masterVolume.setTargetValue(juce::jlimit(0.0f, 1.0f, newVolume));
}

float DigitalSynthesizerAudioProcessor::getMasterVolumeL() const noexcept
{
    return masterVolumeLDb;
}

float DigitalSynthesizerAudioProcessor::getMasterVolumeR() const noexcept
{
    return masterVolumeRDb;
}

void DigitalSynthesizerAudioProcessor::updateOutputPeakLevels(const juce::AudioBuffer<float>& buffer, int startSample, int numSamples)
{
    float peakL = 0.0f;
    float peakR = 0.0f;

    for (int i = startSample; i < startSample + numSamples; ++i)
    {
        peakL = std::max(peakL, std::abs(buffer.getSample(0, i)));
        if (buffer.getNumChannels() > 1)
            peakR = std::max(peakR, std::abs(buffer.getSample(1, i)));
    }

    constexpr float minLevel = 1e-5f; // avoid log(0)
    masterVolumeLDb = 20.0f * std::log10(std::max(peakL, minLevel));
    masterVolumeRDb = 20.0f * std::log10(std::max(peakR, minLevel));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DigitalSynthesizerAudioProcessor();
}

juce::AudioProcessorValueTreeState& DigitalSynthesizerAudioProcessor::getAPVTS()
{
    return apvts;
}

juce::AudioProcessorValueTreeState::ParameterLayout DigitalSynthesizerAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // === Oscillators ===
    for (int i = 0; i < NUM_OF_OSCILLATORS; ++i)
    {
        Oscillator::addParameters(i, layout);
        OscillatorComponent::registerModulationParameters(i, layout);
    }

    // === Envelopes ===
    for (int i = 0; i < NUM_OF_ENVELOPES; ++i)
    {
        Envelope::addParameters(i, layout);
        EnvelopeComponent::registerModulationParameters(i, layout);
    }

    // === Filters ===
    for (int i = 0; i < NUM_OF_FILTERS; ++i)
    {
        Filter::addParameters(i, layout);
        FilterComponent::registerModulationParameters(i, layout);
    }

    // === LFOs ===
    for (int i = 0; i < NUM_OF_LFOS; ++i)
    {
        LFO::addParameters(i, layout);
        LFOComponent::registerModulationParameters(i, layout);
    }

    return layout;
}

void DigitalSynthesizerAudioProcessor::initializeModulationTargets()
{
    // Loop every base parameter ID that our proxies should manage
    for (auto& baseID : ModulationTarget::getAllBaseParameterIDs())
    {
        // Create the proxy and cache it
        auto proxy = std::make_unique<ModulationTarget>(apvts, modulationRouter, baseID);

        // Fetch the companion modulation parameters for this baseID
        auto ids = KnobModulationEngine::getParameterIDsFor(baseID);
        auto* modeParam = apvts.getRawParameterValue(ids[0]);
        auto* indexParam = apvts.getRawParameterValue(ids[1]);
        auto* minParam = apvts.getRawParameterValue(ids[2]);
        auto* maxParam = apvts.getRawParameterValue(ids[3]);

        if (modeParam && indexParam && minParam && maxParam)
        {
            // Read current modulation settings
            int modeInt = static_cast<int>(modeParam->load());
            int index = static_cast<int>(indexParam->load());
            float minNorm = minParam->load();
            float maxNorm = maxParam->load();

            // Configure the proxy’s mode & range
            proxy->setModulationMode(static_cast<ModulationMode>(modeInt));
            proxy->setModulationRange(minNorm, maxNorm);

            // If the proxy is in Envelope or LFO mode, connect it now
            if (static_cast<ModulationMode>(modeInt) == ModulationMode::Envelope)
            {
                modulationRouter.connect({ ModulationSourceType::Envelope, index },
                    proxy.get());
            }
            else if (static_cast<ModulationMode>(modeInt) == ModulationMode::LFO)
            {
                modulationRouter.connect({ ModulationSourceType::LFO, index },
                    proxy.get());
            }
        }

        // Finally, take ownership of the proxy
        modulationTargets.push_back(std::move(proxy));
    }
}
