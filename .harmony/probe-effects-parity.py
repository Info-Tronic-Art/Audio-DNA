#!/usr/bin/env python3
"""probe-effects-parity.py -- REST/pixel half of .harmony/probe-effects-parity.sh (s-rta-0926 hardening).

The .sh owns launch / refuse / quit; this file talks to the already-running app on port 7070 and
decodes every captured PNG with PIL+numpy (never file hashes/sizes). Every render_frame response is
checked, and the output dir is fresh per run, so a failed capture is a FAIL -- never a decode of an
older file (parity-diagnosis.md section 8).

usage: probe-effects-parity.py <root> <fresh-outdir> <media-dir>

Variants (one layer, one clip, opacity 1, Opaque; image P16_01_baseline.png):
  V1   clip [Hue Shift]                       + layer [Saturation]      -- the ms-white2 bug fixture
                                                                           (.harmony/probe-effects-parity.json)
  V5   clip [Hue Shift]                       + layer [Saturation, Invert]
  V6   clip [Hue Shift, Invert, Brightness]   + layer [Saturation]
  REFn the same effects as ONE clip chain, no layer effects (clip fx then layer fx, in order)
Rows:
  non-blank   each of FRAMES frames: alpha>0 fraction >= 0.5 and RGB std >= 3
  parity      Vn frame vs REFn frame: mean |diff| over RGB <= PARITY_TOL. A clip chain followed by a
              layer chain is the same sequence of passes as one longer chain, so the frames must match;
              measured 0.00 for V1/V5/V6 on main (e1ed9cc) and on the xfade build, so 1.0 is a safe bound
              that still catches a stale/aliased pass (which moves the mean by tens).

G1 (s-rta-0926 cleanup lane -- the GLOBAL, single-image EffectChain, src/effects/EffectChain.cpp /
Renderer::effectChain_, reached via /api/load_image + /api/set_effect_chain, never /api/load_composition).
V1/V5/V6 above all go through CompositorEngine's per-clip/per-layer effects instead -- this is the first
live coverage of EffectChain.cpp in this probe. It runs FIRST, before any load_composition call: once a
composition is loaded its active deck stays composited and would shadow /api/load_image's texture
(Renderer::renderOpenGL's deckActive branch runs before the loaded-image fallback).
  PARITY-DOCUMENTS-GAP  neither /api/set_effect_chain nor /api/set_effect nor OSC's
                        /audiodna/effect/{name}/{param} (MainComponent.cpp's onSetEffectParam) exposes
                        Effect::dryWet_ -- all three only match named EffectParam values, verified by
                        reading ApiServer.cpp/TestServer.cpp/MainComponent.cpp. So the literally-requested
                        "second effect at dryWet 0.5" assertion cannot be run live without an API change,
                        which is out of this lane's fence (src/effects/EffectChain.{h,cpp} + this probe
                        only). This row sends the request shape that change would need, then reads
                        /api/state back and asserts dry_wet is STILL the 1.0 default -- proving the gap
                        live and re-checking it every run, rather than trusting a stale comment.
  non-blank             closest reachable regression: the same two-effect global chain end-to-end
                        (Hue Shift then Invert, registration order -- Invert is the non-first effect),
                        through the exact render() ping-pong path this lane's fix touched, even though
                        dryWet itself stays at its default and the mid-chain dry/wet branch specifically
                        cannot be driven live today.
"""
import json, os, sys, time

import numpy as np
import requests
from PIL import Image

A = "http://127.0.0.1:7070"
ROOT, OUT, MEDIA = sys.argv[1], sys.argv[2], sys.argv[3]
FRAMES, PARITY_TOL = 6, 1.0
IMG = os.path.join(MEDIA, "P16_01_baseline.png")
PASS = FAIL = 0


def ok(msg):
    global PASS; PASS += 1; print(f"PASS  {msg}", flush=True)


def no(msg):
    global FAIL; FAIL += 1; print(f"FAIL  {msg}", flush=True)


def fx(name, v):
    return {"name": name, "enabled": True, "bypassed": False, "dryWet": 1.0, "params": [v]}


HUE, SAT, INV, BRI = fx("Hue Shift", 0.5), fx("Saturation", 0.8), fx("Invert", 0.7), fx("Brightness", 0.6)
GENERATED = {
    "REF1": ([HUE, SAT], []),
    "V5": ([HUE], [SAT, INV]),
    "REF5": ([HUE, SAT, INV], []),
    "V6": ([HUE, INV, BRI], [SAT]),
    "REF6": ([HUE, INV, BRI, SAT], []),
}


def fixture(tag):
    if tag == "V1":  # the historic bug fixture, loaded as committed
        return os.path.join(ROOT, ".harmony", "probe-effects-parity.json")
    clip_fx, layer_fx = GENERATED[tag]
    comp = {"name": "parity-" + tag, "activeDeckIndex": 0, "masterOpacity": 1.0,
            "decks": [{"name": "A", "id": 0, "numColumns": 1, "layers": [
                {"name": "L1", "id": 0, "opacity": 1.0, "visible": True, "blendMode": 1, "layerEffects": layer_fx,
                 "clips": [{"name": "img", "id": 1, "mediaType": 1, "mediaFile": IMG, "effects": clip_fx}]}]}]}
    path = os.path.join(OUT, f"fx_{tag}.json")
    json.dump(comp, open(path, "w"), indent=1)
    return path


