#!/usr/bin/env python3
"""Diagnosis fixtures for lane render (s-rta-0926b): R1..R5. Talks to a running app on 7070, decodes every PNG.
usage: diag.py <media> <fresh-out> <case,case,...>   cases: r1 r1c r2 r2b r3 r3b r4a r4b r4c r4d r4e r4f r5
d(X,Y) = mean |X-Y| over RGB, 0..255."""
import json, os, sys, time, threading
import numpy as np, requests
from PIL import Image

A = "http://127.0.0.1:7070"
MEDIA, OUT, CASES = sys.argv[1], sys.argv[2], sys.argv[3].split(",")
IMG_A = os.path.join(MEDIA, "P16_01_baseline.png")
IMG_B = os.path.join(MEDIA, "P16_02_Screen_Split_2x2.png")


def log(*a):
    print(*a, flush=True)


def fx(name, *params, dw=1.0):
    return {"name": name, "enabled": True, "bypassed": False, "dryWet": dw, "params": list(params)}


def clip(cid, img, effects=(), **extra):
    c = {"name": f"c{cid}", "id": cid, "mediaType": 1, "mediaFile": img, "effects": list(effects)}
    c.update(extra)
    return c


def layer(lid, clips, ltype=0, speed=0.0, **extra):
    l = {"name": f"L{lid}", "id": lid, "opacity": 1.0, "visible": True, "blendMode": 0, "type": ltype,
         "transitionSpeed": speed, "layerEffects": [], "clips": clips}
    l.update(extra)
    return l


def deck(did, layers, ncols=3):
    return {"name": f"D{did}", "id": did, "numColumns": ncols, "layers": layers}


def load(tag, decks, active=0):
    comp = {"name": "diag-" + tag, "activeDeckIndex": active, "masterOpacity": 1.0, "globalTransitionSpeed": 0.0,
            "decks": decks}
    path = os.path.join(OUT, f"comp_{tag}.json")
    json.dump(comp, open(path, "w"), indent=1)
    r = requests.post(A + "/api/load_composition", json={"path": path}, timeout=10)
    ok = r.ok and r.json().get("ok") is True
    log(f"  load {tag}: {'ok' if ok else 'REJECTED ' + r.text[:200]}")
    time.sleep(1.0)
    return ok


def trig(li, col):
    requests.post(A + "/api/trigger_clip", json={"layer": li, "column": col}, timeout=6)


def switch(d):
    requests.post(A + "/api/switch_deck", json={"deck": d}, timeout=6)


def cap(name):
    p = os.path.join(OUT, name + ".png")
    r = requests.post(A + "/api/render_frame", json={"output_path": p}, timeout=20)
    b = r.json()
    if not b.get("ok") or not os.path.isfile(p):
        log(f"  CAPTURE FAILED {name}: {b}")
        return None
    return np.asarray(Image.open(p).convert("RGBA")).astype(float)


def d(x, y):
    return float(np.abs(x[..., :3] - y[..., :3]).mean())


def stats(x):
    return f"rgb_mean={x[..., :3].mean():.2f} alpha_mean={x[..., 3].mean():.2f} alpha>0={float((x[..., 3] > 0).mean()):.2f}"


def fit(f, x, y):
    """f ~ x + p (y - x): return p, mean-abs residual."""
    dx = (y - x)[..., :3].ravel(); df = (f - x)[..., :3].ravel()
    den = float(dx @ dx)
    p = float(df @ dx) / den if den > 0 else 0.0
    res = float(np.abs(f[..., :3] - (x[..., :3] + p * (y - x)[..., :3])).mean())
    return p, res


