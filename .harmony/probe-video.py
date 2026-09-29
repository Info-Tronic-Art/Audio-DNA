#!/usr/bin/env python3
"""probe-video.py -- REST/pixel half of .harmony/probe-video.sh (s-rta-0928b video: decode off the GL thread).

The .sh owns launch / refuse / quit; this file encodes the fixtures (--make-fixtures, BEFORE the app starts) and then
talks to the running app on 7070 (production mode). Every captured PNG is decoded with PIL+numpy; every render_frame
response is checked; the output dir is fresh per run. Plan: .harmony/.reports/s-rta-0928b/plan-video.md section 4.6
+ its HARMONY ADOPTION (V2 -> w4b, V4 -> w6b, V5 -> NONREF on/off tables) and ADDENDUM (W1 -> w2 (a)/(a2), W2 -> the
50 ms fps polls and w1's 5 runs).

usage: probe-video.py <root> <fresh-outdir> --make-fixtures [row,row,...]
       probe-video.py <root> <fresh-outdir> [row,row,...]
rows (run order): w5_correctness_gop30 w1_steady_1080x4 w3_retrigger_midgop_1080 w4_deck_return_1080
      w6_crossfade_two_players w6b_retrigger_mid_fade w8_prores_steady w2_steady_4kx4 w3b_retrigger_midgop_4k
      w4b_idle_mid_catchup w7_message_thread_no_wait

Fixtures (probe-video.json "fixtures"; 10 s, 30 fps, 300 frames; ffmpeg nice'd, -threads 2): testsrc2 (b1080: negated)
with a frame-number CODE BAND -- the bottom bandRows rows (64 at 1080p, 128 at 4K), 10 cells, cell c white iff bit c of
(frame number + base) (drawbox per bit). a1080 / b1080 (base 512 = bit 9 "B") / a4k: libx264 GOP 250 (keyframes 0 and
8.333 s, asserted with ffprobe); g30_1080: GOP 30 (the control); pr1080: ProRes 422 (intra-only). $VIDEO_FIXTURES (a dir)
= encode once and reuse (the keyframe list is re-asserted every run); else they go to <out>/media (deleted by the .sh).
Helpers: code(cap, band) thresholds each cell's inner 50 % at 128 -> the frame number; playhead(li) = the layer's active
clip playheadPosition from /api/composition; a frame check reads the playhead right BEFORE and right AFTER the capture
and accepts expected(before) - codeTol <= code <= expected(after) + codeTol (expected(p) = int(p * 300)); settle_late()
polls video_late_frames until its delta over 200 ms is 0 (<= settleLateS: a catch-up ended -- a capture during a hold is a
legitimately held frame, so frame-accuracy checks capture only settled); fixture_frame(f, k) = frame k of fixture f
(ffmpeg select); dbox = 16x16 box means, mean |diff| over RGB. Clip JSON always carries "speed": 1.0 (Clip::fromVar reads it
unguarded). A FIRST trigger plays from 0 (only a retrigger seeks to the in-point, MainComponent handleClipTrigger), so the
mid-GOP rows trigger once and retrigger at once to start at the in-point ("prime"). Perf rows wait for no compiler BEFORE
their first trigger and print the load average and the app's %cpu. A missing /api/state key FAILs the row that needs it.

w5_correctness_gop30 (guard): 1080p canvas; g30. trig; 2 s; caps at +0 / +1 / +2 s. PASS: each code within the playhead
  bracket; dbox(cap, fixture_frame(code)) <= boxTol (a decoded picture, not a stale slot); video_late_frames delta == 0.
w1_steady_1080x4: 4 layers = a1080 (4 players). trig x4; 2 s; s0; 5 s poll; s1 -- w1Repeats times, each on a fresh load
  (new clip ids = new players). The fps rows (w1, w2, w8) poll /api/state every fpsPollS (50 ms, W2: a 15 ms poller
  perturbs the fps it measures); peak_* fields are still maxed over the polls. PASS (a) median fps >= fpsMin in >=
  w1PassMin of the w1Repeats runs (every run's median printed); on the first run: (b) p90 peak_callback_ms <=
  p90CallbackMaxMs; (c) gl_video_decode_calls delta == 0; (d) video_uploads delta in uploads5s; (e) video_late_frames
  delta == 0.
w2_steady_4kx4 (m1): first the same scene with 4 x 4K STILLS (still4k_f100.png = a4k frame 100, one image clip per
  layer, 5 s poll: the upload-free ceiling of this launch), then as w1 at 4K (a4k), one run. PASS (a) median fps >= fps4kMin (W1: the
  upload-bound floor on this M1 Pro; main 36) and (a2) median fps >= stills fps - fps4kBelowStillsMax (both printed);
  (b)-(d) as w1; (e) late delta <= lateMaxSteady4k. "4 x 4K video at the display rate" is filed (zero-copy upload).
w3_retrigger_midgop_1080 (m2): a1080 inPoint 0.5 (5.0 s = frame 150, 150 past keyframe 0). trig + prime; 3 s; capPre;
  s0; RETRIGGER (seekTo(inPoint)); 3 s poll; s1; settle; cap. PASS (a) every poll's peak_callback_ms <= peakMaxMs;
  (b) gl_video_max_decodes_per_call <= 2; (c) late delta <= lateMaxRetrigger1080 (prints the hold in ms); (d) code within
  the bracket and 150 <= code < 300; (e) capPre code in [225, 255] (the clip plays from the in-point).
w3b_retrigger_midgop_4k: as w3 at 4K (a4k); (c) <= lateMaxRetrigger4k; plus capHold 0.3 s after the retrigger (inside the
  hold): (f) mean luma > 20 and code >= 148 or within 3 of capPre's (held or landed) -- never 29 (keyframe + 29, today's
  frozen picture), never 0-2 (a frame-0 stand-in).
w4_deck_return_1080 (rule 15): deck 0 = a1080; deck 1 = warm.png. trig; 2 s; p0 + s0; switch_deck 1; 3.5 s; switch_deck 0;
  1.5 s poll; s1; settle; cap + p1. PASS (a) every poll <= peakMaxMs; (b) video_pending_frames delta == 0; (c) 1 <= late
  delta <= lateMaxReturn (the frames the clock ran past off screen were never decoded: a bounded hold); (d) (p1 - p0) x 10
  >= 5.0 s and the code within the bracket.
w4b_idle_mid_catchup (V2, 4K): deck 0 = a4k inPoint 0.5; deck 1 = warm.png. trig; 3 s; (B) switch_deck 1 (steady play has
  filled the ring), 1.5 s -> video_threads_awake == 0; switch_deck 0; 1 s; (A) retrigger (a catch-up starts), switch_deck 1
  within 100 ms, 1.5 s -> video_threads_awake == 0. video_threads must be 1 (not a vacuous PASS). Both arms stay clear of
  the 10 s Loop wrap (its generation bump would also end a ring-full wait). Teeth (a build without the idle check inside
  the ring-full wait): B FAILs; A parks at the loop top during its catch-up on either build.
w6_crossfade_two_players (Pitfall 35): transitionSpeed 2.0; col 0 = a1080, col 1 = b1080. trig 0; 2 s; s0; trig 1; poll
  2.5 s with capMid at +1.0 s; s1; settle; capAfter. PASS (a) max peak_callback_ms <= peakMaxMs; (b) video_uploads delta in
  [130, 180]; (c) capMid mean luma > 20, dbox to A's and to B's frame both > boxTol (a blend: both chains live); (d) capAfter
  code has bit 9 set and (code & 511) within B's bracket; (e) video_pending_frames delta == 0.
w6b_retrigger_mid_fade (V4): transitionSpeed 2.0; col 0 = a1080 inPoint 0.5, col 1 = b1080 inPoint 0.5. trig 0; 3 s;
  trig 1 (fade starts; B from frame 0); +0.5 s RETRIGGER col 1 (B seeks to 5.0 s, mid-GOP); captures at +0.1 / +0.25 s.
  PASS (a) every poll <= peakMaxMs; (b) the hold captures: mean luma > 20 and code not 29 / 541 (keyframe + 29);
  (c) crossfadeProgress reaches 1 within fadeDoneS of the fade start (the fade keeps running); (d) after settle the code
  has bit 9 and (code & 511) is within B's bracket.
w7_message_thread_no_wait (X1, 4K): layers 101-104 = a4k inPoint 0.5; layer 105 cols 0-1 = warm.png. trig x5; 2 s; s0;
  retrigger 101 (a mid-GOP catch-up starts); 50 ms; then 5 x [a video retrigger of 102 / 103 / 104 (the message thread
  takes videoPlayerMutex_ in getVideoPlayer while a catch-up runs) + trig(105, alternating cols)] 100 ms apart, each image
  trigger timed until /api/composition shows it (it queues behind the video retrigger on the message thread); s1.
  (An image trigger alone never takes the video lock: measured 9 ms on main, 1.44 ms lock wait on the commit-1 app.) PASS (a) msg_video_lock_wait_max_ms <= lockWaitMaxMs; (b) every round trip <=
  triggerRttMaxMs.
w8_prores_steady: pr1080. trig; 2 s; s0; 5 s poll (fpsPollS); s1; settle; cap. PASS: median fps >= fpsMin; decode calls delta 0;
  late delta 0; code within the bracket.
"""
import json, os, statistics, subprocess, sys, threading, time

