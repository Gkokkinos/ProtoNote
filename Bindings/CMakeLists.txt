cmake_minimum_required(VERSION 3.24)
project(ProtoNoteBindings VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Lets VS Code IntelliSense (C_Cpp.default.compileCommands) resolve pybind11,
# Python and JUCE headers straight from the real build flags.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Python + pybind11. Point Python3_EXECUTABLE at the venv interpreter, e.g.
#   cmake -S Bindings -B Bindings/build -G Ninja \
#         -DPython3_EXECUTABLE=$PWD/TestsPy/.venv/bin/python
find_package(Python3 REQUIRED COMPONENTS Interpreter Development.Module)
execute_process(
        COMMAND ${Python3_EXECUTABLE} -m pybind11 --cmakedir
        OUTPUT_VARIABLE pybind11_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE
        COMMAND_ERROR_IS_FATAL ANY)
find_package(pybind11 CONFIG REQUIRED)

get_filename_component(DEFAULT_JUCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../Code/JUCE" ABSOLUTE)
set(JUCE_DIR "${DEFAULT_JUCE_DIR}" CACHE PATH "Path to the JUCE checkout")
add_subdirectory(${JUCE_DIR} JUCE EXCLUDE_FROM_ALL)

pybind11_add_module(protonote
        bindings.cpp
        JuceHeader.h
        ../Source/PluginProcessor.cpp
        ../Source/PluginEditor.cpp)

target_include_directories(protonote PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ../Source
        ../Tests/Framework)

# Same flags the Projucer/Tests build passes, plus the two the juceaide-generated
# JuceHeader would normally define for juce_add_* targets.
target_compile_definitions(protonote PRIVATE
        JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1
        JUCE_STANDALONE_APPLICATION=1
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JucePlugin_Name="ProtoNote"
        JucePlugin_WantsMidiInput=1
        JucePlugin_ProducesMidiOutput=1
        JucePlugin_IsMidiEffect=0
        JucePlugin_IsSynth=0)

target_link_libraries(protonote PRIVATE
        juce::juce_audio_formats
        juce::juce_audio_processors
        juce::juce_gui_basics)
