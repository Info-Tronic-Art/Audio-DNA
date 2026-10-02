#!/usr/bin/env python3
"""probe-render-state.py -- REST/pixel half of .harmony/probe-render-state.sh (s-rta-0926b render lane).

The .sh owns launch / refuse / quit; this file talks to the already-running app on port 7070 and decodes
every captured PNG with PIL+numpy (never file hashes). Every render_frame response is checked; a failed
capture is a FAIL. The output dir is fresh per run.

usage: probe-render-state.py <root> <fresh-outdir> <media-dir> [row,row,...]
rows: r5_hold r5_burst
      (s-rta-0926b render2) r1_temporal r1_control r1_ring r1_retrigger r1_counts r1_cells
      (bf9 Stage P) a4_feedback a4_fxonly a4_fxonly_medialess p_flag_ignored p_flag_ignored_empty p_api_no_field
      p_ignore_column
env RSTATE_REF=<dir>: a previous run's out dir (the BASE arm); the a4_* rows then also apply bar (ii).

Metric: d(X, Y) = mean |X - Y| over RGB, 0..255. non-blank = alpha>0 fraction >= 0.5 and RGB std >= 3.

RETIRED with the Persistent layer feature (bf9 Stage P, s-rta-1002b; ruling-bf9 amendment 12(a)): rows r2_temporal
  r2_ring r4_clip_transform r4_clip_opacity r4_layer_effects r4_layer_transform r4_feedback r4_transition
  r4_opaque_opacity r4_opaque_overlay r4_fxonly_persistent r4_fxonly_medialess r4_mask_skipped r4_empty_active_deck
  and fixture keys r2 r4 r4_transition r4_opaque r4_fxonly r4_empty_active_deck. Their subject (a persistent layer
  drawn while another deck is shown) no longer exists. Where each stage they touched is covered now (ruling-bf9
  R-F9 / R-F10; this map replaces plan-bf9 R2):
    clip transform    probe-crossfade.json:38 (scale 0.5)              -- active deck
    clip opacity      probe-crossfade.json:44 (clipOpacity 0.5)        -- active deck
    layer transform   probe-crossfade.json:59-60, probe-fitmode.py:340 (layerScale 0.5)
    layer effects     probe-effects-parity (V1 / V5 / V6), probe-crossfade
    clip transitions  probe-crossfade (a-f, k-l); an off-screen fade: probe-deck-clock d_fade_finishes
    layer feedback    a4_feedback (below)                              -- NO other live coverage before
    FX Only layer     a4_fxonly (below)                                -- NO other live coverage before
    media-less effect clip on a Transparent layer: a4_fxonly_medialess -- NO other live coverage before
    the per-deck history key at the GL call sites (r2_*): successor = bf9b gate K1t
    r4_opaque_*, r4_mask_skipped, r4_empty_active_deck, r4_transition tested persistent-only behaviour: no successor.
  The R4 rows' only active-deck assertion was "the fixture exercises the stage" (d(reference, absent) >= 5).

R5 hold (deterministic): fresh layer id; col 1 = A + [Screen Split, Freeze 1.0]. Freeze 1.0 outputs its
  history, and a fresh history is cleared to transparent black, so the layer must stay transparent black.
  PASS: 3 frames with mean |RGBA| <= tol. Calibration: 6e8f120 (13, 13, 13, 255) = Screen Split's glClear
  grey held forever (the Freeze pass drew into framebuffer 0 on the creation frame; its target kept only
  the clear); fixed build (0, 0, 0, 0).
R5 burst (the visible symptom): 5 fresh layer ids; col 1 = A + [Freeze 0.5]; back-to-back captures across
  the 0 -> 1 switch, for postSwitchS (2.0 s) after it. PASS: no captured frame after the switch is blank,
  and every attempt captured >= 8 post-switch frames. The window was 0.6 s until the plan4 canvas: a capture
  is now the full 1920x1080 canvas (~100-118 ms here, 150-180 ms under load), so 0.6 s held only 6-7 (Harmony
  ruling, s-rta-0926b canvas merge: widen the window, keep the 8-frame bar). Calibration: 6e8f120 the
  creation frame is fully transparent (0 / 663768 non-zero pixels) and was captured in 3/3 attempts; fixed
  build: no blank frame. This row can only be RED when a burst happens to capture the creation frame (it did
  3/3 on 6e8f120); it cannot false-FAIL on a fixed build. R5 hold is the deterministic row.

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
  and temporal_buffers by <= 2; peak_frame_time_ms read right after the layer's FIRST USE (its ring is
  created) and right after the FIRST fade (the spare ring is created) each <= peakMaxMs = 16.7 ms (one 60 Hz
  frame; s-rta-0927 plan-renderperf C1, was 50 ms on the first fade only); the second fade's peak is printed
  for context (no bar). Quiet rule: waits until no compiler runs; the load average is printed. Calibration:
  base has no such fields (FAIL); per-clip history (option A) would grow by 6. Peaks (s-rta-0927 renderperf):
  dc7adf9 (480 cells created in one frame), this row alone on a fresh app: first use 37.87, first fade 39.42 ms
  (FAIL); C1 (cells created on first write): 1.35-1.66 ms in 3 full runs (both images already uploaded by earlier
  rows) and 9.02 / 11.75 ms with the row alone on a fresh app -- that residual is the first upload of image A / B
  (CompositorEngine::loadKeyImage decodes on the GL thread), not the ring.
R1 cells (s-rta-0928 R5 = C4 of plan-renderperf): a fresh layer id, col 0 = A + [Screen Split 0.15 0.15 0.25 0]; s0;
  trig 0; 0.3 s s1; 1.0 s s2. PASS: (1) 1 <= cells(s1) - cells(s0) < 480; (2) the growth s1 -> s2 is > 0 and at most
  1.3 x (t2 - t1) x fps + 5 (one cell per ring per frame); (3) cells(s2) - cells(s0) <= 480 x the new rings.
  Calibration: main has no frame_ring_cells field (FAIL); lane: .harmony/.reports/s-rta-0928/renderleft.md.
--- bf9 Stage P (s-rta-1002b; ruling .harmony/.reports/s-rta-1002b/ruling-bf9.md amendment 12, gate G5) ---
A4 (active-deck stages, deck 0 only): layer 0 Opaque A; layer id 5 = a4_feedback: Transparent B with the a4.feedback
  values | a4_fxonly: FX Only (type 2) holding the media-less fx clip {mediaType 0, effects [Invert 1.0]} |
  a4_fxonly_medialess: Transparent (type 1) holding that clip. Steps: trig(0, 0), trig(1, 0), settle 2.5 s, cap1
  (<row>_cap1.png), wait 0.5 s, cap2 (<row>_cap2.png); noise = d(cap1, cap2).
  absent = a_only() for the two fx rows; for a4_feedback the same file with feedbackEnabled false (same steps, cap1).
  (i)  PASS iff d(cap1, absent) >= 5 (the fixture exercises the stage).
  (ii) only when RSTATE_REF=<dir> is set (the BRANCH arm; <dir> = the BASE arm's out dir): PASS iff
       d(cap1, REF cap1) <= max(1.5, 4 x the REF run's noise) (REF noise = d(REF cap1, REF cap2)).
P (the Persistent layer feature is removed; floor/tol = tol 1.5):
  p_flag_ignored: subject = deck 0 layer 0 Opaque A; deck 1 layer id 5 Transparent, "persistent": true, clip B
    (covers the frame); control = the same file without the key. Each: load, persist_setup(), settle 2.5 s, cap.
    VALID iff d(control, a_only("pfi")) <= tol. PASS iff nonblank(subject) and d(subject, control) <= tol.
    BASE expected: d(subject, control) ~ d(B, A) (FAIL: B drawn over A). Also prints /api/state frame_time_ms and
    peak_frame_time_ms after the subject capture (ruling-bf9 G6: INFO, no bar).
  p_flag_ignored_empty: subject = deck 0 one Opaque layer whose clip A is never triggered; deck 1 as above; control
    the same without the key. Each: load, persist_setup(active_layer_idx_list=()), settle 2.0 s, cap at lock 756x878.
    VALID iff mean RGB(control) <= 2.0 (switch(0)'s refresh cleared the fallback: black). PASS iff d(subject,
    control) <= tol. BASE expected: black + B (FAIL).
  p_api_no_field: the p_flag_ignored subject loaded; no decks[].layers[] object of GET /api/composition has
    "persistent", and every one has id, visible and activeClipColumn. BASE: the key is present (FAIL).
  p_ignore_column: deck 0 = 2 layers x 2 image columns, layer 1 "ignoreColumnTrigger": true. trigger_clip(1, 0);
    POST /api/trigger_column {"column": 1}; 0.5 s later layer 0 activeClipColumn == 1 and layer 1 == 0. PASS on both
    arms (Ignore Column Trigger unchanged).
"""
import json, os, subprocess, sys, threading, time

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
# One fresh connection per request, never a pooled keep-alive one (s-rta-0927 c1-state-fix). The app's cpp-httplib
# server closes a connection idle for 5 s (CPPHTTPLIB_KEEPALIVE_TIMEOUT_SECOND) -- shutdown, then it drains and
# DISCARDS whatever request arrives in that instant. A reused connection can then fail with RemoteDisconnected
# (~1 run in 3, on the pre-C1 app too), and requests never retries it. Evidence:
# .harmony/.reports/s-rta-0927/c1-state-fix.md.
S.headers["Connection"] = "close"


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
    """render_frame into the fresh dir. Deletes any pre-existing file at p first, then requires the
    response's ok:true, the file to exist, AND its mtime to be at/after the request -- never decode a
    PNG this call did not write (probe hygiene: notebook.md s-rta-0926, "a render probe must check
    render_frame's JSON response and delete old PNGs first"; ported from probe-effects-parity.py's
    cap(), probehygiene lane commit 4f9e78d)."""
    p = os.path.join(OUT, name + ".png")
    if os.path.exists(p):
        os.remove(p)
    req = {"output_path": p}
    if size:
        req.update(width=int(size[0]), height=int(size[1]))
    t0 = time.time()
    try:
        body = S.post(A + "/api/render_frame", json=req, timeout=20).json()
    except Exception as e:  # noqa: BLE001 -- any transport/JSON failure is a failed capture
        no(f"render_frame {name}: {e}")
        return None
    if not body.get("ok"):
        no(f"render_frame {name}: response {body}")
        return None
    if not os.path.isfile(p):
        no(f"render_frame {name}: ok:true but no file written at {p}")
        return None
    mt = os.path.getmtime(p)
    if mt < t0 - 0.01:
        no(f"render_frame {name}: ok:true but {p} mtime {mt:.3f} predates the request {t0:.3f} "
           f"(stale file from a previous run -- never decoded)")
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


