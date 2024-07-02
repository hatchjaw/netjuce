//
// Created by Tommy Rushton on 16/02/2023.
//

#include "MainComponent.h"

MainComponent::MainComponent(const StringArray &audioFilesToPlay)
        : multiChannelSource(std::make_unique<MultiChannelAudioSource>(NUM_SOURCES)),
          netServer(std::make_unique<NetAudioServer>()),
          audioFiles(audioFilesToPlay)
{

    setAudioChannels(0, NUM_SOURCES);

    juce::String typeNameToSet{""}, outputDeviceNameToSet{""};
    auto setup{deviceManager.getAudioDeviceSetup()};
    setup.bufferSize = AUDIO_BLOCK_SAMPLES;
//    setup.sampleRate = 44100; // Doesn't work; sample rate needs to be set at the system level.

    auto &deviceTypes{deviceManager.getAvailableDeviceTypes()};
    for (auto *type: deviceTypes) {
        auto typeName{type->getTypeName()};
#if JUCE_JACK
        if (typeName == "JACK") {
            typeNameToSet = typeName;
            outputDeviceNameToSet = "Built-in Audio Analog Stereo";
//            break;
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

    if (typeNameToSet != "") {
        deviceManager.setCurrentAudioDeviceType(typeNameToSet, true);
    }

    if (outputDeviceNameToSet != "") {
        setup.outputDeviceName = outputDeviceNameToSet;
    }

    std::cout << "Applying audio device setup: " << setup.outputDeviceName
              << ", Sample rate: " << setup.sampleRate
              << ", Buffer size: " << setup.bufferSize
              << "\n";

    auto result{deviceManager.setAudioDeviceSetup(setup, true)};

    if (result.isNotEmpty()) {
        std::cerr << "Failed to set up audio device: \"" << result << "\"\n";
    }
}

MainComponent::~MainComponent()
{
    shutdownAudio();
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    DBG("Fs: " << sampleRate << ", N: " << samplesPerBlockExpected);

    multiChannelSource->prepareToPlay(samplesPerBlockExpected, sampleRate);

    for (auto i{0}; i < audioFiles.size(); ++i) {
        auto file{File(audioFiles[i])};
        if (file.existsAsFile()) {
            multiChannelSource->addSource(static_cast<uint>(i), file);
            multiChannelSource->start();
        }
    }

    netServer->prepare(samplesPerBlockExpected, sampleRate);

    prepared = true;
}

void MainComponent::releaseResources()
{
    multiChannelSource->releaseResources();
    netServer->releaseResources();
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill)
{
    if (!prepared) return;

    multiChannelSource->getNextAudioBlock(bufferToFill);
    netServer->handleAudioBlock(bufferToFill);
}
