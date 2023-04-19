#pragma once

#include "PluginProcessor.h"
#include "../../lib/succulent/ui/XYController/XYController.h"
#include "../../lib/succulent/ui/XYController/XYControllerAttachment.h"

//==============================================================================
class AudioPluginAudioProcessorEditor : public juce::AudioProcessorEditor {
public:
    AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor &,
//                                    std::shared_ptr<juce::ValueTree>,
                                    juce::AudioProcessorValueTreeState &);

    ~AudioPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics &) override;

    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AudioPluginAudioProcessor &processorRef;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)

    XYController xyController;
    std::vector<std::unique_ptr<XYControllerNodeAttachment>> xyNodeAttachments;

//    std::shared_ptr<juce::ValueTree> valueTree;

    juce::AudioProcessorValueTreeState &valueTreeState;
};