def persist_setup(active_layer_idx_list=(0,)):
    """deck 1's layer 0 active, then back on deck 0 with its layers triggered."""
    switch(1); time.sleep(0.5); trig(0, 0); time.sleep(0.5)
    switch(0); time.sleep(0.5)
    for li in active_layer_idx_list:
        trig(li, 0)


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
        time.sleep(spec["postSwitchS"]); stop[0] = True; th.join()
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


def wait_no_compiler(tag, limit_s=1800):
    """Rig rule: perf rows run only when no compiler is running (copied from probe-canvas.py)."""
    t0 = time.time()
    while True:
        # macOS pgrep takes a regex: a bare "clang++" is an invalid pattern (error, empty stdout = never "busy").
        busy = [n for n in ("clang", r"clang\+\+") if subprocess.run(["pgrep", "-x", n], capture_output=True).stdout.strip()]
        if not busy:
            return True
        if time.time() - t0 > limit_s:
            no(f"{tag}: a compiler ({busy}) kept running for {limit_s} s -- perf not measured")
            return False
        print(f"      {tag}: waiting for {busy} to finish (load avg %.2f %.2f %.2f)" % os.getloadavg(), flush=True)
        time.sleep(20)


def r1_counts(spec):
    lid = int(spec["layerId"]); T = float(spec["T"])
    clips = [clip(10 + k, IMG_A if k % 2 == 0 else IMG_B, spec["fx"]) for k in range(6)]
    if not load("r1_counts", [deck(0, [layer(lid, clips, speed=T)], ncols=6)]):
        return
    if not wait_no_compiler("r1_counts"):   # the peak bars are perf rows (s-rta-0927 plan-renderperf C1)
        return
    s0 = state()
    trig(0, 0); time.sleep(1.5)
    s1 = state()                      # peak across the layer's FIRST USE of Screen Split (its ring is created)
    trig(0, 1); time.sleep(0.6)
    s2 = state()                      # peak across the FIRST fade's start (the spare ring is created)
    time.sleep(T)
    trig(0, 2); time.sleep(0.6)
    s2b = state()                     # the SECOND fade's start: the same work without creation (context, no bar)
    time.sleep(T)
    for c in range(3, 6):
        trig(0, c); time.sleep(T + 0.3)
    s3 = state()
    keys = ("frame_rings", "temporal_buffers", "peak_frame_time_ms")
    if any(sx is None or any(kk not in sx for kk in keys) for sx in (s0, s1, s2, s2b, s3)):
        no(f"r1_counts: /api/state has no {keys} fields ({sorted((s0 or {}).keys())[:12]})"); return
    dr = s3["frame_rings"] - s0["frame_rings"]; dtb = s3["temporal_buffers"] - s0["temporal_buffers"]
    print(f"      r1_counts: frame_rings {s0['frame_rings']} -> {s3['frame_rings']} (+{dr}), temporal_buffers "
          f"{s0['temporal_buffers']} -> {s3['temporal_buffers']} (+{dtb}); peak_frame_time_ms first use "
          f"{s1['peak_frame_time_ms']:.2f}, first fade {s2['peak_frame_time_ms']:.2f}, second fade (no creation) "
          f"{s2b['peak_frame_time_ms']:.2f}; load avg %.2f %.2f %.2f" % os.getloadavg(), flush=True)
    (ok if dr == 2 and dtb <= 2 else no)(
        f"r1_counts: 5 fades on one layer (Screen Split on every clip) hold exactly 2 rings (+{dr}) and <= 2 "
        f"temporal buffers (+{dtb})")
    bar = float(spec["peakMaxMs"])
    pu = float(s1["peak_frame_time_ms"])
    (ok if pu <= bar else no)(
        f"r1_counts: longest frame across the layer's first use (its ring created) {pu:.2f} ms <= {bar} ms")
    pk = float(s2["peak_frame_time_ms"])
    (ok if pk <= bar else no)(
        f"r1_counts: longest frame across the first fade (spare ring created) {pk:.2f} ms <= {bar} ms")


