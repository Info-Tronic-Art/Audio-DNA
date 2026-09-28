#!/usr/bin/env python3
"""probe-image-load.py -- REST/pixel half of .harmony/probe-image-load.sh (s-rta-0928 renderleft R1).

The .sh owns launch / refuse / quit; this file writes the fixtures (--make-fixtures, BEFORE the app starts) and
then talks to the running app on 7070. Every captured PNG is decoded with PIL+numpy (never file hashes); every
render_frame response is checked; the output dir is fresh per run.

usage: probe-image-load.py <root> <fresh-outdir> --make-fixtures [row,row,...]
       probe-image-load.py <root> <fresh-outdir> [row,row,...]
rows: i1_layer_1080 i1_layer_4k i2_fade_1080 i2_fade_4k i2m_fade_start i2ms_seq_fade_start i3_capture_after_trigger
      i3s_capture_after_seq_trigger i4_hold_counters i4m_mask_hold i5_legacy_retrigger i6_sequence_1080
      i7_prefetch_retain

Fixtures (<out>/media, numpy seed 928, PIL compress_level 1): "warm" = a flat colour PNG (shown first, so it is
resident before the measured window); "cold" = a noisy gradient R = 255x/W, G = 255y/H, B = base (128, or a
per-frame shift for sequences) + uniform noise in [-64, 64] per channel, alpha 255 -- hard to compress (a slow
decode, like a photo or worse) yet identifiable after box averaging. Every path is new to the app instance.
"Slow cold" = the same at 7680x4320 (a ~0.4-0.5 s decode). The rows that must SEE a pending image (i2m, i3, i3s, i4,
i4m) put a hidden layer 0 of four flat 8192x8192 "fillers" (1 GiB of textures) before the subject layer: from R1.5 a
composition load prefetches its images in order only while < 1 GiB is resident, so once the fillers are resident the
subject's images are never prefetched and the first trigger of the slow cold image takes the demand path (pending ->
hold) whatever the message-thread timing. The row waits until the fillers are resident (no wait on an app without prefetch: they are never
decoded), triggers col 0 (warm) on layer 1, 1 s, then the cold image.
Metrics: peak(s, k) = the /api/state key (reset on read) -- a missing key FAILs the row ("k absent: the app predates
it"); windows are polled every 15 ms keeping the max. dbox(X, Y) = both arrays box-averaged in 16x16 blocks, then
mean |diff| over RGB (Y is resized to X's shape first when they differ) -- robust to a half-pixel shift, and it
still separates "image" from "held picture" from "black". p(F) = dissolve progress fitted on the line warm -> cold.
Perf rows wait until no compiler runs and print the load average (rig rule).

i1_layer_{1080,4k}: canvas = size; col 0 warm flat, col 1 cold noisy gradient at the canvas size, a cut. load, 1 s,
  trig 0, 1.5 s, quiet wait, 0.6 s pre-poll, trig 1, 1.5 s poll, cap. PASS: (a) max peak_frame_time_ms in the window
  <= peakMaxMs; (b) max peak_callback_ms <= peakMaxMs; (c) dbox(cap, cold) <= boxTol. Prints the pre-window max, the
  frames held (image_hold_frames delta) and the load average. RED on main: (a) loadKeyImage decodes in the frame;
  (b) peak_callback_ms absent.
i2_fade_{1080,4k}: as i1 with transitionSpeed 1.0 (a 1 s dissolve onto the cold image). PASS as i1 (a)(b); (c)
  after 1.5 s the picture is the cold image.
i2m_fade_start (C1 of the Harmony adoption; RED on main not required -- the defect it guards appears only with an
  async decode): 1920x1080 canvas, fillers, col 0 warm, col 1 slow cold, 1 s dissolve. trig 0; 1 s; capA (warm); trig 1; poll
  /api/composition until activeClipColumn == 1; f0 at once, f1..f3 back to back; 2 s; capS (cold, settled).
  PASS: fixture dbox(capA, capS) >= fixtureMinD; p(f0) <= i2mFirstMaxP (the dissolve starts when the image lands, it
  never jumps ahead by the decode time); p(f0) <= p(f1) <= p(f2) <= p(f3) + 0.01. Teeth: a build without the
  crossfade pause FAILs p(f0).
i2ms_seq_fade_start (renderleft-fix: C1 for an image sequence): as i2m with col 1 = a fresh ImageSequence of 3 slow
  cold frames (7680x4320, sequenceFps 0.1: frame 0 shows for 10 s). No fillers (a sequence is never prefetched: its first
  frame always takes the demand path). PASS as i2m. RED on the lane head 5f1536f (C1 paused image clips only: the dissolve
  runs on while the first frame decodes and jumps ahead by the decode time).
i3_capture_after_trigger (guard + teeth): canvas 1920x1080; fillers; col 0 warm flat, col 1 slow cold. trig 0, 1 s,
  capA; trig 1; poll /api/composition every 5 ms until activeClipColumn == 1 (<= 2 s); capN at once; 2 s; capS.
  PASS: fixture dbox(capA, capS) >= fixtureMinD; row dbox(capN, capS) <= boxTol. PASS on main (synchronous decode);
  teeth: the R1.2 build without the capture gate FAILs (capN is the held picture).
i3s_capture_after_seq_trigger (C3 of the adoption): as i3 with col 1 = a fresh ImageSequence of 3 1080p frames at
  sequenceFps 0.1 (frame 0 shown for 10 s). PASS on main (synchronous decode); teeth: the no-gate build.
i4_hold_counters: canvas 3840x2160; fillers; col 0 warm, col 1 slow cold. trig 0; 1 s; s0, trig 1, 2 s, s1.
  PASS: image_hold_frames delta >= 1 AND image_skip_frames delta == 0; images_pending(s1) == 0. Prints the frames
  held. RED on main: fields absent.
i4m_mask_hold: 4K; layer 0 Opaque = a warm colour image; layer 1 Mask (type 4): col 0 warm mask (left half white),
  col 1 slow cold mask (after the fillers layer). trig both col 0, 1 s, s0, trig the mask's col 1, 2 s, s1. PASS: hold delta >= 1 AND skip delta == 0.
  No picture check: a held frame never answers a render_frame (the capture gate), so the counters ARE the witness.
i5_legacy_retrigger: canvas 1920x1080; cols = 2 cold 1080p images; trig 0, 1 s, trig 1, 1 s (both resident);
  quiet; 6 triggers 0,1,0,1,0,1 every 0.5 s while polling. PASS: max peak_callback_ms <= peakMaxMs (prints max
  peak_frame_time_ms: low on every build -- the legacy decode sits outside the frame timer). RED on main: absent;
  on R1.0: by value (every trigger re-decodes the legacy single image on the GL thread, F2).
i6_sequence_1080: canvas 1920x1080; col 0 = ImageSequence of 12 fresh 1080p noisy frames (per-frame B shift),
  sequenceFps 10, Loop. load, 1 s, quiet, 0.6 s pre, trig 0, 1.6 s poll, cap. PASS (a)(b) as i1; (c) the cap is
  within boxTol of one of the 12 frames. RED on main: (a) the frames decode in the frame; (b) absent.
i7_prefetch_retain: composition P = deck 0 layer cols 0-2 + deck 1 layer cols 0-2 = 6 cold 1080p images, none
  triggered. load P; poll every 0.1 s (<= prefetchTimeoutS) until images_pending == 0 and image_textures == 6,
  keeping max peak_callback_ms; s1; trig(0, 1); 0.5 s; s2; load Q (one of P's images only); 1.5 s; s3.
  PASS: (1) image_textures == 6 within the timeout; (2) max peak_callback_ms during the prefetch <= peakMaxMs;
  (3) image_hold_frames(s2) - (s1) == 0 (a prefetched image never holds); (4) image_textures(s3) <= 1.
  RED on main: fields absent.
Calibration: .harmony/.reports/s-rta-0928/renderleft.md (main = RED run, lane = GREEN runs).
"""
import json, os, subprocess, sys, threading, time

