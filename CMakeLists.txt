cmake_minimum_required(VERSION 3.15)

project(NetJUCE VERSION 0.0.1)

# cmake flag -DJUCE_INSTALL_PATH="/path/to/JUCE/install"
list(APPEND CMAKE_PREFIX_PATH ${JUCE_INSTALL_PATH})
find_package(JUCE CONFIG REQUIRED)

add_subdirectory(src/console-app)
add_subdirectory(src/wfs-plugin)