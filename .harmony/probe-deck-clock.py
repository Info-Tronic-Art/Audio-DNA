#!/usr/bin/env python3
"""probe-deck-clock.py -- REST/pixel half of .harmony/probe-deck-clock.sh (s-rta-0926b plan4 item 2: decks that are
not on screen keep time; .harmony/.reports/s-rta-0926b/plan4-final.md section 3.6).

The .sh owns launch (TEST MODE) / refuse / quit; this file talks to the running app on 7070 (inject_features is
registered there in test mode) and decodes every captured PNG with PIL+numpy. Every render_frame response is
checked; a failed capture is a FAIL. The output dir is fresh per run.

usage: probe-deck-clock.py <root> <fresh-outdir> <media-dir> [row,row,...]
rows (B1): d_fade_finishes d_persistent_single_advance d_pending_trigger_still_cancelled
rows (B2): d_video_keeps_time d_imageseq_keeps_time d_autopilot_keeps_time d_return_hitch

Metric: d(X, Y) = mean |X - Y| over RGB, 0..255. Reference frames are captured in the same run (probe-crossfade
protocol); floor = max(1, 4 x noise), noise = d between two back-to-back captures of the same reference.
Deck 1 always holds one image clip (A), triggered once, so the app shows something while deck 0 is away.

B1 d_fade_finishes (RED): deck 0 L0 transitionSpeed T = 4 s, col0 = A, col1 = B. refB = col1 triggered FIRST (a
  first trigger never crossfades), refA after the B -> A fade finished. Trigger col1 (A -> B), leave 0.3 s later,
  poll /api/composition every 0.5 s: deck 0 L0 crossfadeProgress rises by >= 0.15 between the first and third
  sample and reaches 1.0 with previousClipColumn == -1 by T + 1 s; back on deck 0 at T + 2 s, 0.5 s later the
  frame == refB within floor. Base: the fields are absent, and the frozen fade resumes on return (p ~ 0.2).
B1 d_persistent_single_advance (guard; needs the new fields -- N/A on the base): deck 0 layer id 5 persistent
  Transparent fading col0 -> col1 over T = 4 s while deck 1 is shown: crossfadeProgress T/2 after the switch is in
  [0.35, 0.65] (compositePersistentLayers advances it; a second advance by the inactive-deck tick reads ~1.0).
B1 d_pending_trigger_still_cancelled (guard, GREEN on both): deck 0 col0 = A, col1 = B with beatSnap; trigger col0,
  then col1 (queued), leave at once (L5 cancels it), 4 injected beat crossings (totalBeatCount moves with the phase
  -- Pitfall 42): deck 0 activeClipColumn still 0.
B2 d_video_keeps_time (RED): deck 0 L0 col0 = ramp12.mp4 (frame mean G encodes t = 12 x meanG / 255). 2 s in:
  t0 (sanity 1.5 < t0 < 3); away 4 s; back 0.3 s: t1 - t0 in [3.5, 5.5] and /api/composition playheadPosition
  within 0.06 of t1 / 12. Base: the video freezes while away (t1 - t0 ~ 0.5-0.7) and the field is absent.
B2 d_imageseq_keeps_time (RED): ImageSequence [A, B] at 0.5 fps (2 s per frame). refs rA (t ~ 0.5) / rB (t ~ 2.5)
  from the sequence itself; reload, 0.5 s in, away 2 s, back 0.3 s: frame == rB within floor. Base: ~1.0 s -> A.
B2 d_autopilot_keeps_time (RED): deck 0 L0 autopilotEnabled, cols A B C each Beat4 / PlayNext; trigger col0, leave,
  6 injected crossings (beatPhase 0.99 / 0.01, 100 ms apart; totalBeatCount moves with the phase -- Pitfall 42):
  deck 0 activeClipColumn >= 1. Base: 0.
B2 d_return_hitch (REPORT; FAIL above 50 ms): deck 0 L0 = ramp12_1080.mp4 (1920x1080, GOP 60); away 5 s; 7070
  /api/state read right before the return (resets peak_frame_time_ms) and 0.3 s after: the peak (the return
  frame's catch-up decode runs inside compositeDeck, i.e. inside frame_time_ms) is printed.

Calibration: the lane report .harmony/.reports/s-rta-0926b/canvas.md (RED lines on the pre-change app, GREEN lines
on the lane build).
"""
import json, os, subprocess, sys, time

import numpy as np
import requests
from PIL import Image

