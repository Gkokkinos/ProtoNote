cmake_minimum_required(VERSION 3.24)
project(ProtoNoteTests VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.18.0.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
# Windows: use the shared CRT
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

include(GoogleTest)


# Integration tests: run the real plugin processor (../Source) without a host.
get_filename_component(DEFAULT_JUCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../Code/JUCE" ABSOLUTE)
set(JUCE_DIR "${DEFAULT_JUCE_DIR}" CACHE PATH "Path to the JUCE checkout")
add_subdirectory(${JUCE_DIR} JUCE EXCLUDE_FROM_ALL)

juce_add_console_app(pluginTest PRODUCT_NAME "pluginTest")
juce_generate_juce_header(pluginTest)

target_sources(pluginTest PRIVATE
        MidiVolumeTest.cpp
        PluginFixture.h
        Framework/BaseFixture.h
        Framework/GoldenCompare.h
        Framework/TestUtils.h
        Framework/PluginHarness.h
        ../Source/PluginProcessor.cpp
        ../Source/PluginEditor.cpp)

target_include_directories(pluginTest PRIVATE ../Source)

# Mirrors the plugin flags the Projucer would pass to the compiler.
target_compile_definitions(pluginTest PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JucePlugin_Name="ProtoNote"
        JucePlugin_WantsMidiInput=1
        JucePlugin_ProducesMidiOutput=1
        JucePlugin_IsMidiEffect=0
        JucePlugin_IsSynth=0
        PLUGIN_TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/Test_Data")

target_link_libraries(pluginTest PRIVATE
        GTest::gtest_main
        juce::juce_audio_formats
        juce::juce_audio_processors
        juce::juce_gui_basics)

gtest_discover_tests(pluginTest)