# ---------------------------------------------------------------- R3: outgoing clip transform
def r3(tag="r3", out_extra=None, out_fx=(), bug_extra=None, bug_fx=None):
    """OUT = A with a clip transform, IN = B; Dissolve 4 s. Compare mid frames to the line refA->refB (correct)
    and to the line refA_bug->refB where refA_bug = OUT rendered the way applyTransition renders it."""
    T = 4.0
    out_extra = out_extra if out_extra is not None else {"scale": 0.5}
    bug_extra = bug_extra if bug_extra is not None else {}
    bug_fx = bug_fx if bug_fx is not None else out_fx
    clips = [clip(1, IMG_A, out_fx, **out_extra), clip(2, IMG_B), clip(3, IMG_A, bug_fx, **bug_extra)]
    if not load(tag, [deck(0, [layer(0, clips, speed=T)])]):
        return
    trig(0, 2); time.sleep(T + 1.5)
    refBug = cap(f"{tag}_refAbug")
    trig(0, 1); time.sleep(T + 1.5)
    refB = cap(f"{tag}_refB")
    trig(0, 0); time.sleep(T + 1.5)
    refA = cap(f"{tag}_refA")
    log(f"  {tag}: d(refA,refB)={d(refA, refB):.2f} d(refA,refAbug)={d(refA, refBug):.2f}")
    trig(0, 1); t0 = time.time(); time.sleep(0.3)
    k = 0
    while time.time() - t0 < 0.85 * T:
        f = cap(f"{tag}_mid{k:02d}"); tt = time.time() - t0; k += 1
        p1, r1 = fit(f, refA, refB); p2, r2 = fit(f, refBug, refB)
        log(f"  {tag}: t={tt:.2f}s  correct-model(refA->refB) p={p1:.2f} resid={r1:.2f} | "
            f"bug-model(refAbug->refB) p={p2:.2f} resid={r2:.2f}")
        time.sleep(0.3)


def r3b():
    # outgoing clip opacity 0.5 + Invert: active path = invert(0.5*A); applyTransition = 0.5*invert(A)
    # bug model clip: can't express "effects then opacity" as a clip, so render the bug model at full opacity
    # with Invert and scale by 0.5 in numpy.
    T = 4.0
    clips = [clip(1, IMG_A, [fx("Invert", 1.0)], clipOpacity=0.5), clip(2, IMG_B), clip(3, IMG_A, [fx("Invert", 1.0)])]
    if not load("r3b", [deck(0, [layer(0, clips, speed=T)])]):
        return
    trig(0, 2); time.sleep(T + 1.5)
    inv = cap("r3b_invA")
    trig(0, 1); time.sleep(T + 1.5)
    refB = cap("r3b_refB")
    trig(0, 0); time.sleep(T + 1.5)
    refA = cap("r3b_refA")
    refBug = inv.copy(); refBug[..., :3] *= 0.5
    log(f"  r3b: d(refA,refB)={d(refA, refB):.2f} d(refA, 0.5*invert(A))={d(refA, refBug):.2f}")
    trig(0, 1); t0 = time.time(); time.sleep(0.3)
    k = 0
    while time.time() - t0 < 0.85 * T:
        f = cap(f"r3b_mid{k:02d}"); tt = time.time() - t0; k += 1
        p1, r1 = fit(f, refA, refB); p2, r2 = fit(f, refBug, refB)
        log(f"  r3b: t={tt:.2f}s  correct-model p={p1:.2f} resid={r1:.2f} | bug-model(0.5*invert(A)->B) p={p2:.2f} resid={r2:.2f}")
        time.sleep(0.3)


