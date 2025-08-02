#include "PresetManager.h"
#include "../../PluginProcessor.h"

PresetManager::PresetManager(juce::AudioProcessorValueTreeState& apvts, DigitalSynthesizerAudioProcessor& processor)
    : apvts(apvts), processor(processor)
{
}

juce::File PresetManager::getDefaultPresetFolder()
{
    auto root = juce::File(__FILE__);
    for (int i = 0; i < 4; ++i)
        root = root.getParentDirectory();

    auto folder = root.getChildFile("Presets");
    folder.createDirectory();
    return folder;
}

void PresetManager::initPreset()
{
    for (auto* knob : processor.getKnobs())
    {
        if (knob != nullptr)
        {
            processor.getModulationRouter().disconnect(knob);
            knob->clearModulation();
            knob->setModulationMode(ModulationMode::Manual);
        }
    }

    apvts.replaceState(juce::ValueTree("PARAMETERS"));
}

bool PresetManager::savePreset(const juce::File& presetFile)
{
    if (auto xml = apvts.copyState().createXml())
        return xml->writeTo(presetFile);
    return false;
}

bool PresetManager::loadPreset(const juce::File& presetFile)
{
    auto xml = juce::XmlDocument::parse(presetFile);
    if (xml)
    {
        auto newTree = juce::ValueTree::fromXml(*xml);
        if (newTree.isValid())
        {
            apvts.replaceState(newTree);

            // Clear stale routing first
            processor.getModulationRouter().disconnectAll();

            // Restore routing from APVTS
            processor.restoreModulationRouting();
            return true;
        }
    }
    return false;
}

void PresetManager::showLoadDialogBox()
{
    auto presetFolder = getDefaultPresetFolder();

    auto* browser = new juce::FileBrowserComponent(
        juce::FileBrowserComponent::openMode |
        juce::FileBrowserComponent::canSelectFiles,
        presetFolder,
        nullptr,
        nullptr);

    auto* dialogBox = new juce::FileChooserDialogBox(
        "Load Preset",
        "Choose a preset file",
        *browser,
        false,
        juce::Colours::darkgrey);

    setDialogBoundsWithAspectRatio(dialogBox);

    dialogBox->enterModalState(true,
        juce::ModalCallbackFunction::create([this, browser, dialogBox](int result)
            {
                if (result != 0)
                {
                    auto file = browser->getSelectedFile(0);
                    if (file.existsAsFile())
                        loadPreset(file);
                }

                delete dialogBox;
                delete browser;
            }),
        false);
}

void PresetManager::showSaveDialogBox()
{
    auto presetFolder = getDefaultPresetFolder();

    auto* browser = new juce::FileBrowserComponent(
        juce::FileBrowserComponent::saveMode |
        juce::FileBrowserComponent::canSelectFiles |
        juce::FileBrowserComponent::warnAboutOverwriting,
        presetFolder,
        nullptr,
        nullptr);

    auto* dialogBox = new juce::FileChooserDialogBox(
        "Save Preset",
        "Choose a location to save the preset",
        *browser,
        false,
        juce::Colours::darkgrey);

    setDialogBoundsWithAspectRatio(dialogBox);

    dialogBox->enterModalState(true,
        juce::ModalCallbackFunction::create([this, browser, dialogBox](int result)
            {
                if (result != 0)
                {
                    auto file = browser->getSelectedFile(0).withFileExtension(".xml");
                    if (file.getFileName().isNotEmpty())
                        savePreset(file);
                }

                delete dialogBox;
                delete browser;
            }),
        false);
}

void PresetManager::setDialogBoundsWithAspectRatio(juce::FileChooserDialogBox* dialog)
{
    const int width = dialogBoxWidth;
    const int height = dialogBoxHeight;

    auto screenArea = juce::Desktop::getInstance().getDisplays().getMainDisplay().userArea;
    juce::Rectangle<int> dialogBounds(
        screenArea.getCentreX() - width / 2,
        screenArea.getCentreY() - height / 2,
        width,
        height);

    dialog->setBounds(dialogBounds);
}