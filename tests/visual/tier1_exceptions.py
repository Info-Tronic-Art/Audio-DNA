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
    # round 2 (s-rta-0927 followups F1 triage): the opposite extreme (a toggle at 0.5 whose default sits at 0.5 --
    # spectral_waterfall Log Scale `> 0.5` -- is only exercised above it) and 0.1 (a 2*pi-scaled angle whose
    # 0.25 / 0.5 steps are symmetries of the shape -- shape_generator Rotation, torus_hole Phi Offset).
    ladder = []
    for v in (primary, 0.5, 0.25, 1.0 - primary, 0.1):
        if abs(v - default) > 0.05 and v not in ladder:
            ladder.append(v)
    return ladder


# H4: t = 1.0 is a no-op for sin(k*2pi*t) / fract(k*t) shaders (Pulse, Color Flash, Strobe, Tunnel; diag E7).
T_PARAM = 1.13

# A strobing picture is dark for part of every period: when a DEFAULT render is black at T_PARAM, the check is
# repeated at T_RETRY (strobe_light 15.5 Hz / fade 0.3 and the Strobe effect 8.5 Hz / duty 0.5 are both lit at an
# even whole second and dark at 1.13 -- EmbeddedShaders.h sourceStrobeLight / strobe). Generic: no name needed.
T_RETRY = 2.0