import numpy as np
import requests
from PIL import Image

A = "http://127.0.0.1:7070"
ROOT, OUT = sys.argv[1], sys.argv[2]
MAKE = len(sys.argv) > 3 and sys.argv[3] == "--make-fixtures"
_rows_arg = (sys.argv[4] if len(sys.argv) > 4 else "") if MAKE else (sys.argv[3] if len(sys.argv) > 3 else "")
ONLY = set(_rows_arg.split(",")) if _rows_arg else None
FIX = json.load(open(os.path.join(ROOT, ".harmony", "probe-video.json")))
PEAK = float(FIX["peakMaxMs"]); TOL = float(FIX["boxTol"]); CT = int(FIX["codeTol"])
LID = FIX["layers"]
SIZES = {"1080": (1920, 1080), "4k": (3840, 2160)}
BAND = {"1080": int(FIX["bandRows"]["1080"]), "4k": int(FIX["bandRows"]["4k"])}
MEDIA = os.environ.get("VIDEO_FIXTURES") or os.path.join(OUT, "media")
PASS = FAIL = 0
S = requests.Session()
S.headers["Connection"] = "close"   # one fresh connection per request (cpp-httplib 5 s keep-alive drop)


def ok(msg):
    global PASS; PASS += 1; print(f"PASS  {msg}", flush=True)


def no(msg):
    global FAIL; FAIL += 1; print(f"FAIL  {msg}", flush=True)


def check(cond, msg):
    (ok if cond else no)(msg)


def mpath(name):
    return os.path.join(MEDIA, name)


# ---------------------------------------------------------------- fixtures (encoded before the app starts)
ROW_FIXTURES = {
    "w5_correctness_gop30": ["g30_1080.mp4"], "w1_steady_1080x4": ["a1080_g250.mp4"],
    "w3_retrigger_midgop_1080": ["a1080_g250.mp4"], "w4_deck_return_1080": ["a1080_g250.mp4"],
    "w6_crossfade_two_players": ["a1080_g250.mp4", "b1080_g250.mp4"], "w6b_retrigger_mid_fade": ["a1080_g250.mp4", "b1080_g250.mp4"],
    "w8_prores_steady": ["pr1080.mov"], "w2_steady_4kx4": ["a4k_g250.mp4"], "w3b_retrigger_midgop_4k": ["a4k_g250.mp4"],
    "w4b_idle_mid_catchup": ["a4k_g250.mp4"], "w7_message_thread_no_wait": ["a4k_g250.mp4"],
}
FPS_POLL = float(FIX["fpsPollS"])   # W2: the fps rows poll at 50 ms (a 15 ms poller perturbs the fps it measures)
STILL4K = "still4k_f100.png"        # w2's upload-free ceiling: a4k frame 100 as a still


