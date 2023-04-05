juce_add_console_app(NetJUCE
        PRODUCT_NAME "netjuce")

target_compile_definitions(NetJUCE
        PRIVATE
        JUCE_JACK=1
        JUCE_JACK_CLIENT_NAME="${PROJECT_NAME}"
        AUDIO_BLOCK_SAMPLES=16
        NUM_SOURCES=2
        DEFAULT_LOCAL_ADDRESS="192.168.10.10"
        DEFAULT_LOCAL_PORT=14841
        DEFAULT_MULTICAST_IP="226.6.38.226"
        DEFAULT_REMOTE_PORT=41814
        )

target_sources(NetJUCE
        PRIVATE
        Main.cpp
        MainComponent.cpp
        ../../lib/succulent/MultiChannelAudioSource/MultiChannelAudioSource.cpp
        ../NetAudioServer.cpp
        ../AudioToNetFifo.cpp
        ../DatagramAudioPacket.cpp
        ../MulticastSocket.cpp
        ../NetAudioPeer.cpp)

target_compile_definitions(NetJUCE
        PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:${PROJECT_NAME},JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:${PROJECT_NAME},JUCE_VERSION>")

target_link_libraries(NetJUCE
        PRIVATE
        juce::juce_core
        juce::juce_audio_basics
        juce::juce_audio_utils
        PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)

if (CMAKE_BUILD_TYPE STREQUAL "Debug" AND CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    # Sign targets to allow debugging without elevated privileges
    add_custom_command(TARGET NetJUCE
            POST_BUILD
            COMMAND /bin/sh ${PROJECT_SOURCE_DIR}/lib/succulent/scripts/add_debug_entitlement.sh
            ${PROJECT_BINARY_DIR}/NetJUCE_artefacts/${CMAKE_BUILD_TYPE}/netjuce)
endif ()