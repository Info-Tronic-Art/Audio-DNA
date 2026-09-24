"""
Onset render-path pulse — counter oracle (plan-onset-render.md section 3 C).

The main Renderer derives its per-frame onset pulse from the monotonic
FeatureSnapshot::onsetCount delta (OnsetPulse), so every onset fires exactly
one pulse frame at any frame rate. /api/state.onset_pulse_frames counts the
frames on which that pulse fired. A PSNR diff is NOT a usable oracle for a
one-frame pulse (render_frame captures of a live stateful source converge to
a steady state), so this test asserts on the counter only.

Test-mode injection mirrors AnalysisThread's bookkeeping: an injected
{"onsetDetected": true} bumps the published onsetCount by one, an explicit
{"onsetCount": N} is used verbatim, a request with neither carries the count,
and /api/reset publishes count 0 (a backwards jump -> consumers re-baseline,
no pulse). The count is resolved in ONE place, under TestServer's inject lock.

Needs a --test-mode build (AUDIODNA_BUILD_TEST_SERVER=ON) with the preview
visible (a detached GL context renders no frames). The GL thread consumes on
its next frame, so the 1 s wait is generous.

    AUDIODNA_NO_SPAWN=1 pytest tests/visual/test_onset_pulse.py -v
"""

import time


def _pulses(app):
    return app.state()["onset_pulse_frames"]


def wait_pulses(app, target, timeout=1.0):
    """Poll onset_pulse_frames every 20 ms until it reaches target (or timeout)."""
    deadline = time.monotonic() + timeout
    value = _pulses(app)
    while value < target and time.monotonic() < deadline:
        time.sleep(0.02)
        value = _pulses(app)
    return value


def test_injected_onsets_pulse_exactly_once(app):
    app.reset()
    s0 = _pulses(app)

    # An injected onset pulses exactly one frame.
    app.inject_features({"onsetDetected": True, "onsetStrength": 0.9})
    assert wait_pulses(app, s0 + 1) == s0 + 1

    # No onset key: the count carries, the render path must not pulse again
    # (the pre-fix sticky bool would have pulsed on every frame).
    app.inject_features({"rms": 0.2})
    time.sleep(0.3)
    assert _pulses(app) == s0 + 1

    # An explicit count jump of 9 collapses into ONE pulse frame.
    app.inject_features({"onsetCount": 10})
    assert wait_pulses(app, s0 + 2) == s0 + 2

    # Same explicit count again: no new onset.
    app.inject_features({"onsetCount": 10})
    time.sleep(0.3)
    assert _pulses(app) == s0 + 2

    # Reset publishes count 0: a backwards jump re-baselines without a pulse.
    app.reset()
    time.sleep(0.3)
    assert _pulses(app) == s0 + 2

    # The count restarted at 0, so the next injected onset (count 1) pulses.
    app.inject_features({"onsetDetected": True})
    assert wait_pulses(app, s0 + 3) == s0 + 3