def band_filter(band, base):
    vf = [f"drawbox=x=0:y=ih-{band}:w=iw:h={band}:color=black:t=fill"]
    for c in range(10):
        vf.append(f"drawbox=x={c}*iw/10:y=ih-{band}:w=iw/10:h={band}:color=white:t=fill:"
                  f"enable='mod(floor((n+{base})/pow(2\\,{c}))\\,2)'")
    return ",".join(vf)


def keyframes(p):
    r = subprocess.run(["ffprobe", "-v", "error", "-select_streams", "v", "-skip_frame", "nokey", "-show_entries",
                        "frame=pts_time", "-of", "csv=p=0", p], capture_output=True, text=True)
    return [float(x.strip().rstrip(",")) for x in r.stdout.split() if x.strip().rstrip(",")]


def make_fixtures():
    os.makedirs(MEDIA, exist_ok=True)
    need = set()
    for r, fs in ROW_FIXTURES.items():
        if ONLY is None or r in ONLY:
            need.update(fs)
    bad = 0
    for name in sorted(need):
        spec = FIX["fixtures"][name]; p = mpath(name)
        if not os.path.exists(p):
            t0 = time.time()
            pre = "negate," if "negate" in spec["src"] else ""
            cmd = (["nice", "-n", "10", "ffmpeg", "-y", "-loglevel", "error", "-filter_threads", "2", "-f", "lavfi", "-i",
                    f"testsrc2=s={spec['size']}:r=30:d=10", "-vf", pre + band_filter(spec["band"], spec["base"])]
                   + spec["enc"] + ["-threads", "2", p + ".tmp" + os.path.splitext(p)[1]])
            r = subprocess.run(cmd, capture_output=True, text=True)
            if r.returncode != 0:
                no(f"fixture {name}: ffmpeg failed: {r.stderr[:300]}"); bad += 1; continue
            os.rename(p + ".tmp" + os.path.splitext(p)[1], p)
            print(f"fixture {name}: encoded in {time.time() - t0:.1f} s ({la()})", flush=True)
        else:
            print(f"fixture {name}: reused from {MEDIA}", flush=True)
        if spec.get("keys"):
            want = FIX[spec["keys"]]; got = keyframes(p)
            if len(got) != len(want) or any(abs(a - b) > 0.02 for a, b in zip(got, want)):
                no(f"fixture {name}: keyframes {got} != {want}"); bad += 1
            else:
                print(f"fixture {name}: keyframes {got} (as designed)", flush=True)
    if ONLY is None or "w2_steady_4kx4" in ONLY:
        sp = mpath(STILL4K)
        if not os.path.exists(sp) and os.path.exists(mpath("a4k_g250.mp4")):
            subprocess.run(["nice", "-n", "10", "ffmpeg", "-y", "-loglevel", "error", "-threads", "2", "-i", mpath("a4k_g250.mp4"),
                            "-vf", "select='eq(n\\,100)'", "-frames:v", "1", sp], capture_output=True)
        if not os.path.exists(sp):
            no(f"fixture {STILL4K}: ffmpeg made nothing"); bad += 1
        else:
            print(f"fixture {STILL4K}: ready", flush=True)
    warm = FIX["warm"]
    a = np.zeros((warm[1], warm[0], 4), np.uint8); a[..., :3] = warm[2]; a[..., 3] = 255
    Image.fromarray(a, "RGBA").save(os.path.join(OUT, "warm.png"))
    sys.exit(1 if bad else 0)


# ---------------------------------------------------------------- REST helpers
def vclip(cid, name, ip=0.0, **extra):
    c = {"name": f"v{cid}", "id": cid, "mediaType": 2, "mediaFile": mpath(name), "speed": 1.0, "transportMode": 0,
         "loopMode": 0, "reverse": False, "inPoint": ip, "outPoint": 1.0, "effects": []}
    c.update(extra); return c


def iclip(cid, path):
    return {"name": f"i{cid}", "id": cid, "mediaType": 1, "mediaFile": path, "effects": []}


def layer(lid, clips, speed=0.0, **extra):
    l = {"name": f"L{lid}", "id": lid, "opacity": 1.0, "visible": True, "blendMode": 0, "type": 0,
         "transitionSpeed": speed, "layerEffects": [], "clips": clips}
    l.update(extra); return l


def deck(did, layers):
    return {"name": f"D{did}", "id": did, "numColumns": max(len(l["clips"]) for l in layers), "layers": layers}


def state():
    try:
        return S.get(A + "/api/state", timeout=6).json()
    except Exception as e:  # noqa: BLE001
        no(f"/api/state: {e}"); return None


def comp_state():
    try:
        return S.get(A + "/api/composition", timeout=6).json()
    except Exception:  # noqa: BLE001
        return None


def load(tag, decks, size, players):
    W, H = SIZES[size]
    comp = {"name": "video-" + tag, "activeDeckIndex": 0, "masterOpacity": 1.0, "globalTransitionSpeed": 0.0,
            "outputWidth": W, "outputHeight": H, "decks": decks}
    path = os.path.join(OUT, f"comp_{tag}.json")
    json.dump(comp, open(path, "w"), indent=1)
    try:
        r = S.post(A + "/api/load_composition", json={"path": path}, timeout=30)
        good = r.ok and r.json().get("ok") is True
    except Exception as e:  # noqa: BLE001
        good = False; r = e
    if not good:
        no(f"{tag}: load_composition rejected: {str(getattr(r, 'text', r))[:160]}"); return False
    t0 = time.time()
    while time.time() - t0 < 15.0:   # the players are opened on the message thread
        s = state()
        if s is None or "video_players" not in s:
            time.sleep(2.0); break   # an app without the field: wait a fixed 2 s
        if s["video_players"] == players:
            break
        time.sleep(0.1)
    time.sleep(0.5)
    return True


def trig(li, col):
    S.post(A + "/api/trigger_clip", json={"layer": li, "column": col}, timeout=6)


def switch(dk):
    S.post(A + "/api/switch_deck", json={"deck": dk}, timeout=6)


def wait_active(li, col, limit=10.0):
    t0 = time.time()
    while time.time() - t0 < limit:
        c = comp_state()
        try:
            if c["decks"][c["activeDeck"]]["layers"][li]["activeClipColumn"] == col:
                return time.time() - t0
        except Exception:  # noqa: BLE001
            pass
        time.sleep(0.005)
    return None