A = "http://127.0.0.1:7070"
ROOT, OUT, MEDIA = sys.argv[1], sys.argv[2], sys.argv[3]
ONLY = set(sys.argv[4].split(",")) if len(sys.argv) > 4 and sys.argv[4] else None
FIX = json.load(open(os.path.join(ROOT, ".harmony", "probe-deck-clock.json")))
IMG_A = FIX["imageA"].replace("@MEDIA@", MEDIA)
IMG_B = FIX["imageB"].replace("@MEDIA@", MEDIA)
IMG_C = FIX["imageC"].replace("@MEDIA@", MEDIA)
PASS = FAIL = 0
S = requests.Session()
# One fresh connection per request, never a pooled keep-alive one (s-rta-0927 c1-state-fix). The app's cpp-httplib
# server closes a connection idle for 5 s (CPPHTTPLIB_KEEPALIVE_TIMEOUT_SECOND) -- shutdown, then it drains and
# DISCARDS whatever request arrives in that instant. d_return_hitch sends s0 5.0 s after switch(1), right on that
# edge: a reused connection then fails with RemoteDisconnected (~1 run in 3, on the pre-C1 app too), and requests
# never retries it. Evidence: .harmony/.reports/s-rta-0927/c1-state-fix.md.
S.headers["Connection"] = "close"


def ok(msg):
    global PASS; PASS += 1; print(f"PASS  {msg}", flush=True)


def no(msg):
    global FAIL; FAIL += 1; print(f"FAIL  {msg}", flush=True)


def na(msg):
    print(f"N/A   {msg}", flush=True)


def clip(cid, img, **extra):
    c = {"name": f"c{cid}", "id": cid, "mediaType": 1, "mediaFile": img, "effects": []}
    c.update(extra)
    return c


def media_clip(cid, **extra):
    """Video / image-sequence clip: speed is loaded unguarded (Clip.cpp:214) -- always 1.0 here."""
    c = {"name": f"c{cid}", "id": cid, "effects": [], "speed": 1.0, "transportMode": 0, "loopMode": 0}
    c.update(extra)
    return c


def layer(lid, clips, ltype=0, speed=0.0, **extra):
    l = {"name": f"L{lid}", "id": lid, "opacity": 1.0, "visible": True, "blendMode": 0, "type": ltype,
         "transitionSpeed": speed, "layerEffects": [], "clips": clips}
    l.update(extra)
    return l


def deck(did, layers, ncols=1):
    return {"name": f"D{did}", "id": did, "numColumns": ncols, "layers": layers}


def away_deck():
    return deck(1, [layer(0, [clip(900, IMG_A)])])


def load(tag, decks):
    comp = {"name": "dclock-" + tag, "activeDeckIndex": 0, "masterOpacity": 1.0, "globalTransitionSpeed": 0.0,
            "decks": decks}
    path = os.path.join(OUT, f"comp_{tag}.json")
    json.dump(comp, open(path, "w"), indent=1)
    r = S.post(A + "/api/load_composition", json={"path": path}, timeout=10)
    good = r.ok and r.json().get("ok") is True
    if not good:
        no(f"{tag}: load_composition rejected: {r.text[:160]}")
    time.sleep(1.0)
    try:
        on = [e.get("name") for e in S.get(A + "/api/state", timeout=6).json().get("effects", []) if e.get("enabled")]
    except Exception:  # noqa: BLE001
        on = ["<state unreadable>"]
    if on:
        no(f"{tag}: rig -- the global effect chain has enabled effects nobody in this probe set: {on[:6]}")
    return good


def trig(li, col):
    S.post(A + "/api/trigger_clip", json={"layer": li, "column": col}, timeout=6)


def switch(dk):
    S.post(A + "/api/switch_deck", json={"deck": dk}, timeout=6)


def prime_away_deck():
    """Deck 1's image clip triggered once (deck 1 then shows A whenever it is active)."""
    switch(1); time.sleep(0.4); trig(0, 0); time.sleep(0.4); switch(0); time.sleep(0.4)


def inject(**kw):
    return S.post(A + "/api/inject_features", json=kw, timeout=6)


# The unwrapped beat count the injected phase sawtooth carries: every beat-crossing reader (Autopilot, projectM
# playlist, slideshow, beat randomize) takes the totalBeatCount delta, not the phase wrap (Pitfall 42's rule).
BEAT = 0


def crossings(n):
    global BEAT
    for _ in range(n):
        inject(beatPhase=0.99, totalBeatCount=BEAT); time.sleep(0.1)
        BEAT += 1
        inject(beatPhase=0.01, totalBeatCount=BEAT); time.sleep(0.1)