# ---------------------------------------------------------------- R1: temporal state shared across a crossfade
def r1(tag="r1", out_temporal=True):
    """WipeLeft over T s: IN occupies x < p, OUT x > p. Both clips [Freeze 0.5] (steady state = own input).
    Measure d(frame, refB) in the IN region and d(frame, refA) in the OUT region."""
    T = 8.0
    out_fx = [fx("Freeze", 0.5)] if out_temporal else []
    clips = [clip(1, IMG_A, out_fx), clip(2, IMG_B, [fx("Freeze", 0.5)])]
    if not load(tag, [deck(0, [layer(0, clips, speed=T, transitionMode=26)])]):
        return
    trig(0, 1); time.sleep(T + 1.5)
    refB = cap(f"{tag}_refB")
    trig(0, 0); time.sleep(T + 1.5)
    refA = cap(f"{tag}_refA")
    W = refA.shape[1]
    log(f"  {tag}: d(refA,refB)={d(refA, refB):.2f}  (OUT fx={[e['name'] for e in out_fx]}, IN fx=[Freeze])")
    trig(0, 1); t0 = time.time(); time.sleep(1.0)
    k = 0
    while time.time() - t0 < 0.8 * T:
        f = cap(f"{tag}_mid{k:02d}"); tt = time.time() - t0; k += 1
        p = tt / T
        xin = int(max(0.0, p - 0.12) * W); xout = int(min(1.0, p + 0.12) * W)
        if xin < 20 or W - xout < 20:
            time.sleep(0.5); continue
        din = d(f[:, :xin], refB[:, :xin]); dab_in = d(refA[:, :xin], refB[:, :xin])
        dout = d(f[:, xout:], refA[:, xout:]); dab_out = d(refA[:, xout:], refB[:, xout:])
        log(f"  {tag}: t={tt:.2f}s p~{p:.2f}  IN region x<{xin}: d(f,refB)={din:.2f} (d(A,B) there {dab_in:.2f}, "
            f"ratio {din / max(dab_in, 1e-6):.2f})  OUT region x>{xout}: d(f,refA)={dout:.2f} (d(A,B) there {dab_out:.2f}, "
            f"ratio {dout / max(dab_out, 1e-6):.2f})")
        time.sleep(0.5)


# ---------------------------------------------------------------- R2: persistent layer keys across decks
def r2(tag="r2", fx_active=None, fx_pers=None):
    """deck0 (active) layer id 0 = A + fx_active; deck1 layer id 0 persistent (Opaque, Normal) = B + fx_pers.
    Reference: deck0 layer without effects. Persistent layer covers the frame, so correct frame == B+fx steady."""
    fx_active = fx_active if fx_active is not None else [fx("Freeze", 0.5)]
    fx_pers = fx_pers if fx_pers is not None else [fx("Freeze", 0.5)]
    frames = {}
    for key, fa in (("reference", []), ("subject", fx_active)):
        # layer id 0 in BOTH decks; the reference keeps the persistent layer's key to itself
        d0 = deck(0, [layer(0, [clip(1, IMG_A, fa)])], ncols=1)
        d1 = deck(1, [layer(0, [clip(2, IMG_B, fx_pers)], persistent=True)], ncols=1)
        if not load(f"{tag}_{key}", [d0, d1]):
            return
        switch(1); time.sleep(0.5); trig(0, 0); time.sleep(0.5); switch(0); time.sleep(0.5); trig(0, 0)
        time.sleep(2.0)
        frames[key] = [cap(f"{tag}_{key}_{i}") for i in range(3)]
    refB_only = frames["reference"][0]
    for i, f in enumerate(frames["subject"]):
        log(f"  {tag}: subject frame {i}: d(subject, reference)={d(f, refB_only):.2f}  {stats(f)}")
    log(f"  {tag}: reference {stats(refB_only)}")
    # A-only render for scale: deck with only the A layer
    if load(f"{tag}_Aonly", [deck(0, [layer(0, [clip(1, IMG_A)])], ncols=1)]):
        trig(0, 0); time.sleep(1.0)
        a = cap(f"{tag}_Aonly")
        log(f"  {tag}: d(A-only, reference(B))={d(a, refB_only):.2f}; subject frame0 fit on A->B line: "
            f"p={fit(frames['subject'][0], refB_only, a)[0]:.2f} (fraction of A)")


