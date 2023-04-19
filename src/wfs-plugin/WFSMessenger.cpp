//
// Created by tar on 13/11/22.
//

#include "WFSMessenger.h"

WFSMessenger::WFSMessenger(juce::AudioProcessorValueTreeState &state) :
        socket(std::make_unique<juce::DatagramSocket>()),
        apvts(state) {}

WFSMessenger::~WFSMessenger() = default;

void WFSMessenger::connect() {
    // Prepare to send OSC messages over UDP multicast.
    // Got to bind to the local address of the appropriate network interface.
    // (This will only work if ethernet is attached with the below IPv4 assigned.)
    // TODO: make these specifiable via the UI
    socket->bindToPort(8888, DEFAULT_LOCAL_ADDRESS);
    // TODO: also make multicast IP and port specifiable via the UI.
    connectToSocket(*socket, DEFAULT_OSC_MULTICAST_IP, 41815);
}

void WFSMessenger::parameterChanged(const juce::String &parameterID, float newValue) {
//    DBG("In apvts::Listener parameterChanged");

    juce::OSCBundle bundle;

    if (parameterID.contains("module")) {
        DBG("Sending OSC: " << parameterID << " " << apvts.state.getProperty(parameterID).toString());
        bundle.addElement(juce::OSCMessage{parameterID, apvts.state.getProperty(parameterID).toString()});
    } else {
        DBG("Sending OSC: " << parameterID << " " << newValue);
        bundle.addElement(juce::OSCMessage{parameterID, newValue});
    }

    send(bundle);
}