def comp():
    try:
        return S.get(A + "/api/composition", timeout=6).json()
    except Exception as e:  # noqa: BLE001
        no(f"/api/composition: {e}")
        return None


def layer_json(c, di, li):
    try:
        return c["decks"][di]["layers"][li]
    except (KeyError, IndexError, TypeError):
        return None


def state():
    try:
        return S.get(A + "/api/state", timeout=6).json()
    except Exception as e:  # noqa: BLE001
        no(f"/api/state: {e}")
        return None


def decode(p):
    return np.asarray(Image.open(p).convert("RGBA")).astype(float)


def cap(name):
    p = os.path.join(OUT, name + ".png")
    if os.path.exists(p):
        os.remove(p)
    t0 = time.time()
    try:
        body = S.post(A + "/api/render_frame", json={"output_path": p}, timeout=30).json()
    except Exception as e:  # noqa: BLE001
        no(f"render_frame {name}: {e}")
        return None
    if not body.get("ok"):
        no(f"render_frame {name}: response {body}")
        return None
    if not os.path.isfile(p):
        no(f"render_frame {name}: ok:true but no file written at {p}")
        return None
    if os.path.getmtime(p) < t0 - 0.01:
        no(f"render_frame {name}: ok:true but {p} predates the request (stale file -- never decoded)")
        return None
    return decode(p)


def d(x, y):
    return float(np.abs(x[..., :3] - y[..., :3]).mean())


def floor_of(a, b):
    return max(1.0, 4.0 * d(a, b))


# ------------------------------------------------------------------ B1 rows
def d_fade_finishes():
    T = float(FIX["fade"]["T"])
    if not load("fade", [deck(0, [layer(0, [clip(1, IMG_A), clip(2, IMG_B)], speed=T)], ncols=2), away_deck()]):
        return
    prime_away_deck()
    trig(0, 1); time.sleep(1.5)                       # first trigger: no crossfade
    rb1, rb2 = cap("fade_refB1"), cap("fade_refB2")
    trig(0, 0); time.sleep(T + 1.5)                   # B -> A fade, finished
    ra = cap("fade_refA")
    if rb1 is None or rb2 is None or ra is None:
        no("d_fade_finishes: reference capture failed"); return
    fl = floor_of(rb1, rb2); dab = d(ra, rb1)
    print(f"      d_fade_finishes: d(A,B)={dab:.2f} FLOOR={fl:.2f}", flush=True)
    trig(0, 1); t0 = time.time()                      # A -> B over T s
    time.sleep(0.3); switch(1)
    samples = []
    while time.time() - t0 < T + 1.2:
        time.sleep(0.5)
        L = layer_json(comp(), 0, 0) or {}
        samples.append((round(time.time() - t0, 2), L.get("crossfadeProgress"), L.get("previousClipColumn")))
    print(f"      d_fade_finishes: deck 0 L0 (t, crossfadeProgress, previousClipColumn) while away: {samples}",
          flush=True)
    prog = [s for s in samples if s[1] is not None]
    if len(prog) < 3:
        no(f"d_fade_finishes: /api/composition has no crossfadeProgress / previousClipColumn fields "
           f"({len(prog)} samples)")
    else:
        rise = float(prog[2][1]) - float(prog[0][1])
        done = [s for s in prog if float(s[1]) >= 1.0 and s[2] == -1 and s[0] <= T + 1.0]
        (ok if rise >= 0.15 else no)(f"d_fade_finishes: the fade keeps advancing while its deck is not shown "
                                     f"(progress +{rise:.2f} between samples 1 and 3)")
        (ok if done else no)(f"d_fade_finishes: the fade is complete (progress 1.0, previousClipColumn -1) by T + 1 s "
                             f"while away (first complete sample {done[0] if done else None})")
    time.sleep(max(0.0, T + 2.0 - (time.time() - t0)))
    switch(0); time.sleep(0.5)
    f = cap("fade_back")
    if f is None:
        return
    dfb = d(f, rb1)
    (ok if dfb <= fl else no)(f"d_fade_finishes: back on deck 0 the incoming clip is showing, fade finished "
                              f"(d(f, refB)={dfb:.2f}, floor {fl:.2f}; d(f, refA)={d(f, ra):.2f})")


