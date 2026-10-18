"""Python port of the C++ test framework in Tests/Framework.

midi_matches  <- PluginTest::midiMatches  (TestUtils.h)
matches_golden / write golden  <- PluginTest::matchesGolden (GoldenCompare.h)

Helpers return an error description on mismatch and "" on success, so tests
read as:  assert not (msg := helper(...)), msg
"""

import os
from pathlib import Path

import numpy as np
import protonote as pn

REPO_ROOT = Path(__file__).resolve().parent.parent
GOLDEN_DIR = REPO_ROOT / "Tests" / "Test_Data" / "golden"


def make_note_with_velocity(velocity: int) -> bytes:
    """Note-on message on channel 1, pitch 60, with the given velocity."""
    return pn.note_on(1, 60, velocity)


def midi_matches(actual: list, expected: list) -> str:
    """Same check as PluginTest::midiMatches: count, sample position, raw bytes."""
    if len(actual) != len(expected):
        return f"MIDI event counts differ: actual {len(actual)}, expected {len(expected)}"

    for i, ((actual_pos, actual_msg), (expected_pos, expected_msg)) in enumerate(
        zip(actual, expected)
    ):
        if actual_pos != expected_pos or actual_msg != expected_msg:
            return (
                f"MIDI event {i} differs: actual [{actual_pos}] {list(actual_msg)}, "
                f"expected [{expected_pos}] {list(expected_msg)}"
            )
    return ""


def matches_golden(
    name: str, actual: np.ndarray, sample_rate: float, tolerance: float = 1e-6
) -> str:
    """Compares against Tests/Test_Data/golden/<name>; UPDATE_GOLDEN=1 records instead."""
    path = GOLDEN_DIR / name

    if os.environ.get("UPDATE_GOLDEN") == "1":
        if not pn.write_wav(str(path), actual, sample_rate):
            return f"could not write golden file {path}"
        return ""

    try:
        expected, expected_rate = pn.read_wav(str(path))
    except RuntimeError as exc:
        return f"{exc} (run with UPDATE_GOLDEN=1 to record it)"

    if expected_rate != sample_rate:
        return f"sample rate differs: actual {sample_rate}, golden {expected_rate}"

    if actual.shape != expected.shape:
        return f"shape differs: actual {actual.shape}, expected {expected.shape}"

    diff = np.abs(actual - expected)
    ch, i = np.unravel_index(np.argmax(diff), diff.shape)
    if diff[ch, i] > tolerance:
        return (
            f"channel {ch} sample {i} differs by {diff[ch, i]} "
            f"(actual {actual[ch, i]}, expected {expected[ch, i]})"
        )
    return ""
