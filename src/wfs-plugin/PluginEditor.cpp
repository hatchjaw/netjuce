#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Utils.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor &p,
                                                                 juce::AudioProcessorValueTreeState &vts,
                                                                 juce::ValueTree &vt)
        : AudioProcessorEditor(&p),
          processorRef(p),
          feels(std::make_unique<juce::Feels>()),
          xyController(NUM_SOURCES),
          valueTreeState(vts),
          dynamicTree(vt) {

    setLookAndFeel(feels.get());

    addAndMakeVisible(sidebar);

    xyController.onAddNode = [this](XYController::Node &n) {
        auto idx{n.getIndex()};
        xyNodeAttachments.push_back(std::make_unique<XYControllerNodeAttachment>(
                valueTreeState,
                njwfs::Utils::getSourcePositionParamID(idx, njwfs::SourcePositionAxis::X),
                njwfs::Utils::getSourcePositionParamID(idx, njwfs::SourcePositionAxis::Y),
                n
        ));
    };
    addAndMakeVisible(xyController);
    for (int i{0}; i < NUM_SOURCES; ++i) {
        xyController.addNode();
    }

    for (uint i = 0; i < NUM_MODULES; ++i) {
        auto cb{new juce::ComboBox};
        addAndMakeVisible(cb);
        cb->onChange = [this, cb, i] {
            auto ip{cb->getText()};
            dynamicTree.setProperty(njwfs::Utils::getModuleParamID(i), ip, nullptr);
        };
        moduleSelectors.add(cb);
    }

    updateModuleLists(processorRef.getConnectedPeers());

    setSize(1200, 900);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor() {
    setLookAndFeel(nullptr);
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint(juce::Graphics &g) {
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    juce::ColourGradient cg = juce::ColourGradient::vertical(
            getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId),
            getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId).darker(.2f),
            getBounds()
    );
    g.setGradientFill(cg);
    g.fillAll();
}

void AudioPluginAudioProcessorEditor::resized() {
    auto bounds{getLocalBounds()};
    auto padding{5}, sidebarW{225}, xyPadX{50}, xyPadY{100};

    sidebar.setBounds(0, 0, sidebarW, getHeight());

    xyController.setBounds(sidebar.getRight() + xyPadX,
                           xyPadY,
                           bounds.getWidth() - 2 * xyPadX - sidebar.getWidth(),
                           bounds.getHeight() - 2 * xyPadY);

    auto moduleSelectorWidth{
            static_cast<int>(
                    roundf(static_cast<float>(xyController.getWidth() + 1) / static_cast<float>(moduleSelectors.size()))
            )
    };
    for (int i{0}; i < moduleSelectors.size(); ++i) {
        moduleSelectors[i]->setBounds(
                xyController.getX() + i * moduleSelectorWidth,
                xyController.getBottom() + padding,
                moduleSelectorWidth - 1,
                30
        );
    }
}

void AudioPluginAudioProcessorEditor::updateModuleLists(const juce::StringArray &peerIPs) {
    // Update the module selector lists.
    for (int j{0}; j < moduleSelectors.size(); ++j) {
        auto prop{njwfs::Utils::getModuleParamID(static_cast<uint>(j))};
        // Store the current property value.
        auto selected{dynamicTree.getProperty(prop).toString()};
        // Refresh the list.
        moduleSelectors[j]->clear(juce::dontSendNotification);
        moduleSelectors[j]->addItemList(peerIPs, 1);
        // Try to set the stored value.
        for (int i{0}; i < peerIPs.size(); ++i) {
            if (peerIPs[i] == selected) { //text) {
                moduleSelectors[j]->setSelectedItemIndex(i);
            }
        }
    }
}
