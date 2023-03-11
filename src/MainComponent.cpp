//
// Created by Tommy Rushton on 16/02/2023.
//

#include "MainComponent.h"

MainComponent::MainComponent(const StringArray &audioFilesToPlay)
        : multiChannelSource(std::make_unique<MultiChannelAudioSource>(NUM_SOURCES)),
          netServer(std::make_unique<NetAudioServer>()),
          audioFiles(audioFilesToPlay) {
    setAudioChannels(0, NUM_SOURCES);

    juce::String typeNameToSet{""}, outputDeviceNameToSet{""};
    auto setup{deviceManager.getAudioDeviceSetup()};
    setup.bufferSize = AUDIO_BLOCK_SAMPLES;

    auto &deviceTypes{deviceManager.getAvailableDeviceTypes()};
    for (auto type: deviceTypes) {
        auto typeName{type->getTypeName()};
#if JUCE_JACK
        if (typeName == "JACK") {
            typeNameToSet = typeName;
            outputDeviceNameToSet = "system";
            break;
        }
#else
        if (typeName == "ALSA") {
            typeNameToSet = typeName;
            type->scanForDevices();
            auto devices{type->getDeviceNames()};
            for (auto &device: devices) {
                DBG(device);
                if (device == "JACK Audio Connection Kit") {
//                if (device == "HD-Audio Generic, ALC294 Analog; Front output / input") {
                    outputDeviceNameToSet = device;
                }
            }
        }
#endif
    }

    ready = true;

    if (typeNameToSet != ""){
        deviceManager.setCurrentAudioDeviceType(typeNameToSet, true);
    }

    if (outputDeviceNameToSet != "") {
        setup.outputDeviceName = outputDeviceNameToSet;
    }

    deviceManager.setAudioDeviceSetup(setup, true);
}

MainComponent::~MainComponent() {
    shutdownAudio();
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate) {
    if (!ready) return;

    multiChannelSource->prepareToPlay(samplesPerBlockExpected, sampleRate);

    for (auto i{0}; i < audioFiles.size(); ++i) {
        auto file{File(audioFiles[i])};
        if (file.existsAsFile()) {
            multiChannelSource->addSource(static_cast<uint>(i), file);
            multiChannelSource->start();
        }
    }

    netServer->prepareToSend(samplesPerBlockExpected, sampleRate);
}

void MainComponent::releaseResources() {
    multiChannelSource->releaseResources();
    netServer->releaseResources();
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) {
    if (!ready) return;

    multiChannelSource->getNextAudioBlock(bufferToFill);
    if (!netServer->handleAudioBlock(bufferToFill)) {
//        DBG("Failed to handleAudioBlock UDP packet.");
//        netServer->disconnect();
    }
}