# ---------------------------------------------------------------- R4: persistent layer vs the same normal layer
def r4(tag, subj_clip_extra=None, subj_layer_extra=None):
    """Base: deck0 layer id 0 Opaque = A. Subject layer id 5 Transparent (Normal blend, Alpha key) = B + stage.
    reference: subject as a NORMAL layer on deck0 above the base. subject: same layer PERSISTENT on deck1."""
    ce = subj_clip_extra or {}; le = subj_layer_extra or {}
    base = layer(0, [clip(1, IMG_A)])
    subj = lambda persistent: layer(5, [clip(2, IMG_B, **ce)], ltype=1, persistent=persistent, **le)
    plain = layer(5, [clip(2, IMG_B)], ltype=1, persistent=True)
    out = {}
    # reference: normal layer
    if not load(f"{tag}_reference", [deck(0, [base, subj(False)], ncols=1)]):
        return
    trig(0, 0); trig(1, 0); time.sleep(2.5)
    out["reference"] = cap(f"{tag}_reference")
    for key, lay in (("subject", subj(True)), ("stage_absent", plain)):
        if not load(f"{tag}_{key}", [deck(0, [base], ncols=1), deck(1, [lay], ncols=1)]):
            return
        switch(1); time.sleep(0.5); trig(0, 0); time.sleep(0.5); switch(0); time.sleep(0.5); trig(0, 0)
        time.sleep(2.5)
        out[key] = cap(f"{tag}_{key}")
    log(f"  {tag}: d(persistent, normal-layer reference)={d(out['subject'], out['reference']):.2f}  "
        f"d(persistent, same layer WITHOUT the stage)={d(out['subject'], out['stage_absent']):.2f}  "
        f"d(reference, stage-absent)={d(out['reference'], out['stage_absent']):.2f}")


def r4f():
    """Persistent layer mid-transition: deck1 layer id 5 (Transparent) OUT=A, IN=B, Dissolve 8 s.
    Trigger OUT then IN on deck1, switch to deck0 immediately, capture; then switch back after 10 s."""
    T = 8.0
    base = layer(0, [clip(1, IMG_A, [fx("Invert", 1.0)])])
    subj = layer(5, [clip(2, IMG_A), clip(3, IMG_B)], ltype=1, persistent=True, speed=T)
    if not load("r4f", [deck(0, [base], ncols=2), deck(1, [subj], ncols=2)]):
        return
    trig(0, 0); time.sleep(0.5)          # deck0 base active (stays active across deck switches)
    switch(1); time.sleep(0.5); trig(0, 0); time.sleep(1.5)
    refA = cap("r4f_deck1_OUT")
    trig(0, 1); time.sleep(T + 1.5)
    refB = cap("r4f_deck1_IN")
    trig(0, 0); time.sleep(T + 1.5)   # back to OUT (transition B->A completes on deck1 while active)
    trig(0, 1); t0 = time.time()      # start OUT->IN on deck1 ...
    time.sleep(0.3); switch(0); time.sleep(0.4)   # ... and leave the deck mid-transition
    for k in range(4):
        f = cap(f"r4f_persist_mid{k}"); tt = time.time() - t0
        p, res = fit(f, refA, refB)
        log(f"  r4f: t={tt:.2f}s persistent frame: d(f,OUT)={d(f, refA):.2f} d(f,IN)={d(f, refB):.2f} fit p={p:.2f} resid={res:.2f}")
        time.sleep(0.8)
    time.sleep(max(0.0, T + 2.0 - (time.time() - t0)))
    switch(1); time.sleep(0.3)
    f = cap("r4f_back_on_deck1"); tt = time.time() - t0
    p, res = fit(f, refA, refB)
    log(f"  r4f: t={tt:.2f}s (> T={T}) after switching BACK to deck1: d(f,OUT)={d(f, refA):.2f} d(f,IN)={d(f, refB):.2f} p={p:.2f} resid={res:.2f}")


