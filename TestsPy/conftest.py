import pytest

import protonote as pn


@pytest.fixture
def plugin():
    # New processor per test, with the default velocity remembered and restored so changes can't leak between tests.
    harness = pn.Harness()  # 44100.0 Hz, 512-sample blocks
    default_velocity = harness.get_note_on_velocity()
    yield harness
    harness.set_note_on_velocity(default_velocity)