import numpy as np
import requests
from PIL import Image

A = "http://127.0.0.1:7070"
ROOT, OUT = sys.argv[1], sys.argv[2]
MAKE = len(sys.argv) > 3 and sys.argv[3] == "--make-fixtures"
_rows_arg = sys.argv[4] if MAKE else (sys.argv[3] if len(sys.argv) > 3 else "")
ONLY = set(_rows_arg.split(",")) if _rows_arg else None
FIX = json.load(open(os.path.join(ROOT, ".harmony", "probe-image-load.json")))
PEAK = float(FIX["peakMaxMs"]); TOL = float(FIX["boxTol"]); FMIN = float(FIX["fixtureMinD"])
SIZES = {k: tuple(v) for k, v in FIX["sizes"].items()}
LID = FIX["layers"]
PENDING_ROWS = ("i2m_fade_start", "i3_capture_after_trigger", "i3s_capture_after_seq_trigger", "i4_hold_counters",
                "i4m_mask_hold")
MEDIA = os.path.join(OUT, "media")
PASS = FAIL = 0
S = requests.Session()
S.headers["Connection"] = "close"   # one fresh connection per request (cpp-httplib 5 s keep-alive drop)


def ok(msg):
    global PASS; PASS += 1; print(f"PASS  {msg}", flush=True)


