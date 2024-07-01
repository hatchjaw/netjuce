//
// Created by tar on 13/11/22.
//

#include "WFSMessenger.h"

WFSMessenger::WFSMessenger() :
        socket(std::make_unique<juce::DatagramSocket>()) {}

WFSMessenger::~WFSMessenger() = default;

void WFSMessenger::connect() {
    // Prepare to send OSC messages over UDP multicast.
    // Got to bind to the local address of the appropriate network interface.
    // (This will only work if ethernet is attached with the below IPv4 assigned.)
    // TODO: make these specifiable via the UI
    socket->bindToPort(DEFAULT_LOCAL_PORT, DEFAULT_LOCAL_ADDRESS); // Might need to use a different local port
    // TODO: also make multicast IP and port specifiable via the UI.
    connectToSocket(*socket, DEFAULT_MULTICAST_IP, DEFAULT_OSC_SEND_PORT);
}

void WFSMessenger::parameterChanged(const juce::String &parameterID, float newValue) {
//    DBG("In apvts::Listener parameterChanged");

    juce::OSCBundle bundle;
    DBG("Sending OSC: " << parameterID << " " << newValue);
    bundle.addElement(juce::OSCMessage{parameterID, newValue});
    send(bundle);
}

void WFSMessenger::valueTreePropertyChanged(juce::ValueTree &tree,
                                            const juce::Identifier &property) {
    juce::OSCBundle bundle;
    const auto &propString{property.toString()};
    if (propString.contains("module")) {
        auto val{tree.getProperty(property).toString()};
        if (val.isNotEmpty()) {
            DBG("Sending OSC: " << propString << " " << val);
            bundle.addElement(juce::OSCMessage{propString, val});
            send(bundle);
        }
    }
}
