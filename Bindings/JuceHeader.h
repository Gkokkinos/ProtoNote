#pragma once

// Stand-in for the header juceaide generates for juce_add_* CMake targets.
// The bindings target isn't created by a juce_add_* function (pybind11 owns it),
// so we list the linked modules by hand. Module config flags (JUCE_WEB_BROWSER=0
// etc.) come from target_compile_definitions, exactly like the generated header.
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

using namespace juce;