def r4g():
    """Active deck has a layer but NO active clip; deck1 persistent layer (B) active. Expected: B visible."""
    base = layer(0, [clip(1, IMG_A)])
    subj = layer(5, [clip(2, IMG_B)], ltype=1, persistent=True)
    if not load("r4g", [deck(0, [base], ncols=1), deck(1, [subj], ncols=1)]):
        return
    switch(1); time.sleep(0.5); trig(0, 0); time.sleep(1.0)
    ref = cap("r4g_deck1_active")
    switch(0); time.sleep(1.0)
    f = cap("r4g_deck0_empty")
    log(f"  r4g: persistent B with an EMPTY active deck: d(f, B-on-deck1)={d(f, ref):.2f} {stats(f)} | ref {stats(ref)}")
    trig(0, 0); time.sleep(1.0)
    f2 = cap("r4g_deck0_base_on")
    log(f"  r4g: same after triggering the active deck's base clip: d(f, B)={d(f2, ref):.2f} {stats(f2)}")


# ---------------------------------------------------------------- R5: temporal buffer created mid-pass
def r5(lid):
    """col0 = A (no fx); col1 = A + [Freeze 0.5], fresh layer id (never used this run -> the temporal buffer is
    created on the first Freeze frame). Burst-capture every frame across the switch."""
    clips = [clip(1, IMG_A), clip(2, IMG_A, [fx("Freeze", 0.5)])]
    if not load(f"r5_{lid}", [deck(0, [layer(lid, clips)])]):
        return
    trig(0, 0); time.sleep(1.5)
    frames = []
    stop = [False]

    def burst():
        k = 0
        while not stop[0] and k < 60:
            p = os.path.join(OUT, f"r5_{lid}_b{k:02d}.png")
            r = requests.post(A + "/api/render_frame", json={"output_path": p}, timeout=20)
            frames.append((time.time(), p if r.json().get("ok") else None)); k += 1
    th = threading.Thread(target=burst); th.start()
    time.sleep(0.35); t_trig = time.time(); trig(0, 1)
    time.sleep(0.6); stop[0] = True; th.join()
    frames = [(t, (np.asarray(Image.open(p).convert("RGBA")).astype(float) if p else None)) for t, p in frames]
    ref = frames[0][1]
    for i, (t, f) in enumerate(frames):
        if f is None:
            log(f"  r5 b{i:02d} CAPTURE FAILED"); continue
        tag = "PRE " if t < t_trig else "POST"
        log(f"  r5 id={lid} b{i:02d} {tag} t={t - t_trig:+.3f}s d(f,A)={d(f, ref):6.2f} {stats(f)}")


def main():
    for c in CASES:
        log(f"=== {c}")
        if c == "r3": r3()
        elif c == "r3b": r3b()
        elif c == "r1": r1()
        elif c == "r1c": r1("r1c", out_temporal=False)
        elif c == "r2": r2()
        elif c == "r2b": r2("r2b", [fx("Frame Stutter", 0.0, 0.0)], [fx("Frame Stutter", 0.0, 0.0)])
        elif c == "r4a": r4("r4a", subj_clip_extra={"scale": 0.5})
        elif c == "r4b": r4("r4b", subj_layer_extra={"layerEffects": [fx("Invert", 1.0)]})
        elif c == "r4c": r4("r4c", subj_layer_extra={"layerScale": 0.5})
        elif c == "r4d": r4("r4d", subj_layer_extra={"feedbackEnabled": True, "feedbackAmount": 0.6,
                                                      "feedbackScaleX": 0.97, "feedbackScaleY": 0.97})
        elif c == "r4e": r4("r4e", subj_clip_extra={"clipOpacity": 0.5})
        elif c == "r4f": r4f()
        elif c == "r4g": r4g()
        elif c.startswith("r5"): r5(int(c[2:]) if len(c) > 2 else 77)
        else: log("unknown case", c)


if __name__ == "__main__":
    main()
