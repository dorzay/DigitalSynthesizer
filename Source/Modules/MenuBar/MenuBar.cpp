#include "MenuBar.h"

MenuBar::MenuBar(DigitalSynthesizerAudioProcessor& processorRef)
    : processor(processorRef)
{
    tabs.push_back(createProjectTab());
    tabs.push_back(createThemeTab());
    tabs.push_back(createPresetsTab());

    setLookAndFeel(&themedLookAndFeel);
    setModel(this);
}

MenuBar::~MenuBar()
{
    setModel(nullptr);
    setLookAndFeel(nullptr);
}

juce::StringArray MenuBar::getMenuBarNames()
{
    juce::StringArray names;
    for (const auto& tab : tabs)
        names.add(tab.name);
    return names;
}

juce::PopupMenu MenuBar::getMenuForIndex(int menuIndex, const juce::String& /*menuName*/)
{
    if (menuIndex >= 0 && menuIndex < static_cast<int>(tabs.size()))
        return tabs[menuIndex].createMenu();

    return {};
}

void MenuBar::menuItemSelected(int menuItemID, int menuIndex)
{
    if (menuIndex >= 0 && menuIndex < static_cast<int>(tabs.size()))
        tabs[menuIndex].handleSelection(menuItemID);
}

void MenuBar::setOnThemeChanged(std::function<void()> callback)
{
    onThemeChanged = std::move(callback);
}

int MenuBar::getHeight()
{
    return height;
}

void MenuBar::updateTheme()
{
    themedLookAndFeel.setColour(juce::PopupMenu::backgroundColourId, UI::Colors::MenuBarBackground);
    themedLookAndFeel.setColour(juce::PopupMenu::textColourId, UI::Colors::MenuBarText);
    themedLookAndFeel.setColour(juce::PopupMenu::headerTextColourId, UI::Colors::MenuBarHeaderText);
    themedLookAndFeel.setColour(juce::PopupMenu::highlightedTextColourId, UI::Colors::MenuBarHighlightedText);
    themedLookAndFeel.setColour(juce::PopupMenu::highlightedBackgroundColourId, UI::Colors::MenuBarHighlightedBackground);

    repaint();

    for (auto* child : getChildren())
        child->repaint();
}

MenuBar::Tab MenuBar::createProjectTab()
{
    constexpr int AboutItem = 1;

    return {
        "Digital Synthesizer",
        [AboutItem] {
            juce::PopupMenu menu;
            menu.addItem(AboutItem, "About");
            return menu;
        },
        [AboutItem](int menuItemID) {
            if (menuItemID == AboutItem)
            {
                juce::URL(projectUrl).launchInDefaultBrowser();
            }
        }
    };
}

MenuBar::Tab MenuBar::createThemeTab()
{
    return {
        "Theme",
        [] {
            juce::PopupMenu menu;
            for (const auto& [id, name] : UI::Colors::getAvailableThemeNames())
                menu.addItem(id, name);
            return menu;
        },
        [this](int menuItemID) {
            UI::Colors::applyThemeByID(menuItemID);
            updateTheme();
            if (onThemeChanged)
                onThemeChanged();

            auto& state = processor.getAPVTS().state;
            state.setProperty("themeID", menuItemID, nullptr);
        }
    };
}

MenuBar::Tab MenuBar::createPresetsTab()
{
    return {
        "Presets",
        [] {
            juce::PopupMenu menu;
            menu.addItem(PresetInit, "Init");
            menu.addSeparator();
            menu.addItem(PresetLoad, "Load");
            menu.addItem(PresetSave, "Save");
            return menu;
        },
        [this](int menuItemID) {
            switch (menuItemID)
            {
                case PresetInit:
                    processor.getPresetManager()->initPreset();
                    break;

                case PresetLoad:
                    processor.getPresetManager()->showLoadDialogBox();
                    break;

                case PresetSave:
                    processor.getPresetManager()->showSaveDialogBox();
                    break;
            }
        }
    };
}