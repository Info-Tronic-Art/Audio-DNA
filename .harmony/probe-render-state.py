#!/usr/bin/env python3
"""probe-render-state.py -- REST/pixel half of .harmony/probe-render-state.sh (s-rta-0926b render lane).

The .sh owns launch / refuse / quit; this file talks to the already-running app on port 7070 and decodes
every captured PNG with PIL+numpy (never file hashes). Every render_frame response is checked; a failed
capture is a FAIL. The output dir is fresh per run.

usage: probe-render-state.py <root> <fresh-outdir> <media-dir> [row,row,...]
rows: r2_temporal r2_ring r4_clip_transform r4_clip_opacity r4_layer_effects r4_layer_transform r4_feedback
      r4_transition r5_hold r5_burst
      (s-rta-0926b render2) r1_temporal r1_control r1_ring r1_retrigger r1_counts r4_opaque_opacity
      r4_opaque_overlay r4_fxonly_persistent r4_fxonly_medialess r4_mask_skipped r4_empty_active_deck

Metric: d(X, Y) = mean |X - Y| over RGB, 0..255. non-blank = alpha>0 fraction >= 0.5 and RGB std >= 3.

R2 (persistent layers' state keys across decks): deck 0 (active) layer id 0 = A + fx, deck 1 layer id 0
  persistent = B + fx (covers the frame). Reference = same composition, deck 0 layer WITHOUT fx.
  PASS: 3 subject frames non-blank and d(subject, reference) <= tol (1.5).
  Calibration: 6e8f120 (pre-change) temporal 9.45 (persistent layer = 33% of deck 0's image), ring 29.39
  (persistent layer shows deck 0's image); fixed build 0.00 / 0.00.
R4 (persistent layer stages): base deck 0 layer id 0 Opaque = A; subject layer id 5 Transparent = B + stage.
  reference = subject as a NORMAL layer on deck 0; subject = the same layer PERSISTENT on deck 1;
  absent = the persistent layer without the stage.
  PASS: d(reference, absent) >= 5 (the fixture exercises the stage) and d(subject, reference) <= tol.
  Calibration (d(subject, reference)): 6e8f120 clip transform 28.26, clip opacity 9.62, layer effects
  228.25, layer transform 30.87, feedback 15.02; fixed build 0.00 on all five.
R4 transition: deck 1 persistent layer (OUT = A, IN = B, Dissolve 8 s); IN triggered on deck 1, deck left
  0.3 s later. 4 persistent frames at ~0.8..3.6 s: each ON the OUT->IN line (least-squares residual <= tol,
  0.03 < p < 0.97) and p rising by >= 0.15 overall; back on deck 1 after T + 2 s: frame == IN within tol.
  Calibration: 6e8f120 every persistent frame == IN (p = 1.00, a hard cut) and back on deck 1 p = 0.07
  (progress frozen while inactive); fixed build p 0.09 -> 0.44, resid <= 0.12, final d(f, IN) = 0.00.
R5 hold (deterministic): fresh layer id; col 1 = A + [Screen Split, Freeze 1.0]. Freeze 1.0 outputs its
  history, and a fresh history is cleared to transparent black, so the layer must stay transparent black.
  PASS: 3 frames with mean |RGBA| <= tol. Calibration: 6e8f120 (13, 13, 13, 255) = Screen Split's glClear
  grey held forever (the Freeze pass drew into framebuffer 0 on the creation frame; its target kept only
  the clear); fixed build (0, 0, 0, 0).
R5 burst (the visible symptom): 5 fresh layer ids; col 1 = A + [Freeze 0.5]; back-to-back captures across
  the 0 -> 1 switch. PASS: no captured frame after the switch is blank, and every attempt captured >= 8
  post-switch frames. Calibration: 6e8f120 the creation frame is fully transparent (0 / 663768 non-zero
  pixels) and was captured in 3/3 attempts; fixed build: no blank frame. This row can only be RED when a
  burst happens to capture the creation frame (it did 3/3 on 6e8f120); it cannot false-FAIL on a fixed
  build. R5 hold is the deterministic row.

--- s-rta-0926b render2 (ruling .harmony/.reports/s-rta-0926b/ruling-render-forks.md; base = 858acd1) ---
R1 (a crossfade's two clip chains share one history): one layer, OUT = col 0 = A + fx, IN = col 1 = B + fx,
  Wipe Left (transitionMode 26) over T = 8 s, so each region shows ONE clip: IN region x < (p - 0.12) W vs
  refB, OUT region x > (p + 0.12) W vs refA (p = elapsed / T; refs = each clip alone, settled). ratio =
  d(frame, ref) / d(refA, refB) in that region; a frame counts when both regions are >= 20 px wide and
  d(refA, refB) >= 5 there (else the ratio is noise). PASS: >= 4 counted frames between 1.0 s and 0.8 T and
  every one has both ratios <= 0.05.
  r1_temporal [Freeze 0.5] both. Calibration: base IN 0.34, OUT 0.32-0.33 (shared buffer fixed point 1/3);
    fixed 0.00 (the incoming clip inherits the layer's picture and loses it within ~7 frames).
  r1_control  OUT without an effect (never saves history): 0.00 on both builds -- guards the metric.
  r1_ring     [Frame Stutter 0 0] ("1 frame ago") both: base ~1.00 (each clip shows the other); fixed 0.00.
  r1_retrigger [Freeze 0.5] both; B triggered, A re-triggered 2 s into the fade (B -> A, A is now IN): the same
    bounds from 1 s after the re-trigger (IN vs refA, OUT vs refB). Base ~0.33; fixed 0.00 (second hand-over,
    the spare slot re-used).
R1 counts (memory bound, /api/state): a FRESH layer id, 6 columns alternating A/B, each [Screen Split 0.15
  0.15 0.25 0], faded through col 0 -> 5 with T = 1 s. /api/state frame_rings / temporal_buffers read before
  col 0 and after the last fade. PASS: frame_rings grew by exactly 2 (the layer's ring + its outgoing slot)
  and temporal_buffers by <= 2; peak_frame_time_ms read right after the FIRST fade (the spare ring is created
  then) <= 50 ms (logged). Calibration: base has no such fields (FAIL); per-clip history (option A) would
  grow by 6.
R4-opaque (persistent Opaque layer over the active deck): base deck 0 layer 0 Opaque = A.
  r4_opaque_opacity subject = deck 1 layer id 5 Opaque, persistent, Normal, opacity 0.5, clip B (covers the
    frame); reference = the same layer as a NORMAL Transparent (Alpha key) layer on deck 0 above A; full =
    the subject at opacity 1.0. PASS: d(reference, full) >= 5 and d(subject, reference) <= tol.
    Calibration (INFERRED): base subject = B (opacity ignored), d ~ 0.5 d(A,B) ~ 15; fixed 0.00.
  r4_opaque_overlay (guard, PASS on both builds -- pins "blend over, never hide the active deck"): the
    subject at opacity 1.0 with clip scale 0.5. PASS: outside the centre (x or y outside 0.2..0.8) the frame
    equals A-only (d <= tol) and inside 0.3..0.7 it does not (d >= 5).
R4-types: base deck 0 layer 0 Opaque = A; fx clip = media-less clip {mediaType 0, effects [Invert 1.0]}.
  r4_fxonly_persistent deck 1 layer id 5 FX Only (type 2), persistent, fx clip; reference = the same layer as
    a normal layer on deck 0 above A. PASS: d(reference, A-only) >= 5 and d(subject, reference) <= tol.
    Calibration: base subject = A (skipped), d = 228 (the r4b Invert stage); fixed 0.00.
  r4_fxonly_medialess the same with a Transparent layer (type 1) holding the media-less fx clip.
  r4_mask_skipped (guard, PASS on both builds): deck 1 layer id 5 Mask (type 4), persistent, clip B.
    PASS: the frame == A-only within tol (Mask persistent layers stay skipped).
R4 empty active deck (Harmony item, wave-1 found_not_fixed #1): deck 0 layer 0 = a black image, deck 1 layer
  id 5 Transparent persistent = B (covers the frame). reference = deck 0's black clip triggered (a black
  active deck); subject = the same composition with NO clip triggered on deck 0 (an empty active deck).
  Both captured at a locked 756x878 (render_frame width/height) so the viewport never depends on the
  fallback image. PASS: reference non-blank and d(subject, reference) <= tol. Calibration: base subject =
  the fallback (black), d ~ 19 (wave-1 r4g 19.46); fixed 0.00.
"""
import json, os, sys, threading, time