def no(msg):
    global FAIL; FAIL += 1; print(f"FAIL  {msg}", flush=True)


# ---------------------------------------------------------------- fixtures (written before the app starts)
def mpath(name):
    return os.path.join(MEDIA, name)


def write_png(name, arr):
    Image.fromarray(arr, "RGBA").save(mpath(name), compress_level=1)


def noisy(w, h, k, base_b=128):
    rng = np.random.default_rng(928 + k)
    x = np.arange(w, dtype=np.float32)[None, :] * (255.0 / w)
    y = np.arange(h, dtype=np.float32)[:, None] * (255.0 / h)
    a = np.empty((h, w, 4), np.float32)
    a[..., 0] = x; a[..., 1] = y; a[..., 2] = base_b
    a[..., :3] += rng.uniform(-64, 64, size=(h, w, 3)).astype(np.float32)
    a[..., 3] = 255
    return np.clip(a, 0, 255).astype(np.uint8)


def flat(w, h, rgb):
    a = np.zeros((h, w, 4), np.uint8); a[..., :3] = rgb; a[..., 3] = 255
    return a


# row -> {name: (w, h, kind, k)}: the fixtures each row needs; kind "cold" / "seq" / "warm" / "warm2" / "mask"
def fixture_plan():
    W1, H1 = SIZES["1080"]; W4, H4 = SIZES["4k"]
    plan = {}
    WS, HS = SIZES["slow"]
    for r, (w, h) in (("i1_layer_1080", (W1, H1)), ("i1_layer_4k", (W4, H4)), ("i2_fade_1080", (W1, H1)),
                      ("i2_fade_4k", (W4, H4)), ("i2m_fade_start", (WS, HS)), ("i3_capture_after_trigger", (WS, HS)),
                      ("i4_hold_counters", (WS, HS)), ("i4m_mask_hold", (WS, HS))):
        plan[r] = {f"{r}_cold.png": (w, h, "cold", LID[r])}
    plan["i5_legacy_retrigger"] = {f"i5_cold{j}.png": (W1, H1, "cold", 500 + j) for j in range(2)}
    plan["i3s_capture_after_seq_trigger"] = {f"i3s_seq{j:02d}.png": (W1, H1, "seq", j) for j in range(3)}
    plan["i2ms_seq_fade_start"] = {f"i2ms_seq{j:02d}.png": (WS, HS, "seq", 10 + j) for j in range(3)}
    plan["i6_sequence_1080"] = {f"i6_seq{j:02d}.png": (W1, H1, "seq", j) for j in range(int(FIX["sequence"]["frames"]))}
    plan["i7_prefetch_retain"] = {f"i7_cold{j}.png": (W1, H1, "cold", 700 + j) for j in range(6)}
    FW, FH = SIZES["filler"]
    for r in PENDING_ROWS:
        plan.setdefault(r, {}).update({f"filler{k}.png": (FW, FH, "filler", k) for k in range(4)})
    return plan


