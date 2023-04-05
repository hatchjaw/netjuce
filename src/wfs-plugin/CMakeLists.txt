juce_add_plugin(njWfsPlugin
        COMPANY_NAME TAR
        IS_SYNTH FALSE
        NEEDS_MIDI_INPUT FALSE
        COPY_PLUGIN_AFTER_BUILD TRUE
        PLUGIN_MANUFACTURER_CODE TarQ
        PLUGIN_CODE Tr13
        FORMATS AU VST3 Standalone
        VST3_CATEGORIES "Network"
        PRODUCT_NAME "njWFS")

target_compile_definitions(njWfsPlugin
        PRIVATE
#        JUCE_JACK=1
#        JUCE_JACK_CLIENT_NAME="${PROJECT_NAME}"
        AUDIO_BLOCK_SAMPLES=32
        NUM_SOURCES=8
        DEFAULT_LOCAL_ADDRESS="192.168.10.10"
        DEFAULT_LOCAL_PORT=14841
        DEFAULT_MULTICAST_IP="226.6.38.226"
        DEFAULT_REMOTE_PORT=41814
        )

target_sources(njWfsPlugin
        PRIVATE
        PluginEditor.cpp
        PluginProcessor.cpp
        ../NetAudioServer.cpp
        ../AudioToNetFifo.cpp
        ../DatagramAudioPacket.cpp
        ../MulticastSocket.cpp
        ../NetAudioPeer.cpp
        ../../lib/succulent/ui/XYController/XYController.cpp
        )

target_compile_definitions(njWfsPlugin
        PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(njWfsPlugin
        PRIVATE
        juce::juce_core
        juce::juce_audio_basics
        juce::juce_audio_utils
        juce::juce_audio_processors
        PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

if (CMAKE_BUILD_TYPE STREQUAL "Debug" AND CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    # Sign targets to allow debugging without elevated privileges
    add_custom_command(TARGET NetJUCE
            POST_BUILD
            COMMAND /bin/sh ${PROJECT_SOURCE_DIR}/lib/succulent/scripts/add_debug_entitlement.sh
            ${PROJECT_BINARY_DIR}/njWfsPlugin_artefacts/${CMAKE_BUILD_TYPE}/...)
endif ()