import numpy as np
import requests
from PIL import Image

A = "http://127.0.0.1:7070"
ROOT, OUT, MEDIA = sys.argv[1], sys.argv[2], sys.argv[3]
ONLY = set(sys.argv[4].split(",")) if len(sys.argv) > 4 and sys.argv[4] else None
FIX = json.load(open(os.path.join(ROOT, ".harmony", "probe-render-state.json")))
IMG_A = FIX["imageA"].replace("@MEDIA@", MEDIA)
IMG_B = FIX["imageB"].replace("@MEDIA@", MEDIA)
TOL = float(FIX["tol"])
PASS = FAIL = 0
S = requests.Session()


def ok(msg):
    global PASS; PASS += 1; print(f"PASS  {msg}", flush=True)


def no(msg):
    global FAIL; FAIL += 1; print(f"FAIL  {msg}", flush=True)


def fx(e):
    return {"name": e[0], "enabled": True, "bypassed": False, "dryWet": 1.0, "params": list(e[1:])}


def clip(cid, img, effects=(), **extra):
    c = {"name": f"c{cid}", "id": cid, "mediaType": 1, "mediaFile": img, "effects": [fx(e) for e in effects]}
    c.update(extra)
    return c


def layer(lid, clips, ltype=0, speed=0.0, **extra):
    l = {"name": f"L{lid}", "id": lid, "opacity": 1.0, "visible": True, "blendMode": 0, "type": ltype,
         "transitionSpeed": speed, "layerEffects": [], "clips": clips}
    if "layerEffects" in extra:
        extra = dict(extra); extra["layerEffects"] = [fx(e) for e in extra["layerEffects"]]
    l.update(extra)
    return l