def make_fixtures():
    os.makedirs(MEDIA, exist_ok=True)
    t0 = time.time()
    todo = {"warm.png": (256, 144, "warm", 0), "warm2.png": (256, 144, "warm2", 0), "mask_warm.png": (256, 144, "mask", 0)}
    for r, fx in fixture_plan().items():
        if ONLY is None or r in ONLY:
            todo.update(fx)
    for name, (w, h, kind, k) in todo.items():
        if kind == "warm":
            write_png(name, flat(w, h, (200, 120, 40)))
        elif kind == "warm2":
            write_png(name, flat(w, h, (40, 160, 200)))
        elif kind == "mask":
            a = flat(w, h, (0, 0, 0)); a[:, : w // 2, :3] = 255; write_png(name, a)
        elif kind == "filler":
            write_png(name, flat(w, h, (30 + 40 * k, 90, 160 - 30 * k)))
        elif kind == "seq":
            write_png(name, noisy(w, h, 300 + k, base_b=(23 * k) % 256))
        else:
            write_png(name, noisy(w, h, k))
    print(f"fixtures: {len(todo)} written to {MEDIA} in {time.time() - t0:.1f} s", flush=True)


# ---------------------------------------------------------------- REST helpers
def clip(cid, path, **extra):
    c = {"name": f"c{cid}", "id": cid, "mediaType": 1, "mediaFile": path, "effects": []}
    c.update(extra); return c


def seq_clip(cid, paths, fps):
    return {"name": f"s{cid}", "id": cid, "mediaType": 5, "mediaFile": "", "sequenceFiles": paths,
            "sequenceFps": float(fps), "effects": []}


def layer(lid, clips, ltype=0, speed=0.0, **extra):
    l = {"name": f"L{lid}", "id": lid, "opacity": 1.0, "visible": True, "blendMode": 0, "type": ltype,
         "transitionSpeed": speed, "layerEffects": [], "clips": clips}
    l.update(extra); return l


def deck(did, layers, ncols=None):
    n = ncols or max(len(l["clips"]) for l in layers)
    return {"name": f"D{did}", "id": did, "numColumns": n, "layers": layers}


def load(tag, decks, size):
    comp = {"name": "imgload-" + tag, "activeDeckIndex": 0, "masterOpacity": 1.0, "globalTransitionSpeed": 0.0,
            "outputWidth": size[0], "outputHeight": size[1], "decks": decks}
    path = os.path.join(OUT, f"comp_{tag}.json")
    json.dump(comp, open(path, "w"), indent=1)
    try:
        r = S.post(A + "/api/load_composition", json={"path": path}, timeout=20)
        good = r.ok and r.json().get("ok") is True
    except Exception as e:  # noqa: BLE001
        good = False; r = e
    if not good:
        no(f"{tag}: load_composition rejected: {str(getattr(r, 'text', r))[:160]}")
    return good


def trig(li, col):
    S.post(A + "/api/trigger_clip", json={"layer": li, "column": col}, timeout=6)


def state():
    try:
        return S.get(A + "/api/state", timeout=6).json()
    except Exception as e:  # noqa: BLE001
        no(f"/api/state: {e}")
        return None


def comp_state():
    try:
        return S.get(A + "/api/composition", timeout=6).json()
    except Exception:  # noqa: BLE001
        return None


def wait_active(li, col, limit=10.0):
    t0 = time.time()
    while time.time() - t0 < limit:
        c = comp_state()
        try:
            if c["decks"][0]["layers"][li]["activeClipColumn"] == col:
                return time.time() - t0
        except Exception:  # noqa: BLE001
            pass
        time.sleep(0.005)
    return None


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


def img(name):
    return np.asarray(Image.open(mpath(name)).convert("RGBA")).astype(np.float32)


def box(a, b=16):
    h, w = a.shape[0] // b * b, a.shape[1] // b * b
    return a[:h, :w, :3].reshape(h // b, b, w // b, b, 3).mean(axis=(1, 3))


def dbox(x, y):
    if x.shape[:2] != y.shape[:2]:
        y = np.asarray(Image.fromarray(y.astype(np.uint8), "RGBA").resize((x.shape[1], x.shape[0]), Image.BOX)).astype(np.float32)
    return float(np.abs(box(x) - box(y)).mean())


def fit_p(f, x, y):
    fx_, xx, yy = box(f).ravel(), box(x).ravel(), box(y).ravel()
    dx = yy - xx; den = float(dx @ dx)
    return float((fx_ - xx) @ dx) / den if den > 0 else 0.0


def wait_no_compiler(tag, limit_s=1800):
    t0 = time.time()
    while True:
        busy = [n for n in ("clang", r"clang\+\+") if subprocess.run(["pgrep", "-x", n], capture_output=True).stdout.strip()]
        if not busy:
            return True
        if time.time() - t0 > limit_s:
            no(f"{tag}: a compiler ({busy}) kept running for {limit_s} s -- perf not measured"); return False
        print(f"      {tag}: waiting for {busy} to finish (load avg %.2f %.2f %.2f)" % os.getloadavg(), flush=True)
        time.sleep(20)


class Poller:
    """Reads /api/state every 15 ms in a thread; the peak fields reset on read, so the max over reads is the max over
    the window. keys missing from the app are remembered (the row FAILs on them)."""
    KEYS = ("peak_frame_time_ms", "peak_callback_ms")

    def __init__(self):
        self.rows = []; self.missing = set(); self._run = False

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
            time.sleep(0.015)

    def window(self, sec):
        self.rows = []; self._run = True
        th = threading.Thread(target=self._loop, daemon=True); th.start()
        time.sleep(sec); self._run = False; th.join()
        return self.maxes()

    def start(self):
        self.rows = []; self._run = True
        self._th = threading.Thread(target=self._loop, daemon=True); self._th.start()

    def stop(self):
        self._run = False; self._th.join(); return self.maxes()

    def maxes(self):
        out = {}
        for k in self.KEYS:
            v = [r[1][k] for r in self.rows if r[1].get(k) is not None]
            out[k] = max(v) if v else None
        return out


def peak(tag, m, k):
    if m.get(k) is None:
        no(f"{tag}: {k} absent (the app predates it)")
        return None
    return float(m[k])


def counter(s, k):
    return None if s is None or k not in s else s[k]


def la():
    return "load avg %.2f %.2f %.2f" % os.getloadavg()


# A REST trigger runs on the message thread (callAsync); anything that holds that thread delays it (before the
# s-rta-0928 restore lane, a load decoded every cell thumbnail there -- seconds with the fillers). Every trigger below
# therefore waits until the model shows it (wait_active) before the row's clock starts.
def filler_layer():
    """A hidden layer 0 holding 4 flat 8192x8192 images = 1 GiB of textures. From R1.5 a composition load prefetches its
    images in order (layer 0 first) only while < 1 GiB is resident: once these four are resident, the subject layer's
    images (layer 1+) are NOT prefetched -- the first trigger of the cold image takes the demand path (pending -> hold)
    whatever the message-thread timing. On an app without prefetch they are never decoded (the layer is hidden)."""
    return layer(900, [clip(900 + k, mpath(f"filler{k}.png")) for k in range(4)], visible=False)


def wait_fillers(tag, limit=15.0):
    t0 = time.time(); s = None
    while time.time() - t0 < limit:
        s = state()
        if s is None or "image_textures" not in s:
            return   # no prefetch in this app (main / R1.0-R1.1): nothing to wait for
        if s["image_textures"] >= 4 and s["images_pending"] == 0:
            print(f"      {tag}: fillers resident after {time.time() - t0:.1f} s ({s['image_texture_mb']:.0f} MB)", flush=True)
            return
        time.sleep(0.1)
    print(f"      {tag}: fillers not all resident after {limit} s: {None if s is None else (s.get('image_textures'), s.get('images_pending'))}", flush=True)


# ---------------------------------------------------------------- rows
def row_layer(tag, size, speed):
    W, H = SIZES[size]; lid = LID[tag]; cold = f"{tag}_cold.png"
    if not load(tag, [deck(0, [layer(lid, [clip(1, mpath("warm.png")), clip(2, mpath(cold))], speed=speed)])], (W, H)):
        return
    time.sleep(1.0); trig(0, 0); time.sleep(1.5)
    if not wait_no_compiler(tag):
        return
    s0 = state(); pol = Poller()
    pre = pol.window(0.6)
    trig(0, 1)
    post = pol.window(1.5)
    s1 = state()
    f = cap(tag)
    pf = peak(tag, post, "peak_frame_time_ms"); pc = peak(tag, post, "peak_callback_ms")
    held = None if counter(s0, "image_hold_frames") is None else counter(s1, "image_hold_frames") - counter(s0, "image_hold_frames")
    print(f"      {tag}: {W}x{H} pre-window max frame {pre.get('peak_frame_time_ms')} callback {pre.get('peak_callback_ms')}; "
          f"window max frame {pf} callback {pc}; frames held {held}; {la()}", flush=True)
    if pf is not None:
        (ok if pf <= PEAK else no)(f"{tag}: (a) longest frame while a cold {W}x{H} image is first shown {pf:.2f} ms <= {PEAK}")
    if pc is not None:
        (ok if pc <= PEAK else no)(f"{tag}: (b) longest render callback in the same window {pc:.2f} ms <= {PEAK}")
    if f is not None:
        dd = dbox(f, img(cold))
        (ok if dd <= TOL else no)(f"{tag}: (c) the picture is the cold image after 1.5 s: dbox {dd:.2f} <= {TOL}")


def i2m(tag, col1_clip=None, fillers=True):
    W, H = SIZES["1080"]; lid = LID[tag]
    if col1_clip is None:
        col1_clip = clip(2, mpath(f"{tag}_cold.png"))
    subject = layer(lid, [clip(1, mpath("warm.png")), col1_clip], speed=1.0)
    if not load(tag, [deck(0, [filler_layer(), subject] if fillers else [subject])], (W, H)):
        return
    li = 1 if fillers else 0
    if fillers:
        wait_fillers(tag)
    trig(li, 0); wait_active(li, 0, 30.0); time.sleep(1.0)
    capA = cap(tag + "_A")
    trig(li, 1)
    seen = wait_active(li, 1)
    fs = [cap(f"{tag}_f{i}") for i in range(4)]
    time.sleep(2.0)
    capS = cap(tag + "_S")
    if capA is None or capS is None or any(f is None for f in fs) or seen is None:
        no(f"{tag}: capture failed or the trigger was never seen ({seen})"); return
    dAS = dbox(capA, capS)
    ps = [fit_p(f, capA, capS) for f in fs]
    print(f"      {tag}: dbox(warm, cold) {dAS:.2f}; trigger seen after {seen * 1000:.0f} ms; p = "
          f"{[round(p, 3) for p in ps]}", flush=True)
    (ok if dAS >= FMIN else no)(f"{tag}: fixture -- warm and cold pictures differ (dbox {dAS:.2f} >= {FMIN})")
    bar = float(FIX["i2mFirstMaxP"])
    (ok if ps[0] <= bar else no)(f"{tag}: the dissolve starts when the image lands (first answered frame p {ps[0]:.3f} <= {bar})")
    mono = all(ps[i] <= ps[i + 1] + 0.01 for i in range(3))
    (ok if mono else no)(f"{tag}: dissolve progress is monotonic across 4 frames {[round(p, 3) for p in ps]}")


def capture_after_trigger(tag, col1_clip):
    W, H = SIZES["1080"]; lid = LID[tag]
    if not load(tag, [deck(0, [filler_layer(), layer(lid, [clip(1, mpath("warm.png")), col1_clip])])], (W, H)):
        return
    wait_fillers(tag); trig(1, 0); wait_active(1, 0, 30.0); time.sleep(1.0)
    capA = cap(tag + "_A")
    trig(1, 1)
    seen = wait_active(1, 1)
    t0 = time.time(); capN = cap(tag + "_N"); tn = time.time() - t0
    time.sleep(2.0)
    capS = cap(tag + "_S")
    if capA is None or capN is None or capS is None or seen is None:
        no(f"{tag}: capture failed or the trigger was never seen ({seen})"); return
    dAS, dNS = dbox(capA, capS), dbox(capN, capS)
    print(f"      {tag}: trigger seen after {seen * 1000:.0f} ms; capN round trip {tn * 1000:.0f} ms; "
          f"dbox(capA, capS) {dAS:.2f}, dbox(capN, capS) {dNS:.2f}", flush=True)
    (ok if dAS >= FMIN else no)(f"{tag}: fixture -- the two pictures differ (dbox {dAS:.2f} >= {FMIN})")
    (ok if dNS <= TOL else no)(f"{tag}: render_frame right after the trigger shows the NEW picture (dbox {dNS:.2f} <= {TOL})")


def i4(tag):
    W, H = SIZES["4k"]; lid = LID[tag]
    if not load(tag, [deck(0, [filler_layer(), layer(lid, [clip(1, mpath("warm.png")), clip(2, mpath(f"{tag}_cold.png"))])])], (W, H)):
        return
    wait_fillers(tag); trig(1, 0); wait_active(1, 0, 30.0); time.sleep(1.0)
    s0 = state(); trig(1, 1); time.sleep(2.0); s1 = state()
    keys = ("image_hold_frames", "image_skip_frames", "images_pending")
    if any(counter(sx, k) is None for sx in (s0, s1) for k in keys):
        no(f"{tag}: /api/state has no {keys} (the app predates them)"); return
    dh = s1["image_hold_frames"] - s0["image_hold_frames"]; dsk = s1["image_skip_frames"] - s0["image_skip_frames"]
    print(f"      {tag}: frames held {dh}, skipped {dsk}, pending after {s1['images_pending']}", flush=True)
    (ok if dh >= 1 and dsk == 0 else no)(f"{tag}: a layer whose new 4K image decodes holds its last picture "
                                         f"(held {dh} >= 1, skipped {dsk} == 0)")
    (ok if s1["images_pending"] == 0 else no)(f"{tag}: nothing pending 1.5 s later ({s1['images_pending']})")


def i4m(tag):
    W, H = SIZES["4k"]; lid = LID[tag]
    base = layer(lid - 1, [clip(1, mpath("warm2.png"))])
    mask = layer(lid, [clip(2, mpath("mask_warm.png")), clip(3, mpath(f"{tag}_cold.png"))], ltype=4)
    if not load(tag, [deck(0, [filler_layer(), base, mask], ncols=4)], (W, H)):
        return
    wait_fillers(tag); trig(1, 0); trig(2, 0); wait_active(2, 0, 30.0); time.sleep(1.0)
    s0 = state(); trig(2, 1); time.sleep(2.0); s1 = state()
    keys = ("image_hold_frames", "image_skip_frames")
    if any(counter(sx, k) is None for sx in (s0, s1) for k in keys):
        no(f"{tag}: /api/state has no {keys} (the app predates them)"); return
    dh = s1["image_hold_frames"] - s0["image_hold_frames"]; dsk = s1["image_skip_frames"] - s0["image_skip_frames"]
    print(f"      {tag}: mask frames held {dh}, skipped {dsk}", flush=True)
    (ok if dh >= 1 and dsk == 0 else no)(f"{tag}: a Mask whose new 4K image decodes holds its last mask "
                                         f"(held {dh} >= 1, skipped {dsk} == 0)")


def i5(tag):
    W, H = SIZES["1080"]; lid = LID[tag]
    if not load(tag, [deck(0, [layer(lid, [clip(1, mpath("i5_cold0.png")), clip(2, mpath("i5_cold1.png"))])])], (W, H)):
        return
    time.sleep(1.0); trig(0, 0); time.sleep(1.0); trig(0, 1); time.sleep(1.0)
    if not wait_no_compiler(tag):
        return
    state(); pol = Poller(); pol.start()
    for c in (0, 1, 0, 1, 0, 1):
        trig(0, c); time.sleep(0.5)
    m = pol.stop()
    pc = peak(tag, m, "peak_callback_ms")
    print(f"      {tag}: 6 re-triggers of 2 resident 1080p images: max callback {pc}, max frame "
          f"{m.get('peak_frame_time_ms')}; {la()}", flush=True)
    if pc is not None:
        (ok if pc <= PEAK else no)(f"{tag}: re-triggering resident images never re-decodes on the GL thread "
                                   f"(max callback {pc:.2f} ms <= {PEAK})")


def i6(tag):
    W, H = SIZES["1080"]; lid = LID[tag]; n = int(FIX["sequence"]["frames"])
    frames = [mpath(f"i6_seq{j:02d}.png") for j in range(n)]
    if not load(tag, [deck(0, [layer(lid, [seq_clip(1, frames, FIX["sequence"]["fps"])])])], (W, H)):
        return
    time.sleep(1.0)
    if not wait_no_compiler(tag):
        return
    state(); pol = Poller()
    pre = pol.window(0.6)
    trig(0, 0)
    post = pol.window(1.6)
    f = cap(tag)
    pf = peak(tag, post, "peak_frame_time_ms"); pc = peak(tag, post, "peak_callback_ms")
    print(f"      {tag}: pre max frame {pre.get('peak_frame_time_ms')}; window max frame {pf} callback {pc}; {la()}", flush=True)
    if pf is not None:
        (ok if pf <= PEAK else no)(f"{tag}: (a) longest frame while a 1080p sequence plays its first frames {pf:.2f} ms <= {PEAK}")
    if pc is not None:
        (ok if pc <= PEAK else no)(f"{tag}: (b) longest render callback {pc:.2f} ms <= {PEAK}")
    if f is not None:
        ds = [dbox(f, img(os.path.basename(p))) for p in frames]
        j = int(np.argmin(ds))
        (ok if ds[j] <= TOL else no)(f"{tag}: (c) the picture is one of the {n} frames (frame {j}, dbox {ds[j]:.2f} <= {TOL})")


def i7(tag):
    W, H = SIZES["1080"]; lid = LID[tag]
    p = [mpath(f"i7_cold{j}.png") for j in range(6)]
    d0 = deck(0, [layer(lid, [clip(1, p[0]), clip(2, p[1]), clip(3, p[2])])])
    d1 = deck(1, [layer(lid, [clip(4, p[3]), clip(5, p[4]), clip(6, p[5])])])
    if not wait_no_compiler(tag):
        return
    if not load(tag + "_P", [d0, d1], (W, H)):
        return
    t0 = time.time(); pol = Poller(); pol.start(); last = None; reached = None
    while time.time() - t0 < float(FIX["prefetchTimeoutS"]):
        last = state()
        if counter(last, "image_textures") is None or counter(last, "images_pending") is None:
            break
        if last["images_pending"] == 0 and last["image_textures"] == 6:
            reached = time.time() - t0; break
        time.sleep(0.1)
    m = pol.stop()
    if counter(last, "image_textures") is None:
        no(f"{tag}: /api/state has no image_textures / images_pending (the app predates them)"); return
    pc = peak(tag, m, "peak_callback_ms")
    print(f"      {tag}: 6 images resident after {reached} s (textures {last.get('image_textures')}, "
          f"{last.get('image_texture_mb')} MB); max callback during the prefetch {pc}; {la()}", flush=True)
    (ok if reached is not None else no)(f"{tag}: (1) a composition load prefetches its 6 images (resident after {reached} s)")
    if pc is not None:
        (ok if pc <= PEAK else no)(f"{tag}: (2) longest render callback during the prefetch {pc:.2f} ms <= {PEAK}")
    s1 = state(); trig(0, 1); time.sleep(0.5); s2 = state()
    dh = s2["image_hold_frames"] - s1["image_hold_frames"]
    (ok if dh == 0 else no)(f"{tag}: (3) a prefetched image shows at once (frames held {dh} == 0)")
    if not load(tag + "_Q", [deck(0, [layer(lid, [clip(1, p[0])])])], (W, H)):
        return
    time.sleep(1.5); s3 = state()
    (ok if s3["image_textures"] <= 1 else no)(f"{tag}: (4) images no longer in the composition are released "
                                              f"(textures {s3['image_textures']} <= 1)")


def main():
    if MAKE:
        make_fixtures(); return
    W1 = SIZES["1080"]
    seq3 = [mpath(f"i3s_seq{j:02d}.png") for j in range(3)]
    seq2ms = [mpath(f"i2ms_seq{j:02d}.png") for j in range(3)]
    # Perf rows first: the pending rows below load 1 GiB of filler textures, and the frames after their release are
    # not the frames the perf rows measure.
    rows = [("i1_layer_1080", lambda: row_layer("i1_layer_1080", "1080", 0.0)),
            ("i1_layer_4k", lambda: row_layer("i1_layer_4k", "4k", 0.0)),
            ("i2_fade_1080", lambda: row_layer("i2_fade_1080", "1080", 1.0)),
            ("i2_fade_4k", lambda: row_layer("i2_fade_4k", "4k", 1.0)),
            ("i5_legacy_retrigger", lambda: i5("i5_legacy_retrigger")),
            ("i6_sequence_1080", lambda: i6("i6_sequence_1080")),
            ("i7_prefetch_retain", lambda: i7("i7_prefetch_retain")),
            ("i2m_fade_start", lambda: i2m("i2m_fade_start")),
            ("i2ms_seq_fade_start", lambda: i2m("i2ms_seq_fade_start", seq_clip(2, seq2ms, 0.1), fillers=False)),
            ("i3_capture_after_trigger", lambda: capture_after_trigger(
                "i3_capture_after_trigger", clip(2, mpath("i3_capture_after_trigger_cold.png")))),
            ("i3s_capture_after_seq_trigger", lambda: capture_after_trigger(
                "i3s_capture_after_seq_trigger", seq_clip(2, seq3, 0.1))),
            ("i4_hold_counters", lambda: i4("i4_hold_counters")),
            ("i4m_mask_hold", lambda: i4m("i4m_mask_hold"))]
    for name, fn in rows:
        if ONLY is None or name in ONLY:
            print(f"--- {name}", flush=True)
            fn()
    print(f"\nPY {PASS} PASS / {FAIL} FAIL", flush=True)
    sys.exit(0 if FAIL == 0 else 1)


if __name__ == "__main__":
    main()
