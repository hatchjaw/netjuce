//
// Created by tar on 21/04/23.
//

#include "Sidebar.h"

Sidebar::Sidebar() {
    addAndMakeVisible(listOfClients);
}

void Sidebar::paint(juce::Graphics &g) {
    g.setColour(juce::Colours::grey);
    g.drawRect(getLocalBounds(), 1);   // draw an outline around the component

    auto x{getX()};
    auto y{getY()};
    auto w{getWidth()};
    auto h{getHeight()};

    g.drawText("Connected Modules", x, y, w, h, juce::Justification::centred);
}

void Sidebar::resized() {
    auto padding{5};
    auto w{getWidth() - 2 * padding};
    auto h{200};

    listOfClients.setBounds(padding, padding, w, h);
    listOfClients.setJustificationType(juce::Justification::centredTop);
    juce::String text;
    text << "some text" << juce::newLine << "some more text";
    listOfClients.setText(text, juce::dontSendNotification);
}

Sidebar::~Sidebar() = default;