def deck(did, layers, ncols=1):
    return {"name": f"D{did}", "id": did, "numColumns": ncols, "layers": layers}


def load(tag, decks):
    comp = {"name": "rstate-" + tag, "activeDeckIndex": 0, "masterOpacity": 1.0, "globalTransitionSpeed": 0.0,
            "decks": decks}
    path = os.path.join(OUT, f"comp_{tag}.json")
    json.dump(comp, open(path, "w"), indent=1)
    r = S.post(A + "/api/load_composition", json={"path": path}, timeout=10)
    good = r.ok and r.json().get("ok") is True
    if not good:
        no(f"{tag}: load_composition rejected: {r.text[:160]}")
    time.sleep(1.0)
    # s-rta-0926b render2 rig guard: nothing in this probe touches the global effect chain, so an enabled global
    # effect means another process drove the app (seen: a lane harness whose stub could not bind 7070).
    try:
        on = [e.get("name") for e in S.get(A + "/api/state", timeout=6).json().get("effects", []) if e.get("enabled")]
    except Exception:  # noqa: BLE001
        on = ["<state unreadable>"]
    if on:
        no(f"{tag}: rig -- the global effect chain has enabled effects nobody in this probe set: {on[:6]}")
    return good


def trig(li, col):
    S.post(A + "/api/trigger_clip", json={"layer": li, "column": col}, timeout=6)


def switch(d):
    S.post(A + "/api/switch_deck", json={"deck": d}, timeout=6)


def cap(name, size=None):
    p = os.path.join(OUT, name + ".png")
    req = {"output_path": p}
    if size:
        req.update(width=int(size[0]), height=int(size[1]))
    try:
        body = S.post(A + "/api/render_frame", json=req, timeout=20).json()
    except Exception as e:  # noqa: BLE001 -- any transport/JSON failure is a failed capture
        no(f"render_frame {name}: {e}")
        return None
    if not body.get("ok") or not os.path.isfile(p):
        no(f"render_frame {name}: response {body}")
        return None
    return decode(p)


def state():
    try:
        return S.get(A + "/api/state", timeout=6).json()
    except Exception as e:  # noqa: BLE001
        no(f"/api/state: {e}")
        return None


def decode(p):
    return np.asarray(Image.open(p).convert("RGBA")).astype(float)


def d(x, y):
    return float(np.abs(x[..., :3] - y[..., :3]).mean())


def nonblank(a):
    return float((a[..., 3] > 0).mean()) >= 0.5 and float(a[..., :3].std()) >= 3.0


def fit_line(f, x, y):
    dx = (y - x)[..., :3].ravel(); df = (f - x)[..., :3].ravel()
    den = float(dx @ dx)
    p = float(df @ dx) / den if den > 0 else 0.0
    return p, float(np.abs(f[..., :3] - (x[..., :3] + p * (y - x)[..., :3])).mean())


