#include "ComboBox.h"

ComboBox::ComboBox(Mode initialMode)
    : mode(initialMode)
{
    setLookAndFeel(this);
    juce::ComboBox::setColour(juce::ComboBox::textColourId, UI::Colors::ComboBoxData);
}

ComboBox::~ComboBox()
{
    setLookAndFeel(nullptr);
}

void ComboBox::setImageList(const std::vector<std::unique_ptr<juce::Drawable>>& newImages)
{
    imageRefs.clear();
    for (const auto& img : newImages)
        imageRefs.push_back(img.get());
}

void ComboBox::addTextItem(int id, const juce::String& text)
{
    addItem(spacePrefix + text, id);
}

void ComboBox::drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
    bool /*isSeparator*/, bool /*isActive*/, bool isHighlighted,
    bool /*isTicked*/, bool /*hasSubMenu*/,
    const juce::String& text, const juce::String& /*shortcutKeyText*/,
    const juce::Drawable* image, const juce::Colour* /*textColour*/)
{
    auto backgroundColor = UI::Colors::ComboBoxBackground;
    auto highlightColor = UI::Colors::ComboBoxHighlightBackground;

    g.fillAll(isHighlighted ? highlightColor : backgroundColor);

    if (mode == Mode::Image && image != nullptr)
    {
        auto reservedWidth = area.getWidth() * 0.2f;
        auto rawImageArea = area.withTrimmedRight(static_cast<int>(reservedWidth));

        juce::Rectangle<int> imageArea(
            rawImageArea.getX(),
            rawImageArea.getY(),
            (rawImageArea.getWidth() / 2) * 2,
            (rawImageArea.getHeight() / 2) * 2
        );

        drawTintedDrawable(g, image, imageArea, UI::Colors::ComboBoxData);
    }
    else
    {
        g.setColour(UI::Colors::ComboBoxData);
        g.setFont(juce::Font(15.0f));
        g.drawText(spacePrefix + text, area, juce::Justification::centredLeft);
    }
}

void ComboBox::drawComboBox(juce::Graphics& g, int width, int height, bool /*isButtonDown*/,
    int buttonX, int buttonY, int buttonW, int buttonH,
    juce::ComboBox& box)
{
    auto backgroundColor = UI::Colors::ComboBoxBackground;
    auto outlineColor = UI::Colors::ComboBoxOutline;

    g.fillAll(backgroundColor);
    g.setColour(outlineColor);
    g.drawRect(juce::Rectangle<int>(0, 0, width, height).reduced(2), 1);

    int selectedId = box.getSelectedId() - 1;
    if (mode == Mode::Image && selectedId >= 0 && selectedId < imageRefs.size())
    {
        auto imageArea = juce::Rectangle<int>(0, 0, static_cast<int>(width * 0.8f), height).reduced(4);
        drawTintedDrawable(g, imageRefs[selectedId], imageArea, UI::Colors::ComboBoxData);
    }

    // Draw dropdown arrow
    juce::Path arrow;
    arrow.addTriangle(buttonX + buttonW / 2.0f - 4.0f, buttonY + buttonH / 2.0f - 1.0f,
        buttonX + buttonW / 2.0f + 4.0f, buttonY + buttonH / 2.0f - 1.0f,
        buttonX + buttonW / 2.0f, buttonY + buttonH / 2.0f + 3.0f);
    g.setColour(UI::Colors::ComboBoxArrow);
    g.fillPath(arrow);
}

void ComboBox::updateTheme()
{
    juce::ComboBox::setColour(juce::ComboBox::textColourId, UI::Colors::ComboBoxData);
}

void ComboBox::drawTintedDrawable(juce::Graphics& g, const juce::Drawable* drawable,
    const juce::Rectangle<int>& bounds, juce::Colour tintColor)
{
    if (drawable == nullptr)
        return;

    juce::Image tempImage(juce::Image::ARGB, bounds.getWidth(), bounds.getHeight(), true);
    juce::Graphics tempG(tempImage);

    drawable->drawWithin(tempG, tempImage.getBounds().toFloat(),
        juce::RectanglePlacement::centred, 1.0f);

    g.setColour(tintColor);
    g.drawImage(tempImage, bounds.toFloat(), juce::RectanglePlacement::stretchToFit, true);
}

juce::Font ComboBox::getComboBoxFont(juce::ComboBox&)
{
    return juce::Font(15.0f);
}
