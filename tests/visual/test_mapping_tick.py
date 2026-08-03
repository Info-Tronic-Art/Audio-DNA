"""
MappingTick 4-state param-tracking probe (outputwindow-arc-design.md W7(ii)).

Proves whether the mapping cadence (MappingEngine::processFrame) is alive in
each of the four GL attach states, by asserting that a mapped effect param
tracks injected RMS flips within 500 ms. CPU-observable only (U5): the probe
reads param values over the API — it never inspects pixels.

The four attach states (U2 "all four"):

  state 1  preview          normal layout, output window closed
  state 2  preview_output   normal layout, output window OPEN (Cmd+F)
  state 3  signalbar        SignalBar EXPANDED (preview hidden -> main GL
                            context auto-detached by JUCE), output closed
  state 4  signalbar_output SignalBar expanded AND output window open

Pre-W5 (mapping tick driven from the main renderer's GL callback), states
3-4 MUST FAIL: the main GL context is detached, processFrame never runs,
the mapped param freezes. That failure run is the fail-first evidence
(captured to .harmony/ow-freeze-before.log). After W5 (message-thread
juce::Timer tick) all four states must PASS.

UI-state driving is NOT automated: the SignalBar cycler and Cmd+F are
operator steps (synthetic clicks on the ~15px cycler are a documented
flake — .harmony/gotchas.md 2026-07-30 #9). Select the state under test
with OW_PROBE_STATE after putting the app in that state:

    cd tests/visual
    AUDIODNA_NO_SPAWN=1 OW_PROBE_STATE=preview \
        python3 -m pytest test_mapping_tick.py -v -s
    # then change the app's layout and repeat with
    # OW_PROBE_STATE=preview_output / signalbar / signalbar_output

With OW_PROBE_STATE unset, only state 1 (preview) runs and the manual-layout
states skip — safe for the normal spawned-app suite. AUDIODNA_NO_SPAWN=1
attaches to an already-running test-mode app (port 8080 via localhost; the
TestServer binds ::1, so never probe 127.0.0.1 directly).
"""

import os
import time

import pytest

TRACK_TIMEOUT_S = 0.5   # the <=500ms tracking bound from the design (W7 ii)
POLL_INTERVAL_S = 0.05
HIGH, LOW = 0.9, 0.1    # thresholds for rms=1 / rms=0 with alpha=1 mapping

PROBE_STATE = os.environ.get("OW_PROBE_STATE", "preview")

STATES = ["preview", "preview_output", "signalbar", "signalbar_output"]

MANUAL_SETUP = {
    "preview": "normal layout, output window closed",
    "preview_output": "normal layout, output window OPEN (Cmd+F)",
    "signalbar": "SignalBar EXPANDED so the preview panel is hidden, "
                 "output window closed",
    "signalbar_output": "SignalBar EXPANDED (preview hidden) AND output "
                        "window OPEN",
}


def _pick_target(app):
    """Pick the first effect with at least one param from live state.

    No hardcoded effect names: mapping writes land regardless of the
    effect's enabled flag (processFrame checks existence + param bounds
    only), so any effect works and nothing changes visually.
    """
    st = app.state()
    for fx in st["effects"]:
        if fx["params"]:
            return fx["name"], fx["params"][0]["name"]
    pytest.fail("No effect with params found in /api/state")


def _read_param(app, effect_name, param_name):
    st = app.state()
    for fx in st["effects"]:
        if fx["name"] == effect_name:
            for p in fx["params"]:
                if p["name"] == param_name:
                    return p["value"]
    pytest.fail(f"Param {effect_name}.{param_name} vanished from /api/state")


def _wait_for(app, effect_name, param_name, predicate):
    """Poll the param until predicate(value) or the tracking bound expires.

    Returns (ok, last_value).
    """
    deadline = time.time() + TRACK_TIMEOUT_S
    while time.time() < deadline:
        last = _read_param(app, effect_name, param_name)
        if predicate(last):
            return True, last
        time.sleep(POLL_INTERVAL_S)
    last = _read_param(app, effect_name, param_name)
    return predicate(last), last


@pytest.fixture()
def mapped_param(app):
    """Set up content + one RMS->param mapping; tear both down after."""
    # Open the render gate: without content the main renderer early-outs
    # BEFORE its processFrame call, which would look like a freeze even in
    # state 1. Checkerboard is the standard procedural source for this
    # (.harmony/gotchas.md 2026-05-19).
    app.load_source("checkerboard")

    effect_name, param_name = _pick_target(app)

    # alpha 1.0 = no EMA lag: isolates the cadence question from smoothing.
    r = app.add_mapping(effect_name, param_name, smoothing=1.0)
    assert r["ok"], f"add_mapping failed: {r}"

    yield effect_name, param_name

    # Drain all mappings (apply is async; num_mappings_before reports the
    # count at request time, so repeat until it says 0).
    for _ in range(16):
        if app.remove_mapping(0).get("num_mappings_before", 0) == 0:
            break
    app.inject_features({"rms": 0.0})


def _assert_tracking(app, effect_name, param_name, state_name):
    """Two full rms flip cycles; assert the param follows each edge."""
    for cycle in (1, 2):
        app.inject_features({"rms": 1.0})
        ok, last = _wait_for(app, effect_name, param_name, lambda v: v >= HIGH)
        assert ok, (
            f"[{state_name}] cycle {cycle}: param {effect_name}.{param_name} "
            f"FROZEN at {last} for {int(TRACK_TIMEOUT_S * 1000)}ms after "
            f"rms->1.0 — mapping tick is not running in this attach state"
        )

        app.inject_features({"rms": 0.0})
        ok, last = _wait_for(app, effect_name, param_name, lambda v: v <= LOW)
        assert ok, (
            f"[{state_name}] cycle {cycle}: param {effect_name}.{param_name} "
            f"FROZEN at {last} for {int(TRACK_TIMEOUT_S * 1000)}ms after "
            f"rms->0.0 — mapping tick is not running in this attach state"
        )


def _state_test(app, mapped_param, state_name):
    if PROBE_STATE != state_name:
        pytest.skip(
            f"OW_PROBE_STATE={PROBE_STATE!r} — set OW_PROBE_STATE="
            f"{state_name} after putting the app in: "
            f"{MANUAL_SETUP[state_name]}"
        )
    effect_name, param_name = mapped_param
    _assert_tracking(app, effect_name, param_name, state_name)


def test_state1_preview_tracks(app, mapped_param):
    """State 1: normal layout, no output window."""
    _state_test(app, mapped_param, "preview")


def test_state2_preview_output_tracks(app, mapped_param):
    """State 2: normal layout + output window open."""
    _state_test(app, mapped_param, "preview_output")


def test_state3_signalbar_tracks(app, mapped_param):
    """State 3: SignalBar expanded (preview hidden, main GL detached).

    MUST FAIL pre-W5 — that failure is the ow-freeze-before.log evidence.
    """
    _state_test(app, mapped_param, "signalbar")


def test_state4_signalbar_output_tracks(app, mapped_param):
    """State 4: SignalBar expanded + output window open.

    MUST FAIL pre-W5 — the output keeps rendering while the mapped param
    freezes, the arc's headline symptom.
    """
    _state_test(app, mapped_param, "signalbar_output")
