#!/usr/bin/env python3
"""probe-boxes.py -- REST/pixel half of .harmony/probe-boxes.sh (lane bf9b, s-rta-1002b: decks are boxes of clips; the
layers are ONE shared playing stack; plan-bf9b S4.1, ruling-bf9b amendment 14 + the FINAL CONSOLIDATED GATE LIST).

Boris: "when I switch between decks, do not change the clips playing in the layers or how they are playing. treat the
decks as just a box of clips and I can switch between 20 decks looking for a clip and the playing will not be affected."
/ "Whatever is in the layer should be what is playing and there should be nothing else."

The .sh owns launch (TEST MODE: inject_features) / refuse / quit-ours; this file talks to the running app on 7070
(OSC deck switches on UDP 8000) and decodes every captured PNG with PIL+numpy. Every render_frame response is checked;
a failed capture is a FAIL. The out dir is fresh per run; pictures "D<deck> C<col>" are drawn with PIL there and
ramp12.mp4 is made with ffmpeg there.

usage: probe-boxes.py <root> <fresh-outdir> <media-dir> [row,row,...]
env:   BOXES_OLD_TAKE=<take folder>  the pre-bf9b take k7_old_take replays on the BF9B arm (the STAGE_P arm records it
                                     and prints its path: "k7_old_take: recorded take <folder>")

ARMS (ruling-bf9b): STAGE_P = the STAGE_P_HEAD app (RED expected), BF9B = the lane head (GREEN expected). The arm is
read from GET /api/composition: a top-level "layers" array exists only on BF9B. PER-ARM READERS: STAGE_P reads
decks[activeDeck].layers[*].activeClipColumn / previousClipColumn; BF9B reads top-level layers[*].activeClip /
previousClip {deck, deckId, column, retired}. "BF9B-only" rows (K2v, K4b, K8b, K9a-c, K10, and K1a's remove / undo
drivers) print N/A on STAGE_P (the driver or the field does not exist there: no RED arm).

Metric: d(X, Y) = mean |X - Y| over RGB, 0..255. noise = d of two captures of one steady scene 0.5 s apart, same run;
floor = max(1.5, 4 x noise); tol = 1.5; t(frame) = 12 x meanG / 255 for ramp12.mp4. Bars VERBATIM from the gate list.

ROWS
k1a_switch_static   K1a: deck 0 layer 0 = static picture, fired; the other decks' cells hold other clips, none fired.
                    Each driver: capture the first frame requested right after the request returns, +0.5 s, +2 s:
                    every d(after, before) <= floor, and the driver took effect (activeDeck / numDecks). Drivers: REST
                    switch_deck, OSC /audiodna/deck/1, /api/debug/duplicate_deck and /api/debug/load_deck (each shows
                    the new deck; the deck file has the show's row count), BF9B-only: /api/debug/remove_deck of the
                    SHOWN deck no layer plays from (the grid moves) and /api/debug/undo of that removal (the ui lane's
                    one undo route, {"redo": false}; Harmony ruling R-S3). The take-replay
                    activeDeck lane is K7's k7_old_take (same bar). Switch paths with no non-synthetic driver (tab
                    click, MIDI binding, keyboard): unit T1 + lints B4d / B4g.
k1b_switch_video    K1b: deck 0 layer 0 = ramp12.mp4, t >= 4 s at the switch; at +2 s |t - (t_switch + elapsed)| <= 0.5
                    (elapsed = the time between the two capture requests, ~2.0 s, printed) and the REST playhead moved by
                    elapsed / 12 within 0.06. Then, in the same flow, k1b_duplicate.
k1b_duplicate       K1b (bf9b fix round): K1's duplicate_deck driver on K1b's video: POST /api/debug/duplicate_deck
                    {"deck": 0} shows the copy; K1b's t bar at +2 s. A registered row (gates-r2 NIT 4): in a run that
                    includes k1b_switch_video it runs inside that row's flow on the shared fixture (t ~ 7 s) and is
                    not run twice; selected alone it sets K1b's show up itself (same lead, t >= 4 s).
k1c_switch_midfade  K1c: Dissolve, T = 4 s; switch 1.0 s into the A -> B fade; every frame after the switch (before 0.85 T)
                    on the OUT -> IN line (residual <= tol, 0.02 < p < 0.98); REST p rises >= 0.15 between +0.5 s and
                    +2 s after the switch; complete (p 1, no previous) by T + 1 s; the final frame == B within floor.
k1t_history_freeze  K1t: k1a's REST driver with the clip carrying [Freeze 0.5]: d <= floor at +0.5 s and +2 s.
k1t_history_feedback K1t: the a4 feedback fixture (layer 0 Opaque A, layer 1 Transparent B with feedbackEnabled,
                    amount 0.6, scale 0.97 x 0.97, as probe-render-state a4_feedback): d <= floor at +0.5 s and +2 s.
k1d_ia_speed_half   K1d i-a: ramp12.mp4 speed 0.5, Loop, Cut, fired at T0; W = T0 + 4; switch 0 -> 1 at W - 0.5, 1 -> 0 at
                    W + 0.75; t(W + 1.5) - t(W) in [0.25, 1.25] and the REST playhead moved 0.0625 +- 0.06.
k1d_ib_pingpong     K1d i-b: PingPong, speed 1.0; W = T0 + 13; t delta in [-2.0, -1.0], playhead -0.125 +- 0.06. A window
                    that straddles a wrap (t(W) > 11.7 or t(W + 1.5) < 0.3) is re-run once, then FAIL.
k1d_ii_opacity_blend K1d ii: layer 0 Opaque A; layer 1 Transparent B, opacity 0.5, blendMode Screen. VALID iff d(R, A-only)
                    >= 5; captures at W, W + 0.5, W + 2.0 within floor of R.
k1d_iii_connection  K1d iii: deck 0 layer 0 Opaque A whose CLIP opacity is connected to the signal "Volume" (= RMS),
                    range 0.2 .. 1.0, driven by inject_features rms 1.0 / 0.0 (deterministic: test mode runs no analysis
                    thread). VALID iff d(Rmax, Rmin) >= 5; with deck 1 shown d(Cmax, Rmax) and d(Cmin, Rmin) <= floor.
k2_nothing_unseen   K2: deck 0 row 0 and deck 1 row 1 = copies of ramp12.mp4 (the second fired with deck 1 shown); 5
                    switches 0 <-> 1 within 10 s; every clip's playhead (decks[*].layers[*].clips[*]) read twice 1 s
                    apart; advancing = delta > 0.005; "in a layer" by the per-arm reader (STAGE_P: the SHOWN deck's
                    layers). VALID iff >= 2 clips advance. PASS iff (advancing AND not in a layer) == 0.
k2v_decode          K2v (BF9B-only; BAR): 20 decks, rows 0-2 = 60 copies of ramp12.mp4; VALID iff video_players == 60.
                    Deck 4's row-0 clip plays 3 s in layer 0, then is replaced (the "left" clip); layers 0/1/2 <- deck
                    0/7/13 col 0 (each fired with its deck shown); show deck 19; settle 3 s; 11 /api/state reads 0.5 s
                    apart (the first 10 are the 10 samples; the 5-s decode delta is read 0 -> 10). PASS iff (1) every
                    sample video_threads_awake <= 4 and >= 8 of 10 == 3; (2) the 5-s delta of video_frames_decoded in
                    [3 x fps x 5 x 0.5, 3 x fps x 5 x 1.5 + 30]; (3) the left clip's and every non-layer clip's
                    playhead moved <= 0.001 (GET /api/composition at the first and the last sample).
k3_autopilot        K3: layer 0 autopilot PlayNext / Beat4 over A, B, C; 12 injected beats (totalBeatCount moves with
                    the phase, Pitfall 42); R1 no switch, R2 switches at beats 3 (-> deck 1) and 9 (-> deck 0). PASS iff
                    R2's frames at beats 4, 8, 12 equal R1's within floor AND no deck-row / clip outside a playing layer
                    (per-arm reader, in both snapshots of a beat) changes activeClipColumn or playheadPosition. VALID iff
                    R1 advanced (column 1 at beat 4). beatsPlayed has no REST field on either arm: not read (said).
k4_ignore_column_across K4: 3 layers, each in its own third (layer transforms); layer 2 Ignore Column Trigger, plays X =
                    deck 0 col 1, the others deck 0 col 0. Show deck 1; POST /api/trigger_column {"column": 1} (the only
                    column entry with a non-synthetic driver; every entry is handleColumnTrigger). PASS iff layer 2
                    still plays X (REST) and its region is within floor of before, and layers 0 / 1 play deck 1 col 1.
k4b_empty_cell      K4b (BF9B-only; Q5's default): deck 1 col 1 holds a clip in row 0 only; layer 2 Ignore Column. Show
                    deck 1, fire column 1: layers[0] == {deck1.id, 1}; layers[1] none and its region == the capture
                    taken before layer 1 was fired (within floor); layers[2] == {deck0.id, 0}, region within floor.
k5_queue_link_off   K5: a Bar-snapped trigger queued on deck 0 at beatInBar 1; switch to deck 1 before the bar; beats 2,
                    3 (not fired yet), then the bar: PASS iff it fires on that bar (per-arm REST: deck 0 col 1 active)
                    and the frame == B within floor.
k5_queue_link_on    BLOCKED: neither arm is built with Ableton Link (CMake AUDIODNA_BUILD_LINK OFF) and Link's only
                    switch is the TopBar toggle (no REST / OSC; no synthetic input) -- reported, never dropped.
k7_old_show         K7: an old 2-deck show (layer settings differ; Deck 1 / L1 "persistent": true; a Deck-2 layer
                    connection; a 1.2 s deck fade). PASS iff layer settings == Deck 1's (per-arm reader), exactly one
                    "old show converted:" logLine (the app's stdout + stderr logs in the out dir); save + reload
                    (bf9b fix round; BF9B-only driver POST /api/debug/save_composition, N/A on STAGE_P): the saved
                    file has top-level "layers", no "persistent" / "globalTransitionSpeed", its reload logs no note
                    and the first deck's settings come back; then a new-format show: no new logLine.
                    s-rta-1003 (plan-bf9b-merge HARMONY ADOPTION item 11; K7 / B5): the four load_notice clauses
                    are gone, every other clause is unchanged. Boris: "We don't need any text indicating what has
                    happened or what has happened. That is something that happens online and is not necessary in
                    this application. It is extra overhead and bloat. Please remove it cleanly and completely."
k7_old_take         K7 (+ K1's take-replay driver): a take recorded on the STAGE_P arm (pre-bf9b PerfState: no "layers"
                    in checkpoint0) with an activeDeck lane replays (perf/load + perf/play, wall clock) with no error,
                    its lane moves activeDeck, and every capture during the replay is within floor of before.
k8_twenty_decks     K8: 20 decks; switch_deck through all 20 within 10 s; at every switch activeDeck == the target, K1a's
                    bar (d(frame, before) <= floor) and peak_frame_time_ms across the switch <= 50 ms.
k8b_browse_fire     K8b (BF9B-only): K8's show; layer 0 = deck 0's picture in the left half, layer 1 in the right half.
                    Rright = deck 7 col 2 fired in a pre-step, captured, then cleared (an empty cell fired). Walk 1..7, on
                    deck 7 fire layer 1 col 2, walk 8..19 and back to 0. PASS iff at every switch the left half is within
                    floor of its pre-walk capture, after the fire the right half is within floor of Rright and
                    layers[1].activeClip == {deck7.id, 2}, and peak_frame_time_ms <= 50 ms.
k9a_remove_playing  K9a (BF9B-only): layer 1 plays deck 2's ramp12.mp4; show deck 0; remove deck 2: numDecks - 1,
                    retiredDeckCount 1, layers[1].activeClip.retired, K1b's bar at +2 s.
k9b_remove_midfade  K9b (BF9B-only): layer 1 Dissolve T = 4 s FROM deck 2's picture TO deck 0's; deck 2 also holds 2
                    videos; remove deck 2 at p ~ 0.3: K1c's bars; retiredDeckCount 1 while p < 1; then duplicate_deck
                    {"deck": 0} (a fenced edit): retiredDeckCount 0 and video_players drops by 2.
k9c_remove_undo     K9c (BF9B-only): k9a, then POST /api/debug/undo: numDecks restored, deck 2 back at index 2 with its id,
                    retiredDeckCount 0, layers[1].activeClip == {deck2.id, 0} not retired, K1b's bar across the undo.
k10_fresh_and_resume K10 (BF9B-only): deck 3 row 0 = ramp12.mp4 never played; Cut. Show deck 3, fire, show deck 0 within
                    0.2 s: the first capture <= 0.3 s after the fire has t <= 0.5. At t ~ 4 fire deck 0's picture into
                    layer 0 (t_r = t of the last capture before); 5 s; show deck 3, re-fire: the first capture <= 0.3 s
                    after has |t - t_r| <= 0.5 and the clip's REST playhead moved <= 0.001 during the 5 s.

K6 -- probe-deck-clock's rows (ruling-bf9 K6, plan 4.B): d_fade_finishes -> INVERTED (k1c_switch_midfade);
d_single_advance -> RETIRED with DeckClock (successor: K1c's "p rises >= 0.15"); d_pending_trigger_still_cancelled ->
INVERTED (k5_queue_link_off / _on); d_video_keeps_time, d_imageseq_keeps_time -> INVERTED (k1b_switch_video +
k2_nothing_unseen); d_autopilot_keeps_time -> INVERTED (k3_autopilot); d_return_hitch -> RETIRED, its 50 ms bar moved to
k8_twenty_decks.
"""
import colorsys, json, os, shutil, socket, struct, subprocess, sys, time

