juce_add_console_app(njConsole
        PRODUCT_NAME "njConsole")

target_compile_definitions(njConsole
        PRIVATE
        JUCE_JACK=1
        JUCE_JACK_CLIENT_NAME="${PROJECT_NAME}"
        AUDIO_BLOCK_SAMPLES=32
        NUM_SOURCES=2
        DEFAULT_LOCAL_ADDRESS="192.168.10.10"
        DEFAULT_LOCAL_PORT=14841
        DEFAULT_MULTICAST_IP="224.4.224.4"
        DEFAULT_AUDIO_SEND_PORT=6664
        )

target_sources(njConsole
        PRIVATE
        Main.cpp
        MainComponent.cpp
        ../../lib/succulent/MultiChannelAudioSource/MultiChannelAudioSource.cpp
        ../NetAudioServer.cpp
        ../AudioToNetFifo.cpp
        ../DatagramAudioPacket.cpp
        ../MulticastSocket.cpp
        ../NetAudioPeer.cpp)

target_compile_definitions(njConsole
        PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:njConsole,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:njConsole,JUCE_VERSION>")

target_link_libraries(njConsole
        PRIVATE
        juce::juce_core
        juce::juce_audio_basics
        juce::juce_audio_utils
        PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)

if (CMAKE_BUILD_TYPE STREQUAL "Debug" AND CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    # Sign targets to allow debugging without elevated privileges
    add_custom_command(TARGET njConsole
            POST_BUILD
            COMMAND /bin/sh ${PROJECT_SOURCE_DIR}/lib/succulent/scripts/add_debug_entitlement.sh
            ${PROJECT_BINARY_DIR}/njConsole_artefacts/${CMAKE_BUILD_TYPE}/netjuce)
endif ()
