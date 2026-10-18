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

## Python bindings (pytest)

The same harness is exposed to Python via pybind11:
[`Bindings/bindings.cpp`](Bindings/bindings.cpp) builds a `protonote` extension
module (`Harness`, `note_on`, `read_wav`/`write_wav`), and [`TestsPy/`](TestsPy/)
replicates `MidiVolumeTest.cpp` test-for-test with pytest. Building the extension
requires Python development headers (`Python.h`).

```bash
# one-time setup
python3 -m venv TestsPy/.venv
TestsPy/.venv/bin/pip install -r TestsPy/requirements.txt

# if your distro's Python has no dev headers, use a standalone build instead:
#   uv python install 3.14
#   uv venv --seed --python 3.14 TestsPy/.venv
#   uv pip install --python TestsPy/.venv/bin/python -r TestsPy/requirements.txt

# build the extension
cmake -S Bindings -B Bindings/build -G Ninja \
      -DPython3_EXECUTABLE=$PWD/TestsPy/.venv/bin/python
ninja -C Bindings/build

# run the suite (TestsPy/pytest.ini puts Bindings/build on sys.path)
TestsPy/.venv/bin/python -m pytest TestsPy -v
```

`UPDATE_GOLDEN=1` re-records golden files here as well (see
[`TestsPy/helpers.py`](TestsPy/helpers.py)).

## Build the plugin

Open [`ProtoNote.jucer`](ProtoNote.jucer) in the Projucer, save, and build from
[`Builds/LinuxMakefile`](Builds/LinuxMakefile).
