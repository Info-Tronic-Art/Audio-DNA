"""
Tier-1 shared helpers and known exceptions (s-rta-0927 follow-ups F1; diagnosis: .harmony/.reports/s-rta-0927/tier1-diag.md).

The four 256x256 Tier-1 files (test_sources, test_effects, test_audio_reactivity, test_time_sweep) import this ONE
module. Every exception entry carries its justification as its value -- the strings ARE the documentation
(SHADER_VERIFICATION.md "known exceptions"). An app defect is never listed here: it keeps its test red and is named
in the lane report with file:line.
"""

import cv2
import numpy as np


def is_black(path):
    """H3: a frame is black when the brightest 0.5 % of its pixels stays below 16 (per-pixel max channel).
    Sparse line art has a mean of 1-5 but a bright top 0.5 % (diag: this flags exactly the 9 truly black default
    frames of 108; the dimmest drawn frame scores 34)."""
    img = cv2.imread(path)
    return True if img is None else float(np.percentile(img.max(axis=2), 99.5)) < 16.0


def candidate_values(default):
    """H4: the old rule first (a param that passed before renders exactly as before), then the periodic/quantised
    fallbacks: 1.0 == 0.0 for anything scaled by 2*pi or wrapped with fract, an angle repeats every pi (0.25), and a
    quantised param can land in its default's bucket. (Not named test_*: pytest would collect it from every module
    that imports it.)"""
    primary = 0.0 if default > 0.3 else 1.0
    return [v for v in (primary, 0.5, 0.25) if abs(v - default) > 0.05]


# H4: t = 1.0 is a no-op for sin(k*2pi*t) / fract(k*t) shaders (Pulse, Color Flash, Strobe, Tunnel; diag E7).
T_PARAM = 1.13

# E: the audio-native sources/effects are black in silence. Constant across a param's default and test renders, so
# the comparison stays fair; a source whose look depends on audio gets a different but stable default frame.
FEATURES_ACTIVE = {
    "rms": 0.6, "peak": 0.8, "bpm": 120.0, "beatPhase": 0.25, "totalBeatCount": 1, "barPhase": 0.3,
    "bandEnergies": [0.6] * 7, "spectralCentroid": 2000.0, "spectralFlux": 0.3, "spectralFlatness": 0.3,
    "spectralRolloff": 4000.0, "onsetStrength": 0.5, "chromagram": [0.5] * 12, "mfccs": [0.2] * 13,
    "detectedKey": 0, "keyIsMajor": True, "structuralState": 1,
}


def arm_audio(app):
    """Publish FEATURES_ACTIVE. Call after EVERY app.reset(): the reset publishes a cleared snapshot."""
    app.inject_features(FEATURES_ACTIVE)


# Sources skipped by the non-black / params / time-sweep checks.
BLACK_SOURCES = {
    "layer_router": "Pitfall 21: returns another deck layer's output texture; the Eyes legacy path has no deck layer "
                    "(Renderer::renderSource layer_router branch returns 0)",
}

# (source_id, time) pairs not counted as "black at time" in the time sweep.
BLACK_AT_TIMES = {}

# (source_id, uniform) -> "GLSL: <EmbeddedShaders.h:line> -- <term> is zero at the value, nothing is drawn".
# Admission (both required): (a) the shader term is provably zero-output at that value, AND (b) the ladder proved
# the param has an effect at another candidate. Anything else is an app defect: no entry, named in the report.
BLACK_AT_EXTREME = {}

# Effects the params test skips: the legacy /api/set_effect chain cannot exercise them.
LEGACY_CHAIN_EFFECTS = {
    "Screen Split": "Pitfall 19: compositor-only frame history (CompositorEngine applyClipEffects); the legacy "
                    "/api/set_effect chain runs it as a plain shader",
    "Frame Stutter": "same as Screen Split",
}

# Effects whose params test renders over an animated source (plasma) instead of the static test card.
ANIMATED_INPUT_EFFECTS = {}