import numpy as np
import requests
from PIL import Image, ImageDraw, ImageFont

A = "http://127.0.0.1:7070"
ROOT, OUT, MEDIA = sys.argv[1], sys.argv[2], sys.argv[3]
ONLY = set(sys.argv[4].split(",")) if len(sys.argv) > 4 and sys.argv[4] else None
FIX = json.load(open(os.path.join(ROOT, ".harmony", "probe-boxes.json")))
OLD_TAKE = os.environ.get("BOXES_OLD_TAKE", "")
TOL = float(FIX["floor"]["tol"])
PASS = FAIL = BLOCKED = 0
BLOCKED_ROWS = []   # the names of the rows that printed BLOCKED (ruling-bf9b-merge AM-9): probe-boxes.sh prints them
ROW = None          # the row main() is running
S = requests.Session()
# One fresh connection per request (s-rta-0927 c1-state-fix: the app's cpp-httplib server drops a request that lands
# on a connection it is closing after 5 s idle).
S.headers["Connection"] = "close"
ARM = None


def ok(msg):
    global PASS; PASS += 1; print(f"PASS  {msg}", flush=True)


def no(msg):
    global FAIL; FAIL += 1; print(f"FAIL  {msg}", flush=True)


def na(msg):
    print(f"N/A   {msg}", flush=True)


def blocked(msg):
    global BLOCKED; BLOCKED += 1; print(f"BLOCKED  {msg}", flush=True)
    if ROW not in BLOCKED_ROWS:
        BLOCKED_ROWS.append(ROW)


def info(msg):
    print(f"      {msg}", flush=True)


def check(cond, msg):
    (ok if cond else no)(msg)
    return bool(cond)


# ------------------------------------------------------------------ fixtures
try:
    FONT = ImageFont.truetype("/System/Library/Fonts/Supplemental/Arial Bold.ttf", 120)
except Exception:  # noqa: BLE001
    FONT = ImageFont.load_default()


def picture(d, c):
    """"D<d> C<c>": one hue per deck, brightness per column, the name in big numerals (PIL, drawn once per run)."""
    p = os.path.join(OUT, "img", f"D{d}C{c}.png")
    if not os.path.exists(p):
        os.makedirs(os.path.dirname(p), exist_ok=True)
        r, g, b = colorsys.hsv_to_rgb((d * 0.137) % 1.0, 0.8, 0.45 + 0.17 * (c % 3))
        im = Image.new("RGB", (640, 360), (int(r * 255), int(g * 255), int(b * 255)))
        dr = ImageDraw.Draw(im)
        t = f"D{d} C{c}"
        bb = dr.textbbox((0, 0), t, font=FONT)
        dr.text(((640 - (bb[2] - bb[0])) / 2 - bb[0], (360 - (bb[3] - bb[1])) / 2 - bb[1]), t, fill=(255, 255, 255),
                font=FONT)
        im.save(p)
    return p


def ramp(name):
    """A fresh copy of ramp12.mp4 (one file per clip, so no two clips share a decoder)."""
    base = os.path.join(OUT, "ramp12.mp4")
    if not os.path.exists(base):
        r = subprocess.run(["ffmpeg", "-y", "-loglevel", "error"] + FIX["video"]["ffmpeg"] + [base],
                           capture_output=True, text=True)
        if r.returncode != 0 or not os.path.exists(base):
            no(f"ffmpeg could not make ramp12.mp4: {r.stderr[:200]}")
            return None
    p = os.path.join(OUT, "vid", name + ".mp4")
    os.makedirs(os.path.dirname(p), exist_ok=True)
    if not os.path.exists(p):
        shutil.copyfile(base, p)
    return p


CID = [1000]


def img(d, c, **extra):
    CID[0] += 1
    cj = {"name": f"D{d}C{c}", "id": CID[0], "mediaType": 1, "mediaFile": picture(d, c), "effects": []}
    cj.update(extra)
    return cj


def vid(path, **extra):
    """Video clip: speed is loaded unguarded (Clip.cpp) -- always set."""
    CID[0] += 1
    cj = {"name": os.path.basename(path)[:-4], "id": CID[0], "mediaType": 2, "mediaFile": path, "effects": [],
          "speed": 1.0, "transportMode": 0, "loopMode": 0, "inPoint": 0.0, "outPoint": 1.0}
    cj.update(extra)
    return cj


def fx(e):
    return {"name": e[0], "enabled": True, "bypassed": False, "dryWet": 1.0, "params": list(e[1:])}


def layer(lid, clips, ltype=0, speed=0.0, **extra):
    lj = {"name": f"L{lid}", "id": lid, "opacity": 1.0, "visible": True, "blendMode": 0, "type": ltype,
          "transitionSpeed": speed, "layerEffects": [], "clips": clips}
    lj.update(extra)
    return lj


def THIRD(i):
    """Layer transform putting a layer in the i-th third of the middle band (x in [i/3, (i+1)/3), y in [1/3, 2/3))."""
    return {"layerScale": 1.0 / 3.0, "positionX": float(i), "positionY": 1.0}


def HALF(i):
    """Layer transform: x in [i/2, (i+1)/2), y in [1/4, 3/4)."""
    return {"layerScale": 0.5, "positionX": float(i), "positionY": 0.5}


def show(tag, ndecks, nlayers, ncols, cells, layer_kw=None, fade=0.0):
    """An OLD-format composition (decks[*].layers[*] with settings + clips) with the SAME layer settings on every
    deck, so both arms read one file (BF9B converts it -- no note: nothing differs). cells: {(deck, row, col): clip}."""
    layer_kw = layer_kw or {}
    decks = []
    for d in range(ndecks):
        ls = []
        for li in range(nlayers):
            kw = dict(layer_kw.get(li, {}))
            ltype = kw.pop("type", 0)
            speed = kw.pop("transitionSpeed", 0.0)
            ls.append(layer(li, [cells.get((d, li, c)) for c in range(ncols)], ltype, speed, **kw))
        decks.append({"name": f"Deck {d + 1}", "id": 100 + d, "numColumns": ncols, "layers": ls})
    return load(tag, {"name": "boxes-" + tag, "activeDeckIndex": 0, "masterOpacity": 1.0,
                      "globalTransitionSpeed": fade, "decks": decks})


def load(tag, comp_json):
    global ARM
    path = os.path.join(OUT, f"comp_{tag}.json")
    json.dump(comp_json, open(path, "w"), indent=1)
    try:
        r = S.post(A + "/api/load_composition", json={"path": path}, timeout=15)
        good = r.ok and r.json().get("ok") is True
    except Exception as e:  # noqa: BLE001
        good = False; r = e
    if not good:
        no(f"{tag}: load_composition rejected: {str(getattr(r, 'text', r))[:160]}")
        return False
    time.sleep(1.2)
    c = comp()
    if c is not None:
        arm = "BF9B" if isinstance(c.get("layers"), list) else "STAGE_P"
        if ARM is None:
            ARM = arm
            info(f"ARM = {ARM} (top-level 'layers' {'present' if arm == 'BF9B' else 'absent'} in GET /api/composition)")
    try:
        on = [e.get("name") for e in S.get(A + "/api/state", timeout=6).json().get("effects", []) if e.get("enabled")]
    except Exception:  # noqa: BLE001
        on = ["<state unreadable>"]
    if on:
        no(f"{tag}: rig -- the global effect chain has enabled effects nobody in this probe set: {on[:6]}")
    return True


# ------------------------------------------------------------------ REST
def post(path, body=None, timeout=10):
    try:
        r = S.post(A + path, json=body or {}, timeout=timeout)
        try:
            return r.status_code, r.json()
        except Exception:  # noqa: BLE001
            return r.status_code, {"text": r.text[:200]}
    except Exception as e:  # noqa: BLE001
        return 0, {"error": str(e)}


def comp():
    try:
        return S.get(A + "/api/composition", timeout=6).json()
    except Exception as e:  # noqa: BLE001
        no(f"/api/composition: {e}")
        return None


def state():
    try:
        return S.get(A + "/api/state", timeout=6).json()
    except Exception as e:  # noqa: BLE001
        no(f"/api/state: {e}")
        return None


def trig(li, col):
    S.post(A + "/api/trigger_clip", json={"layer": li, "column": col}, timeout=6)


def column(col):
    S.post(A + "/api/trigger_column", json={"column": col}, timeout=6)


def active_deck():
    c = comp()
    return None if c is None else c.get("activeDeck")


def wait_for(pred, limit=2.0, step=0.05):
    t0 = time.time()
    while time.time() - t0 < limit:
        c = comp()
        if c is not None and pred(c):
            return c
        time.sleep(step)
    return None


def switch(dk, wait=True):
    S.post(A + "/api/switch_deck", json={"deck": dk}, timeout=6)
    if wait:
        return wait_for(lambda c: c.get("activeDeck") == dk) is not None
    return True