def layer_json(li, c=None):
    c = c or comp_state()
    try:
        return c["decks"][c["activeDeck"]]["layers"][li]
    except Exception:  # noqa: BLE001
        return None


def playhead(li, col=None):
    L = layer_json(li)
    if L is None:
        return None
    want = L["activeClipColumn"] if col is None else col
    for cj in L.get("clips", []):
        if cj.get("column") == want:
            return float(cj.get("playheadPosition", -1.0))
    return None


def expected(p):
    return int(p * 300) % 300


def cap(name):
    p = os.path.join(OUT, name + ".png")
    if os.path.exists(p):
        os.remove(p)
    t0 = time.time()
    try:
        body = S.post(A + "/api/render_frame", json={"output_path": p, "time": 0.0}, timeout=30).json()
    except Exception as e:  # noqa: BLE001
        no(f"render_frame {name}: {e}"); return None
    if not body.get("ok") or not os.path.isfile(p) or os.path.getmtime(p) < t0 - 0.01:
        no(f"render_frame {name}: response {body} / file {os.path.isfile(p)}"); return None
    return np.asarray(Image.open(p).convert("RGBA")).astype(np.float32)


def cap_bracket(name, li, col=None):
    """(frame, code-ready tuple): playhead before, the capture, playhead after."""
    pb = playhead(li, col); f = cap(name); pa = playhead(li, col)
    return f, pb, pa


def code(a, rows):
    h, w = a.shape[:2]; v = 0
    y0, y1 = h - rows + rows // 4, h - rows // 4
    for c in range(10):
        x0, x1 = int(c * w / 10 + w / 40), int(c * w / 10 + 3 * w / 40)
        if a[y0:y1, x0:x1, :3].mean() >= 128:
            v |= 1 << c
    return v


def in_bracket(c, pb, pa, mask=None):
    """c (or c & mask) within [expected(pb) - CT, expected(pa) + CT] (wrap-aware: a bracket across the loop point)."""
    if pb is None or pa is None:
        return False, "no playhead"
    v = c if mask is None else (c & mask)
    lo, hi = expected(pb) - CT, expected(pa) + CT
    if hi < lo:   # wrapped between the two reads
        hi += 300
        if v < lo:
            v += 300
    return lo <= v <= hi, f"[{expected(pb)}-{CT}, {expected(pa)}+{CT}]"


def luma(a):
    return float((0.299 * a[..., 0] + 0.587 * a[..., 1] + 0.114 * a[..., 2]).mean())