def r1_cells(spec):
    """s-rta-0928 R5 (C4): /api/state frame_ring_cells = ring cells created so far, over every ring. Cells are created
    on first write (one per ring per frame), so a fresh ring's count grows at most by the frames rendered."""
    lid = int(spec["layerId"])
    if not load("r1_cells", [deck(0, [layer(lid, [clip(10, IMG_A, spec["fx"])])])]):
        return
    s0 = state()
    trig(0, 0); time.sleep(0.3)
    s1 = state(); t1 = time.time()
    time.sleep(1.0)
    s2 = state(); t2 = time.time()
    if any(sx is None or "frame_ring_cells" not in sx for sx in (s0, s1, s2)):
        no(f"r1_cells: /api/state has no frame_ring_cells ({sorted((s0 or {}).keys())[:12]})"); return
    c1 = s1["frame_ring_cells"] - s0["frame_ring_cells"]; c2 = s2["frame_ring_cells"] - s0["frame_ring_cells"]
    fps = max(float(s1.get("fps", 0) or 0), float(s2.get("fps", 0) or 0))
    rings = s2["frame_rings"] - s0["frame_rings"]
    hi = c1 + 1.3 * (t2 - t1) * fps + 5
    print(f"      r1_cells: cells +{c1} after 0.3 s, +{c2} after 1.3 s (fps {fps:.1f}, bound {hi:.0f}); rings +{rings}",
          flush=True)
    (ok if 1 <= c1 < 480 else no)(f"r1_cells: a fresh ring creates cells on first write (+{c1} in 0.3 s, 1..479)")
    (ok if c1 < c2 <= hi else no)(f"r1_cells: at most one cell per ring per frame (+{c1} < +{c2} <= {hi:.0f})")
    (ok if c2 <= 480 * rings else no)(f"r1_cells: never more than 480 cells per ring (+{c2} <= 480 x {rings})")


