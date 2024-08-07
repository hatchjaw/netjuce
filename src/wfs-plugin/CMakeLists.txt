juce_add_plugin(njWfsPlugin
        COMPANY_NAME TAR
        IS_SYNTH FALSE
        NEEDS_MIDI_INPUT FALSE
        COPY_PLUGIN_AFTER_BUILD TRUE
        PLUGIN_MANUFACTURER_CODE TarQ
        PLUGIN_CODE Tr14
        FORMATS AU VST3 Standalone
        VST3_CATEGORIES "Network"
        PRODUCT_NAME "njWFS")

# TODO: Follow IANA standards for port numbers
# https://www.iana.org/assignments/service-names-port-numbers/service-names-port-numbers.xhtml
# Local port should be ephemeral (49152-65535)
target_compile_definitions(njWfsPlugin
        PRIVATE
        NUM_SOURCES=8
        NUM_MODULES=8
        DEFAULT_LOCAL_ADDRESS="192.168.10.10"
        DEFAULT_LOCAL_PORT=14841
        DEFAULT_MULTICAST_IP="224.4.224.4"
        DEFAULT_AUDIO_SEND_PORT=6664
        DEFAULT_OSC_SEND_PORT=6663
        )

target_sources(njWfsPlugin
        PRIVATE
        PluginEditor.cpp
        PluginProcessor.cpp
        WFSMessenger.cpp
        ../NetAudioServer.cpp
#        ../NetAudioCohort.cpp
        ../AudioToNetFifo.cpp
        ../DatagramAudioPacket.cpp
        ../MulticastSocket.cpp
        ../NetAudioPeer.cpp
        ../../lib/succulent/ui/XYController/XYController.cpp
        ../../lib/succulent/ui/XYController/XYControllerAttachment.cpp
        Feels.cpp
        Utils.cpp
        ui/SettingsComponent.cpp
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
        juce::juce_data_structures
        juce::juce_osc
        juce::juce_gui_basics
        PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

if (CMAKE_BUILD_TYPE STREQUAL "Debug" AND CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    # Sign targets to allow debugging without elevated privileges
    add_custom_command(TARGET njWfsPlugin
            POST_BUILD
            COMMAND /bin/sh ${PROJECT_SOURCE_DIR}/lib/succulent/scripts/add_debug_entitlement.sh
            ${PROJECT_BINARY_DIR}/src/wfs-plugin/njWfsPlugin_artefacts/${CMAKE_BUILD_TYPE}/AU/njWFS.component)
endif ()
