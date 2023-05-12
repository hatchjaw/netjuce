//
// Created by tar on 21/04/23.
//

#include "SettingsComponent.h"

SettingsComponent::SettingsComponent(juce::AudioProcessorValueTreeState &apvts) :
        feels(std::make_unique<Feels>()),
        valueTreeState{apvts} {
    setLookAndFeel(feels.get());

    addAndMakeVisible(multicastIPLabel);
    multicastIPLabel.setText("Multicast IP: " + juce::String{DEFAULT_MULTICAST_IP},
                             juce::dontSendNotification);

    addAndMakeVisible(audioPortLabel);
    audioPortLabel.setText("Audio port: " + juce::String{DEFAULT_AUDIO_SEND_PORT},
                           juce::dontSendNotification);

    addAndMakeVisible(oscPortLabel);
    oscPortLabel.setText("OSC port: " + juce::String{DEFAULT_OSC_SEND_PORT},
                         juce::dontSendNotification);

    addAndMakeVisible(listOfClients);
    listOfClients.setJustificationType(juce::Justification::centredTop);
    listOfClients.setText("No peers connected", juce::dontSendNotification);

    //==========================================================================
    addAndMakeVisible(speakerSpacingSlider);
    speakerSpacingSlider.setSliderStyle(juce::Slider::SliderStyle::IncDecButtons);
    speakerSpacingSlider.setNormalisableRange({.05, .4, .001});
    speakerSpacingSlider.setValue(.2f);
    speakerSpacingSlider.setTextBoxStyle(juce::Slider::TextBoxLeft,
                                         false,
                                         speakerSpacingSlider.getTextBoxWidth(),
                                         25);
    speakerSpacingSlider.setIncDecButtonsMode(juce::Slider::incDecButtonsDraggable_Vertical);

    addAndMakeVisible(speakerSpacingLabel);
    speakerSpacingLabel.attachToComponent(&speakerSpacingSlider, false);
    speakerSpacingLabel.setText("Speaker spacing (m)", juce::dontSendNotification);
    speakerSpacingLabel.setJustificationType(juce::Justification::centredTop);

    speakerSpacingAttachment = std::make_unique<SliderAttachment>(
            valueTreeState,
            njwfs::Utils::speakerSpacingParamID,
            speakerSpacingSlider
    );
}

void SettingsComponent::paint(juce::Graphics &g) {
    // Draw a nice line down the middle.
    g.setColour(juce::Colours::grey);
    g.fillRect(getWidth() / 2, 5, 1, getBottom() - 5);
}

void SettingsComponent::resized() {
    auto padding{5};
    auto bounds{getBounds()};

    //==========================================================================

    auto leftCol{bounds.removeFromLeft(getWidth() / 2)};
    leftCol.removeFromLeft(padding);
    leftCol.removeFromRight(padding);

    leftCol.removeFromTop(25);
    multicastIPLabel.setBounds(leftCol.removeFromTop(25));
    leftCol.removeFromTop(25);
    audioPortLabel.setBounds(leftCol.removeFromTop(25));
    leftCol.removeFromTop(25);
    oscPortLabel.setBounds(leftCol.removeFromTop(25));
    leftCol.removeFromTop(25);
    listOfClients.setBounds(leftCol);

    //==========================================================================

    auto rightCol{bounds.removeFromRight(getWidth() / 2)};

    rightCol.removeFromTop(200);
    rightCol.removeFromLeft(50);
    rightCol.removeFromRight(50);

    speakerSpacingSlider.setBounds(rightCol.removeFromTop(25));
}

SettingsComponent::~SettingsComponent() {
    setLookAndFeel(nullptr);
}

void SettingsComponent::valueTreePropertyChanged(juce::ValueTree &treeWhosePropertyHasChanged,
                                                 const juce::Identifier &property) {
    if (property.toString() == njwfs::Utils::connectedPeersParamID.toString()) {
        juce::String c;

        const auto& peers{treeWhosePropertyHasChanged.getProperty(property)};

        if (!peers.isArray() || peers.getArray()->isEmpty()) {
            c << "No peers connected.";
        } else {
            c << "Connected Peers:" << juce::newLine << juce::newLine;

            for (const auto& p: *peers.getArray()) {
                c << juce::String{p} << juce::newLine;
            }
        }

        listOfClients.setText(c, juce::dontSendNotification);
    }
}
