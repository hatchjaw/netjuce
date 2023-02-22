//
// Created by Tommy Rushton on 16/02/2023.
//
#include <juce_core/juce_core.h>

#include <memory>
#include "MainComponent.h"

/**
 * Maybe use juce::ConsoleApplication instead?
 */
class Application : public juce::JUCEApplication {
    const juce::String getApplicationName() override { return JUCE_APPLICATION_NAME_STRING; }

    const juce::String getApplicationVersion() override { return JUCE_APPLICATION_VERSION_STRING; }

    bool moreThanOneInstanceAllowed() override { return true; }

    void initialise(const juce::String &commandLine) override {
        // This method is where you should put your application's initialisation code..
        juce::ignoreUnused(commandLine);
        auto params{getCommandLineParameterArray()};
        StringArray files;
        for (int i = 0; i < params.size(); ++i) {
            if (params[i] == "-s") {
                files.add(params[i + 1]);
                ++i;
            }
        }

        mainComponent = std::make_unique<MainComponent>(files);
    }

    void shutdown() override {
    }

    //==============================================================================
    void systemRequestedQuit() override {
        quit();
    }

    void anotherInstanceStarted(const juce::String &commandLine) override {
        juce::ignoreUnused(commandLine);
    }

private:
    std::unique_ptr<MainComponent> mainComponent;
};

START_JUCE_APPLICATION (Application)