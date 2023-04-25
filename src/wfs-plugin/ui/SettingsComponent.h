//
// Created by tar on 21/04/23.
//

#ifndef NETJUCE_SETTINGSCOMPONENT_H
#define NETJUCE_SETTINGSCOMPONENT_H

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "../Feels.h"
#include "../Utils.h"

class SettingsComponent : public juce::Component,
                          public juce::ValueTree::Listener {
public:
    explicit SettingsComponent(juce::AudioProcessorValueTreeState &);

    ~SettingsComponent() override;

    void paint(juce::Graphics &) override;

    void resized() override;

    void
    valueTreePropertyChanged(juce::ValueTree &treeWhosePropertyHasChanged, const juce::Identifier &property) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsComponent)

    std::unique_ptr<Feels> feels;

    juce::AudioProcessorValueTreeState &valueTreeState;

    // Network settings
    /**
     * Multicast IP
     */
    juce::Label multicastIPLabel;

    /**
     * Audio port
     */
    juce::Label audioPortLabel;

    /**
     * OSC port
     */
    juce::Label oscPortLabel;

    /**
     * List of connected clients, for display only.
     */
    juce::Label listOfClients;

    // DSP settings
    /**
     * Speaker spacing
     */
    juce::Slider speakerSpacingSlider;
    juce::Label speakerSpacingLabel;
    std::unique_ptr<SliderAttachment> speakerSpacingAttachment;
};


#endif //NETJUCE_SETTINGSCOMPONENT_H