def base_only():
    return layer(0, [clip(1, IMG_A)])


def a_only(tag):
    if not load(f"{tag}_Aonly", [deck(0, [base_only()])]):
        return None
    trig(0, 0); time.sleep(2.0)
    return cap(f"{tag}_Aonly")


def fx_clip(cid, effects):
    return {"name": f"c{cid}", "id": cid, "mediaType": 0, "effects": [fx(e) for e in effects]}


def comp():
    try:
        return S.get(A + "/api/composition", timeout=6).json()
    except Exception as e:  # noqa: BLE001
        no(f"/api/composition: {e}")
        return None


# ------------------------------------------------------------------ bf9 Stage P rows
REF = os.environ.get("RSTATE_REF", "")


def a4(tag, kind):
    cfg = FIX["a4"]
    if kind == "feedback":
        subj = layer(5, [clip(2, IMG_B)], ltype=1, **cfg["feedback"])
    else:
        subj = layer(5, [fx_clip(2, cfg["fx"])], ltype=(2 if kind == "fxonly" else 1))
    if not load(f"{tag}", [deck(0, [base_only(), subj])]):
        return
    trig(0, 0); trig(1, 0); time.sleep(2.5)
    c1 = cap(f"{tag}_cap1"); time.sleep(0.5)
    c2 = cap(f"{tag}_cap2")
    if kind == "feedback":
        off = layer(5, [clip(2, IMG_B)], ltype=1, **dict(cfg["feedback"], feedbackEnabled=False))
        absent = None
        if load(f"{tag}_absent", [deck(0, [base_only(), off])]):
            trig(0, 0); trig(1, 0); time.sleep(2.5)
            absent = cap(f"{tag}_absent")
    else:
        absent = a_only(tag)
    if c1 is None or c2 is None or absent is None:
        no(f"{tag}: capture failed"); return
    noise = d(c1, c2); dab = d(c1, absent)
    print(f"      {tag}: d(cap1, absent)={dab:.2f} noise d(cap1, cap2)={noise:.2f}", flush=True)
    (ok if dab >= 5.0 else no)(f"{tag}: (i) the fixture exercises the stage on the active deck (d(cap1, absent)="
                               f"{dab:.2f} >= 5)")
    if not REF:
        print(f"      {tag}: (ii) not applied (RSTATE_REF unset -- this is the REF / BASE arm)", flush=True)
        return
    rp1, rp2 = os.path.join(REF, f"{tag}_cap1.png"), os.path.join(REF, f"{tag}_cap2.png")
    if not (os.path.isfile(rp1) and os.path.isfile(rp2)):
        no(f"{tag}: (ii) RSTATE_REF={REF} has no {tag}_cap1.png / _cap2.png"); return
    r1, r2 = decode(rp1), decode(rp2)
    rnoise = d(r1, r2); fl = max(1.5, 4.0 * rnoise); dr = d(c1, r1)
    print(f"      {tag}: (ii) d(cap1, REF cap1)={dr:.2f} REF noise={rnoise:.2f} floor={fl:.2f}", flush=True)
    (ok if dr <= fl else no)(f"{tag}: (ii) renders as on the BASE arm (d(cap1, REF cap1)={dr:.2f} <= floor {fl:.2f})")


