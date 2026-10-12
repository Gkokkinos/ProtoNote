#include "PluginFixture.h"

using PluginTest::makeMidi;
using PluginTest::matchesGolden;
using PluginTest::midiMatches;

namespace {

// Helper function to create a note-on message 1, pitch 60 with a specific velocity.
juce::MidiMessage makeNoteWithVelocity(int velocity) {
    return juce::MidiMessage::noteOn(1, 60, (juce::uint8) velocity);
}

RenderResult renderWithVelocity(PluginHarness<ProtoNoteAudioProcessor>& plugin, float velocity,
                                const juce::MidiBuffer& input, int numSamples = 512) {
    plugin.processor.setNoteOnVelocity(velocity);
    return plugin.render(input, numSamples);
}

}  // namespace

class MidiVolumeTest : public PluginFixture {};

// Steps: set the slider to 50, play a velocity-100 note-on.
// Expect: the same note-on comes out with velocity 50.
TEST_F(MidiVolumeTest, NoteOnVelocityFollowsVolume) {
    const auto input = makeMidi({{0, makeNoteWithVelocity(100)}});
    const auto expected = makeMidi({{0, makeNoteWithVelocity(50)}});

    const auto result = renderWithVelocity(plugin_, 50.0f, input);

    EXPECT_TRUE(midiMatches(result.midi, expected));
}

// Steps: leave the slider untouched, play a velocity-100 note-on.
// Expect: the note-on comes out with the default velocity (64).
TEST_F(MidiVolumeTest, DefaultVelocityIsApplied) {
    const auto input = makeMidi({{0, makeNoteWithVelocity(100)}});
    const auto expected = makeMidi({{0, makeNoteWithVelocity(64)}});

    const auto result = plugin_.render(input, 512);

    EXPECT_TRUE(midiMatches(result.midi, expected));
}

// Steps: render 1024 samples in 128-sample blocks, play a note-on at sample 300 with the slider at 32.
// Expect: the note-on stays at sample 300, with velocity 32.
TEST_F(MidiVolumeTest, EventTimingIsPreservedAcrossBlocks) {
    PluginHarness<ProtoNoteAudioProcessor> smallBlocks(44100.0, 128);
    const auto input = makeMidi({{300, makeNoteWithVelocity(100)}});
    const auto expected = makeMidi({{300, makeNoteWithVelocity(32)}});

    const auto result = renderWithVelocity(smallBlocks, 32.0f, input, 1024);

    EXPECT_TRUE(midiMatches(result.midi, expected));
}

// Steps: set the slider to 64, render one second with a note-on.
// Expect: the audio matches the recorded reference (currently silence, the plugin emits no sound).
TEST_F(MidiVolumeTest, AudioOutputMatchesGolden) {
    const auto input = makeMidi({{0, makeNoteWithVelocity(100)}});
    const int oneSecond = static_cast<int>(plugin_.sampleRate());

    const auto result = renderWithVelocity(plugin_, 64.0f, input, oneSecond);

    EXPECT_TRUE(matchesGolden("midi_volume_64.wav", result.audio, plugin_.sampleRate()));
}
