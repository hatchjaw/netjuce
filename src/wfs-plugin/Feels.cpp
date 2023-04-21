//
// Created by tar on 21/04/23.
//

#include "Feels.h"

juce::Feels::Feels() {
    auto bg{juce::Colours::ghostwhite};
    auto fg{bg.contrasting(.75f)};
    setColour(juce::ResizableWindow::backgroundColourId, bg);

    setColour(juce::ComboBox::textColourId, fg);
    setColour(juce::ComboBox::arrowColourId, fg);
    setColour(juce::ComboBox::backgroundColourId, juce::Colours::whitesmoke);

    setColour(juce::Label::textColourId, fg);
}

void
juce::Feels::drawComboBox(Graphics &g, int width, int height, bool isButtonDown, int buttonX, int buttonY,
                          int buttonW, int buttonH, ComboBox &box) {
    Rectangle<int> boxBounds(0, 0, width, height);

    g.setColour(box.findColour(ComboBox::backgroundColourId));
    g.fillRect(boxBounds.toFloat());

    g.setColour(box.findColour(ComboBox::outlineColourId));
    g.drawRect(boxBounds.toFloat().reduced(0.5f, 0.5f), 1.0f);

    Rectangle<int> arrowZone(width - kArrowWidth, 0, 15, height);
    Path path;
    path.startNewSubPath((float) arrowZone.getX() + 3.0f, (float) arrowZone.getCentreY() - 2.0f);
    path.lineTo((float) arrowZone.getCentreX(), (float) arrowZone.getCentreY() + 3.0f);
    path.lineTo((float) arrowZone.getRight() - 3.0f, (float) arrowZone.getCentreY() - 2.0f);

    g.setColour(box.findColour(ComboBox::arrowColourId).withAlpha((box.isEnabled() ? 0.9f : 0.2f)));
    g.strokePath(path, PathStrokeType(2.0f));
}

void juce::Feels::positionComboBoxText(juce::ComboBox &box, juce::Label &label) {
    label.setBounds(1, 1,
                    box.getWidth() - kArrowWidth,
                    box.getHeight() - 2);

    label.setFont(getComboBoxFont(box));
}