def persist_setup(active_layer_idx_list=(0,)):
    """deck 1's layer 0 active, then back on deck 0 with its layers triggered."""
    switch(1); time.sleep(0.5); trig(0, 0); time.sleep(0.5)
    switch(0); time.sleep(0.5)
    for li in active_layer_idx_list:
        trig(li, 0)


def r2(tag, fxs):
    frames = {}
    for key, fa in (("reference", []), ("subject", fxs)):
        d0 = deck(0, [layer(0, [clip(1, IMG_A, fa)])])
        d1 = deck(1, [layer(0, [clip(2, IMG_B, fxs)], persistent=True)])
        if not load(f"{tag}_{key}", [d0, d1]):
            return
        persist_setup(); time.sleep(2.0)
        frames[key] = [cap(f"{tag}_{key}_{i}") for i in range(3)]
        if any(f is None for f in frames[key]):
            no(f"{tag}: capture failed ({key})"); return
    ref = frames["reference"][0]
    for i, f in enumerate(frames["subject"]):
        de = d(f, ref); nb = nonblank(f)
        (ok if nb and de <= TOL else no)(
            f"{tag}: persistent layer id 0 (deck 1) keeps its own state vs deck 0's layer id 0 - frame {i} "
            f"non-blank={nb} d(subject, reference)={de:.2f} (tol {TOL})")


def r4(tag, spec):
    ce = spec.get("clip", {}); le = spec.get("layer", {})
    base = layer(0, [clip(1, IMG_A)])
    out = {}
    if not load(f"{tag}_reference", [deck(0, [base, layer(5, [clip(2, IMG_B, **ce)], ltype=1, **le)])]):
        return
    trig(0, 0); trig(1, 0); time.sleep(2.5)
    out["reference"] = cap(f"{tag}_reference")
    for key, lay in (("subject", layer(5, [clip(2, IMG_B, **ce)], ltype=1, persistent=True, **le)),
                     ("absent", layer(5, [clip(2, IMG_B)], ltype=1, persistent=True))):
        if not load(f"{tag}_{key}", [deck(0, [base]), deck(1, [lay])]):
            return
        persist_setup(); time.sleep(2.5)
        out[key] = cap(f"{tag}_{key}")
    if any(v is None for v in out.values()):
        no(f"{tag}: capture failed"); return
    dra = d(out["reference"], out["absent"]); dsr = d(out["subject"], out["reference"])
    print(f"      {tag}: d(reference, absent)={dra:.2f} d(subject, reference)={dsr:.2f} "
          f"d(subject, absent)={d(out['subject'], out['absent']):.2f}", flush=True)
    if dra < 5.0:
        no(f"{tag}: fixture does not exercise the stage (d(reference, absent)={dra:.2f} < 5)"); return
    (ok if nonblank(out["subject"]) and dsr <= TOL else no)(
        f"{tag}: persistent layer renders like the same layer on the active deck (d={dsr:.2f}, tol {TOL})")


def r4_transition(spec):
    T = float(spec["T"])
    base = layer(0, [clip(1, IMG_A, [["Invert", 1.0]])])
    subj = layer(5, [clip(2, IMG_A), clip(3, IMG_B)], ltype=1, persistent=True, speed=T)
    if not load("r4_transition", [deck(0, [base], ncols=2), deck(1, [subj], ncols=2)]):
        return
    trig(0, 0); time.sleep(0.5)
    switch(1); time.sleep(0.5); trig(0, 0); time.sleep(1.5)
    ref_out = cap("r4t_OUT")
    trig(0, 1); time.sleep(T + 1.5)
    ref_in = cap("r4t_IN")
    trig(0, 0); time.sleep(T + 1.5)
    if ref_out is None or ref_in is None:
        no("r4_transition: reference capture failed"); return
    trig(0, 1); t0 = time.time()
    time.sleep(0.3); switch(0); time.sleep(0.4)
    ps, bad = [], []
    for k in range(4):
        f = cap(f"r4t_persist_mid{k}"); tt = time.time() - t0
        if f is None:
            bad.append(f"frame {k} capture failed"); continue
        p, res = fit_line(f, ref_out, ref_in); ps.append(p)
        print(f"      r4_transition: t={tt:.2f}s p={p:.2f} residual={res:.2f} d(f,IN)={d(f, ref_in):.2f}", flush=True)
        if not (res <= TOL and 0.03 < p < 0.97):
            bad.append(f"t={tt:.2f}s p={p:.2f} res={res:.2f}")
        time.sleep(0.8)
    (ok if len(ps) == 4 and not bad else no)(
        f"r4_transition: persistent layer mid-crossfade frames lie between OUT and IN ({4 - len(bad)}/4): {bad[:3]}")
    rising = len(ps) >= 2 and ps[-1] - ps[0] >= 0.15
    (ok if rising else no)(f"r4_transition: crossfade keeps advancing while the deck is inactive "
                           f"(p {[round(p, 2) for p in ps]})")
    time.sleep(max(0.0, T + 2.0 - (time.time() - t0)))
    switch(1); time.sleep(0.3)
    f = cap("r4t_back_on_deck1")
    if f is None:
        return
    df = d(f, ref_in)
    (ok if df <= TOL else no)(f"r4_transition: back on its deck after T + 2 s the crossfade is complete "
                              f"(d(f, IN)={df:.2f}, tol {TOL})")


