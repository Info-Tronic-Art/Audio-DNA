#!/usr/bin/env python3
"""probe-crossfade.py -- REST/pixel half of .harmony/probe-crossfade.sh (s-rta-0926 xfade lane).

The .sh owns launch / refuse / quit; this file only talks to the already-running app on port 7070
and decodes every captured PNG (never file hashes/sizes). Every render_frame response is checked;
a failed capture is a FAIL, never a decode of some older file (the output dir is fresh per run).

usage: probe-crossfade.py <root> <fresh-outdir> <media-dir> [case,case,...]

Metric: d(X, Y) = mean |X - Y| over the RGB channels of the whole frame, 0..255 (alpha ignored).

CROSSFADE cases (one layer; column 0 = OUT, column 1 = IN; Dissolve over T = transitionSpeed s):
  1. trigger IN first (a first trigger never crossfades), capture refB x2;
     trigger OUT, wait T + 1.5 s, capture refA x2 (references rendered without the transition under test).
  2. trigger IN (OUT -> IN dissolve), capture a frame every ~0.35 s for ~T s, then the final frame at T + 1.5 s.
  Dissolve is a per-pixel mix, so a frame at progress p has d(f,A) = p*d(A,B), d(f,B) = (1-p)*d(A,B).
  Asserts:
    refs distinct    d(A,B) >= 20
    between          every mid frame (captured before 0.85*T): d(f,A) > FLOOR and d(f,B) > FLOOR and
                     |d(f,A) + d(f,B) - d(A,B)| <= 0.10*d(A,B) + FLOOR
    progress         p = d(f,A)/d(A,B) never drops by more than 0.05 frame to frame, and at least one
                     mid frame has 0.25 <= p <= 0.75
    final            d(final, B) <= FLOOR
    on-line          (only cases with "lineTol", s-rta-0926b render R3) every mid frame lies ON the refA->refB
                     line: least-squares residual mean|f - (A + p(B-A))| <= lineTol and 0.02 < p < 0.98.
                     Stricter than "between": an outgoing clip rendered differently from its own reference
                     (transform dropped, opacity/effect order swapped) sits off the line even when the
                     10% "between" slack hides it (case l: excess 2.4-10.2 vs slack 23.6).
  FLOOR = max(1.0, 4 * noise), noise = max(d(refA1,refA2), d(refB1,refB2)) measured per case.
  Calibration k/l (render lane, s-rta-0926b): residual 0.12-0.40 on the fixed build, 4.19-21.78 on 6e8f120
  (build/, pre-change) -> lineTol 1.0. Clip-level extras (transform, clipOpacity) go in the case's "extra".
  Calibration (xfade lane, s-rta-0926, both builds): noise measured 0.00 on every case (static images,
  solid colours), so FLOOR = 1.0. The RED frames on the unfixed build sit at d(f,A) = 0.00 exactly
  (the outgoing clip on both dissolve inputs), 1.0 below the floor; healthy mid frames sit at
  p*d(A,B) >= 2.8 for the first capture (p ~ 0.1; d(A,B) is 29.4 .. 170.0 on these fixtures).

STILL cases (other sites of the same bug class; transitionSpeed 0, so no crossfade):
  mix          MIX == (1-w)*DRY + w*WET within 1.5 (8-bit rounding of three separate renders);
               measured 9.81 on the unfixed build, 0.05 on the fixed one
  equal        SUBJECT == REFERENCE within 1.5, SUBJECT non-blank (alpha>0 fraction >= the case's minAlpha:
               a layer scaled to 0.5 covers 0.25 of the frame, so that case uses 0.2). Case j measured on the
               unfixed build: d = 113.95 (flat 50% grey), control (layer chain alone) 0.16
  self_router  every frame after switching to a self-routed Layer Router is non-blank and within 3.0 of
               the image frame it holds (the saved layer output is resampled once more)
  non-blank = alpha>0 fraction >= 0.5 and RGB std >= 3 (same rule as probe-effects-parity.sh).
  Cases h and i were GL feedback loops (a draw sampling its own colour attachment -- undefined behaviour)
  that happen to render correctly on this Metal-backed driver: they PASS on both builds and are kept as
  regression guards; their pre-fix evidence is the FBO trace (FEEDBACK! flag), not these pixels.
"""
import json, os, sys, time

import numpy as np
import requests
from PIL import Image

A = "http://127.0.0.1:7070"
ROOT, OUT, MEDIA = sys.argv[1], sys.argv[2], sys.argv[3]
ONLY = set(sys.argv[4].split(",")) if len(sys.argv) > 4 and sys.argv[4] else None
FIX = json.load(open(os.path.join(ROOT, ".harmony", "probe-crossfade.json")))
T = float(FIX["transitionSpeed"])
PASS = FAIL = 0


def ok(msg):
    global PASS; PASS += 1; print(f"PASS  {msg}", flush=True)


def no(msg):
    global FAIL; FAIL += 1; print(f"FAIL  {msg}", flush=True)


