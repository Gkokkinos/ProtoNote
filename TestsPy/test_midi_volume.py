import protonote as pn
from helpers import make_note_with_velocity, matches_golden, midi_matches


def render_with_velocity(plugin, velocity, input_events, num_samples=512):
    plugin.set_note_on_velocity(velocity)
    return plugin.render(input_events, num_samples)


class TestMidiVolume:
    # Steps: set the slider to 50, play a velocity-100 note-on.
    # Expect: the same note-on comes out with velocity 50.
    def test_note_on_velocity_follows_volume(self, plugin):
        input_events = [(0, make_note_with_velocity(100))]
        expected = [(0, make_note_with_velocity(50))]

        _, midi = render_with_velocity(plugin, 50.0, input_events)

        assert not (msg := midi_matches(midi, expected)), msg

    # Steps: leave the slider untouched, play a velocity-100 note-on.
    # Expect: the note-on comes out with the default velocity (64).
    def test_default_velocity_is_applied(self, plugin):
        input_events = [(0, make_note_with_velocity(100))]
        expected = [(0, make_note_with_velocity(64))]

        _, midi = plugin.render(input_events, 512)

        assert not (msg := midi_matches(midi, expected)), msg

    # Steps: render 1024 samples in 128-sample blocks, play a note-on at sample
    # 300 with the slider at 32.
    # Expect: the note-on stays at sample 300, with velocity 32.
    def test_event_timing_is_preserved_across_blocks(self):
        small_blocks = pn.Harness(44100.0, 128)
        input_events = [(300, make_note_with_velocity(100))]
        expected = [(300, make_note_with_velocity(32))]

        _, midi = render_with_velocity(small_blocks, 32.0, input_events, 1024)

        assert not (msg := midi_matches(midi, expected)), msg

    # Steps: set the slider to 64, render one second with a note-on.
    # Expect: the audio matches the recorded reference (currently silence,
    # the plugin emits no sound).
    def test_audio_output_matches_golden(self, plugin):
        input_events = [(0, make_note_with_velocity(100))]
        one_second = int(plugin.sample_rate())

        audio, _ = render_with_velocity(plugin, 64.0, input_events, one_second)

        assert not (
            msg := matches_golden("midi_volume_64.wav", audio, plugin.sample_rate())
        ), msg