def r5_hold(spec):
    lid = int(spec["layerId"])
    if not load("r5_hold", [deck(0, [layer(lid, [clip(1, IMG_A), clip(2, IMG_A, spec["fx"])])], ncols=2)]):
        return
    trig(0, 0); time.sleep(1.0)
    pre = cap("r5_hold_pre")
    trig(0, 1); time.sleep(1.0)
    for i in range(3):
        f = cap(f"r5_hold_{i}")
        if f is None or pre is None:
            continue
        m = float(np.abs(f).mean())
        print(f"      r5_hold: frame {i} mean RGBA = {tuple(round(float(f[..., c].mean()), 1) for c in range(4))}",
              flush=True)
        (ok if nonblank(pre) and m <= TOL else no)(
            f"r5_hold: Freeze 1.0 on a fresh layer holds its cleared history (transparent black), never the pass "
            f"target's glClear colour - frame {i} mean|RGBA|={m:.2f} (tol {TOL})")
        time.sleep(0.3)


def r5_burst(spec):
    blank_hits, short = [], []
    for lid in spec["layerIds"]:
        if not load(f"r5_burst_{lid}", [deck(0, [layer(lid, [clip(1, IMG_A), clip(2, IMG_A, spec["fx"])])], ncols=2)]):
            return
        trig(0, 0); time.sleep(1.5)
        shots, stop = [], [False]

        def burst():
            k = 0
            while not stop[0] and k < 60:
                p = os.path.join(OUT, f"r5_burst_{lid}_b{k:02d}.png")
                try:
                    good = S.post(A + "/api/render_frame", json={"output_path": p}, timeout=20).json().get("ok")
                except Exception:  # noqa: BLE001
                    good = False
                shots.append((time.time(), p if good else None)); k += 1
        th = threading.Thread(target=burst); th.start()
        time.sleep(0.35); t_trig = time.time(); trig(0, 1)
        time.sleep(0.6); stop[0] = True; th.join()
        post = [(t, p) for t, p in shots if t >= t_trig]
        if any(p is None for _, p in shots):
            no(f"r5_burst id={lid}: a render_frame capture failed")
        if len(post) < 8:
            short.append(lid)
        for i, (t, p) in enumerate(post):
            if p is None:
                continue
            f = decode(p)
            if not nonblank(f):
                blank_hits.append(f"id={lid} +{t - t_trig:.3f}s alpha>0={float((f[..., 3] > 0).mean()):.2f} "
                                  f"rgb={f[..., :3].mean():.2f}")
        print(f"      r5_burst id={lid}: {len(post)} frames after the switch", flush=True)
    (ok if not short else no)(f"r5_burst: every attempt captured >= 8 frames after the switch (short: {short})")
    (ok if not blank_hits else no)(
        f"r5_burst: no blank frame when a temporal effect first runs on a layer ({len(spec['layerIds'])} fresh "
        f"layers): {blank_hits[:3]}")


