//
// Created by tar on 21/04/23.
//

#include "Feels.h"

Feels::Feels() {
    auto bg{juce::Colours::ghostwhite};
    auto fg{bg.contrasting(.75f)};
    setColour(juce::ResizableWindow::backgroundColourId, bg);

    setColour(juce::ComboBox::textColourId, fg);
    setColour(juce::ComboBox::arrowColourId, fg);
    setColour(juce::ComboBox::backgroundColourId, juce::Colours::whitesmoke);

    setColour(juce::Label::textColourId, fg);

    setColour(juce::TextButton::textColourOnId, fg);
    setColour(juce::TextButton::textColourOffId, fg);
    setColour(juce::TextButton::buttonColourId, juce::Colours::white);
    setColour(juce::TextButton::buttonOnColourId, juce::Colours::ghostwhite);

    setColour(juce::Slider::thumbColourId, fg.withLightness(.66f));
    setColour(juce::Slider::textBoxTextColourId, fg);

    setColour(juce::Label::textColourId, fg);
}

void Feels::drawComboBox(juce::Graphics &g, int width, int height, bool isButtonDown, int buttonX, int buttonY,
                          int buttonW, int buttonH, juce::ComboBox &box) {
    juce::ignoreUnused(isButtonDown, buttonX, buttonY, buttonW, buttonH);

    juce::Rectangle<int> boxBounds(0, 0, width, height);

    g.setColour(box.findColour(juce::ComboBox::backgroundColourId));
    g.fillRect(boxBounds.toFloat());

    g.setColour(box.findColour(juce::ComboBox::outlineColourId));
    g.drawRect(boxBounds.toFloat().reduced(0.5f, 0.5f), 1.0f);

    juce::Rectangle<int> arrowZone(width - kArrowWidth, 0, 15, height);
    juce::Path path;
    path.startNewSubPath((float) arrowZone.getX() + 3.0f, (float) arrowZone.getCentreY() - 2.0f);
    path.lineTo((float) arrowZone.getCentreX(), (float) arrowZone.getCentreY() + 3.0f);
    path.lineTo((float) arrowZone.getRight() - 3.0f, (float) arrowZone.getCentreY() - 2.0f);

    g.setColour(box.findColour(juce::ComboBox::arrowColourId).withAlpha((box.isEnabled() ? 0.9f : 0.2f)));
    g.strokePath(path, juce::PathStrokeType(2.0f));
}

void Feels::positionComboBoxText(juce::ComboBox &box, juce::Label &label) {
    label.setBounds(1, 1,
                    box.getWidth() - kArrowWidth,
                    box.getHeight() - 2);

    label.setFont(getComboBoxFont(box));
}
