#pragma once

#include "PluginProcessor.h"
#include "../../lib/succulent/ui/XYController/XYController.h"
#include "../../lib/succulent/ui/XYController/XYControllerAttachment.h"
#include "Feels.h"
#include "ui/SettingsComponent.h"
#include "Utils.h"

//==============================================================================
class AudioPluginAudioProcessorEditor : public juce::AudioProcessorEditor {
public:
    AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor &,
                                    juce::AudioProcessorValueTreeState &,
                                    juce::ValueTree &vt);

    ~AudioPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics &) override;

    void resized() override;

    void updateModuleLists(const juce::StringArray& peerIPs);

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)

    void showSettings();

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AudioPluginAudioProcessor &processorRef;

    juce::AudioProcessorValueTreeState &valueTreeState;
    juce::ValueTree &dynamicTree;

    std::unique_ptr<Feels> feels;

    XYController xyController;
    std::vector<std::unique_ptr<XYControllerNodeAttachment>> xyNodeAttachments;

    juce::OwnedArray<juce::ComboBox> moduleSelectors;

    // NB. order of declaration is important; declaring the attachment before
    // the slider means the attachment will be created first and deleted last,
    // by which time the slider, and its list of listeners, which should
    // contain the attachment, will already have been deleted.
    // https://stackoverflow.com/questions/2254263/order-of-member-constructor-and-destructor-calls
    juce::Slider gainSlider;
    std::unique_ptr<SliderAttachment> gainAttachment;
    juce::Label gainLabel;

    juce::TextButton settingsButton;

    SafePointer<juce::DialogWindow> settingsWindow;
    std::unique_ptr<SettingsComponent> settingsComponent;
};