# ------------------------------------------------------------------ s-rta-0926b render2 rows
def r1_wipe(tag, spec):
    cfg = FIX["r1"]; T = float(cfg["T"]); bound = float(cfg["bound"]); mind = float(cfg["minRegionD"])
    clips = [clip(1, IMG_A, spec["out"]), clip(2, IMG_B, spec["in"])]
    if not load(tag, [deck(0, [layer(0, clips, speed=T, transitionMode=26)], ncols=2)]):
        return
    trig(0, 1); time.sleep(T + 1.5)
    refB = cap(f"{tag}_refB")
    trig(0, 0); time.sleep(T + 1.5)
    refA = cap(f"{tag}_refA")
    if refA is None or refB is None:
        no(f"{tag}: reference capture failed"); return
    trig(0, 1); t0 = time.time()
    in_ref, out_ref, what = refB, refA, "IN=B OUT=A"
    if spec.get("retrigger"):
        time.sleep(float(spec["retrigger"])); trig(0, 0); t0 = time.time()
        in_ref, out_ref, what = refA, refB, "re-triggered: IN=A OUT=B"
    time.sleep(1.0)
    W = refA.shape[1]; counted, bad, k = 0, [], 0
    while time.time() - t0 < 0.8 * T:
        ta = time.time(); f = cap(f"{tag}_mid{k:02d}"); tt = (ta + time.time()) / 2 - t0; k += 1
        if f is None:
            bad.append(f"frame {k} capture failed"); continue
        pp = tt / T
        xin = int(max(0.0, pp - 0.12) * W); xout = int(min(1.0, pp + 0.12) * W)
        if xin < 20 or W - xout < 20:
            time.sleep(0.4); continue
        dab_in = d(refA[:, :xin], refB[:, :xin]); dab_out = d(refA[:, xout:], refB[:, xout:])
        rin = d(f[:, :xin], in_ref[:, :xin]) / max(dab_in, 1e-6)
        rout = d(f[:, xout:], out_ref[:, xout:]) / max(dab_out, 1e-6)
        use = dab_in >= mind and dab_out >= mind
        print(f"      {tag}: t={tt:.2f}s p~{pp:.2f} IN x<{xin} ratio {rin:.2f} (d(A,B) {dab_in:.2f})  "
              f"OUT x>{xout} ratio {rout:.2f} (d(A,B) {dab_out:.2f}){'' if use else '  [not counted]'}", flush=True)
        if use:
            counted += 1
            if rin > bound or rout > bound:
                bad.append(f"t={tt:.2f}s in {rin:.2f} out {rout:.2f}")
        time.sleep(0.4)
    (ok if counted >= 4 and not bad else no)(
        f"{tag}: during a crossfade each clip keeps its own history ({what}; {counted} counted frames, "
        f"ratios <= {bound}): {bad[:3]}")


def r1_counts(spec):
    lid = int(spec["layerId"]); T = float(spec["T"])
    clips = [clip(10 + k, IMG_A if k % 2 == 0 else IMG_B, spec["fx"]) for k in range(6)]
    if not load("r1_counts", [deck(0, [layer(lid, clips, speed=T)], ncols=6)]):
        return
    s0 = state()
    trig(0, 0); time.sleep(1.5)
    s1 = state()                      # resets the peak (first use of the layer's ring)
    trig(0, 1); time.sleep(0.6)
    s2 = state()                      # peak across the FIRST fade's start (the spare ring is created)
    time.sleep(T)
    for c in range(2, 6):
        trig(0, c); time.sleep(T + 0.3)
    s3 = state()
    keys = ("frame_rings", "temporal_buffers", "peak_frame_time_ms")
    if any(sx is None or any(kk not in sx for kk in keys) for sx in (s0, s1, s2, s3)):
        no(f"r1_counts: /api/state has no {keys} fields ({sorted((s0 or {}).keys())[:12]})"); return
    dr = s3["frame_rings"] - s0["frame_rings"]; dtb = s3["temporal_buffers"] - s0["temporal_buffers"]
    print(f"      r1_counts: frame_rings {s0['frame_rings']} -> {s3['frame_rings']} (+{dr}), temporal_buffers "
          f"{s0['temporal_buffers']} -> {s3['temporal_buffers']} (+{dtb}); peak_frame_time_ms first use "
          f"{s1['peak_frame_time_ms']:.2f}, first fade {s2['peak_frame_time_ms']:.2f}", flush=True)
    (ok if dr == 2 and dtb <= 2 else no)(
        f"r1_counts: 5 fades on one layer (Screen Split on every clip) hold exactly 2 rings (+{dr}) and <= 2 "
        f"temporal buffers (+{dtb})")
    pk = float(s2["peak_frame_time_ms"])
    (ok if pk <= float(spec["peakMaxMs"]) else no)(
        f"r1_counts: longest frame across the first fade (spare ring created) {pk:.2f} ms <= {spec['peakMaxMs']} ms")


def base_only():
    return layer(0, [clip(1, IMG_A)])


