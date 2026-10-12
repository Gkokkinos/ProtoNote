# ProtoNote

A trivial JUCE plugin from the
[basic plug-in tutorial](https://juce.com/tutorials/tutorial_code_basic_plugin/)
(a slider sets the velocity of incoming MIDI notes), used to showcase a small
**host-free test framework**.

## The idea

Everything a plugin does goes through `processBlock`. The tests in
[`Tests/`](Tests/) instantiate the real `ProtoNoteAudioProcessor` in a console app
and drive `processBlock` directly with MIDI/audio input, then assert on the output —
integration-level tests for the plugin's main function, running as ordinary unit tests
in milliseconds. No DAW, no UI, no audio device.

[`Tests/MidiVolumeTest.cpp`](Tests/MidiVolumeTest.cpp) shows examples: velocity remapping,
default parameter values, timing preserved across blocks, and golden-file audio comparison
(re-record with `UPDATE_GOLDEN=1`).

## Build and run the tests

Requires CMake ≥ 3.24, Ninja, and a JUCE checkout (expected at `../../../Code/JUCE`
relative to `Tests/`; override with `-DJUCE_DIR=/path/to/JUCE`).

```bash
cmake -S Tests -B Tests/build -G Ninja
ninja -C Tests/build
./Tests/build/pluginTest_artefacts/pluginTest
```

## Build the plugin

Open [`ProtoNote.jucer`](ProtoNote.jucer) in the Projucer, save, and build from
[`Builds/LinuxMakefile`](Builds/LinuxMakefile).