def clip_json(spec, cid):
    c = {"name": f"c{cid}", "id": cid, "mediaType": spec["mediaType"], "effects": []}
    if "mediaFile" in spec:
        c["mediaFile"] = spec["mediaFile"].replace("@MEDIA@", MEDIA)
    if "sourceType" in spec:
        c["sourceType"] = spec["sourceType"]
        c["sourceParams"] = [{"name": n, "uniform": u, "value": v, "default": v} for n, u, v in spec["sourceParams"]]
    c["effects"] = [fx_json(fx) for fx in spec["effects"]]
    c.update(spec.get("extra", {}))
    return c


def fx_json(fx):
    """Compact fixture effect [name, value(, dryWet)] -> composition JSON effect slot."""
    return {"name": fx[0], "enabled": True, "bypassed": False, "dryWet": fx[2] if len(fx) > 2 else 1.0,
            "params": [fx[1]]}


def load(tag, clips, speed, layer_over=None):
    layer = {"name": "L1", "id": 0, "opacity": 1.0, "visible": True, "blendMode": 0, "type": 0,
             "transitionSpeed": speed, "layerEffects": [],
             "clips": [clip_json(s, i + 1) for i, s in enumerate(clips)]}
    over = dict(layer_over or {})
    if "layerEffects" in over:
        over["layerEffects"] = [fx_json(fx) for fx in over["layerEffects"]]
    layer.update(over)
    comp = {"name": "xfade-" + tag, "activeDeckIndex": 0, "masterOpacity": 1.0,
            "decks": [{"name": "A", "id": 0, "numColumns": len(clips), "layers": [layer]}]}
    path = os.path.join(OUT, f"fx_{tag}.json")
    json.dump(comp, open(path, "w"), indent=1)
    r = requests.post(A + "/api/load_composition", json={"path": path}, timeout=10)
    good = r.ok and '"ok": true' in r.text.replace('"ok":true', '"ok": true')
    if not good:
        no(f"{tag}: load_composition rejected: {r.text[:120]}")
    time.sleep(1.0)
    return good


def trig(col):
    requests.post(A + "/api/trigger_clip", json={"layer": 0, "column": col}, timeout=6)


def cap(name):
    """render_frame into the fresh dir; returns the decoded RGBA float array, or None (and a FAIL)."""
    p = os.path.join(OUT, name + ".png")
    try:
        r = requests.post(A + "/api/render_frame", json={"output_path": p}, timeout=20)
        body = r.json()
    except Exception as e:  # noqa: BLE001 -- any transport/JSON failure is a failed capture
        no(f"render_frame {name}: {e}")
        return None
    if not body.get("ok") or not os.path.isfile(p):
        no(f"render_frame {name}: response {body}")
        return None
    return np.asarray(Image.open(p).convert("RGBA")).astype(float)


def d(x, y):
    return float(np.abs(x[..., :3] - y[..., :3]).mean())


def fit_line(f, x, y):
    """f ~ x + p (y - x) by least squares over RGB: returns (p, mean |residual|)."""
    dx = (y - x)[..., :3].ravel(); df = (f - x)[..., :3].ravel()
    den = float(dx @ dx)
    p = float(df @ dx) / den if den > 0 else 0.0
    return p, float(np.abs(f[..., :3] - (x[..., :3] + p * (y - x)[..., :3])).mean())


def nonblank(a, min_alpha=0.5):
    return float((a[..., 3] > 0).mean()) >= min_alpha and float(a[..., :3].std()) >= 3.0