def a_only(tag):
    if not load(f"{tag}_Aonly", [deck(0, [base_only()])]):
        return None
    trig(0, 0); time.sleep(2.0)
    return cap(f"{tag}_Aonly")


def r4_opaque_opacity(spec):
    op = float(spec["opacity"]); out = {}
    if not load("r4oo_reference", [deck(0, [base_only(), layer(5, [clip(2, IMG_B)], ltype=1, opacity=op)])]):
        return
    trig(0, 0); trig(1, 0); time.sleep(2.5)
    out["reference"] = cap("r4oo_reference")
    for key, o in (("subject", op), ("full", 1.0)):
        subj = layer(5, [clip(2, IMG_B)], ltype=0, persistent=True, opacity=o)
        if not load(f"r4oo_{key}", [deck(0, [base_only()]), deck(1, [subj])]):
            return
        persist_setup(); time.sleep(2.5)
        out[key] = cap(f"r4oo_{key}")
    if any(v is None for v in out.values()):
        no("r4_opaque_opacity: capture failed"); return
    drf = d(out["reference"], out["full"]); dsr = d(out["subject"], out["reference"])
    print(f"      r4_opaque_opacity: d(reference, full)={drf:.2f} d(subject, reference)={dsr:.2f} "
          f"d(subject, full)={d(out['subject'], out['full']):.2f}", flush=True)
    if drf < 5.0:
        no(f"r4_opaque_opacity: fixture does not exercise opacity (d(reference, full)={drf:.2f} < 5)"); return
    (ok if nonblank(out["subject"]) and dsr <= TOL else no)(
        f"r4_opaque_opacity: a persistent Opaque layer at opacity {op} blends over the active deck like the "
        f"same layer at that opacity on the deck (d={dsr:.2f}, tol {TOL})")


def r4_opaque_overlay(spec):
    a = a_only("r4ov")
    subj = layer(5, [clip(2, IMG_B, scale=float(spec["overlayScale"]))], ltype=0, persistent=True)
    if a is None or not load("r4ov_subject", [deck(0, [base_only()]), deck(1, [subj])]):
        no("r4_opaque_overlay: setup failed"); return
    persist_setup(); time.sleep(2.5)
    f = cap("r4ov_subject")
    if f is None:
        return
    H, W = f.shape[:2]; yy, xx = np.mgrid[0:H, 0:W]
    outside = (xx < 0.2 * W) | (xx > 0.8 * W) | (yy < 0.2 * H) | (yy > 0.8 * H)
    centre = (xx > 0.3 * W) & (xx < 0.7 * W) & (yy > 0.3 * H) & (yy < 0.7 * H)
    dout = float(np.abs(f[..., :3] - a[..., :3])[outside].mean())
    din = float(np.abs(f[..., :3] - a[..., :3])[centre].mean())
    print(f"      r4_opaque_overlay: outside centre d(f, A)={dout:.2f}; centre d(f, A)={din:.2f}", flush=True)
    (ok if dout <= TOL and din >= 5.0 else no)(
        f"r4_opaque_overlay: a persistent Opaque layer never hides the active deck (outside d={dout:.2f} "
        f"<= {TOL}, centre d={din:.2f} >= 5)")


def fx_clip(cid, effects):
    return {"name": f"c{cid}", "id": cid, "mediaType": 0, "effects": [fx(e) for e in effects]}


def r4_fxonly(tag, spec, ltype):
    fc = fx_clip(2, spec["fx"])
    a = a_only(tag)
    if a is None or not load(f"{tag}_reference", [deck(0, [base_only(), layer(5, [fc], ltype=ltype)])]):
        no(f"{tag}: setup failed"); return
    trig(0, 0); trig(1, 0); time.sleep(2.5)
    ref = cap(f"{tag}_reference")
    if not load(f"{tag}_subject", [deck(0, [base_only()]), deck(1, [layer(5, [fc], ltype=ltype, persistent=True)])]):
        return
    persist_setup(); time.sleep(2.5)
    subj = cap(f"{tag}_subject")
    if ref is None or subj is None:
        no(f"{tag}: capture failed"); return
    dra = d(ref, a); dsr = d(subj, ref)
    print(f"      {tag}: d(reference, A-only)={dra:.2f} d(subject, reference)={dsr:.2f} d(subject, A-only)="
          f"{d(subj, a):.2f}", flush=True)
    if dra < 5.0:
        no(f"{tag}: fixture does not exercise the effect (d(reference, A)={dra:.2f} < 5)"); return
    (ok if dsr <= TOL else no)(
        f"{tag}: a persistent layer (type {ltype}) with a media-less effect clip applies its effects over the "
        f"active deck like the same layer on the deck (d={dsr:.2f}, tol {TOL})")