def d_persistent_single_advance():
    T = float(FIX["persistent"]["T"]); lo, hi = FIX["persistent"]["range"]
    subj = layer(5, [clip(1, IMG_A), clip(2, IMG_B)], ltype=1, speed=T, persistent=True)
    if not load("persist", [deck(0, [subj], ncols=2), away_deck()]):
        return
    prime_away_deck()
    trig(0, 0); time.sleep(1.0)
    trig(0, 1); t0 = time.time(); switch(1)
    time.sleep(max(0.0, T / 2 - (time.time() - t0)))
    L = layer_json(comp(), 0, 0) or {}
    p = L.get("crossfadeProgress")
    if p is None:
        na("d_persistent_single_advance: /api/composition has no crossfadeProgress field (pre-change binary)")
        return
    (ok if lo <= float(p) <= hi else no)(
        f"d_persistent_single_advance: a persistent layer's fade advances once per frame while its deck is away "
        f"(progress {float(p):.2f} at T/2 = {T / 2:.1f} s, expected in [{lo}, {hi}])")


def d_pending_trigger_still_cancelled():
    n = int(FIX["pending"]["crossings"])
    if not load("pending", [deck(0, [layer(0, [clip(1, IMG_A), clip(2, IMG_B, beatSnap=True, beatSnapMode=1)])],
                                 ncols=2), away_deck()]):
        return
    prime_away_deck()
    inject(beatPhase=0.5, totalBeatCount=BEAT); time.sleep(0.2)
    trig(0, 0); time.sleep(0.5)
    trig(0, 1); time.sleep(0.15); switch(1); time.sleep(0.4)
    crossings(n); time.sleep(0.3)
    L = layer_json(comp(), 0, 0) or {}
    col = L.get("activeClipColumn")
    (ok if col == 0 else no)(f"d_pending_trigger_still_cancelled: a quantized trigger left waiting on the deck is "
                             f"cancelled when you leave it (L5): deck 0 activeClipColumn {col} after {n} crossings")


def ramp_video(name, key):
    p = os.path.join(OUT, name)
    if not os.path.exists(p):
        r = subprocess.run(["ffmpeg", "-y", "-loglevel", "error"] + FIX["video"][key] + [p], capture_output=True, text=True)
        if r.returncode != 0 or not os.path.exists(p):
            no(f"ffmpeg could not make {name}: {r.stderr[:200]}")
            return None
    return p


def t_of(f):
    return 12.0 * float(f[..., 1].mean()) / 255.0


def clip_json(c, di, li, col):
    L = layer_json(c, di, li) or {}
    for cj in L.get("clips", []):
        if cj.get("column") == col:
            return cj
    return {}


def d_video_keeps_time():
    cfg = FIX["video"]
    v = ramp_video("ramp12.mp4", "ffmpeg")
    if v is None:
        return
    if not load("video", [deck(0, [layer(0, [media_clip(1, mediaType=2, mediaFile=v)])]), away_deck()]):
        return
    prime_away_deck()
    trig(0, 0); time.sleep(float(cfg["before"]))
    v0 = cap("video_v0")
    if v0 is None:
        return
    t0 = t_of(v0)
    if not (1.5 < t0 < 3.0):
        no(f"d_video_keeps_time: oracle broken -- t0 = {t0:.2f} s after {cfg['before']} s of play"); return
    switch(1); time.sleep(float(cfg["away"])); switch(0); time.sleep(0.3)
    v1 = cap("video_v1")
    cj = clip_json(comp(), 0, 0, 0)
    if v1 is None:
        return
    t1 = t_of(v1); lo, hi = cfg["window"]
    (ok if lo <= t1 - t0 <= hi else no)(
        f"d_video_keeps_time: a video keeps playing while its deck is away (t0 {t0:.2f} s, t1 {t1:.2f} s, "
        f"t1 - t0 = {t1 - t0:.2f} s after {cfg['away']} s away, expected in [{lo}, {hi}])")
    ph = cj.get("playheadPosition")
    if ph is None:
        no("d_video_keeps_time: /api/composition has no clip playheadPosition field")
    else:
        (ok if abs(float(ph) - t1 / 12.0) <= float(cfg["playheadTol"]) else no)(
            f"d_video_keeps_time: /api/composition playheadPosition {float(ph):.3f} matches the frame "
            f"({t1 / 12.0:.3f} +- {cfg['playheadTol']})")