def crossfade(tag, case):
    if not load(tag, [case["out"], case["in"]], T):
        return
    trig(1); time.sleep(1.5)
    b1, b2 = cap(f"{tag}_refB1"), cap(f"{tag}_refB2")
    trig(0); time.sleep(T + 1.5)
    a1, a2 = cap(f"{tag}_refA1"), cap(f"{tag}_refA2")
    if any(x is None for x in (a1, a2, b1, b2)):
        no(f"{tag}: reference capture failed"); return
    noise = max(d(a1, a2), d(b1, b2)); floor = max(1.0, 4.0 * noise); dab = d(a1, b1)
    print(f"      {tag}: d(A,B)={dab:.2f} noise={noise:.2f} FLOOR={floor:.2f}", flush=True)
    if dab < 20:
        no(f"{tag}: references not distinct (d(A,B)={dab:.2f} < 20)"); return
    trig(1); t0 = time.time(); mids = []
    time.sleep(0.4)
    while time.time() - t0 < 0.85 * T:
        f = cap(f"{tag}_mid{len(mids):02d}")
        tt = time.time() - t0
        if f is not None and tt < 0.85 * T:
            mids.append((tt, f))
        time.sleep(0.25)
    time.sleep(max(0.0, T + 1.5 - (time.time() - t0)))
    fin = cap(f"{tag}_final")
    bad_between, ps, rows = [], [], []
    for tt, f in mids:
        da, db = d(f, a1), d(f, b1); p = da / dab; ps.append(p)
        rows.append(f"t={tt:.2f}s dA={da:.2f} dB={db:.2f} p={p:.2f}")
        if not (da > floor and db > floor and abs(da + db - dab) <= 0.10 * dab + floor):
            bad_between.append(f"t={tt:.2f}s dA={da:.2f} dB={db:.2f}")
    for r in rows:
        print(f"      {tag}: {r}", flush=True)
    if "lineTol" in case:
        tol = float(case["lineTol"]); off = []
        for tt, f in mids:
            p, res = fit_line(f, a1, b1)
            print(f"      {tag}: t={tt:.2f}s on-line fit p={p:.2f} residual={res:.2f}", flush=True)
            if not (res <= tol and 0.02 < p < 0.98):
                off.append(f"t={tt:.2f}s p={p:.2f} res={res:.2f}")
        (ok if mids and not off else no)(
            f"{tag}: {len(mids) - len(off)}/{len(mids)} mid frames ON the OUT->IN line (residual <= {tol}): {off[:3]}")
    if len(mids) < 4:
        no(f"{tag}: only {len(mids)} mid-transition frames captured (need >= 4)")
    elif bad_between:
        no(f"{tag}: {len(bad_between)}/{len(mids)} mid frames NOT between OUT and IN: {bad_between[:3]}")
    else:
        ok(f"{tag}: {len(mids)}/{len(mids)} mid frames strictly between OUT and IN")
    drops = [i for i in range(1, len(ps)) if ps[i] < ps[i - 1] - 0.05]
    if ps and not drops and any(0.25 <= p <= 0.75 for p in ps):
        ok(f"{tag}: progress moves toward IN ({ps[0]:.2f} -> {ps[-1]:.2f})")
    else:
        no(f"{tag}: progress wrong (p={[round(p, 2) for p in ps]})")
    if fin is None:
        return
    dfb = d(fin, b1)
    (ok if dfb <= floor else no)(f"{tag}: final frame matches IN (d={dfb:.2f}, floor {floor:.2f})")


def still_capture(tag, n=2):
    frames = [cap(f"{tag}_{k}") for k in range(n)]
    return None if any(f is None for f in frames) else frames


def still(tag, case):
    kind = case["check"]
    if kind == "mix":
        if not load(tag, [case["dry"], case["wet"], case["mixed"]], 0.0):
            return
        got = {}
        for col, key in enumerate(("dry", "wet", "mixed")):
            trig(col); time.sleep(1.2)
            fr = still_capture(f"{tag}_{key}")
            if fr is None:
                no(f"{tag}: capture failed ({key})"); return
            got[key] = fr[0]
        w = float(case["weight"])
        expect = (1.0 - w) * got["dry"] + w * got["wet"]
        dm = d(got["mixed"], expect); dwd = d(got["dry"], got["wet"])
        print(f"      {tag}: d(DRY,WET)={dwd:.2f} d(MIX,expected)={dm:.2f}", flush=True)
        (ok if nonblank(got["mixed"]) else no)(f"{tag}: MIX frame non-blank")
        (ok if dwd >= 20 and dm <= 1.5 else no)(f"{tag}: MIX == {1 - w:.2f}*DRY + {w:.2f}*WET (d={dm:.2f}, tol 1.5)")
    elif kind == "equal":
        frames = {}
        for key in ("reference", "subject"):
            sub = case[key]
            if not load(f"{tag}_{key}", [sub["clip"]], 0.0, sub["layer"]):
                return
            trig(0); time.sleep(1.2)
            fr = still_capture(f"{tag}_{key}")
            if fr is None:
                no(f"{tag}: capture failed ({key})"); return
            frames[key] = fr
        ma = float(case.get("minAlpha", 0.5))
        for k, f in enumerate(frames["subject"]):
            de = d(f, frames["reference"][0]); nb = nonblank(f, ma)
            (ok if nb and de <= 1.5 else no)(
                f"{tag}: subject frame {k} non-blank={nb} and == reference (d={de:.2f}, tol 1.5)")
    elif kind == "self_router":
        if not load(tag, [case["image"], case["router"]], 0.0):
            return
        trig(0); time.sleep(1.2)
        ref = still_capture(f"{tag}_image", 1)
        if ref is None:
            no(f"{tag}: capture failed (image)"); return
        trig(1); time.sleep(1.2)
        for k in range(3):
            f = cap(f"{tag}_router{k}")
            if f is None:
                continue
            de = d(f, ref[0])
            (ok if nonblank(f) and de <= 3.0 else no)(
                f"{tag}: router frame {k} non-blank={nonblank(f)} and holds the image (d={de:.2f}, tol 3.0)")
            time.sleep(0.4)
    else:
        no(f"{tag}: unknown check {kind}")


def main():
    for tag, case in FIX["crossfade"].items():
        if ONLY is None or tag in ONLY:
            crossfade(tag, case)
    for tag, case in FIX["still"].items():
        if ONLY is None or tag in ONLY:
            still(tag, case)
    print(f"\nPY {PASS} PASS / {FAIL} FAIL", flush=True)
    sys.exit(0 if FAIL == 0 else 1)


if __name__ == "__main__":
    main()
