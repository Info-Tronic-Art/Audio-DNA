#!/usr/bin/env python3
"""probe-render-state.py -- REST/pixel half of .harmony/probe-render-state.sh (s-rta-0926b render lane).

The .sh owns launch / refuse / quit; this file talks to the already-running app on port 7070 and decodes
every captured PNG with PIL+numpy (never file hashes). Every render_frame response is checked; a failed
capture is a FAIL. The output dir is fresh per run.

usage: probe-render-state.py <root> <fresh-outdir> <media-dir> [row,row,...]
rows: r2_temporal r2_ring r4_clip_transform r4_clip_opacity r4_layer_effects r4_layer_transform r4_feedback
      r4_transition r5_hold r5_burst

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
    return good


def trig(li, col):
    S.post(A + "/api/trigger_clip", json={"layer": li, "column": col}, timeout=6)


def switch(d):
    S.post(A + "/api/switch_deck", json={"deck": d}, timeout=6)


def cap(name):
    p = os.path.join(OUT, name + ".png")
    try:
        body = S.post(A + "/api/render_frame", json={"output_path": p}, timeout=20).json()
    except Exception as e:  # noqa: BLE001 -- any transport/JSON failure is a failed capture
        no(f"render_frame {name}: {e}")
        return None
    if not body.get("ok") or not os.path.isfile(p):
        no(f"render_frame {name}: response {body}")
        return None
    return decode(p)


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
    for name, fn in rows:
        if ONLY is None or name in ONLY:
            print(f"--- {name}", flush=True)
            fn()
    print(f"\nPY {PASS} PASS / {FAIL} FAIL", flush=True)
    sys.exit(0 if FAIL == 0 else 1)


if __name__ == "__main__":
    main()
