//
// Created by tar on 13/11/22.
//

#include "WFSMessenger.h"

WFSMessenger::WFSMessenger(std::shared_ptr<juce::ValueTree> tree) :
        socket(std::make_unique<juce::DatagramSocket>()),
        valueTree(tree) {
    valueTree->addListener(this);
}

WFSMessenger::~WFSMessenger() {
    valueTree->removeListener(this);
}

void WFSMessenger::connect() {
    // Prepare to send OSC messages over UDP multicast.
    // Got to bind to the local address of the appropriate network interface.
    // (This will only work if ethernet is attached with the below IPv4 assigned.)
    // TODO: make these specifiable via the UI
    socket->bindToPort(8888, DEFAULT_LOCAL_ADDRESS);
    // TODO: also make multicast IP and port specifiable via the UI.
    connectToSocket(*socket, DEFAULT_MULTICAST_IP, 41815);
}

void WFSMessenger::valueTreePropertyChanged(juce::ValueTree &treeWhosePropertyHasChanged,
                                            const juce::Identifier &property) {
    ignoreUnused(treeWhosePropertyHasChanged);

    juce::OSCBundle bundle;

    if (property.toString().contains("module")) {
        DBG("Sending OSC: " << property.toString() << " " << valueTree->getProperty(property).toString());
        bundle.addElement(juce::OSCMessage{property.toString(), valueTree->getProperty(property).toString()});
    } else {
        DBG("Sending OSC: " << property.toString() << " " << static_cast<float>(valueTree->getProperty(property)));
        bundle.addElement(juce::OSCMessage{property.toString(), static_cast<float>(valueTree->getProperty(property))});
    }

    send(bundle);
}


