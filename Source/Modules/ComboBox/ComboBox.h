#pragma once

#include "../../Common.h"
#include <JuceHeader.h>

/**
 * @class ComboBox
 * @brief A ComboBox supporting both image-based and text-based items.
 */
class ComboBox : public juce::ComboBox, public juce::LookAndFeel_V4
{
public:
    /**
     * @enum Mode
     * @brief Specifies the rendering mode of the ComboBox.
     */
    enum class Mode
    {
        Text,  ///< Render using text-based items.
        Image  ///< Render using image-based items.
    };

    /**
     * @brief Constructor.
     * @param initialMode The mode to use (text or image).
     */
    ComboBox(Mode initialMode = Mode::Text);

    /** 
     * @brief Destructor.
     */
    ~ComboBox() override;

    /**
     * @brief Sets the list of images to be used in the ComboBox.
     * @param newImages Vector of Drawable image pointers.
     */
    void setImageList(const std::vector<std::unique_ptr<juce::Drawable>>& newImages);

    /**
     * @brief Adds a text item to the ComboBox.
     * @param id The item ID.
     * @param text The display text.
     */
    void addTextItem(int id, const juce::String& text);

    /**
     * @brief Draws an individual popup menu item.
     * @param g Graphics context for drawing.
     * @param area The area within which to draw the item.
     * @param isHighlighted Whether the item is highlighted.
     * @param text The display text (only used in text mode).
     * @param image The associated image (only used in image mode).
     */
    void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
        bool isSeparator, bool isActive, bool isHighlighted,
        bool isTicked, bool hasSubMenu,
        const juce::String& text, const juce::String& shortcutKeyText,
        const juce::Drawable* image, const juce::Colour* textColour) override;

    /**
     * @brief Draws the ComboBox with the selected item and dropdown arrow.
     * @param g Graphics context for drawing.
     * @param width Width of the ComboBox.
     * @param height Height of the ComboBox.
     * @param isButtonDown True if the dropdown arrow is pressed.
     * @param buttonX X position of the dropdown arrow.
     * @param buttonY Y position of the dropdown arrow.
     * @param buttonW Width of the dropdown arrow.
     * @param buttonH Height of the dropdown arrow.
     * @param box Reference to the ComboBox being drawn.
     */
    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
        int buttonX, int buttonY, int buttonW, int buttonH,
        juce::ComboBox& box) override;

    /**
     * @brief Updates the ComboBox's appearance to match the active theme.
     *
     * This function re-applies the text color used for displaying the selected
     * item in text mode, based on the current theme's ComboBoxData color.
     * It should be called whenever the theme changes to ensure the selected text
     * color reflects the updated UI::Colors.
     */
    void ComboBox::updateTheme();

    /** @brief Shared constants used for drawing and layout. */
    static constexpr const char* spacePrefix = " ";      ///< Prefix used for aligning text-based items.
    static constexpr int imageWidth = 128;               ///< Standard width for image rendering.
    static constexpr int imageHeight = 64;               ///< Standard height for image rendering.
    static constexpr float popupImageScaleFactor = 2.0f; ///< Scale factor for popup image rendering.

private:
    Mode mode; ///< Rendering mode of the ComboBox.

    std::vector<const juce::Drawable*> imageRefs; ///< Vector storing references to images used in image mode.

    /**
     * @brief Renders a tinted version of a white Drawable into the given area.
     * @param g Target graphics context.
     * @param drawable Drawable to render (assumed white on transparent).
     * @param bounds Area in which to draw.
     * @param tintColor The theme color to tint with.
     */
    void drawTintedDrawable(juce::Graphics& g, const juce::Drawable* drawable,
        const juce::Rectangle<int>& bounds, juce::Colour tintColor);

    /** 
     * @brief Returns the font used to render the selected item text.
     */
    juce::Font getComboBoxFont(juce::ComboBox&) override;
};