# E: the audio-native sources/effects are black in silence. Constant across a param's default and test renders, so
# the comparison stays fair; a source whose look depends on audio gets a different but stable default frame.
# Round 2: the chromagram is uneven (a flat one sums Harmonic Displacement's 12 direction vectors to zero,
# EmbeddedShaders.h harmonicDisplace), harmonicChangeDetection is set (chromatic_ring's ripple is `* u_hcdf`), and a
# pitch is set (cymatics clamps u_dominantPitch 0 -> 50 Hz, where its Chladni modes n == m make the pattern 0).
FEATURES_ACTIVE = {
    "rms": 0.6, "peak": 0.8, "bpm": 120.0, "beatPhase": 0.25, "totalBeatCount": 1, "barPhase": 0.3,
    "bandEnergies": [0.6] * 7, "spectralCentroid": 2000.0, "spectralFlux": 0.3, "spectralFlatness": 0.3,
    "spectralRolloff": 4000.0, "onsetStrength": 0.5,
    "chromagram": [1.0, 0.2, 0.5, 0.1, 0.8, 0.3, 0.6, 0.2, 0.9, 0.1, 0.4, 0.7], "mfccs": [0.2] * 13,
    "detectedKey": 0, "keyIsMajor": True, "structuralState": 1, "harmonicChangeDetection": 0.5,
    "dominantPitch": 440.0, "pitchConfidence": 0.8,
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
BLACK_AT_TIMES = {
    ("strobe_light", 1.13): "EmbeddedShaders.h sourceStrobeLight: phase = fract(t * 15.5) = 0.515 > fade 0.3 -> flash 0, "
                            "the Color 2 (black) half of the period",
    ("strobe_light", 5.13): "same: fract(5.13 * 15.5) = 0.515 -> the dark half of the period",
}

# (source_id, uniform) -> "GLSL: <EmbeddedShaders.h:line> -- <term> is zero at the value, nothing is drawn".
# Admission (both required): (a) the shader term is provably zero-output at that value, AND (b) the ladder proved
# the param has an effect at another candidate. Anything else is an app defect: no entry, named in the report.
BLACK_AT_EXTREME = {
    ("fire_wall", "u_src_density"): "GLSL: EmbeddedShaders.h:9743 heightMask *= u_src_density * 1.5 -- 0 at 0.0, so "
                                    "fire = n * heightMask = 0 and the (default hot) palette mix(vec3(0.0), ..., 0) "
                                    "is black",
    ("concentric_rings", "u_src_width"): "GLSL: EmbeddedShaders.h:4771-4772 width = u_src_width * 0.5 = 0 -> "
                                         "mask = smoothstep(0.0, -0.02, ring) = 0 for every ring >= 0",
    ("metaballs", "u_src_size"): "GLSL: EmbeddedShaders.h:4863 blobSize = u_src_size * 0.1 = 0 -> field = 0 -> "
                                 "mask = smoothstep(threshold >= 1.0, ..., 0.0) = 0",
    ("line_pattern", "u_src_width"): "GLSL: EmbeddedShaders.h:4748-4749 width = u_src_width * 0.5 = 0 -> "
                                     "mask = smoothstep(0.0, -0.02, lineVal) = 0 for every lineVal >= 0",
}

# Effects the params test skips: the legacy /api/set_effect chain cannot exercise them.
LEGACY_CHAIN_EFFECTS = {
    "Screen Split": "Pitfall 19: compositor-only frame history (CompositorEngine applyClipEffects); the legacy "
                    "/api/set_effect chain runs it as a plain shader",
    "Frame Stutter": "same as Screen Split",
}

# Effects whose params test renders over an animated source (plasma) instead of the static test card.
_HISTORY = ("mixes the frame with u_prev_frame (EmbeddedShaders.h {}): over the static test card the previous "
            "frame IS the current frame, so the mix is invisible; over plasma it is not")
ANIMATED_INPUT_EFFECTS = {
    "Channel Delay": _HISTORY.format("channelDelay: mix(current.r, prev.r, u_delay_r)"),
    "Posterize Time": _HISTORY.format("frameHold: mix(current, posterized, u_hold_amount)"),
    "Freeze": _HISTORY.format("timeFreeze: mix(current, frozen, u_freeze_amount)"),
    "Selective Color": "EmbeddedShaders.h selectiveColor: range widens the hue band around the target (red, hue 0) "
                       "that keeps its colour; the test card has no pixel in that band at any range, plasma "
                       "sweeps every hue",
}

# (effect, param) the params test skips: the param only acts on the compositor's feedback buffer, which the
# legacy /api/set_effect chain never binds.
_FEEDBACK = ("reads u_feedbackTex, bound only by CompositorEngine (the deck path); the legacy chain leaves it "
             "unbound -- {}")
LEGACY_CHAIN_PARAMS = {
    ("Point Zoom", "hue shift"): _FEEDBACK.format("EmbeddedShaders.h feedback: hue shift acts on `prev` only"),
    ("Point Zoom", "saturation"): _FEEDBACK.format("saturation is applied to `prev`, and only while hue shift > 0"),
    ("Directional Feedback", "speed"): _FEEDBACK.format("directionalFeedback: speed moves the `prev` read"),
}

# (effect, param) -> (feature overrides, reason): the param acts only in one structural state, so its default and
# every candidate are rendered with FEATURES_ACTIVE + these overrides.
GATED_EFFECT_PARAMS = {
    ("Structural Morph", "normal style"): ({"structuralState": 0},
        "EmbeddedShaders.h structuralMorph: `normal` is weighted by w0 = 1 - |state|, zero outside state 0"),
    ("Structural Morph", "drop style"): ({"structuralState": 2},
        "EmbeddedShaders.h structuralMorph: `drop` is weighted by w2 = 1 - |state - 2|, zero outside state 2"),
}

# (source_id, uniform) -> (gate params, reason): the param acts only when another param opens its gate, so its
# default and every candidate are rendered with the gate open (the param itself is still tested).
_SLICE = "EmbeddedShaders.h: useSlice = abs(u_src_slice - 0.5) > 0.01 -- slices exist only off Cross Section 0.5"
_MULTI = _SLICE + "; sliceSpacing is read only when sliceCount = int(u_src_slice_count * 4) + 1 > 1"
_TRAIL = "EmbeddedShaders.h: numTrails = int(u_src_trail_dist * 5.0); trailDecay is read only when numTrails > 0"
GATED_SOURCE_PARAMS = {}
for _s in ("mandelbulb", "apollonian_3d", "sierpinski_tetra", "julia_set_3d", "kifs", "menger_sponge",
           "burning_ship_3d"):
    GATED_SOURCE_PARAMS[(_s, "u_src_slice_count")] = ({"u_src_slice": 0.55}, _SLICE)
    GATED_SOURCE_PARAMS[(_s, "u_src_slice_dist")] = ({"u_src_slice": 0.55, "u_src_slice_count": 0.5}, _MULTI)
    GATED_SOURCE_PARAMS[(_s, "u_src_trail_fade")] = ({"u_src_trail_dist": 0.6}, _TRAIL)
GATED_SOURCE_PARAMS[("torus_hole", "u_src_lens_rotate")] = (
    {"u_src_lens_shape": 0.5}, "EmbeddedShaders.h:8416-8423: the lens is rotated, stretched by 1 + lens_shape * 2, "
                               "rotated back -- at Lens Shape 0 (circular) the rotation is the identity")

# (source_id, uniform) the params test skips: the param only acts on the compositor's feedback buffer.
DECK_ONLY_SOURCE_PARAMS = {
    ("line_generator", u): "EmbeddedShaders.h sourceLineGenerator: the built-in feedback reads u_feedbackTex = "
                           "CompositorEngine's feedback buffer (ProceduralSource::setFeedbackTexture), which only "
                           "the deck path writes; on the Eyes legacy path `prev` never changes"
    for u in ("u_src_feedback", "u_src_fb_zoom", "u_src_fb_rotation", "u_src_fb_decay")
}