def r4_mask_skipped(spec):
    a = a_only("r4mask")
    if a is None or not load("r4mask_subject", [deck(0, [base_only()]),
                                                deck(1, [layer(5, [clip(2, IMG_B)], ltype=4, persistent=True)])]):
        no("r4_mask_skipped: setup failed"); return
    persist_setup(); time.sleep(2.5)
    f = cap("r4mask_subject")
    if f is None:
        return
    df = d(f, a)
    (ok if df <= TOL else no)(f"r4_mask_skipped: a persistent Mask layer stays skipped (frame == A-only, d={df:.2f}, "
                              f"tol {TOL})")


def r4_empty_active_deck(spec):
    lock = spec["lock"]
    black = os.path.join(OUT, "black.png")
    Image.new("RGBA", Image.open(IMG_A).size, (0, 0, 0, 255)).save(black)
    comp = [deck(0, [layer(0, [clip(1, black)])]), deck(1, [layer(5, [clip(2, IMG_B)], ltype=1, persistent=True)])]
    if not load("r4e_reference", comp):
        return
    persist_setup(); time.sleep(2.0)              # deck 0's black clip triggered: a BLACK active deck
    ref = cap("r4e_reference", lock)
    if not load("r4e_subject", comp):
        return
    persist_setup(active_layer_idx_list=()); time.sleep(2.0)   # nothing triggered on deck 0: an EMPTY active deck
    subj = cap("r4e_subject", lock)
    if ref is None or subj is None:
        no("r4_empty_active_deck: capture failed"); return
    dsr = d(subj, ref)
    print(f"      r4_empty_active_deck: reference rgb {ref[..., :3].mean():.2f}, subject rgb {subj[..., :3].mean():.2f}, "
          f"d(subject, reference)={dsr:.2f}", flush=True)
    (ok if nonblank(ref) and dsr <= TOL else no)(
        f"r4_empty_active_deck: persistent layers render over an EMPTY active deck exactly as over a black one "
        f"(d={dsr:.2f}, tol {TOL})")


def main():
    rows = [
        ("r2_temporal", lambda: r2("r2_temporal", FIX["r2"]["temporal"]["fx"])),
        ("r2_ring", lambda: r2("r2_ring", FIX["r2"]["ring"]["fx"])),
    ]
    for k in ("clip_transform", "clip_opacity", "layer_effects", "layer_transform", "feedback"):
        rows.append((f"r4_{k}", (lambda k=k: r4(f"r4_{k}", FIX["r4"][k]))))
    rows += [("r4_transition", lambda: r4_transition(FIX["r4_transition"])),
             ("r5_hold", lambda: r5_hold(FIX["r5_hold"])),
             ("r5_burst", lambda: r5_burst(FIX["r5_burst"]))]
    for k in ("temporal", "control", "ring", "retrigger"):
        rows.append((f"r1_{k}", (lambda k=k: r1_wipe(f"r1_{k}", FIX["r1"][k]))))
    rows += [("r1_counts", lambda: r1_counts(FIX["r1"]["counts"])),
             ("r4_opaque_opacity", lambda: r4_opaque_opacity(FIX["r4_opaque"])),
             ("r4_opaque_overlay", lambda: r4_opaque_overlay(FIX["r4_opaque"])),
             ("r4_fxonly_persistent", lambda: r4_fxonly("r4_fxonly_persistent", FIX["r4_fxonly"], 2)),
             ("r4_fxonly_medialess", lambda: r4_fxonly("r4_fxonly_medialess", FIX["r4_fxonly"], 1)),
             ("r4_mask_skipped", lambda: r4_mask_skipped(FIX["r4_fxonly"])),
             ("r4_empty_active_deck", lambda: r4_empty_active_deck(FIX["r4_empty_active_deck"]))]
    for name, fn in rows:
        if ONLY is None or name in ONLY:
            print(f"--- {name}", flush=True)
            fn()
    print(f"\nPY {PASS} PASS / {FAIL} FAIL", flush=True)
    sys.exit(0 if FAIL == 0 else 1)


if __name__ == "__main__":
    main()