def p_flag_files(tag, deck0, flag):
    extra = {"persistent": True} if flag else {}
    return [deck0, deck(1, [layer(5, [clip(2, IMG_B)], ltype=1, **extra)])]


def p_flag_ignored(spec):
    a = a_only("pfi")
    out = {}
    for key, flag in (("control", False), ("subject", True)):
        if not load(f"pfi_{key}", p_flag_files(key, deck(0, [base_only()]), flag)):
            return
        persist_setup(); time.sleep(2.5)
        out[key] = cap(f"pfi_{key}")
    st = state() or {}   # G6 (INFO, no bar): frame time at the end of p_flag_ignored, printed on both arms
    print(f"      p_flag_ignored: G6 INFO frame_time_ms={st.get('frame_time_ms')} "
          f"peak_frame_time_ms={st.get('peak_frame_time_ms')}", flush=True)
    if a is None or any(v is None for v in out.values()):
        no("p_flag_ignored: capture failed"); return
    dva = d(out["control"], a); dsc = d(out["subject"], out["control"]); nb = nonblank(out["subject"])
    print(f"      p_flag_ignored: d(control, A-only)={dva:.2f} d(subject, control)={dsc:.2f} "
          f"d(subject, A-only)={d(out['subject'], a):.2f} non-blank={nb}", flush=True)
    if dva > TOL:
        no(f"p_flag_ignored: INVALID -- the control is not A-only (d={dva:.2f} > tol {TOL})"); return
    (ok if nb and dsc <= TOL else no)(
        f"p_flag_ignored: a file's \"persistent\": true is ignored -- the other deck's layer never draws "
        f"(d(subject, control)={dsc:.2f}, tol {TOL})")