def osc(addr, val):
    a = addr.encode() + b"\0"
    a += b"\0" * ((4 - len(a) % 4) % 4)
    pkt = a + b",f\0\0" + struct.pack(">f", float(val))
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.sendto(pkt, ("127.0.0.1", 8000))
    s.close()


def inject(**kw):
    S.post(A + "/api/inject_features", json=kw, timeout=6)


BEAT = 0
BIB = 0   # beatInBar of the last injected beat


def beats(n=1):
    """n injected beat crossings: beatPhase 0.99 -> 0.01, totalBeatCount + 1 and beatInBar + 1 (mod 4) with the phase
    (Pitfall 42: every beat reader takes the totalBeatCount delta)."""
    global BEAT, BIB
    for _ in range(n):
        inject(beatPhase=0.99, totalBeatCount=BEAT, beatInBar=BIB); time.sleep(0.1)
        BEAT += 1; BIB = (BIB + 1) % 4
        inject(beatPhase=0.01, totalBeatCount=BEAT, beatInBar=BIB); time.sleep(0.1)


def plays(c):
    """PER-ARM READER: per layer index -> {"active": (deck, col, retired, deckId) | None, "previous": ..., "p": ...}.
    STAGE_P: decks[activeDeck].layers[*]; BF9B: the top-level layers[*] (the truth; F8)."""
    out = []
    if c is None:
        return out
    if isinstance(c.get("layers"), list):
        def ref(r):
            if not isinstance(r, dict) or int(r.get("column", -1)) < 0:
                return None
            return (int(r.get("deck", -1)), int(r["column"]), bool(r.get("retired")), int(r.get("deckId", -1)))
        for L in c["layers"]:
            out.append({"active": ref(L.get("activeClip")), "previous": ref(L.get("previousClip")),
                        "pending": ref(L.get("pendingClip")), "p": float(L.get("crossfadeProgress", 1.0))})
    else:
        d = int(c.get("activeDeck", 0))
        for L in c["decks"][d]["layers"]:
            a, pv = int(L.get("activeClipColumn", -1)), int(L.get("previousClipColumn", -1))
            out.append({"active": (d, a, False, -1) if a >= 0 else None,
                        "previous": (d, pv, False, -1) if pv >= 0 else None, "pending": None,
                        "p": float(L.get("crossfadeProgress", 1.0))})
    return out


def at(c, li, key="active"):
    r = plays(c)
    return r[li][key] if li < len(r) else None


def dc(ref):
    return None if ref is None else ref[:2]


def clips(c):
    """{(deck, row, col): clip json} from the decks[*].layers[*].clips[*] mirror (both arms)."""
    out = {}
    if c is None:
        return out
    for d, dk in enumerate(c.get("decks", [])):
        for li, L in enumerate(dk.get("layers", [])):
            for cj in L.get("clips", []):
                out[(d, li, int(cj.get("column", -1)))] = cj
    return out


def playhead(c, key):
    cj = clips(c).get(key)
    return None if cj is None else cj.get("playheadPosition")


def in_layer(c, key):
    """Per-arm: is the clip at (deck, row, col) the active or the previous ref of its row's layer?"""
    d, li, col = key
    r = plays(c)
    if li >= len(r):
        return False
    return dc(r[li]["active"]) == (d, col) or dc(r[li]["previous"]) == (d, col)


def logcount(pattern):
    n = 0
    for name in ("out.log", "err.log"):
        p = os.path.join(OUT, name)
        if os.path.exists(p):
            with open(p, errors="replace") as f:
                n += sum(1 for line in f if pattern in line)
    return n


# ------------------------------------------------------------------ pixels
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
    return np.asarray(Image.open(p).convert("RGBA")).astype(float)


def d(x, y):
    return float(np.abs(x[..., :3] - y[..., :3]).mean())


def floor_of(x, y):
    return max(float(FIX["floor"]["min"]), float(FIX["floor"]["k"]) * d(x, y))


def ref_pair(tag):
    """Two captures of one steady scene 0.5 s apart -> (reference, floor)."""
    a = cap(f"{tag}_ref1"); time.sleep(0.5); b = cap(f"{tag}_ref2")
    if a is None or b is None:
        return None, None
    fl = floor_of(a, b)
    info(f"{tag}: noise d(ref1, ref2) = {d(a, b):.2f} -> floor {fl:.2f}")
    return a, fl


def t_of(f):
    return 12.0 * float(f[..., 1].mean()) / 255.0