def d_imageseq_keeps_time():
    cfg = FIX["sequence"]
    seq = media_clip(1, mediaType=5, sequenceFiles=[IMG_A, IMG_B], sequenceFps=float(cfg["fps"]))
    if not load("seq_ref", [deck(0, [layer(0, [seq])]), away_deck()]):
        return
    trig(0, 0); t0 = time.time(); time.sleep(0.5)
    rA1, rA2 = cap("seq_rA1"), cap("seq_rA2")
    time.sleep(max(0.0, 2.5 - (time.time() - t0)))
    rB = cap("seq_rB")
    if rA1 is None or rA2 is None or rB is None:
        no("d_imageseq_keeps_time: reference capture failed"); return
    fl = floor_of(rA1, rA2)
    print(f"      d_imageseq_keeps_time: d(rA, rB)={d(rA1, rB):.2f} FLOOR={fl:.2f}", flush=True)
    if not load("seq", [deck(0, [layer(0, [seq])]), away_deck()]):
        return
    prime_away_deck()
    trig(0, 0); time.sleep(float(cfg["before"]))
    switch(1); time.sleep(float(cfg["away"])); switch(0); time.sleep(0.3)
    f = cap("seq_back")
    if f is None:
        return
    dfb = d(f, rB)
    (ok if dfb <= fl else no)(f"d_imageseq_keeps_time: an image sequence keeps its clock while its deck is away "
                              f"(back after {cfg['away']} s: d(f, frame B)={dfb:.2f}, floor {fl:.2f}; "
                              f"d(f, frame A)={d(f, rA1):.2f})")


def d_autopilot_keeps_time():
    n = int(FIX["autopilot"]["crossings"])
    ap = dict(autopilotAction=2, autopilotDuration=3)
    L0 = layer(0, [clip(1, IMG_A, **ap), clip(2, IMG_B, **ap), clip(3, IMG_C, **ap)], autopilotEnabled=True)
    if not load("autopilot", [deck(0, [L0], ncols=3), away_deck()]):
        return
    prime_away_deck()
    inject(beatPhase=0.5, totalBeatCount=BEAT); time.sleep(0.2)
    trig(0, 0); time.sleep(0.5)
    switch(1); time.sleep(0.4)
    crossings(n); time.sleep(0.3)
    col = (layer_json(comp(), 0, 0) or {}).get("activeClipColumn")
    (ok if isinstance(col, int) and col >= 1 else no)(
        f"d_autopilot_keeps_time: autopilot keeps advancing a deck that is away ({n} beat crossings, Beat4 / "
        f"PlayNext: deck 0 activeClipColumn {col}, expected >= 1)")


def d_return_hitch():
    cfg = FIX["hitch"]
    v = ramp_video("ramp12_1080.mp4", "ffmpeg1080")
    if v is None:
        return
    if not load("hitch", [deck(0, [layer(0, [media_clip(1, mediaType=2, mediaFile=v)])]), away_deck()]):
        return
    prime_away_deck()
    trig(0, 0); time.sleep(1.5)
    switch(1); time.sleep(float(cfg["away"]))
    s0 = state()
    switch(0); time.sleep(0.3)
    s1 = state()
    if not s0 or not s1:
        return
    pk = float(s1.get("peak_frame_time_ms", -1.0))
    print(f"      d_return_hitch: peak_frame_time_ms while away {float(s0.get('peak_frame_time_ms', -1.0)):.2f}, "
          f"across the return {pk:.2f} (fps {float(s1.get('fps', 0.0)):.1f})", flush=True)
    (ok if 0.0 <= pk <= float(cfg["failMs"]) else no)(
        f"d_return_hitch: the return frame's catch-up decode (1080p, GOP 60, {cfg['away']} s away) stays under "
        f"{cfg['failMs']} ms (peak {pk:.2f} ms; REPORT -- target <= 16)")


def main():
    rows = [("d_fade_finishes", d_fade_finishes),
            ("d_persistent_single_advance", d_persistent_single_advance),
            ("d_pending_trigger_still_cancelled", d_pending_trigger_still_cancelled)]
    rows += [("d_video_keeps_time", d_video_keeps_time),
             ("d_imageseq_keeps_time", d_imageseq_keeps_time),
             ("d_autopilot_keeps_time", d_autopilot_keeps_time),
             ("d_return_hitch", d_return_hitch)]
    for name, fn in rows:
        if ONLY is None or name in ONLY:
            print(f"--- {name}", flush=True)
            fn()
    print(f"\nPY {PASS} PASS / {FAIL} FAIL", flush=True)
    sys.exit(0 if FAIL == 0 else 1)


if __name__ == "__main__":
    main()