def p_flag_ignored_empty(spec):
    lock = spec["lock"]
    out = {}
    for key, flag in (("control", False), ("subject", True)):
        if not load(f"pfie_{key}", p_flag_files(key, deck(0, [layer(0, [clip(1, IMG_A)])]), flag)):
            return
        persist_setup(active_layer_idx_list=()); time.sleep(2.0)
        out[key] = cap(f"pfie_{key}", lock)
    if any(v is None for v in out.values()):
        no("p_flag_ignored_empty: capture failed"); return
    mc = float(out["control"][..., :3].mean()); dsc = d(out["subject"], out["control"])
    print(f"      p_flag_ignored_empty: control mean RGB {mc:.2f}, subject mean RGB "
          f"{float(out['subject'][..., :3].mean()):.2f}, d(subject, control)={dsc:.2f}", flush=True)
    if mc > 2.0:
        no(f"p_flag_ignored_empty: INVALID -- the control is not black (mean RGB {mc:.2f} > 2.0)"); return
    (ok if dsc <= TOL else no)(
        f"p_flag_ignored_empty: an empty shown deck stays black -- another deck's \"persistent\" layer never draws "
        f"(d(subject, control)={dsc:.2f}, tol {TOL})")


def p_api_no_field(spec):
    if not load("papi_subject", p_flag_files("subject", deck(0, [base_only()]), True)):
        return
    c = comp()
    if c is None:
        return
    layers = [L for dk in c.get("decks", []) for L in dk.get("layers", [])]
    has_key = [L.get("id") for L in layers if "persistent" in L]
    missing = [L.get("id") for L in layers if not all(k in L for k in ("id", "visible", "activeClipColumn"))]
    print(f"      p_api_no_field: {len(layers)} layer objects; with \"persistent\": {has_key}; missing id / visible / "
          f"activeClipColumn: {missing}", flush=True)
    (ok if layers and not has_key and not missing else no)(
        f"p_api_no_field: GET /api/composition layers carry no \"persistent\" and keep id / visible / activeClipColumn "
        f"({len(layers)} layers)")


def p_ignore_column(spec):
    l0 = layer(0, [clip(1, IMG_A), clip(2, IMG_B)])
    l1 = layer(1, [clip(3, IMG_A), clip(4, IMG_B)], ltype=1, ignoreColumnTrigger=True)
    if not load("pic", [deck(0, [l0, l1], ncols=2)]):
        return
    trig(1, 0); time.sleep(0.3)
    S.post(A + "/api/trigger_column", json={"column": 1}, timeout=6)
    time.sleep(0.5)
    c = comp()
    if c is None:
        return
    try:
        L = c["decks"][0]["layers"]
        a0, a1 = L[0].get("activeClipColumn"), L[1].get("activeClipColumn")
    except (KeyError, IndexError, TypeError):
        no(f"p_ignore_column: /api/composition has no deck 0 with 2 layers"); return
    (ok if a0 == 1 and a1 == 0 else no)(
        f"p_ignore_column: a column trigger skips the Ignore Column Trigger layer (layer 0 activeClipColumn {a0} "
        f"== 1, layer 1 {a1} == 0)")


def main():
    rows = [("r5_hold", lambda: r5_hold(FIX["r5_hold"])),
            ("r5_burst", lambda: r5_burst(FIX["r5_burst"]))]
    for k in ("temporal", "control", "ring", "retrigger"):
        rows.append((f"r1_{k}", (lambda k=k: r1_wipe(f"r1_{k}", FIX["r1"][k]))))
    rows += [("r1_counts", lambda: r1_counts(FIX["r1"]["counts"])),
             ("r1_cells", lambda: r1_cells(FIX["r1"]["cells"])),
             ("a4_feedback", lambda: a4("a4_feedback", "feedback")),
             ("a4_fxonly", lambda: a4("a4_fxonly", "fxonly")),
             ("a4_fxonly_medialess", lambda: a4("a4_fxonly_medialess", "medialess")),
             ("p_flag_ignored", lambda: p_flag_ignored(FIX["p_flag"])),
             ("p_flag_ignored_empty", lambda: p_flag_ignored_empty(FIX["p_flag"])),
             ("p_api_no_field", lambda: p_api_no_field(FIX["p_flag"])),
             ("p_ignore_column", lambda: p_ignore_column(FIX["p_flag"]))]
    for name, fn in rows:
        if ONLY is None or name in ONLY:
            print(f"--- {name}", flush=True)
            fn()
    print(f"\nPY {PASS} PASS / {FAIL} FAIL", flush=True)
    sys.exit(0 if FAIL == 0 else 1)


if __name__ == "__main__":
    main()