def run(tag):
    """Load, trigger, capture FRAMES frames. Returns the list of decoded frames (None for a failed capture)."""
    r = requests.post(A + "/api/load_composition", json={"path": fixture(tag)}, timeout=10)
    if not (r.ok and r.json().get("ok")):
        no(f"{tag}: fixture rejected: {r.text[:120]}")
        return []
    time.sleep(1.0)
    requests.post(A + "/api/trigger_clip", json={"layer": 0, "column": 0}, timeout=6)
    time.sleep(2.0)
    frames = []
    for k in range(FRAMES):
        p = os.path.join(OUT, f"{tag}_{k}.png")
        try:
            body = requests.post(A + "/api/render_frame", json={"output_path": p}, timeout=20).json()
        except Exception as e:  # noqa: BLE001 -- any transport/JSON failure is a failed capture
            body = {"error": str(e)}
        if not body.get("ok") or not os.path.isfile(p):
            no(f"{tag} frame {k}: render_frame failed: {body}")
            frames.append(None)
        else:
            frames.append(np.asarray(Image.open(p).convert("RGBA")).astype(float))
        time.sleep(0.3)
    return frames


def run_global_chain():
    """G1 -- see the module docstring. Must run BEFORE any load_composition call
    (this file's own V1/etc. included): once a composition is loaded, its active
    deck stays composited and would shadow /api/load_image's texture."""
    r = requests.post(A + "/api/load_image", json={"filepath": IMG}, timeout=10)
    if not (r.ok and r.json().get("ok", True)):
        no(f"G1: load_image rejected: {r.text[:120]}")
        return

    # The literally-requested shape: dryWet on the params object of the SECOND
    # (non-first, registration-order) effect. Hue Shift is registered before
    # Invert in EffectLibrary.cpp's "color" category, so Invert is the
    # non-first effect Renderer::initEffectChain() ends up with enabled here.
    attempt = {"effects": [
        {"name": "Hue Shift", "params": {"amount": 0.5}},
        {"name": "Invert", "dryWet": 0.5, "params": {"amount": 0.7}},
    ]}
    requests.post(A + "/api/set_effect_chain", json=attempt, timeout=10)
    time.sleep(0.3)

    state = requests.get(A + "/api/state", timeout=10).json()
    invert = next((e for e in state.get("effects", []) if e.get("name") == "Invert"), None)
    if invert is None:
        no("G1 PARITY-DOCUMENTS-GAP: 'Invert' missing from /api/state after set_effect_chain")
    else:
        # Documents the gap live: dryWet stayed at its 1.0 default -- the
        # request's "dryWet": 0.5 was silently ignored (not a settable field
        # on /api/set_effect_chain today). If this ever starts failing, the
        # API gained dryWet support and the numeric mid-chain assertion this
        # row's docstring describes as unreachable should be added for real.
        got_dw = invert.get("dry_wet")
        (ok if got_dw is not None and abs(got_dw - 1.0) < 1e-6 else no)(
            f"G1 PARITY-DOCUMENTS-GAP: /api/set_effect_chain has no dryWet field "
            f"(Invert dry_wet={got_dw}, expected unchanged 1.0 default despite "
            f"the request's \"dryWet\": 0.5 -- closest reachable path used instead)")

    frames = []
    for k in range(FRAMES):
        p = os.path.join(OUT, f"G1_{k}.png")
        try:
            body = requests.post(A + "/api/render_frame", json={"output_path": p}, timeout=20).json()
        except Exception as e:  # noqa: BLE001 -- any transport/JSON failure is a failed capture
            body = {"error": str(e)}
        if not body.get("ok") or not os.path.isfile(p):
            no(f"G1 frame {k}: render_frame failed: {body}")
            frames.append(None)
        else:
            frames.append(np.asarray(Image.open(p).convert("RGBA")).astype(float))
        time.sleep(0.3)

    for k, a in enumerate(frames):
        if a is None:
            continue
        af, sd = float((a[..., 3] > 0).mean()), float(a[..., :3].std())
        (ok if af >= 0.5 and sd >= 3 else no)(
            f"G1 frame {k} non-blank (alpha>0 frac {af:.3f}, RGB std {sd:.1f}) -- "
            f"the GLOBAL EffectChain path (src/effects/EffectChain.cpp), not CompositorEngine")


def main():
    run_global_chain()
    got = {}
    for tag in ("V1", "REF1", "V5", "REF5", "V6", "REF6"):
        got[tag] = run(tag)
        for k, a in enumerate(got[tag]):
            if a is None:
                continue
            af, sd = float((a[..., 3] > 0).mean()), float(a[..., :3].std())
            (ok if af >= 0.5 and sd >= 3 else no)(
                f"{tag} frame {k} non-blank (alpha>0 frac {af:.3f}, RGB std {sd:.1f})")
    for v, ref in (("V1", "REF1"), ("V5", "REF5"), ("V6", "REF6")):
        pairs = [(x, y) for x, y in zip(got[v], got[ref]) if x is not None and y is not None]
        if not pairs:
            no(f"PARITY {v} vs {ref}: no frame pair captured"); continue
        dmax = max(float(np.abs(x[..., :3] - y[..., :3]).mean()) for x, y in pairs)
        (ok if dmax <= PARITY_TOL else no)(
            f"PARITY {v} vs single-chain {ref}: max mean |diff| {dmax:.2f} over {len(pairs)} frame pairs (tol {PARITY_TOL})")
    print(f"\nPY {PASS} PASS / {FAIL} FAIL", flush=True)
    sys.exit(0 if FAIL == 0 else 1)


if __name__ == "__main__":
    main()
