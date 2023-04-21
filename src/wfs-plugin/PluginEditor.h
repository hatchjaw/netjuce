#pragma once

#include "PluginProcessor.h"
#include "../../lib/succulent/ui/XYController/XYController.h"
#include "../../lib/succulent/ui/XYController/XYControllerAttachment.h"
#include "Feels.h"
#include "ui/Sidebar.h"

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
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AudioPluginAudioProcessor &processorRef;

    std::unique_ptr<juce::Feels> feels;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)

    XYController xyController;
    std::vector<std::unique_ptr<XYControllerNodeAttachment>> xyNodeAttachments;

    juce::OwnedArray<juce::ComboBox> moduleSelectors;

    juce::AudioProcessorValueTreeState &valueTreeState;
    juce::ValueTree &dynamicTree;

    Sidebar sidebar;
};