def region(f, i, n=3):
    H, W = f.shape[:2]
    m = 6
    if n == 3:
        return f[H // 3 + m:2 * H // 3 - m, i * W // 3 + m:(i + 1) * W // 3 - m]
    return f[H // 4 + m:3 * H // 4 - m, i * W // 2 + m:(i + 1) * W // 2 - m]


def sleep_until(t):
    dt = t - time.time()
    if dt > 0:
        time.sleep(dt)


def bf9b_only(row):
    if ARM != "BF9B":
        na(f"{row}: BF9B-only (the driver / the top-level layers do not exist on {ARM})")
        return False
    return True


def ensure_arm():
    if ARM is None:
        comp_ = comp()
        globals()["ARM"] = "BF9B" if comp_ is not None and isinstance(comp_.get("layers"), list) else "STAGE_P"


# ------------------------------------------------------------------ K1
def switch_bar(tag, ref, fl, act, effect, back=None, times=(0.0, 0.5, 2.0)):
    """Run `act`, capture at each of `times` after it (0.0 = the first frame requested right after the request
    returns), then `effect()` -> (bool, detail): the driver took effect. PASS iff it did and every d <= floor."""
    t0 = time.time()
    act()
    ds = []
    for k, tt in enumerate(times):
        sleep_until(t0 + tt)
        f = cap(f"{tag}_{k}")
        if f is not None:
            ds.append((tt, round(d(f, ref), 2)))
    took, detail = effect()
    good = took and len(ds) == len(times) and all(x <= fl for _, x in ds)
    check(good, f"{tag}: the picture does not change ({detail}; d(after, before) at +s {ds}, floor {fl:.2f})")
    if back:
        back()


def k1a_switch_static():
    cells = {(dk, 0, c): img(dk, c) for dk in range(3) for c in range(2)}
    if not show("k1a", 3, 1, 2, cells):
        return
    trig(0, 0); time.sleep(1.0)
    ref, fl = ref_pair("k1a")
    if ref is None:
        no("k1a_switch_static: reference capture failed"); return
    # REST switch_deck
    switch_bar("k1a_rest", ref, fl, lambda: switch(1, wait=False),
               lambda: (active_deck() == 1, f"REST switch_deck -> activeDeck {active_deck()}"),
               back=lambda: switch(0))
    # OSC /audiodna/deck/1
    switch_bar("k1a_osc", ref, fl, lambda: osc("/audiodna/deck/1", 1.0),
               lambda: (active_deck() == 1, f"OSC /audiodna/deck/1 -> activeDeck {active_deck()}"),
               back=lambda: switch(0))
    # duplicate_deck of deck 0 (shows the copy)
    n0 = len((comp() or {}).get("decks", []))

    def dup_effect():
        c = comp() or {}
        return (len(c.get("decks", [])) == n0 + 1 and c.get("activeDeck") not in (None, 0),
                f"duplicate_deck 0 -> numDecks {len(c.get('decks', []))} (was {n0}), activeDeck {c.get('activeDeck')}")
    switch_bar("k1a_duplicate", ref, fl, lambda: post("/api/debug/duplicate_deck", {"deck": 0}), dup_effect,
               back=lambda: switch(0))
    # load_deck: a deck file with the show's row count (1), other pictures
    deck_file = os.path.join(OUT, "k1a_deck.json")
    json.dump({"name": "Loaded", "id": 4000, "numColumns": 2,
               "layers": [layer(0, [img(7, 0), img(7, 1)])]}, open(deck_file, "w"), indent=1)
    n1 = len((comp() or {}).get("decks", []))

    def load_effect():
        c = wait_for(lambda c: len(c.get("decks", [])) == n1 + 1, limit=3.0) or comp() or {}
        return (len(c.get("decks", [])) == n1 + 1 and c.get("activeDeck") == n1,
                f"load_deck -> numDecks {len(c.get('decks', []))} (was {n1}), activeDeck {c.get('activeDeck')}")
    switch_bar("k1a_load_deck", ref, fl, lambda: post("/api/debug/load_deck", {"path": deck_file}), load_effect,
               back=lambda: switch(0))
    # BF9B-only: remove the SHOWN deck no layer plays from, then undo that removal
    if ARM != "BF9B":
        na("k1a_remove_deck / k1a_undo: BF9B-only driver (/api/debug/remove_deck; the undo row undoes that removal)")
        return
    switch(1)
    n2 = len((comp() or {}).get("decks", []))

    def rm_effect():
        c = wait_for(lambda c: len(c.get("decks", [])) == n2 - 1, limit=3.0) or comp() or {}
        return (len(c.get("decks", [])) == n2 - 1 and c.get("retiredDeckCount") == 0,
                f"remove_deck 1 (shown, nothing plays from it) -> numDecks {len(c.get('decks', []))} (was {n2}), "
                f"retiredDeckCount {c.get('retiredDeckCount')}, activeDeck {c.get('activeDeck')}")
    switch_bar("k1a_remove_deck", ref, fl, lambda: post("/api/debug/remove_deck", {"deck": 1}), rm_effect)

    def undo_effect():
        c = wait_for(lambda c: len(c.get("decks", [])) == n2, limit=3.0) or comp() or {}
        return (len(c.get("decks", [])) == n2,
                f"undo -> numDecks {len(c.get('decks', []))}, activeDeck {c.get('activeDeck')}")
    switch_bar("k1a_undo", ref, fl, lambda: post("/api/debug/undo", {"redo": False}), undo_effect)


def k1b_switch_video():
    cfg = FIX["k1b"]
    v = ramp("k1b")
    if v is None or not show("k1b", 2, 1, 1, {(0, 0, 0): vid(v), (1, 0, 0): img(1, 0)}):
        return
    trig(0, 0); time.sleep(float(cfg["lead"]))
    wb = time.time(); fb = cap("k1b_before"); pb = playhead(comp(), (0, 0, 0))
    switch(1, wait=False)
    sleep_until(wb + float(cfg["after"]))
    wa = time.time(); fa = cap("k1b_after"); pa = playhead(comp(), (0, 0, 0))
    deck_now = active_deck()
    switch(0)
    if fb is None or fa is None or pb is None or pa is None:
        no("k1b_switch_video: capture / playhead read failed"); return
    tb, ta, el = t_of(fb), t_of(fa), wa - wb
    if tb < 4.0:
        no(f"k1b_switch_video: oracle broken -- t at the switch {tb:.2f} s < 4 s"); return
    check(deck_now == 1 and abs(ta - (tb + el)) <= float(cfg["tTol"]),
          f"k1b_switch_video: the video keeps playing on screen across the switch (activeDeck {deck_now}; t_switch "
          f"{tb:.2f} s, t at +{el:.2f} s = {ta:.2f} s, expected {tb + el:.2f} +- {cfg['tTol']})")
    check(abs((float(pa) - float(pb)) - el / 12.0) <= float(cfg["phTol"]),
          f"k1b_switch_video: REST playheadPosition moved {float(pa) - float(pb):.4f} (expected {el / 12.0:.4f} "
          f"+- {cfg['phTol']})")
    k1b_duplicate(cfg)


_K1B_DUP_RAN = False


def k1b_duplicate(cfg=None):
    # gates-r2 NIT 5 (ruling-bf9b-merge AM-18) -- Boris: "when I switch between decks, do not change the clips playing
    # in the layers or how they are playing. treat the decks as just a box of clips"
    """K1's duplicate_deck driver on K1b's VIDEO fixture (bf9b fix round): with K1a's static picture a Duplicate that
    restarts the copied clip looks the same, so that sub-row has no RED arm. Same show, the video still playing in
    layer 0 (t ~ 7 s): POST /api/debug/duplicate_deck {"deck": 0} (it shows the copy); K1b's bar on the canvas at
    +2 s: |t - (t_dup + elapsed)| <= tTol, and the copy is shown (numDecks + 1, activeDeck == the copy).
    cfg given = called from k1b_switch_video's flow (the shared fixture); cfg None = the registered row: it runs once
    per launch, and selected alone it sets K1b's show up itself first."""
    global _K1B_DUP_RAN
    if cfg is None:
        if _K1B_DUP_RAN:
            print("k1b_duplicate: ran inside k1b_switch_video's flow on the shared fixture (see above)", flush=True)
            return
        cfg = FIX["k1b"]
        v = ramp("k1b")
        if v is None or not show("k1b_dup", 2, 1, 1, {(0, 0, 0): vid(v), (1, 0, 0): img(1, 0)}):
            return
        trig(0, 0); time.sleep(float(cfg["lead"]))
    _K1B_DUP_RAN = True
    n0 = len((comp() or {}).get("decks", []))
    wb = time.time(); fb = cap("k1b_dup_before")
    code, _ = post("/api/debug/duplicate_deck", {"deck": 0})
    sleep_until(wb + float(cfg["after"]))
    wa = time.time(); fa = cap("k1b_dup_after")
    c = wait_for(lambda c: len(c.get("decks", [])) == n0 + 1, limit=1.0) or comp() or {}
    nd, ad = len(c.get("decks", [])), c.get("activeDeck")
    switch(0)
    if fb is None or fa is None:
        no("k1b_duplicate: capture failed"); return
    tb, ta, el = t_of(fb), t_of(fa), wa - wb
    if tb < 4.0:
        no(f"k1b_duplicate: oracle broken -- t at the duplicate {tb:.2f} s < 4 s"); return
    check(code == 200 and nd == n0 + 1 and ad == n0 and abs(ta - (tb + el)) <= float(cfg["tTol"]),
          f"k1b_duplicate: the video keeps playing on screen across Duplicate Deck (HTTP {code}; numDecks {nd} (was "
          f"{n0}), activeDeck {ad}; t_dup {tb:.2f} s, t at +{el:.2f} s = {ta:.2f} s, expected {tb + el:.2f} "
          f"+- {cfg['tTol']})")


def k1c_switch_midfade():
    cfg = FIX["k1c"]; T = float(cfg["T"])
    cells = {(0, 0, 0): img(0, 0), (0, 0, 1): img(0, 1), (1, 0, 0): img(1, 0), (1, 0, 1): img(1, 1)}
    if not show("k1c", 2, 1, 2, cells, {0: {"transitionSpeed": T}}):
        return
    trig(0, 1); time.sleep(1.5)                    # a first trigger never crossfades
    b1 = cap("k1c_refB1"); time.sleep(0.5); b2 = cap("k1c_refB2")
    trig(0, 0); time.sleep(T + 1.5)
    a1 = cap("k1c_refA1"); time.sleep(0.5); a2 = cap("k1c_refA2")
    if any(x is None for x in (a1, a2, b1, b2)):
        no("k1c_switch_midfade: reference capture failed"); return
    fl = max(floor_of(a1, a2), floor_of(b1, b2))
    info(f"k1c_switch_midfade: d(A,B)={d(a1, b1):.2f} floor {fl:.2f}")
    trig(0, 1); t0 = time.time()                    # A -> B over T
    sleep_until(t0 + float(cfg["switchAt"]))
    switch(1, wait=False); ts = time.time()
    mids, samples, k = [], [], 0
    while time.time() - t0 < T + 1.2:
        c = comp(); tt = time.time()
        r = plays(c)
        if r:
            samples.append((round(tt - ts, 2), round(tt - t0, 2), r[0]["p"], dc(r[0]["previous"]), c.get("activeDeck")))
        if tt - t0 < 0.85 * T:
            f = cap(f"k1c_mid{k:02d}"); k += 1
            if f is not None and time.time() - t0 < 0.85 * T:
                mids.append((round(time.time() - ts, 2), f))
        time.sleep(0.15)
    sleep_until(t0 + T + 1.5)
    fin = cap("k1c_final")
    switch(0)
    info(f"k1c_switch_midfade: REST (s after switch, s after fire, p, previous, activeDeck): {samples}")
    off = []
    for tt, f in mids:
        p, res = fit(f, a1, b1)
        info(f"k1c_switch_midfade: +{tt:.2f} s after the switch: on-line p={p:.2f} residual={res:.2f}")
        if not (res <= TOL and 0.02 < p < 0.98):
            off.append((tt, round(p, 2), round(res, 2)))
    check(len(mids) >= 3 and not off, f"k1c_switch_midfade: {len(mids) - len(off)}/{len(mids)} frames after the switch "
                                      f"ON the OUT -> IN line (residual <= {TOL}, 0.02 < p < 0.98): off {off[:3]}")

    def p_at(s):
        later = [x for x in samples if x[0] >= s]
        return later[0][2] if later else None
    p05, p2 = p_at(0.5), p_at(2.0)
    check(p05 is not None and p2 is not None and p2 - p05 >= float(cfg["rise"]),
          f"k1c_switch_midfade: REST p rises >= {cfg['rise']} between +0.5 s and +2 s after the switch ({p05} -> {p2})")
    done = [x for x in samples if x[2] >= 1.0 and x[3] is None and x[1] <= T + 1.0 and x[0] > 0]
    check(bool(done), f"k1c_switch_midfade: the fade is complete (p 1, no previous) by T + 1 s (first {done[:1]})")
    if fin is not None:
        check(d(fin, b1) <= fl, f"k1c_switch_midfade: the final frame is B (d={d(fin, b1):.2f}, floor {fl:.2f})")


def fit(f, x, y):
    """f ~ x + p (y - x) by least squares over RGB: (p, mean |residual|) (the retired r4_transition's line)."""
    dx = (y - x)[..., :3].ravel(); df = (f - x)[..., :3].ravel()
    den = float(dx @ dx)
    p = float(df @ dx) / den if den > 0 else 0.0
    return p, float(np.abs(f[..., :3] - (x[..., :3] + p * (y - x)[..., :3])).mean())


def k1t(tag, cells, layer_kw, fire):
    if not show(tag, 2, len(layer_kw) or 1, 1, cells, layer_kw):
        return
    for li in fire:
        trig(li, 0)
    time.sleep(2.5)
    ref, fl = ref_pair(tag)
    if ref is None:
        no(f"{tag}: reference capture failed"); return
    switch_bar(tag, ref, fl, lambda: switch(1, wait=False),
               lambda: (active_deck() == 1, f"REST switch_deck -> activeDeck {active_deck()}"),
               back=lambda: switch(0), times=(0.5, 2.0))


def k1t_history_freeze():
    k1t("k1t_history_freeze", {(0, 0, 0): img(0, 0, effects=[fx(["Freeze", 0.5])]), (1, 0, 0): img(1, 0)},
        {0: {}}, [0])


def k1t_history_feedback():
    fb = {"type": 1, "feedbackEnabled": True, "feedbackAmount": 0.6, "feedbackScaleX": 0.97, "feedbackScaleY": 0.97}
    cells = {(0, 0, 0): img(0, 0), (0, 1, 0): img(0, 1), (1, 0, 0): img(1, 0), (1, 1, 0): img(1, 1)}
    k1t("k1t_history_feedback", cells, {0: {}, 1: fb}, [0, 1])


def k1d_video(tag, cfg, loop_mode, retry=True):
    v = ramp(tag)
    if v is None or not show(tag, 2, 1, 1, {(0, 0, 0): vid(v, speed=float(cfg["speed"]), loopMode=loop_mode),
                                             (1, 0, 0): img(1, 0)}):
        return
    trig(0, 0); T0 = time.time(); W = T0 + float(cfg["W"])
    sleep_until(W - 0.5); switch(1, wait=False)
    sleep_until(W); fW = cap(f"{tag}_W"); pW = playhead(comp(), (0, 0, 0)); dW = active_deck()
    sleep_until(W + 0.75); switch(0, wait=False)
    sleep_until(W + 1.5); f2 = cap(f"{tag}_W15"); p2 = playhead(comp(), (0, 0, 0))
    if fW is None or f2 is None or pW is None or p2 is None:
        no(f"{tag}: capture / playhead read failed"); return
    tW, t2 = t_of(fW), t_of(f2)
    if loop_mode == 1 and retry and (tW > 11.7 or t2 < 0.3):
        info(f"{tag}: the window straddles a wrap (t(W) {tW:.2f}, t(W+1.5) {t2:.2f}) -- re-run once")
        return k1d_video(tag + "_rerun", cfg, loop_mode, retry=False)
    lo, hi = cfg["dt"]
    check(dW == 1 and lo <= t2 - tW <= hi, f"{tag}: speed {cfg['speed']} keeps its clock across 0 -> 1 -> 0 (deck "
                                           f"shown at W {dW}; t(W) {tW:.2f}, t(W+1.5) {t2:.2f}, delta {t2 - tW:.2f} "
                                           f"in [{lo}, {hi}])")
    dp = float(p2) - float(pW)
    check(abs(dp - float(cfg["dp"])) <= float(cfg["dpTol"]),
          f"{tag}: REST playheadPosition moved {dp:.4f} (expected {cfg['dp']} +- {cfg['dpTol']})")


def k1d_ia_speed_half():
    k1d_video("k1d_ia_speed_half", FIX["k1d"]["ia"], 0)


def k1d_ib_pingpong():
    k1d_video("k1d_ib_pingpong", FIX["k1d"]["ib"], 1)


def k1d_ii_opacity_blend():
    cfg = FIX["k1d"]["ii"]
    cells = {(0, 0, 0): img(0, 0), (0, 1, 0): img(0, 1), (1, 0, 0): img(1, 0), (1, 1, 0): img(1, 1)}
    if not show("k1d_ii", 2, 2, 1, cells, {0: {}, 1: {"type": 1, "opacity": float(cfg["opacity"]),
                                                       "blendMode": int(cfg["blendMode"])}}):
        return
    trig(0, 0); time.sleep(1.0)
    aonly = cap("k1d_ii_Aonly")
    trig(1, 0); time.sleep(1.0)
    R, fl = ref_pair("k1d_ii")
    if aonly is None or R is None:
        no("k1d_ii_opacity_blend: capture failed"); return
    if not check(d(R, aonly) >= float(cfg["valid"]),
                 f"k1d_ii_opacity_blend: VALID -- layer 1's opacity 0.5 + Screen shows (d(R, A-only)={d(R, aonly):.2f} "
                 f">= {cfg['valid']})"):
        return
    W = time.time() + 0.6
    sleep_until(W - 0.5); switch(1, wait=False)
    sleep_until(W); f0 = cap("k1d_ii_W")
    sleep_until(W + 0.5); f1 = cap("k1d_ii_W05"); dk = active_deck()
    sleep_until(W + 0.75); switch(0, wait=False)
    sleep_until(W + 2.0); f2 = cap("k1d_ii_W2")
    ds = [round(d(f, R), 2) for f in (f0, f1, f2) if f is not None]
    check(dk == 1 and len(ds) == 3 and all(x <= fl for x in ds),
          f"k1d_ii_opacity_blend: opacity + blend unchanged across 0 -> 1 -> 0 (deck shown at W+0.5 {dk}; d to R at W, "
          f"W+0.5, W+2.0 = {ds}, floor {fl:.2f})")


def k1d_iii_connection():
    cfg = FIX["k1d"]["iii"]
    conn = {"opacity": {"src": {"kind": "signal", "name": "Volume"},
                        "shape": {"min": float(cfg["min"]), "max": float(cfg["max"]), "invert": False,
                                  "playback": "forward", "loop": True, "curve": "Linear", "inMin": 0.0, "inMax": 1.0,
                                  "smoothMs": 0.0}, "enabled": True}}
    if not show("k1d_iii", 2, 1, 1, {(0, 0, 0): img(0, 0, conns=conn), (1, 0, 0): img(1, 0)}):
        return
    trig(0, 0); time.sleep(0.8)
    info("k1d_iii_connection: source = signal 'Volume' (RMS) driven by POST /api/inject_features rms")
    inject(rms=1.0); time.sleep(0.6)
    Rmax, fl = ref_pair("k1d_iii_max")
    inject(rms=0.0); time.sleep(0.6)
    Rmin = cap("k1d_iii_min")
    if Rmax is None or Rmin is None:
        no("k1d_iii_connection: capture failed"); return
    if not check(d(Rmax, Rmin) >= float(cfg["valid"]),
                 f"k1d_iii_connection: VALID -- the connection drives the clip opacity (d(Rmax, Rmin)="
                 f"{d(Rmax, Rmin):.2f} >= {cfg['valid']})"):
        return
    switch(1)
    inject(rms=1.0); time.sleep(0.6); Cmax = cap("k1d_iii_Cmax")
    inject(rms=0.0); time.sleep(0.6); Cmin = cap("k1d_iii_Cmin")
    dk = active_deck()
    switch(0)
    if Cmax is None or Cmin is None:
        no("k1d_iii_connection: capture failed"); return
    check(dk == 1 and d(Cmax, Rmax) <= fl and d(Cmin, Rmin) <= fl,
          f"k1d_iii_connection: the connected clip parameter keeps working with deck 1 shown (activeDeck {dk}; "
          f"d(Cmax, Rmax)={d(Cmax, Rmax):.2f}, d(Cmin, Rmin)={d(Cmin, Rmin):.2f}, floor {fl:.2f})")


# ------------------------------------------------------------------ K2
def k2_nothing_unseen():
    cfg = FIX["k2"]
    va, vb = ramp("k2_d0r0"), ramp("k2_d1r1")
    if va is None or vb is None or not show("k2", 2, 2, 1, {(0, 0, 0): vid(va), (1, 1, 0): vid(vb)}, {1: {"type": 1}}):
        return
    switch(0); trig(0, 0); time.sleep(0.5)
    switch(1); trig(1, 0); time.sleep(0.5)
    t0 = time.time()
    for dk in (0, 1, 0, 1, 0):
        switch(dk, wait=False); time.sleep(1.5)
    info(f"k2_nothing_unseen: 5 switches in {time.time() - t0:.1f} s; activeDeck {active_deck()}")
    c1 = comp(); time.sleep(1.0); c2 = comp()
    adv, unseen = [], []
    for key, cj in clips(c1).items():
        p1, p2 = cj.get("playheadPosition"), playhead(c2, key)
        if p1 is None or p2 is None:
            continue
        if float(p2) - float(p1) > float(cfg["adv"]):
            adv.append(key)
            if not (in_layer(c1, key) or in_layer(c2, key)):
                unseen.append((key, round(float(p2) - float(p1), 4)))
    info(f"k2_nothing_unseen: advancing clips (deck, row, col) {adv}")
    if not check(len(adv) >= int(cfg["minAdv"]), f"k2_nothing_unseen: VALID -- {len(adv)} clips advance (>= "
                                                 f"{cfg['minAdv']})"):
        return
    check(not unseen, f"k2_nothing_unseen: no clip advances unseen (advancing AND not in a layer = {len(unseen)}: "
                      f"{unseen})")


def k2v_decode():
    if not bf9b_only("k2v_decode"):
        return
    cfg = FIX["k2v"]; fps = float(FIX["video"]["fps"])
    cells = {}
    for dk in range(20):
        for li in range(3):
            v = ramp(f"k2v_d{dk}r{li}")
            if v is None:
                return
            cells[(dk, li, 0)] = vid(v)
    if not show("k2v", 20, 3, 1, cells, {1: {"type": 1}, 2: {"type": 1}}):
        return
    t0 = time.time(); st = {}
    while time.time() - t0 < 15.0:
        st = state() or {}
        if st.get("video_players") == int(cfg["players"]):
            break
        time.sleep(0.5)
    if not check(st.get("video_players") == int(cfg["players"]),
                 f"k2v_decode: VALID -- video_players {st.get('video_players')} == {cfg['players']}"):
        return
    switch(4); trig(0, 0); time.sleep(3.0)                        # the "left" clip: deck 4 row 0
    switch(0); trig(0, 0); time.sleep(0.3)
    switch(7); trig(1, 0); time.sleep(0.3)
    switch(13); trig(2, 0); time.sleep(0.3)
    switch(19); time.sleep(3.0)
    t0 = time.time(); ss = []; cA = cB = None
    for k in range(11):
        sleep_until(t0 + 0.5 * k)
        ss.append(state() or {})
        if k == 0:
            cA = comp()
        if k == 10:
            cB = comp()
    awake = [s.get("video_threads_awake") for s in ss[:10]]
    dec = [s.get("video_frames_decoded") for s in ss]
    info(f"k2v_decode: video_threads_awake per sample {awake}; video_frames_decoded {dec[0]} -> {dec[10]}")
    check(all(isinstance(a, int) and a <= int(cfg["awakeMax"]) for a in awake)
          and sum(1 for a in awake if a == int(cfg["awakeEq"])) >= int(cfg["awakeEqMin"]),
          f"k2v_decode: (1) only the 3 layer clips decode (awake <= {cfg['awakeMax']} on every sample, == "
          f"{cfg['awakeEq']} on {sum(1 for a in awake if a == int(cfg['awakeEq']))} of 10, need >= {cfg['awakeEqMin']})")
    lo, hi = 3 * fps * 5 * 0.5, 3 * fps * 5 * 1.5 + 30
    delta = (dec[10] - dec[0]) if isinstance(dec[0], int) and isinstance(dec[10], int) else None
    check(delta is not None and lo <= delta <= hi,
          f"k2v_decode: (2) video_frames_decoded 5-s delta {delta} in [{lo:.0f}, {hi:.0f}] (fps {fps:.0f})")
    moved = []
    for key, cj in clips(cA).items():
        p1, p2 = cj.get("playheadPosition"), playhead(cB, key)
        if p1 is None or p2 is None:
            continue
        if (key == (4, 0, 0) or not (in_layer(cA, key) or in_layer(cB, key))) and abs(float(p2) - float(p1)) > float(cfg["still"]):
            moved.append((key, round(float(p2) - float(p1), 4)))
    check(not moved, f"k2v_decode: (3) the left clip (deck 4 row 0) and every non-layer clip stay still (moved > "
                     f"{cfg['still']}: {moved[:6]})")


# ------------------------------------------------------------------ K3
def k3_autopilot():
    cfg = FIX["k3"]
    ap = dict(autopilotAction=2, autopilotDuration=3)
    cells = {(dk, 0, c): img(dk, c, **ap) for dk in range(2) for c in range(3)}
    sw = {int(k): v for k, v in cfg["switches"].items()}

    def run(tag, switches):
        global BEAT, BIB
        if not show(tag, 2, 1, 3, cells, {0: {"autopilotEnabled": True}}):
            return None
        inject(beatPhase=0.5, totalBeatCount=BEAT, beatInBar=BIB); time.sleep(0.3)
        trig(0, 0); time.sleep(0.8)
        r0, fl = ref_pair(tag)
        snaps, caps = [comp()], {}
        for k in range(1, int(cfg["beats"]) + 1):
            beats(1)
            if k in switches:
                switch(switches[k])
            time.sleep(0.3)
            if k in cfg["at"]:
                caps[k] = cap(f"{tag}_beat{k}")
            snaps.append(comp())
        return r0, fl, caps, snaps

    R1 = run("k3_R1", {})
    R2 = run("k3_R2", sw)
    if R1 is None or R2 is None or R1[0] is None:
        no("k3_autopilot: run failed"); return
    col4 = dc(at(R1[3][4], 0))
    if not check(col4 == (0, 1), f"k3_autopilot: VALID -- R1's autopilot advanced (layer 0 at beat 4 plays {col4}, "
                                 f"expected (0, 1))"):
        return
    fl = max(R1[1], R2[1])
    ds = {k: (round(d(R2[2][k], R1[2][k]), 2) if R1[2].get(k) is not None and R2[2].get(k) is not None else None)
          for k in cfg["at"]}
    check(all(v is not None and v <= fl for v in ds.values()),
          f"k3_autopilot: R2 (switches at beats 3 and 9) shows R1's frames at beats 4 / 8 / 12 (d {ds}, floor {fl:.2f})")
    changes = []
    snaps = R2[3]
    for k in range(1, len(snaps)):
        a, b = snaps[k - 1], snaps[k]
        if a is None or b is None:
            continue
        for dk, (da, db) in enumerate(zip(a.get("decks", []), b.get("decks", []))):
            for li, (la, lb) in enumerate(zip(da.get("layers", []), db.get("layers", []))):
                playing_a = dc(at(a, li)) is not None and dc(at(a, li))[0] == dk
                playing_b = dc(at(b, li)) is not None and dc(at(b, li))[0] == dk
                if ARM == "STAGE_P":
                    playing_a = a.get("activeDeck") == dk
                    playing_b = b.get("activeDeck") == dk
                if not playing_a and not playing_b and la.get("activeClipColumn") != lb.get("activeClipColumn"):
                    changes.append((k, f"deck {dk} row {li} activeClipColumn {la.get('activeClipColumn')} -> "
                                       f"{lb.get('activeClipColumn')}"))
        ca, cb = clips(a), clips(b)
        for key, cj in ca.items():
            if in_layer(a, key) or in_layer(b, key):
                continue
            p1, p2 = cj.get("playheadPosition"), playhead(b, key)
            if p1 is not None and p2 is not None and abs(float(p2) - float(p1)) > 0.001:
                changes.append((k, f"clip {key} playhead {p1} -> {p2}"))
    check(not changes, f"k3_autopilot: nothing outside a playing layer changes (activeClipColumn / playheadPosition; "
                       f"{len(changes)} changes: {changes[:4]})")
    info("k3_autopilot: beatsPlayed is not in GET /api/composition on either arm -- not read")


# ------------------------------------------------------------------ K4
def k4_ignore_column_across():
    cells = {(dk, li, c): img(dk, 3 * li + c) for dk in range(2) for li in range(3) for c in range(2)}
    kw = {li: dict(THIRD(li), type=1) for li in range(3)}
    kw[2]["ignoreColumnTrigger"] = True
    if not show("k4", 2, 3, 2, cells, kw):
        return
    column(0); time.sleep(0.5)
    trig(2, 1); time.sleep(1.0)
    c0 = comp()
    info(f"k4_ignore_column_across: before (deck 0 shown): layers play {[dc(r['active']) for r in plays(c0)]}")
    before, fl = ref_pair("k4")
    if before is None:
        no("k4_ignore_column_across: capture failed"); return
    switch(1)
    column(1); time.sleep(0.5)
    c = comp(); f = cap("k4_after")
    r = [dc(x["active"]) for x in plays(c)]
    info(f"k4_ignore_column_across: after (deck 1 shown, column 1 fired): layers play {r}")
    if f is None:
        no("k4_ignore_column_across: capture failed"); return
    d2 = d(region(f, 2), region(before, 2))
    check(len(r) == 3 and r[2] == (0, 1) and d2 <= fl,
          f"k4_ignore_column_across: the Ignore Column layer keeps deck 0's X (REST {r[2] if len(r) == 3 else r}; "
          f"its region d={d2:.2f}, floor {fl:.2f})")
    check(len(r) == 3 and r[0] == (1, 1) and r[1] == (1, 1),
          f"k4_ignore_column_across: the other layers play deck 1's column 1 ({r[:2]})")
    info(f"k4_ignore_column_across: regions 0 / 1 moved d={d(region(f, 0), region(before, 0)):.2f} / "
         f"{d(region(f, 1), region(before, 1)):.2f}")
    switch(0)


def k4b_empty_cell():
    if not bf9b_only("k4b_empty_cell"):
        return
    cells = {(0, li, 0): img(0, li) for li in range(3)}
    cells[(1, 0, 1)] = img(1, 1)
    kw = {li: dict(THIRD(li), type=1) for li in range(3)}
    kw[2]["ignoreColumnTrigger"] = True
    if not show("k4b", 2, 3, 2, cells, kw):
        return
    trig(0, 0); trig(2, 0); time.sleep(1.0)
    empty1 = cap("k4b_layer1_empty")
    trig(1, 0); time.sleep(1.0)
    before, fl = ref_pair("k4b")
    switch(1)
    column(1); time.sleep(0.5)
    c = comp(); f = cap("k4b_after")
    if empty1 is None or before is None or f is None or c is None:
        no("k4b_empty_cell: capture failed"); return
    ids = [dk.get("id") for dk in c.get("decks", [])]
    L = c.get("layers", [])
    a = [(x.get("activeClip") or {}) for x in L]
    info(f"k4b_empty_cell: deck ids {ids}; layers' activeClip {[(x.get('deckId'), x.get('column')) for x in a]}")
    check(len(a) == 3 and a[0].get("deckId") == ids[1] and a[0].get("column") == 1,
          f"k4b_empty_cell: layer 0 plays deck 1's column 1 ({a[0] if a else None})")
    d1 = d(region(f, 1), region(empty1, 1))
    check(len(a) == 3 and int(a[1].get("column", -1)) < 0 and d1 <= fl,
          f"k4b_empty_cell: layer 1 goes empty (Q5's default; activeClip column {a[1].get('column') if a else None}; "
          f"region d to the layer-1-empty capture {d1:.2f}, floor {fl:.2f})")
    d2 = d(region(f, 2), region(before, 2))
    check(len(a) == 3 and a[2].get("deckId") == ids[0] and a[2].get("column") == 0 and d2 <= fl,
          f"k4b_empty_cell: the Ignore Column layer keeps deck 0's clip ({a[2] if a else None}; region d={d2:.2f})")
    switch(0)


# ------------------------------------------------------------------ K5
def k5_queue_link_off():
    global BIB
    cells0 = {(0, 0, 0): img(0, 0), (0, 0, 1): img(0, 1), (1, 0, 0): img(1, 0)}
    if not show("k5_ref", 2, 1, 2, cells0):
        return
    trig(0, 1); time.sleep(1.0)
    refB, fl = ref_pair("k5_refB")
    cells = {(0, 0, 0): img(0, 0), (0, 0, 1): img(0, 1, beatSnap=True, beatSnapMode=2), (1, 0, 0): img(1, 0)}
    if refB is None or not show("k5", 2, 1, 2, cells):
        no("k5_queue_link_off: setup failed"); return
    inject(beatPhase=0.5, totalBeatCount=BEAT, beatInBar=BIB); time.sleep(0.3)
    while BIB != 0:                                   # land on a bar line first
        beats(1)
    trig(0, 0); time.sleep(0.5)
    beats(1)                                          # beatInBar 1
    trig(0, 1); time.sleep(0.3)
    cq = comp()
    info(f"k5_queue_link_off: queued at beatInBar {BIB}: layer 0 active {dc(at(cq, 0))}, pending "
         f"{dc(at(cq, 0, 'pending'))}")
    switch(1); time.sleep(0.4)
    beats(2)                                          # beatInBar 2, 3: no bar yet
    c_mid = comp()
    held = dc(at(c_mid, 0))
    beats(1)                                          # the bar (beatInBar 0)
    time.sleep(0.4)
    c = comp(); f = cap("k5_after_bar")
    fired = dc(at(c, 0))
    info(f"k5_queue_link_off: activeDeck {c.get('activeDeck') if c else None}; layer 0 before the bar {held}, after "
         f"{fired}")
    check(held == (0, 0) and fired == (0, 1),
          f"k5_queue_link_off: the queued trigger survives the switch and fires ON the bar (before the bar {held}, "
          f"after {fired}; expected (0, 0) -> (0, 1))")
    if f is not None:
        check(d(f, refB) <= fl, f"k5_queue_link_off: the frame after the bar is B (d={d(f, refB):.2f}, floor {fl:.2f})")
    switch(0)


def k5_queue_link_on():
    blocked("k5_queue_link_on: no driver -- neither arm is built with Ableton Link (CMake AUDIODNA_BUILD_LINK OFF, "
            "build-lane/CMakeCache.txt) and Link's only switch is the TopBar toggle (no REST / OSC route; no synthetic "
            "input). Bar unchanged; needs a Link build + a Link driver (Harmony's call)")


# ------------------------------------------------------------------ K7
def k7_old_show():
    ensure_arm()
    pat = "old show converted:"
    conn = {"opacity": {"src": {"kind": "signal", "name": "Volume"},
                        "shape": {"min": 0.2, "max": 1.0, "invert": False, "playback": "forward", "loop": True,
                                  "curve": "Linear", "inMin": 0.0, "inMax": 1.0, "smoothMs": 0.0}, "enabled": True}}
    d1 = [layer(0, [img(0, 0)], 0, opacity=1.0), layer(1, [img(0, 1)], 1, opacity=0.8, persistent=True)]
    d2 = [layer(0, [img(1, 0)], 0, opacity=0.5),
          layer(1, [img(1, 1)], 1, opacity=0.6, blendMode=2, conns=conn, layerEffects=[fx(["Invert", 1.0])])]
    n0 = logcount(pat)
    if not load("k7_old_show", {"name": "boxes-k7-old", "activeDeckIndex": 0, "masterOpacity": 1.0,
                                "globalTransitionSpeed": float(FIX["k7"]["fade"]),
                                "decks": [{"name": "Deck 1", "id": 100, "numColumns": 1, "layers": d1},
                                          {"name": "Deck 2", "id": 101, "numColumns": 1, "layers": d2}]}):
        return
    time.sleep(0.8)
    c = comp() or {}
    if ARM == "BF9B":
        L = c.get("layers", [])
        got = [(x.get("type"), round(float(x.get("opacity", -1)), 2), x.get("blendMode")) for x in L]
    else:
        L = (c.get("decks") or [{}])[0].get("layers", [])
        got = [(None, round(float(x.get("opacity", -1)), 2), x.get("blendMode")) for x in L]
    info(f"k7_old_show: layer settings (type, opacity, blendMode) {got}")
    check(len(got) == 2 and got[0][1:] == (1.0, 0) and got[1][1:] == (0.8, 0),
          f"k7_old_show: the first deck's layer settings win ({got[:2]})")
    n1 = logcount(pat)
    lines = []
    for name in ("out.log", "err.log"):
        p = os.path.join(OUT, name)
        if os.path.exists(p):
            lines += [ln.strip() for ln in open(p, errors="replace") if pat in ln]
    info(f"k7_old_show: logLine(s): {lines[-1:] if lines else []}")
    check(n1 - n0 == 1, f"k7_old_show: exactly one '{pat}' logLine for the load ({n1 - n0})")
    # s-rta-1003, adoption item 11 (K7 / B5): the "load_notice is non-empty" clause is gone -- Boris: "We don't need any
    # text indicating what has happened or what has happened. [...] Please remove it cleanly and completely." The
    # logLine clause above is the trace of the conversion.
    k7_save_reload(pat, n1)
    # the reload half: a NEW-format show (what Save writes: top-level layers, rows of clips only)
    new = {"name": "boxes-k7-new", "activeDeckIndex": 0, "masterOpacity": 1.0,
           "layers": [{k: v for k, v in x.items() if k not in ("clips", "persistent")} for x in d1],
           "decks": [{"name": "Deck 1", "id": 100, "numColumns": 1, "layers": [{"clips": [img(0, 0)]},
                                                                               {"clips": [img(0, 1)]}]},
                     {"name": "Deck 2", "id": 101, "numColumns": 1, "layers": [{"clips": [img(1, 0)]},
                                                                               {"clips": [img(1, 1)]}]}]}
    if not load("k7_new_show", new):
        return
    time.sleep(0.8)
    n2 = logcount(pat)
    # s-rta-1003, adoption item 11: the "load_notice empty" clause is gone (Boris's sentence at the row's first clause)
    check(n2 == n1, f"k7_old_show: a new-format show loads with no note (new logLines {n2 - n1})")


def k7_save_reload(pat, n_before):
    # gates-r2 NIT 5 (ruling-bf9b-merge AM-18) -- Boris: "when I switch between decks, do not change the clips playing
    # in the layers or how they are playing. treat the decks as just a box of clips"
    """K7 / B5 "save + reload" (bf9b fix round): with the converted old show loaded, POST /api/debug/save_composition
    (File > Save As... to a file in the out dir, no chooser) -> the file is written in the new shape (top-level
    "layers"; no "persistent", no "globalTransitionSpeed"); POST /api/load_composition of that file -> no new "old show
    converted:" logLine. The route is BF9B-only (STAGE_P has no save driver: N/A). s-rta-1003, adoption item 11: the two
    load_notice clauses ("the save retires the load notice", "load_notice empty" after the reload) are gone -- Boris:
    "We don't need any text indicating what has happened or what has happened. [...] Please remove it cleanly and
    completely."."""
    saved = os.path.join(OUT, "k7_saved.json")
    code, body = post("/api/debug/save_composition", {"path": saved})
    if code == 404:
        na(f"k7_old_show save + reload: /api/debug/save_composition is a BF9B-only driver (HTTP {code} on this arm)")
        return
    if code != 200 or body.get("ok") is not True:
        no(f"k7_old_show save + reload: save_composition rejected (HTTP {code} {json.dumps(body)[:160]})"); return
    t0 = time.time()
    while not os.path.exists(saved) and time.time() - t0 < 5.0:
        time.sleep(0.1)
    time.sleep(0.3)
    try:
        sj = json.load(open(saved))
    except Exception as e:  # noqa: BLE001
        no(f"k7_old_show save + reload: the saved file is missing or unreadable ({e})"); return
    txt = open(saved).read()
    has_persist, has_fade = '"persistent"' in txt, '"globalTransitionSpeed"' in txt
    nl = len(sj.get("layers") or []) if isinstance(sj.get("layers"), list) else -1
    check(nl == 2 and not has_persist and not has_fade,
          f"k7_old_show save: the saved show has the new shape (top-level layers {nl}; key 'persistent' {has_persist}; "
          f"key 'globalTransitionSpeed' {has_fade})")
    try:
        r = S.post(A + "/api/load_composition", json={"path": saved}, timeout=15)
        good = r.ok and r.json().get("ok") is True
    except Exception as e:  # noqa: BLE001
        good = False; r = e
    if not good:
        no(f"k7_old_show reload: load_composition of the saved file rejected: {str(getattr(r, 'text', r))[:160]}"); return
    time.sleep(1.2)
    n2 = logcount(pat)
    c = comp() or {}
    got = [(round(float(x.get("opacity", -1)), 2), x.get("blendMode")) for x in c.get("layers", [])]
    check(n2 == n_before and got == [(1.0, 0), (0.8, 0)],
          f"k7_old_show reload: the saved show reloads with no note (new logLines {n2 - n_before}) and the first "
          f"deck's layer settings {got}")


def take_is_old(folder):
    try:
        tj = json.load(open(os.path.join(folder, "take.json")))
    except Exception as e:  # noqa: BLE001
        return False, f"take.json unreadable: {e}"
    cp0 = tj.get("checkpoint0") or {}
    lane = [ln for ln in tj.get("lanes", []) if (ln.get("key") or {}).get("control") == "activeDeck"]
    pts = len(lane[0].get("points", [])) if lane else 0
    return ("layers" not in cp0 and pts >= 2), f"checkpoint0 has 'layers': {'layers' in cp0}; activeDeck lane points {pts}"


def k7_old_take():
    ensure_arm()
    cells = {(0, 0, 0): img(0, 0), (1, 0, 0): img(1, 0)}
    take = OLD_TAKE
    if ARM == "STAGE_P" and not take:
        if not show("k7_take_rec", 2, 1, 1, cells):
            return
        trig(0, 0); time.sleep(1.0)
        name = "probe-boxes-k7-" + time.strftime("%Y%m%d-%H%M%S")
        code, r = post("/api/perf/record", {"name": name, "audio": False})
        T = time.time()
        for off, dk in ((1.0, 1), (2.0, 0), (3.0, 1)):
            sleep_until(T + off); switch(dk, wait=False)
        sleep_until(T + 4.0); post("/api/perf/stop")
        time.sleep(2.0)
        st = S.get(A + "/api/perf/status", timeout=6).json()
        src = st.get("takeFolder", "")
        if not src or not os.path.isdir(src):
            no(f"k7_old_take: recording failed (record {code} {r}; status takeFolder {src!r})"); return
        take = os.path.join(OUT, "k7-old-take.adna-take")
        shutil.move(src, take)
        info(f"k7_old_take: recorded take {take} (moved out of {os.path.dirname(src)})")
    if not take:
        no("k7_old_take: the BF9B arm needs BOXES_OLD_TAKE = a take the STAGE_P arm recorded"); return
    old, why = take_is_old(take)
    if not check(old, f"k7_old_take: the take is pre-bf9b with an activeDeck lane ({why})"):
        return
    if not show("k7_take_play", 2, 1, 1, cells):
        return
    trig(0, 0); time.sleep(1.0)
    before, fl = ref_pair("k7_take")
    if before is None:
        no("k7_old_take: capture failed"); return
    code1, r1 = post("/api/perf/load", {"folder": take}); time.sleep(1.5)
    code2, r2 = post("/api/perf/play", {"withAudio": False}); t0 = time.time()
    decks, ds, k = set(), [], 0
    while time.time() - t0 < 5.5:
        decks.add(active_deck())
        f = cap(f"k7_take_play{k:02d}"); k += 1
        if f is not None:
            ds.append(round(d(f, before), 2))
        time.sleep(0.2)
    st = S.get(A + "/api/perf/status", timeout=6).json()
    post("/api/perf/stop_play")
    err = (st.get("lastError") or "").strip()
    info(f"k7_old_take: load {code1} {r1}, play {code2} {r2}; lastError {err!r}; activeDeck values during the replay "
         f"{sorted(x for x in decks if x is not None)}")
    check(code1 == 200 and code2 == 200 and r1.get("ok") and r2.get("ok") and not err,
          "k7_old_take: the pre-bf9b take loads and replays without error")
    check(len([x for x in decks if x is not None]) >= 2, "k7_old_take: its activeDeck lane replays (activeDeck moves)")
    check(len(ds) >= 10 and all(x <= fl for x in ds),
          f"k7_old_take: the lane changes only the grid -- every capture during the replay within floor (max "
          f"{max(ds) if ds else None}, floor {fl:.2f}, {len(ds)} captures)")


# ------------------------------------------------------------------ K8
def k8_twenty_decks():
    cfg = FIX["k8"]
    cells = {(dk, 0, 0): img(dk, 0) for dk in range(20)}
    if not show("k8", 20, 1, 1, cells):
        return
    trig(0, 0); time.sleep(1.0)
    before, fl = ref_pair("k8")
    if before is None:
        no("k8_twenty_decks: capture failed"); return
    bad, rows = [], []
    t0 = time.time()
    for dk in list(range(1, 20)) + [0]:
        state()                                       # resets peak_frame_time_ms
        switch(dk, wait=False)
        c = wait_for(lambda c: c.get("activeDeck") == dk, limit=0.4, step=0.03)
        f = cap(f"k8_d{dk:02d}")
        s = state() or {}
        pk = float(s.get("peak_frame_time_ms", -1.0))
        dd = round(d(f, before), 2) if f is not None else None
        rows.append((dk, None if c is None else c.get("activeDeck"), dd, round(pk, 1)))
        if c is None or dd is None or dd > fl or not (0.0 <= pk <= float(cfg["peakMs"])):
            bad.append(rows[-1])
    el = time.time() - t0
    info(f"k8_twenty_decks: (target, activeDeck, d, peak ms) {rows}")
    check(el <= float(cfg["within"]), f"k8_twenty_decks: 20 switches in {el:.2f} s (<= {cfg['within']})")
    check(not bad, f"k8_twenty_decks: every switch shows its target, the picture holds (floor {fl:.2f}) and the peak "
                   f"frame time stays <= {cfg['peakMs']} ms ({len(bad)} bad: {bad[:4]})")


def k8b_browse_fire():
    if not bf9b_only("k8b_browse_fire"):
        return
    cfg = FIX["k8"]
    cells = {(dk, li, c): img(dk, 3 * li + c) for dk in range(20) for li in range(2) for c in range(3)}
    del cells[(7, 1, 0)]                               # deck 7 row 1 col 0 is empty: firing it clears layer 1
    if not show("k8b", 20, 2, 3, cells, {0: dict(HALF(0), type=1), 1: dict(HALF(1), type=1)}):
        return
    switch(7); trig(1, 2); time.sleep(1.0)
    rr = cap("k8b_Rright")
    trig(1, 0); time.sleep(0.8)                        # cleared
    switch(0); trig(0, 0); time.sleep(1.0)
    pre, fl = ref_pair("k8b_pre")
    if rr is None or pre is None:
        no("k8b_browse_fire: capture failed"); return
    c = comp() or {}
    info(f"k8b_browse_fire: before the walk layers play {[dc(r['active']) for r in plays(c)]}")
    deck7 = c["decks"][7]["id"]
    bad, rows, fired = [], [], False
    for dk in list(range(1, 20)) + [0]:
        state()
        switch(dk)
        f = cap(f"k8b_d{dk:02d}")
        s = state() or {}
        pk = float(s.get("peak_frame_time_ms", -1.0))
        cc = comp() or {}
        if f is None:
            bad.append((dk, "capture failed")); continue
        a1 = (cc.get("layers", [{}, {}])[1].get("activeClip") or {})
        dl = round(d(region(f, 0, 2), region(pre, 0, 2)), 2) if f is not None else None
        dr = round(d(region(f, 1, 2), region(rr, 1, 2)), 2) if f is not None else None
        rows.append((dk, dl, dr if fired else "-", (a1.get("deckId"), a1.get("column")), round(pk, 1)))
        if dl > fl or not (0.0 <= pk <= float(cfg["peakMs"])):
            bad.append(rows[-1])
        if fired and (dr > fl or a1.get("deckId") != deck7 or a1.get("column") != 2):
            bad.append(rows[-1])
        if dk == 7:
            trig(1, 2); time.sleep(0.8); fired = True
            f2 = cap("k8b_fired")
            cc = comp() or {}
            a1 = (cc.get("layers", [{}, {}])[1].get("activeClip") or {})
            dr2 = round(d(region(f2, 1, 2), region(rr, 1, 2)), 2) if f2 is not None else None
            rows.append(("fire", None, dr2, (a1.get("deckId"), a1.get("column")), None))
            if dr2 is None or dr2 > fl or a1.get("deckId") != deck7 or a1.get("column") != 2:
                bad.append(rows[-1])
    info(f"k8b_browse_fire: (deck, d left vs pre, d right vs Rright, layer 1 activeClip, peak ms) {rows}")
    check(not bad, f"k8b_browse_fire: browse 20 decks and fire on deck 7: the left half holds, the right half shows "
                   f"deck 7 col 2 from the fire on, peak <= {cfg['peakMs']} ms (floor {fl:.2f}; {len(bad)} bad: "
                   f"{bad[:4]})")


# ------------------------------------------------------------------ K9 / K10
def k9_setup(tag):
    v = ramp(tag)
    if v is None:
        return None
    cells = {(dk, 0, 0): img(dk, 0) for dk in range(3)}
    cells[(2, 1, 0)] = vid(v)
    if not show(tag, 3, 2, 1, cells):
        return None
    switch(2); trig(1, 0); switch(0)
    time.sleep(4.0)
    return v


def k1b_bar(tag, act, check_rest):
    """K1b's bar on layer 1 (the top, Opaque, full frame) across `act`: t at +2 s within 0.5 of t_act + elapsed."""
    wb = time.time(); fb = cap(f"{tag}_before")
    act()
    c = check_rest()
    sleep_until(wb + 2.0)
    wa = time.time(); fa = cap(f"{tag}_after2")
    if fb is None or fa is None:
        no(f"{tag}: capture failed"); return
    tb, ta, el = t_of(fb), t_of(fa), wa - wb
    check(tb >= 3.0 and abs(ta - (tb + el)) <= 0.5,
          f"{tag}: K1b's bar on layer 1 (t {tb:.2f} s -> {ta:.2f} s over {el:.2f} s; expected {tb + el:.2f} +- 0.5)")
    return c


def k9a_remove_playing():
    if not bf9b_only("k9a_remove_playing"):
        return
    if k9_setup("k9a") is None:
        return
    c0 = comp() or {}
    n0 = len(c0.get("decks", []))

    def rm():
        post("/api/debug/remove_deck", {"deck": 2})
    c = k1b_bar("k9a", rm, lambda: wait_for(lambda c: len(c.get("decks", [])) == n0 - 1, limit=1.5))
    c = comp() or {}
    a1 = (c.get("layers", [{}, {}])[1].get("activeClip") or {})
    check(len(c.get("decks", [])) == n0 - 1 and c.get("retiredDeckCount") == 1 and a1.get("retired") is True,
          f"k9a_remove_playing: deck 2 retired while its clip plays (numDecks {n0} -> {len(c.get('decks', []))}, "
          f"retiredDeckCount {c.get('retiredDeckCount')}, layers[1].activeClip {a1})")


def k9c_remove_undo():
    if not bf9b_only("k9c_remove_undo"):
        return
    if k9_setup("k9c") is None:
        return
    c0 = comp() or {}
    n0 = len(c0.get("decks", []))
    id2 = c0["decks"][2]["id"]
    post("/api/debug/remove_deck", {"deck": 2})
    wait_for(lambda c: len(c.get("decks", [])) == n0 - 1, limit=1.5)
    time.sleep(1.0)
    k1b_bar("k9c", lambda: post("/api/debug/undo", {"redo": False}), lambda: wait_for(lambda c: len(c.get("decks", [])) == n0, limit=1.5))
    c = comp() or {}
    a1 = (c.get("layers", [{}, {}])[1].get("activeClip") or {})
    ds = c.get("decks", [])
    check(len(ds) == n0 and len(ds) > 2 and ds[2].get("id") == id2 and c.get("retiredDeckCount") == 0
          and a1.get("deckId") == id2 and a1.get("column") == 0 and a1.get("retired") is False,
          f"k9c_remove_undo: undo brings deck 2 back at index 2 with id {id2}, nothing retired, layer 1 still plays "
          f"it (numDecks {len(ds)}, deck 2 id {ds[2].get('id') if len(ds) > 2 else None}, retiredDeckCount "
          f"{c.get('retiredDeckCount')}, layers[1].activeClip {a1})")


def k9b_remove_midfade():
    if not bf9b_only("k9b_remove_midfade"):
        return
    cfg = FIX["k9"]; T = float(cfg["T"])
    va, vb = ramp("k9b_a"), ramp("k9b_b")
    if va is None or vb is None:
        return
    cells = {(0, 1, 0): img(0, 0), (2, 1, 0): img(2, 0), (2, 0, 0): vid(va), (2, 0, 1): vid(vb), (1, 1, 0): img(1, 0)}
    if not show("k9b", 3, 2, 2, cells, {1: {"transitionSpeed": T}}):
        return
    switch(0); trig(1, 0); time.sleep(1.5)            # first trigger: no fade
    b1 = cap("k9b_refIN1"); time.sleep(0.5); b2 = cap("k9b_refIN2")
    switch(2); trig(1, 0); time.sleep(T + 1.5)        # deck 2's picture (a fade 0 -> 2)
    a1 = cap("k9b_refOUT1"); time.sleep(0.5); a2 = cap("k9b_refOUT2")
    if any(x is None for x in (a1, a2, b1, b2)):
        no("k9b_remove_midfade: reference capture failed"); return
    fl = max(floor_of(a1, a2), floor_of(b1, b2))
    vp0 = (state() or {}).get("video_players")
    switch(0); trig(1, 0); t0 = time.time()            # OUT (deck 2) -> IN (deck 0) over T
    sleep_until(t0 + float(cfg["removeAt"]))
    post("/api/debug/remove_deck", {"deck": 2}); tr = time.time()
    mids, samples, k = [], [], 0
    while time.time() - t0 < T + 1.2:
        c = comp(); tt = time.time()
        if c is not None:
            L1 = c.get("layers", [{}, {}])[1]
            samples.append((round(tt - tr, 2), round(tt - t0, 2), float(L1.get("crossfadeProgress", -1)),
                            (L1.get("previousClip") or {}).get("column"), c.get("retiredDeckCount")))
        if tt - t0 < 0.85 * T:
            f = cap(f"k9b_mid{k:02d}"); k += 1
            if f is not None and time.time() - t0 < 0.85 * T:
                mids.append((round(time.time() - tr, 2), f))
        time.sleep(0.15)
    info(f"k9b_remove_midfade: (s after remove, s after fire, p, previous column, retiredDeckCount) {samples}")
    off = []
    for tt, f in mids:
        p, res = fit(f, a1, b1)
        if not (res <= TOL and 0.02 < p < 0.98):
            off.append((tt, round(p, 2), round(res, 2)))
    check(len(mids) >= 3 and not off, f"k9b_remove_midfade: {len(mids) - len(off)}/{len(mids)} frames after the remove "
                                      f"ON the OUT -> IN line (residual <= {TOL}): off {off[:3]}")
    later = lambda s: next((x[2] for x in samples if x[0] >= s), None)   # noqa: E731
    p05, p2 = later(0.5), later(2.0)
    check(p05 is not None and p2 is not None and p2 - p05 >= 0.15,
          f"k9b_remove_midfade: p rises >= 0.15 between +0.5 s and +2 s ({p05} -> {p2})")
    done = [x for x in samples if x[2] >= 1.0 and (x[3] is None or int(x[3]) < 0) and x[1] <= T + 1.0]
    check(bool(done), f"k9b_remove_midfade: the fade completes by T + 1 s ({done[:1]})")
    alive = [x for x in samples if 0 <= x[2] < 1.0 and x[0] > 0.1]
    check(bool(alive) and all(x[4] == 1 for x in alive),
          f"k9b_remove_midfade: retiredDeckCount == 1 while p < 1 ({[x[4] for x in alive]})")
    post("/api/debug/duplicate_deck", {"deck": 0}); time.sleep(1.5)
    c = comp() or {}
    vp1 = (state() or {}).get("video_players")
    check(c.get("retiredDeckCount") == 0 and isinstance(vp0, int) and isinstance(vp1, int) and vp0 - vp1 == 2,
          f"k9b_remove_midfade: a fenced edit (duplicate_deck 0) reaps it (retiredDeckCount {c.get('retiredDeckCount')}; "
          f"video_players {vp0} -> {vp1}, expected -2)")


def k10_fresh_and_resume():
    if not bf9b_only("k10_fresh_and_resume"):
        return
    cfg = FIX["k10"]
    v = ramp("k10")
    if v is None:
        return
    cells = {(dk, 0, 0): img(dk, 0) for dk in range(4)}
    cells[(3, 0, 0)] = vid(v)
    if not show("k10", 4, 1, 1, cells):
        return
    switch(3)
    trig(0, 0); tf = time.time()
    sleep_until(tf + 0.15); switch(0, wait=False)
    sleep_until(tf + 0.25); el = time.time() - tf; f = cap("k10_first")
    if f is None:
        no("k10_fresh_and_resume: capture failed"); return
    check(el <= 0.3 and t_of(f) <= float(cfg["firstMax"]),
          f"k10_fresh_and_resume: (i) a never-played clip starts at its in-point (t {t_of(f):.2f} s at +{el:.2f} s "
          f"after the fire; <= {cfg['firstMax']})")
    t_r, k = None, 0
    while time.time() - tf < 9.0:
        g = cap(f"k10_run{k:02d}"); k += 1
        if g is not None:
            t_r = t_of(g)
            if t_r >= 4.0:
                break
        time.sleep(0.3)
    trig(0, 0)                                         # deck 0's picture into layer 0 (deck 0 is shown)
    time.sleep(0.3)
    p0 = playhead(comp(), (3, 0, 0))
    time.sleep(float(cfg["away"]))
    p1 = playhead(comp(), (3, 0, 0))
    switch(3); trig(0, 0); tf2 = time.time()
    sleep_until(tf2 + 0.2); el2 = time.time() - tf2; g = cap("k10_refire")
    switch(0)
    if g is None or t_r is None or p0 is None or p1 is None:
        no("k10_fresh_and_resume: capture / playhead failed"); return
    check(el2 <= 0.3 and abs(t_of(g) - t_r) <= float(cfg["resumeTol"]),
          f"k10_fresh_and_resume: (ii) the re-fired clip resumes where it left (t_r {t_r:.2f} s, t {t_of(g):.2f} s at "
          f"+{el2:.2f} s; tol {cfg['resumeTol']})")
    check(abs(float(p1) - float(p0)) <= float(cfg["still"]),
          f"k10_fresh_and_resume: (ii) its REST playhead stays still while it is in no layer ({p0} -> {p1})")


def main():
    global ROW
    rows = [("k1a_switch_static", k1a_switch_static), ("k1b_switch_video", k1b_switch_video),
            ("k1b_duplicate", k1b_duplicate),
            ("k1c_switch_midfade", k1c_switch_midfade), ("k1t_history_freeze", k1t_history_freeze),
            ("k1t_history_feedback", k1t_history_feedback), ("k1d_ia_speed_half", k1d_ia_speed_half),
            ("k1d_ib_pingpong", k1d_ib_pingpong), ("k1d_ii_opacity_blend", k1d_ii_opacity_blend),
            ("k1d_iii_connection", k1d_iii_connection), ("k2_nothing_unseen", k2_nothing_unseen),
            ("k2v_decode", k2v_decode), ("k3_autopilot", k3_autopilot),
            ("k4_ignore_column_across", k4_ignore_column_across), ("k4b_empty_cell", k4b_empty_cell),
            ("k5_queue_link_off", k5_queue_link_off), ("k5_queue_link_on", k5_queue_link_on),
            ("k7_old_show", k7_old_show), ("k7_old_take", k7_old_take), ("k8_twenty_decks", k8_twenty_decks),
            ("k8b_browse_fire", k8b_browse_fire), ("k9a_remove_playing", k9a_remove_playing),
            ("k9b_remove_midfade", k9b_remove_midfade), ("k9c_remove_undo", k9c_remove_undo),
            ("k10_fresh_and_resume", k10_fresh_and_resume)]
    unknown = (ONLY or set()) - {n for n, _ in rows}
    if unknown:
        no(f"unknown row(s): {sorted(unknown)}")
    for name, fn in rows:
        if ONLY is None or name in ONLY:
            print(f"--- {name}", flush=True)
            ROW = name
            ensure_arm()
            fn()
    # ruling-bf9b-merge AM-9: the names of the blocked rows and the size of the row list, for probe-boxes.sh's verdict
    # line and its row-count pin (EXPECTED_ROWS).
    print(f"\nPY-ROWS registered {len(rows)}", flush=True)
    print(f"PY-BLOCKED-ROWS [{', '.join(BLOCKED_ROWS)}]", flush=True)
    print(f"PY {PASS} PASS / {FAIL} FAIL / {BLOCKED} BLOCKED (arm {ARM})", flush=True)
    # bf9b fix round (Harmony ruling R-N3): a BLOCKED bar never ran, so it is never a pass -- exit 0 only when nothing
    # failed AND nothing was blocked; FAIL == 0 with BLOCKED rows exits 3 (probe-boxes.sh prints "PROBE-BOXES BLOCKED <n>").
    sys.exit(1 if FAIL else (3 if BLOCKED else 0))


if __name__ == "__main__":
    main()
