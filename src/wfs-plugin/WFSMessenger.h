//
// Created by tar on 13/11/22.
//

#pragma once

#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_osc/juce_osc.h>

class WFSMessenger : public juce::OSCSender, public juce::ValueTree::Listener {
public:
    explicit WFSMessenger(std::shared_ptr<juce::ValueTree> tree);

    ~WFSMessenger() override;

    void connect();

    void valueTreePropertyChanged(
            juce::ValueTree &treeWhosePropertyHasChanged,
            const juce::Identifier &property) override;

private:
    std::unique_ptr<juce::DatagramSocket> socket;
    std::shared_ptr<juce::ValueTree> valueTree;
};