def box(a, b=16):
    h, w = a.shape[0] // b * b, a.shape[1] // b * b
    return a[:h, :w, :3].reshape(h // b, b, w // b, b, 3).mean(axis=(1, 3))


def dbox(x, y):
    if x.shape[:2] != y.shape[:2]:
        y = np.asarray(Image.fromarray(y.astype(np.uint8), "RGBA").resize((x.shape[1], x.shape[0]), Image.BOX)).astype(np.float32)
    return float(np.abs(box(x) - box(y)).mean())


def fixture_frame(name, k):
    p = os.path.join(OUT, f"ff_{os.path.splitext(name)[0]}_{k}.png")
    if not os.path.exists(p):
        subprocess.run(["ffmpeg", "-y", "-loglevel", "error", "-threads", "2", "-i", mpath(name), "-vf",
                        f"select='eq(n\\,{k})'", "-frames:v", "1", p], capture_output=True)
    if not os.path.exists(p):
        no(f"fixture_frame {name} {k}: ffmpeg made nothing"); return None
    return np.asarray(Image.open(p).convert("RGBA")).astype(np.float32)


def counter(s, k):
    return None if s is None or k not in s else s[k]


def delta(tag, s0, s1, k):
    a, b = counter(s0, k), counter(s1, k)
    if a is None or b is None:
        no(f"{tag}: {k} absent (the app predates it)"); return None
    return b - a


def field(tag, s, k):
    v = counter(s, k)
    if v is None:
        no(f"{tag}: {k} absent (the app predates it)")
    return v


def settle_late(limit=None):
    limit = float(FIX["settleLateS"]) if limit is None else limit
    t0 = time.time(); s = state()
    if s is None or "video_late_frames" not in s:
        return None
    last = s["video_late_frames"]
    while time.time() - t0 < limit:
        time.sleep(0.2)
        s = state()
        if s is None:
            return None
        if s["video_late_frames"] == last:
            return time.time() - t0
        last = s["video_late_frames"]
    print(f"      settle_late: video_late_frames still moving after {limit} s", flush=True)
    return None


def la():
    return "load avg %.2f %.2f %.2f" % os.getloadavg()


def app_cpu():
    r = subprocess.run(["ps", "-eo", "pid=,%cpu=,ucomm="], capture_output=True, text=True)
    for line in r.stdout.splitlines():
        parts = line.split(None, 2)
        if len(parts) == 3 and parts[2].strip() == "Audio-DNA":
            return parts[1]
    return "?"


def wait_no_compiler(tag, limit_s=1800):
    t0 = time.time()
    while True:
        busy = [n for n in ("clang", r"clang\+\+") if subprocess.run(["pgrep", "-x", n], capture_output=True).stdout.strip()]
        if not busy:
            return True
        if time.time() - t0 > limit_s:
            no(f"{tag}: a compiler ({busy}) kept running for {limit_s} s -- perf not measured"); return False
        print(f"      {tag}: waiting for {busy} to finish ({la()})", flush=True)
        time.sleep(20)


class Poller:
    """Reads /api/state every `interval` s (15 ms; the fps rows 50 ms, W2) in a thread; peak_* reset on read, so the max
    over reads is the max over the window. Keys missing from the app are remembered (the row FAILs on them)."""
    KEYS = ("peak_callback_ms", "peak_frame_time_ms", "fps", "gl_video_max_decodes_per_call", "peak_video_upload_ms",
            "msg_video_lock_wait_max_ms")   # the *_max_* / peak_* fields reset on read: max over the polls

    def __init__(self, interval=0.015):
        self.rows = []; self.missing = set(); self._run = False; self._th = None; self.interval = interval

    def _loop(self):
        s = requests.Session(); s.headers["Connection"] = "close"
        while self._run:
            try:
                d = s.get(A + "/api/state", timeout=6).json()
                self.rows.append((time.time(), {k: d.get(k) for k in self.KEYS}))
                for k in self.KEYS:
                    if k not in d:
                        self.missing.add(k)
            except Exception:  # noqa: BLE001
                pass
            time.sleep(self.interval)

    def start(self):
        self.rows = []; self._run = True
        self._th = threading.Thread(target=self._loop, daemon=True); self._th.start()

    def stop(self):
        self._run = False; self._th.join(); return self

    def window(self, sec):
        self.start(); time.sleep(sec); return self.stop()

    def vals(self, k):
        return [float(r[1][k]) for r in self.rows if r[1].get(k) is not None]

    def max(self, k):
        v = self.vals(k); return max(v) if v else None

    def p90(self, k):
        v = sorted(self.vals(k)); return v[int(0.9 * (len(v) - 1))] if v else None

    def median(self, k):
        v = self.vals(k); return statistics.median(v) if v else None

    def over(self, k, lim):
        return [round(x, 1) for x in self.vals(k) if x > lim]


def pmax(tag, pol, s, k):
    """A reset-on-read field over a polled window: the max over the polls and the closing read."""
    v = [x for x in (pol.max(k), counter(s, k)) if x is not None]
    if not v:
        no(f"{tag}: {k} absent (the app predates it)"); return None
    return max(v)


def need(tag, pol, k):
    if k in pol.missing or not pol.vals(k):
        no(f"{tag}: {k} absent (the app predates it)"); return False
    return True


# ---------------------------------------------------------------- rows
def w5(tag):
    if not load(tag, [deck(0, [layer(LID[tag], [vclip(1, "g30_1080.mp4")])])], "1080", 1):
        return
    trig(0, 0); wait_active(0, 0); time.sleep(2.0)
    s0 = state()
    for i in range(3):
        t0 = time.time()
        f, pb, pa = cap_bracket(f"{tag}_{i}", 0)
        if f is None:
            return
        c = code(f, BAND["1080"]); good, br = in_bracket(c, pb, pa)
        check(good, f"{tag}: cap {i} code {c} within the playhead bracket {br}")
        ref = fixture_frame("g30_1080.mp4", c)
        if ref is not None:
            d = dbox(f, ref)
            check(d <= TOL, f"{tag}: cap {i} is the decoded frame {c}: dbox(cap, fixture frame {c}) {d:.2f} <= {TOL}")
        time.sleep(max(0.0, 1.0 - (time.time() - t0)))
    s1 = state(); late = delta(tag, s0, s1, "video_late_frames")
    if late is not None:
        check(late == 0, f"{tag}: video_late_frames delta over the captures {late} == 0")


def steady_scene(tag, size, clips, players):
    """One layer per clip on a fresh load; trigger all; 2 s; s0; a 5 s window polled every FPS_POLL; s1."""
    lids = LID[tag] if isinstance(LID[tag], list) else [LID[tag]]
    if not load(tag, [deck(0, [layer(lids[i], [c]) for i, c in enumerate(clips)])], size, players):
        return None
    if not wait_no_compiler(tag):
        return None
    for i in range(len(clips)):
        trig(i, 0); wait_active(i, 0)
    time.sleep(2.0)
    s0 = state(); pol = Poller(FPS_POLL).window(5.0); s1 = state()
    if not need(tag, pol, "fps") or not need(tag, pol, "peak_callback_ms"):
        return None
    return s0, pol, s1


def steady(tag, size, name, n=4):
    stills = None
    if size == "4k":   # W1: the same launch's upload-free ceiling (4 x 4K stills on the 4K canvas)
        r = steady_scene(tag, size, [iclip(200 + i, mpath(STILL4K)) for i in range(n)], 0)
        if r is None:
            return
        stills = r[1].median("fps")
        print(f"      {tag}: 4 x 4K STILLS median fps {stills:.1f}, p90 callback {r[1].p90('peak_callback_ms'):.2f} ms, "
              f"polls {len(r[1].vals('fps'))}, {la()}", flush=True)
    reps = int(FIX["w1Repeats"]) if size == "1080" else 1
    fpsl = []; first = None
    for k in range(reps):
        r = steady_scene(tag, size, [vclip(10 + 10 * k + i, name) for i in range(n)], n)
        if r is None:
            return
        s0, pol, s1 = r
        fps = pol.median("fps"); p90 = pol.p90("peak_callback_ms"); fpsl.append(round(fps, 1))
        dec = delta(tag, s0, s1, "gl_video_decode_calls"); upl = counter(s1, "video_uploads")
        upl = None if upl is None or counter(s0, "video_uploads") is None else upl - s0["video_uploads"]
        late = delta(tag, s0, s1, "video_late_frames")
        extra = {kk: (None if counter(s0, kk) is None or counter(s1, kk) is None else s1[kk] - s0[kk])
                 for kk in ("video_frames_decoded", "video_frames_dropped", "video_frames_skipped", "video_hold_frames", "video_seeks")}
        print(f"      {tag} run {k + 1}/{reps}: median fps {fps:.1f}, p90 callback {p90:.2f} ms, max callback "
              f"{pol.max('peak_callback_ms'):.2f} ms, polls {len(pol.vals('fps'))}, decode calls {dec}, uploads {upl}, late {late}, "
              f"{extra}, peak upload {pol.max('peak_video_upload_ms')} ms, %cpu {app_cpu()}, {la()}", flush=True)
        if first is None:
            first = (fps, p90, dec, upl, late)
    fps, p90, dec, upl, late = first
    if size == "1080":
        lim, need_n = float(FIX["fpsMin"]), int(FIX["w1PassMin"])
        good = sum(1 for f in fpsl if f >= lim)
        check(good >= need_n, f"{tag}: (a) median fps >= {lim:g} in {good} of {reps} runs (>= {need_n}): {fpsl}")
    else:
        lo, below = float(FIX["fps4kMin"]), float(FIX["fps4kBelowStillsMax"])
        check(fps >= lo, f"{tag}: (a) median fps {fps:.1f} >= {lo:g} (W1: the upload-bound floor)")
        check(fps >= stills - below, f"{tag}: (a2) median fps {fps:.1f} >= this launch's 4 x 4K stills fps {stills:.1f} - {below:g} "
                                     f"= {stills - below:.1f}")
    check(p90 <= float(FIX["p90CallbackMaxMs"]), f"{tag}: (b) p90 peak_callback_ms {p90:.2f} <= {FIX['p90CallbackMaxMs']}")
    if dec is not None:
        check(dec == 0, f"{tag}: (c) gl_video_decode_calls delta {dec} == 0 (no decode on the render thread)")
    if upl is None:
        no(f"{tag}: (d) video_uploads absent (the app predates it)")
    else:
        lo, hi = FIX["uploads5s"]
        check(lo <= upl <= hi, f"{tag}: (d) video_uploads delta {upl} in [{lo}, {hi}] (one upload per content frame)")
    if late is not None:
        lim = 0 if size == "1080" else int(FIX["lateMaxSteady4k"])
        check(late <= lim, f"{tag}: (e) video_late_frames delta {late} <= {lim}")


def w8(tag):
    if not load(tag, [deck(0, [layer(LID[tag], [vclip(1, "pr1080.mov")])])], "1080", 1):
        return
    if not wait_no_compiler(tag):
        return
    trig(0, 0); wait_active(0, 0); time.sleep(2.0)
    s0 = state(); pol = Poller(FPS_POLL).window(5.0); s1 = state()
    if not need(tag, pol, "fps"):
        return
    fps = pol.median("fps"); dec = delta(tag, s0, s1, "gl_video_decode_calls"); late = delta(tag, s0, s1, "video_late_frames")
    print(f"      {tag}: median fps {fps:.1f}, max callback {pol.max('peak_callback_ms')}, decode calls {dec}, late {late}, "
          f"%cpu {app_cpu()}, {la()}", flush=True)
    check(fps >= float(FIX["fpsMin"]), f"{tag}: median fps {fps:.1f} >= {FIX['fpsMin']}")
    if dec is not None:
        check(dec == 0, f"{tag}: gl_video_decode_calls delta {dec} == 0")
    if late is not None:
        check(late == 0, f"{tag}: video_late_frames delta {late} == 0")
    settle_late()
    f, pb, pa = cap_bracket(tag, 0)
    if f is not None:
        c = code(f, BAND["1080"]); good, br = in_bracket(c, pb, pa)
        check(good, f"{tag}: code {c} within the playhead bracket {br}")


def retrigger(tag, size, name):
    lid = LID[tag]; band = BAND[size]
    lim = int(FIX["lateMaxRetrigger1080" if size == "1080" else "lateMaxRetrigger4k"])
    if not load(tag, [deck(0, [layer(lid, [vclip(1, name, ip=0.5)])])], size, 1):
        return
    if not wait_no_compiler(tag):
        return
    trig(0, 0); wait_active(0, 0); trig(0, 0)   # prime: a first trigger plays from 0; the retrigger seeks to the in-point
    time.sleep(3.0)
    fpre, pbp, pap = cap_bracket(f"{tag}_pre", 0)
    if fpre is None:
        return
    cpre = code(fpre, band); good, br = in_bracket(cpre, pbp, pap)
    check(good and 225 <= cpre <= 255, f"{tag}: (e) capPre code {cpre} in [225, 255] and within the bracket {br} "
                                       f"(the clip plays from the in-point)")
    s0 = state(); pol = Poller(); pol.start(); t0 = time.time()
    trig(0, 0)   # RETRIGGER: seekTo(inPoint 0.5) = frame 150, 150 frames past keyframe 0
    hold = None
    if size == "4k":
        time.sleep(max(0.0, 0.3 - (time.time() - t0)))
        hold = cap(f"{tag}_hold")
    time.sleep(max(0.0, 3.0 - (time.time() - t0)))
    pol.stop(); s1 = state()
    settle_late()
    f, pb, pa = cap_bracket(tag, 0)
    if not need(tag, pol, "peak_callback_ms"):
        return
    over = pol.over("peak_callback_ms", PEAK)
    late = delta(tag, s0, s1, "video_late_frames"); mx = pmax(tag, pol, s1, "gl_video_max_decodes_per_call")
    fps = pol.median("fps") or 0.0
    hold_ms = None if late is None or fps <= 0 else late * 1000.0 / fps
    print(f"      {tag}: polls {len(pol.vals('peak_callback_ms'))}, max callback {pol.max('peak_callback_ms'):.2f} ms, "
          f"polls over {PEAK}: {len(over)} {over[:8]}, max decodes/call {mx}, late {late} (hold ~{hold_ms} ms at median "
          f"fps {fps:.1f}), dropped {delta(tag, s0, s1, 'video_frames_dropped') if counter(s0, 'video_frames_dropped') is not None else None}, "
          f"seeks {s1.get('video_seeks', 0) - s0.get('video_seeks', 0) if 'video_seeks' in s0 else None}, {la()}", flush=True)
    check(not over, f"{tag}: (a) every peak_callback_ms poll <= {PEAK} ({len(over)} over)")
    if mx is not None:
        check(mx <= 2, f"{tag}: (b) gl_video_max_decodes_per_call {mx} <= 2")
    if late is not None:
        check(late <= lim, f"{tag}: (c) video_late_frames delta {late} <= {lim} (hold ~{hold_ms and round(hold_ms)} ms)")
    if f is not None:
        c = code(f, band); good, br = in_bracket(c, pb, pa)
        check(good and 150 <= c < 300, f"{tag}: (d) code {c} within the bracket {br} and in [150, 300) (resumed from the in-point)")
    if hold is not None:
        ch = code(hold, band); lu = luma(hold)
        check(lu > 20 and (ch >= 148 or abs(ch - cpre) <= 3) and ch != 29 and ch > 2,
              f"{tag}: (f) capHold at +0.3 s: luma {lu:.1f} > 20, code {ch} >= 148 or within 3 of capPre {cpre} "
              f"(held or landed; never 29 = keyframe + 29, never a frame-0 stand-in)")


def w4(tag):
    if not load(tag, [deck(0, [layer(LID[tag], [vclip(1, "a1080_g250.mp4")])]),
                      deck(1, [layer(LID["w4_away"], [iclip(2, os.path.join(OUT, "warm.png"))])])], "1080", 1):
        return
    if not wait_no_compiler(tag):
        return
    trig(0, 0); wait_active(0, 0); time.sleep(2.0)
    p0 = playhead(0); s0 = state()
    switch(1); time.sleep(3.5)
    pol = Poller(); pol.start(); switch(0); time.sleep(1.5); pol.stop()
    s1 = state(); settle_late()
    f, pb, pa = cap_bracket(tag, 0)
    if not need(tag, pol, "peak_callback_ms"):
        return
    over = pol.over("peak_callback_ms", PEAK)
    pend = delta(tag, s0, s1, "video_pending_frames"); late = delta(tag, s0, s1, "video_late_frames")
    print(f"      {tag}: max callback {pol.max('peak_callback_ms'):.2f} ms, polls over {PEAK}: {len(over)} {over[:8]}, "
          f"pending {pend}, late {late}, p0 {p0}, p1 {pb}, {la()}", flush=True)
    check(not over, f"{tag}: (a) every peak_callback_ms poll across the return <= {PEAK} ({len(over)} over)")
    if pend is not None:
        check(pend == 0, f"{tag}: (b) video_pending_frames delta {pend} == 0 (shown before: never nothing)")
    if late is not None:
        check(1 <= late <= int(FIX["lateMaxReturn"]), f"{tag}: (c) 1 <= video_late_frames delta {late} <= {FIX['lateMaxReturn']} "
                                                       f"(a bounded hold on return)")
    if f is not None and p0 is not None and pb is not None:
        c = code(f, BAND["1080"]); good, br = in_bracket(c, pb, pa)
        ran = (pb - p0) * 10.0
        check(ran >= 5.0 and good, f"{tag}: (d) the clock ran {ran:.2f} s >= 5.0 off and on screen; code {c} within the bracket {br}")


def w4b(tag):
    if not load(tag, [deck(0, [layer(LID[tag], [vclip(1, "a4k_g250.mp4", ip=0.5)])]),
                      deck(1, [layer(LID["w4_away"], [iclip(2, os.path.join(OUT, "warm.png"))])])], "4k", 1):
        return
    # Clock arithmetic keeps both arms clear of the 10 s Loop wrap (a wrap bumps the request generation, which also
    # ends a ring-full wait and would hide a missing inner idle check): B leaves at 3.0 s, A spans 5.0 -> 6.55 s.
    trig(0, 0); wait_active(0, 0); time.sleep(3.0)
    switch(1)                                      # (B) steady play has filled the ring; the deck leaves
    time.sleep(1.5); sB = state()
    switch(0); time.sleep(1.0)
    trig(0, 0); time.sleep(0.05); switch(1)        # (A) a retrigger starts a catch-up; the deck leaves within 100 ms
    time.sleep(1.5); sA = state()
    for arm, s in (("B (ring full)", sB), ("A (mid catch-up)", sA)):
        th = field(tag, s, "video_threads"); aw = field(tag, s, "video_threads_awake")
        if th is None or aw is None:
            continue
        print(f"      {tag} {arm}: video_threads {th}, video_threads_awake {aw}", flush=True)
        check(th == 1 and aw == 0, f"{tag}: {arm}: 1.5 s after the deck left, video_threads_awake {aw} == 0 "
                                   f"(video_threads {th} == 1: not vacuous)")
    switch(0)


def w6(tag):
    if not load(tag, [deck(0, [layer(LID[tag], [vclip(1, "a1080_g250.mp4"), vclip(2, "b1080_g250.mp4")], speed=2.0)])],
                "1080", 2):
        return
    if not wait_no_compiler(tag):
        return
    trig(0, 0); wait_active(0, 0); time.sleep(2.0)
    s0 = state(); pol = Poller(); pol.start(); t0 = time.time()
    trig(0, 1)
    time.sleep(max(0.0, 1.0 - (time.time() - t0)))
    pA0, pB0 = playhead(0, 0), playhead(0, 1)
    mid = cap(f"{tag}_mid")
    pA1, pB1 = playhead(0, 0), playhead(0, 1)
    time.sleep(max(0.0, 2.5 - (time.time() - t0)))
    pol.stop(); s1 = state(); settle_late()
    f, pb, pa = cap_bracket(tag, 0, 1)
    if not need(tag, pol, "peak_callback_ms"):
        return
    upl = delta(tag, s0, s1, "video_uploads"); pend = delta(tag, s0, s1, "video_pending_frames")
    print(f"      {tag}: max callback {pol.max('peak_callback_ms'):.2f} ms, uploads {upl}, pending {pend}, {la()}", flush=True)
    check(pol.max("peak_callback_ms") <= PEAK, f"{tag}: (a) max peak_callback_ms over the fade {pol.max('peak_callback_ms'):.2f} <= {PEAK}")
    if upl is not None:
        check(130 <= upl <= 180, f"{tag}: (b) video_uploads delta {upl} in [130, 180] (2 players x 30 x 2 s + B alone)")
    if mid is not None and None not in (pA0, pA1, pB0, pB1):
        fa = fixture_frame("a1080_g250.mp4", expected((pA0 + pA1) / 2)); fb = fixture_frame("b1080_g250.mp4", expected((pB0 + pB1) / 2))
        if fa is not None and fb is not None:
            da, db, lu = dbox(mid, fa), dbox(mid, fb), luma(mid)
            check(lu > 20 and da > TOL and db > TOL, f"{tag}: (c) capMid is a blend: luma {lu:.1f} > 20, dbox to A {da:.1f} "
                                                      f"and to B {db:.1f} both > {TOL}")
    if f is not None:
        c = code(f, BAND["1080"]); good, br = in_bracket(c, pb, pa, mask=511)
        check(bool(c & 512) and good, f"{tag}: (d) capAfter code {c} has bit 9 (B) and {c & 511} within B's bracket {br}")
    if pend is not None:
        check(pend == 0, f"{tag}: (e) video_pending_frames delta {pend} == 0 (B's frame 0 came from open)")


def w6b(tag):
    if not load(tag, [deck(0, [layer(LID[tag], [vclip(1, "a1080_g250.mp4", ip=0.5), vclip(2, "b1080_g250.mp4", ip=0.5)],
                                     speed=2.0)])], "1080", 2):
        return
    if not wait_no_compiler(tag):
        return
    trig(0, 0); wait_active(0, 0); time.sleep(3.0)
    pol = Poller(); pol.start()
    trig(0, 1); wait_active(0, 1); tf = time.time()
    time.sleep(max(0.0, 0.5 - (time.time() - tf)))
    s0 = state(); tr = time.time()
    trig(0, 1)   # RETRIGGER the incoming clip (the active column) mid-fade: B seeks to 5.0 s (mid-GOP)
    holds = []
    for k, at in enumerate((0.1, 0.25)):
        time.sleep(max(0.0, at - (time.time() - tr)))
        holds.append(cap(f"{tag}_hold{k}"))
    done = None
    while time.time() - tf < float(FIX["fadeDoneS"]):
        L = layer_json(0)
        if L is not None and float(L.get("crossfadeProgress", 0.0)) >= 1.0:
            done = time.time() - tf; break
        time.sleep(0.02)
    time.sleep(0.3); pol.stop(); s1 = state(); settle_late()
    f, pb, pa = cap_bracket(tag, 0, 1)
    if not need(tag, pol, "peak_callback_ms"):
        return
    over = pol.over("peak_callback_ms", PEAK); late = delta(tag, s0, s1, "video_late_frames")
    print(f"      {tag}: max callback {pol.max('peak_callback_ms'):.2f} ms, polls over {PEAK}: {len(over)} {over[:8]}, "
          f"late {late}, fade done at {done} s, {la()}", flush=True)
    check(not over, f"{tag}: (a) every peak_callback_ms poll <= {PEAK} ({len(over)} over)")
    for k, h in enumerate(holds):
        if h is None:
            continue
        ch, lu = code(h, BAND["1080"]), luma(h)
        check(lu > 20 and ch not in (29, 29 + 512), f"{tag}: (b) hold capture {k}: luma {lu:.1f} > 20 and code {ch} is not "
                                                    f"29 / 541 (keyframe + 29)")
    check(done is not None, f"{tag}: (c) crossfadeProgress reached 1 within {FIX['fadeDoneS']} s of the fade start "
                            f"(at {done and round(done, 2)} s: the fade kept running)")
    if f is not None:
        c = code(f, BAND["1080"]); good, br = in_bracket(c, pb, pa, mask=511)
        check(bool(c & 512) and good, f"{tag}: (d) after settle code {c} has bit 9 (B) and {c & 511} within B's bracket {br}")


def w7(tag):
    lids = LID[tag]; warm = os.path.join(OUT, "warm.png")
    layers = [layer(lids[i], [vclip(20 + i, "a4k_g250.mp4", ip=0.5)]) for i in range(4)]
    layers.append(layer(LID["w7_images"], [iclip(30, warm), iclip(31, warm)]))
    if not load(tag, [deck(0, layers)], "4k", 4):
        return
    if not wait_no_compiler(tag):
        return
    for i in range(4):
        trig(i, 0); wait_active(i, 0)
    trig(4, 0); wait_active(4, 0); time.sleep(2.0)
    s0 = state()
    trig(0, 0)   # retrigger 101: a mid-GOP catch-up starts (main: the GL thread decodes under videoPlayerMutex_)
    time.sleep(0.05)
    rtts = []
    for i in range(5):
        t0 = time.time(); col = 1 if i % 2 == 0 else 0
        # a message-thread video retrigger (getVideoPlayer takes videoPlayerMutex_) while a catch-up runs, then the
        # image trigger queued behind it: its round trip is how long the message thread was held
        trig(1 + i % 3, 0)
        trig(4, col); w = wait_active(4, col, limit=5.0)
        rtts.append(None if w is None else round((time.time() - t0) * 1000.0, 1))
        time.sleep(max(0.0, 0.1 - (time.time() - t0)))
    s1 = state(); lw = field(tag, s1, "msg_video_lock_wait_max_ms")
    print(f"      {tag}: trigger round trips {rtts} ms, msg_video_lock_wait_max_ms {lw}, {la()}", flush=True)
    if lw is not None:
        check(lw <= float(FIX["lockWaitMaxMs"]), f"{tag}: (a) msg_video_lock_wait_max_ms {lw:.2f} <= {FIX['lockWaitMaxMs']}")
    check(all(r is not None and r <= float(FIX["triggerRttMaxMs"]) for r in rtts),
          f"{tag}: (b) every trigger round trip {rtts} <= {FIX['triggerRttMaxMs']} ms")


def main():
    if MAKE:
        make_fixtures(); return
    rows = [("w5_correctness_gop30", lambda: w5("w5_correctness_gop30")),
            ("w1_steady_1080x4", lambda: steady("w1_steady_1080x4", "1080", "a1080_g250.mp4")),
            ("w3_retrigger_midgop_1080", lambda: retrigger("w3_retrigger_midgop_1080", "1080", "a1080_g250.mp4")),
            ("w4_deck_return_1080", lambda: w4("w4_deck_return_1080")),
            ("w6_crossfade_two_players", lambda: w6("w6_crossfade_two_players")),
            ("w6b_retrigger_mid_fade", lambda: w6b("w6b_retrigger_mid_fade")),
            ("w8_prores_steady", lambda: w8("w8_prores_steady")),
            ("w2_steady_4kx4", lambda: steady("w2_steady_4kx4", "4k", "a4k_g250.mp4")),
            ("w3b_retrigger_midgop_4k", lambda: retrigger("w3b_retrigger_midgop_4k", "4k", "a4k_g250.mp4")),
            ("w4b_idle_mid_catchup", lambda: w4b("w4b_idle_mid_catchup")),
            ("w7_message_thread_no_wait", lambda: w7("w7_message_thread_no_wait"))]
    for name, fn in rows:
        if ONLY is None or name in ONLY:
            print(f"--- {name}", flush=True)
            fn()
    print(f"\nPY {PASS} PASS / {FAIL} FAIL", flush=True)
    sys.exit(0 if FAIL == 0 else 1)


if __name__ == "__main__":
    main()
