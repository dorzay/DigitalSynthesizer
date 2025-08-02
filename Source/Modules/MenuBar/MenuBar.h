#pragma once

#include "../../Common.h"
#include "../../PluginProcessor.h"
#include <JuceHeader.h>

/**
 * @file MenuBar.h
 * @brief Declares the MenuBar class, a modular menu bar supporting multiple tabs.
 */

 /**
  * @class MenuBar
  * @brief A customizable menu bar component for the synthesizer UI.

  */
class MenuBar : public juce::MenuBarComponent, private juce::MenuBarModel
{
public:
    /**
     * @brief Constructs the MenuBar component and initializes tabs.
     * @param processorRef Reference to the audio processor, enabling preset management.
     */
    MenuBar(DigitalSynthesizerAudioProcessor& processorRef);

    /**
     * @brief Destructor for the MenuBar component.
     */
    ~MenuBar() override;

    /**
     * @brief Returns the names of the top-level menu items.
     * @return A string array of top-level menu names.
     */
    juce::StringArray getMenuBarNames() override;

    /**
     * @brief Creates the popup menu for a given top-level tab.
     * @param menuIndex Index of the top-level menu.
     * @param menuName Name of the top-level menu.
     * @return A juce::PopupMenu for the given tab.
     */
    juce::PopupMenu getMenuForIndex(int menuIndex, const juce::String& menuName) override;

    /**
     * @brief Handles selection of a menu item within a tab.
     * @param menuItemID ID of the selected item.
     * @param menuIndex Index of the top-level tab from which the item was selected.
     */
    void menuItemSelected(int menuItemID, int menuIndex) override;

    /**
     * @brief Sets the callback to be called when the theme is changed.
     * @param callback A function to invoke after a theme is applied (e.g., to trigger repaint).
     */
    void setOnThemeChanged(std::function<void()> callback);

    /**
     * @brief Returns the height of the MenuBar component.
     * @return The height in pixels (default height used in layout).
     */
    static int getHeight();

    /**
     * @brief Applies theme-based colors (text, highlight, background) to the MenuBar.
     */
    void updateTheme();

private:
    DigitalSynthesizerAudioProcessor& processor; ///< Reference to the audio processor.

    /**
     * @brief Menu IDs for preset actions.
     */
    enum PresetMenuItemIDs
    {
        PresetInit = 1,
        PresetLoad,
        PresetSave
    };

    /**
     * @struct Tab
     * @brief Represents a single top-level tab in the menu bar.
     */
    struct Tab
    {
        juce::String name;                           ///< Display name of the tab.
        std::function<juce::PopupMenu()> createMenu; ///< Callback to generate the menu for this tab.
        std::function<void(int)> handleSelection;    ///< Callback to handle selection of a menu item.
    };

    std::vector<Tab> tabs;                           ///< List of all registered menu tabs.
    std::function<void()> onThemeChanged = nullptr;  ///< Optional callback invoked after theme change.
    static constexpr int height = 24;                ///< The fixed height of the MenuBar in pixels.
    juce::LookAndFeel_V4 themedLookAndFeel;          ///< LookAndFeel object used to apply theme-specific popup styling.

    /**
     * @brief Constructs the Theme menu tab.
     * @return A fully initialized Tab object for theme switching.
     */
    Tab createThemeTab();

    /**
     * @brief Constructs the Presets menu tab.
     * @return A fully initialized Tab object for preset management.
     */
    Tab createPresetsTab();

    /**
     * @brief Constructs the project tab with static branding and an About link.
     * @return A Tab object with "Digital Synthesizer" label and an About menu.
     */
    Tab createProjectTab();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MenuBar)
};
