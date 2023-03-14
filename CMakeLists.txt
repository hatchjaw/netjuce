cmake_minimum_required(VERSION 3.15)

project(NetJUCE VERSION 0.0.1)

# cmake flag -DJUCE_INSTALL_PATH="/path/to/JUCE/install"
list(APPEND CMAKE_PREFIX_PATH ${JUCE_INSTALL_PATH})
find_package(JUCE CONFIG REQUIRED)

juce_add_console_app(NetJUCE
        PRODUCT_NAME "netjuce")

target_compile_definitions(NetJUCE
        PRIVATE
        JUCE_JACK=1
        JUCE_JACK_CLIENT_NAME="${PROJECT_NAME}"
        AUDIO_BLOCK_SAMPLES=32
        NUM_SOURCES=2
        DEFAULT_LOCAL_ADDRESS="192.168.10.10"
        DEFAULT_LOCAL_PORT=14841
        DEFAULT_MULTICAST_IP="226.6.38.226"
#        DEFAULT_MULTICAST_IP="241.81.18.142"
        DEFAULT_REMOTE_PORT=41814
)

target_sources(NetJUCE
        PRIVATE
        src/Main.cpp
        src/MainComponent.cpp
        lib/succulent/MultiChannelAudioSource/MultiChannelAudioSource.cpp
        src/NetAudioServer.cpp
        src/AudioToNetFifo.cpp
        src/DatagramPacket.cpp
        src/MulticastSocket.cpp)

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