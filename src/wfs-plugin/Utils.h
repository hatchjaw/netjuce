//
// Created by tar on 20/04/23.
//

#pragma once

#include <juce_core/juce_core.h>
#include <juce_audio_processors/juce_audio_processors.h>

using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

namespace njwfs {
    enum class SourcePositionAxis : char {
        X = 'x',
        Y = 'y'
    };

    class Utils {
    public:
        /**
         * Get an authoritative parameter-ID/OSC-path for a sound source index
         * and positional axis, to be sent to and handled by the network
         * clients.
         * @param index Sound source index.
         * @param axis Positional axis.
         * @return Parameter ID / OSC path.
         */
        static juce::String getSourcePositionParamID(uint index, SourcePositionAxis axis) {
            return "/source/" + juce::String{index} + "/" + static_cast<char>(axis);
        }

        /**
         * Get a parameter-ID/OSC-path for the module index parameter, to be
         * sent to and handled by the network clients.
         * @param index Module index.
         * @return Parameter ID / OSC path.
         */
        static juce::String getModuleIndexParamID(uint index) {
            return "/module/" + juce::String{index};
        }

        /**
         * Parameter-ID/OSC-path for the speaker spacing parameter, to be sent
         * to and handled by the network clients.
         */
        static const juce::StringRef speakerSpacingParamID;
        /**
         * Parameter ID for the gain parameter, to be handled by the server.
         */
        static const juce::StringRef gainParamID;

        static const juce::Identifier connectedPeersParamID;

        static constexpr int settingsButtonW{70}, settingsButtonH{25};
    };
}
