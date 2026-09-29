#!/usr/bin/env python3
"""probe-media-open.py -- REST/pixel half of .harmony/probe-media-open.sh (s-rta-0928b mediaopen).

Plan: .harmony/.reports/s-rta-0928b/plan-mediaopen.md 4.10 + its HARMONY ADOPTION (commit 7, the asynchronous loads,
is deferred: rows m2 / m2b move with it; the load's fence row m2c stays). The .sh owns launch / refuse / quit; this file
writes the fixtures (--make-fixtures, BEFORE the app starts) and then talks to the running app on 7070. Every captured
PNG is decoded with PIL+numpy; every render_frame response is checked; the output dir is fresh per run.

usage: probe-media-open.py <root> <fresh-outdir> --make-fixtures [row,row,...]
       probe-media-open.py <root> <fresh-outdir> [row,row,...]
rows: m1_heartbeat m2c_load_fence m3_drop_4k_video m3b_drop_1080_video m3c_drop_4k_image m3d_drop_seq300
      m4_load_two_seq m5_missing_frame_repeats m6_retrigger_seek m7_presence m8_speed_default end_hold_witness

Witnesses (/api/state): fence_hold_frames / fence_black_frames -- cumulative frames the renderer found FENCED
(UndoService::withDeckDetached) and deck-less: HELD the canvas (the fix) / fell to the black "nothing to render" path
(the defect); peak_message_stall_ms (TEST-ONLY, POST /api/debug/heartbeat {"on":true,"period_ms":4}) -- the longest
message-thread wait of a heartbeat ping, reset on read, polled every 15 ms keeping the max. Drops go through the
TEST-ONLY POST /api/debug/drop_files {layer, column, files} -- the handlers a Finder drop reaches (ClipCell::classifyDrop
then the same DeckView callback). /api/composition clips: mediaMissing / clipWidth / clipHeight / thumbnailW / thumbnailH.
dbox(X, Y) = both arrays box-averaged in 16x16 blocks, mean |diff| over RGB (Y resized to X first) -- probe-image-load's.

Fixtures (<out>/media): v4k.mp4 / v1080.mp4 (flat 0x3060c0, ffmpeg libx264 yuv420p, .json "video") + their first frame
as ref PNGs; v1080_m8.mp4 (a copy); img4k.png (probe-image-load's noisy gradient, 3840x2160); warm / base / top /
present (= top's twin) flat 256x144; seqA / seqB / seqD: 300 frames each -- frame 0 a NOISY 1080p PNG (a slow decode),
frames 1-299 a flat 1080p PNG per set (A green, B magenta, D orange); seq12 / seq12m: 12 frames 640x360, frame k flat
(20k, 0, 255-20k) -> code(cap) = the nearest of the 12; seq12m's frame 05 is deleted before its load.

m1_heartbeat: POST heartbeat on (4 ms); 0.5 s; s. PASS: 200 ok; message_heartbeat_on true; peak_message_stall_ms
  present. RED on main: 404.
m2c_load_fence (the load's fence frame, plan m2 (c)): load W1 (warm) and trigger it; 1 s; s0; then 3 loads of image
  compositions (W2, W3, W4), each waited for (the new layer id in /api/composition); 0.3 s; s1. PASS: fence_black_frames
  delta 0. RED on the counters-only app: >= 1 per load (diag-media S2: 1 per load, 5/5).
m3_drop_4k_video / m3b_drop_1080_video / m3c_drop_4k_image / m3d_drop_seq300: load W (layer 0 col 0 warm.png triggered,
  layer 1 empty, 1 column); 1 s; heartbeat on; s0; drop_files {layer 1, column 0, [file(s)]}; poll /api/composition
  until layer 1 column 0 holds a clip (<= 5 s) while polling the stall; 0.3 s; s1. PASS: (a) fence_black_frames delta 0;
  (b) fence_hold_frames delta <= holdMaxPerDrop; (c) max stall <= the row's bound (m3 dropStall4kMaxMs, m3b
  dropStall1080MaxMs, m3d stallMaxMs; m3c none); (d) trigger layer 1 col 0, then the picture: dbox(cap, ref) <= boxTol
  (video: its first frame; image: img4k; sequence at 2.5 fps after 1 s: seqD's flat frame). RED on the counters-only app:
  m3 (a) 13-14, m3b (a) 5-6, m3d (a) 4-5; m3c (a) 0-2 (a GUARD: counted RED only if >= 4 of 5 runs fail, adoption P4).
m4_load_two_seq: load E (warm); 1 s; heartbeat on; s0; load S2 (layer 0 = seqA, layer 1 = seqB, 30 fps, not triggered),
  polling the stall until S2's layers show (<= 5 s); 0.3 s; s1. PASS: (a) max stall <= stallMaxMs; (b)
  fence_black_frames delta 0; (c) trigger both, 1 s, dbox(cap, seqB flat) <= boxTol. RED on the counters-only app:
  (a) by value (each sequence's frame 0 decoded twice on the message thread + 600 stats).
m5_missing_frame_repeats: seq12m (frame 05 deleted), 1 fps, Loop; trigger; at 5.4 s after the trigger, cap. PASS:
  code(cap) == 4 (the missing frame repeats the previous one). RED on main: code 6 (open() dropped the missing file,
  so slot 5 plays file 06).
m6_retrigger_seek (guard): seq12, 1 fps, inPoint 0.5; trigger; 1.2 s; trigger again (retrigger -> seekTo(inPoint));
  0.25 s; comp + cap. PASS: playheadPosition in [0.5, 0.8); code(cap) in {6, 7}. PASS on main too (a request consumed
  on the next GL frame lands within one frame).
m7_presence: load P (layer 0 = base.png, layer 1 = present.png, both triggered); 1 s; capA (~ top); delete present.png;
  poll the clip's mediaMissing every 50 ms (<= 2.5 s); capB; rewrite present.png; poll until false; capC. PASS:
  mediaMissing true within presenceMaxS; dbox(capB, base) <= boxTol (the missing layer is skipped -- today's
  semantics); false again within presenceMaxS; dbox(capC, capA) <= boxTol. RED on main: the field is absent.
m8_speed_default: a v1080 clip whose JSON has NO "speed"; trigger; 1.0 s; comp. PASS: playheadPosition >=
  speedMinPlayhead. RED on main: 0.0 (fromVar read the missing key as 0 -- frozen).
end_hold_witness: fence_hold_frames over the whole run >= 1 (the hold path ran). RED on the counters-only app: 0.
Calibration: .harmony/.reports/s-rta-0928b/mediaopen.md.
"""
import json, os, shutil, subprocess, sys, threading, time

import numpy as np
import requests
from PIL import Image

A = "http://127.0.0.1:7070"
ROOT, OUT = sys.argv[1], sys.argv[2]
MAKE = len(sys.argv) > 3 and sys.argv[3] == "--make-fixtures"
_rows_arg = (sys.argv[4] if len(sys.argv) > 4 else "") if MAKE else (sys.argv[3] if len(sys.argv) > 3 else "")
ONLY = set(_rows_arg.split(",")) if _rows_arg else None
FIX = json.load(open(os.path.join(ROOT, ".harmony", "probe-media-open.json")))
TOL = float(FIX["boxTol"]); LID = FIX["layers"]; CW, CH = FIX["canvas"]
MEDIA = os.path.join(OUT, "media")
PASS = FAIL = 0
S = requests.Session()
S.headers["Connection"] = "close"   # one fresh connection per request (cpp-httplib 5 s keep-alive drop)

FLAT = {"A": (40, 200, 90), "B": (200, 40, 170), "D": (230, 140, 20)}
SEQ12 = [(20 * k, 0, 255 - 20 * k) for k in range(12)]


def ok(msg):
    global PASS; PASS += 1; print(f"PASS  {msg}", flush=True)


def no(msg):
    global FAIL; FAIL += 1; print(f"FAIL  {msg}", flush=True)


def la():
    return "load avg %.2f %.2f %.2f" % os.getloadavg()


# ---------------------------------------------------------------- fixtures (written before the app starts)
def mpath(*name):
    return os.path.join(MEDIA, *name)


def write_png(path, arr):
    Image.fromarray(arr, "RGBA").save(path, compress_level=1)


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


def ffmpeg():
    return shutil.which("ffmpeg") or "/opt/homebrew/bin/ffmpeg"


def make_video(name, key):
    r = subprocess.run([ffmpeg(), "-y", "-loglevel", "error"] + FIX["video"][key] + [mpath(name)],
                       capture_output=True, text=True)
    if r.returncode != 0 or not os.path.exists(mpath(name)):
        raise SystemExit(f"ffmpeg could not make {name}: {r.stderr[:300]}")
    ref = mpath(name.replace(".mp4", "_ref.png"))
    r = subprocess.run([ffmpeg(), "-y", "-loglevel", "error", "-i", mpath(name), "-frames:v", "1", ref],
                       capture_output=True, text=True)
    if r.returncode != 0 or not os.path.exists(ref):
        raise SystemExit(f"ffmpeg could not extract {ref}: {r.stderr[:300]}")


def seq_dir(name):
    return mpath(name)


def seq_files(name, n):
    return [os.path.join(seq_dir(name), f"f{j:03d}.png") for j in range(n)]


def make_seq300(name, k):
    os.makedirs(seq_dir(name), exist_ok=True)
    files = seq_files(name, int(FIX["seqFrames"]))
    write_png(files[0], noisy(1920, 1080, 40 + k))
    write_png(files[1], flat(1920, 1080, FLAT[name[-1]]))
    for f in files[2:]:
        shutil.copyfile(files[1], f)


def make_fixtures():
    os.makedirs(MEDIA, exist_ok=True)
    t0 = time.time()
    make_video("v4k.mp4", "v4k")
    make_video("v1080.mp4", "v1080")
    shutil.copyfile(mpath("v1080.mp4"), mpath("v1080_m8.mp4"))
    write_png(mpath("img4k.png"), noisy(3840, 2160, 7))
    for name, rgb in (("warm.png", (200, 120, 40)), ("base.png", (30, 60, 200)), ("top.png", (220, 220, 60)),
                      ("present.png", (220, 220, 60)), ("w2.png", (90, 30, 30)), ("w3.png", (30, 90, 30)),
                      ("w4.png", (30, 30, 90))):
        write_png(mpath(name), flat(256, 144, rgb))
    for k, name in enumerate(("seqA", "seqB", "seqD")):
        make_seq300(name, k)
    for name in ("seq12", "seq12m"):
        os.makedirs(seq_dir(name), exist_ok=True)
        for j, f in enumerate(seq_files(name, 12)):
            write_png(f, flat(640, 360, SEQ12[j]))
    print(f"fixtures written to {MEDIA} in {time.time() - t0:.1f} s", flush=True)


# ---------------------------------------------------------------- REST helpers
def iclip(cid, path):
    return {"name": f"c{cid}", "id": cid, "mediaType": 1, "mediaFile": path, "effects": []}


def vclip(cid, path, speed=True):
    c = {"name": f"v{cid}", "id": cid, "mediaType": 2, "mediaFile": path, "transportMode": 0, "loopMode": 0,
         "effects": []}
    if speed:
        c["speed"] = 1.0
    return c


def sclip(cid, paths, fps, **extra):
    c = {"name": f"s{cid}", "id": cid, "mediaType": 5, "mediaFile": "", "sequenceFiles": paths,
         "sequenceFps": float(fps), "speed": 1.0, "transportMode": 0, "loopMode": 0, "effects": []}
    c.update(extra); return c


def layer(lid, clips):
    return {"name": f"L{lid}", "id": lid, "opacity": 1.0, "visible": True, "blendMode": 0, "type": 0,
            "transitionSpeed": 0.0, "layerEffects": [], "clips": clips}


def load(tag, layers, ncols=1):
    comp = {"name": "mediaopen-" + tag, "activeDeckIndex": 0, "masterOpacity": 1.0, "globalTransitionSpeed": 0.0,
            "outputWidth": CW, "outputHeight": CH,
            "decks": [{"name": "D0", "id": 0, "numColumns": ncols, "layers": layers}]}
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
        return {}


def comp_state():
    try:
        return S.get(A + "/api/composition", timeout=6).json()
    except Exception:  # noqa: BLE001
        return None


def layer_json(li):
    c = comp_state()
    try:
        return c["decks"][0]["layers"][li]
    except Exception:  # noqa: BLE001
        return None


def cell_json(li, col):
    L = layer_json(li)
    for c in (L or {}).get("clips", []):
        if c.get("column") == col:
            return c
    return None


def wait_until(pred, limit, step=0.02):
    t0 = time.time()
    while time.time() - t0 < limit:
        v = pred()
        if v:
            return time.time() - t0, v
        time.sleep(step)
    return None, None


def wait_layer_id(li, lid, limit=5.0):
    t, _ = wait_until(lambda: (layer_json(li) or {}).get("id") == lid, limit)
    return t


def wait_active(li, col, limit=10.0):
    t, _ = wait_until(lambda: (layer_json(li) or {}).get("activeClipColumn") == col, limit, 0.005)
    return t


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


def img(path):
    return np.asarray(Image.open(path).convert("RGBA")).astype(np.float32)


def flat_img(rgb):
    return flat(64, 36, rgb).astype(np.float32)


def box(a, b=16):
    h, w = a.shape[0] // b * b, a.shape[1] // b * b
    return a[:h, :w, :3].reshape(h // b, b, w // b, b, 3).mean(axis=(1, 3))


def dbox(x, y):
    if x.shape[:2] != y.shape[:2]:
        y = np.asarray(Image.fromarray(y.astype(np.uint8), "RGBA").resize((x.shape[1], x.shape[0]), Image.BOX)).astype(np.float32)
    return float(np.abs(box(x) - box(y)).mean())


def code(capture):
    m = capture[..., :3].reshape(-1, 3).mean(axis=0)
    d = [float(np.abs(m - np.array(c, np.float32)).sum()) for c in SEQ12]
    return int(np.argmin(d)), m


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


def hb_on():
    try:
        r = S.post(A + "/api/debug/heartbeat", json={"on": True, "period_ms": int(FIX["heartbeatPeriodMs"])}, timeout=6)
        return r.status_code, (r.json() if r.status_code == 200 else None)
    except Exception as e:  # noqa: BLE001
        return None, str(e)


class Poller:
    """Reads /api/state every 15 ms in a thread; peak_message_stall_ms resets on read, so the max over reads is the
    max over the window."""
    KEYS = ("peak_message_stall_ms", "peak_callback_ms")

    def __init__(self):
        self.rows = []; self._run = False

    def _loop(self):
        s = requests.Session(); s.headers["Connection"] = "close"
        while self._run:
            try:
                d = s.get(A + "/api/state", timeout=6).json()
                self.rows.append({k: d.get(k) for k in self.KEYS})
            except Exception:  # noqa: BLE001
                pass
            time.sleep(0.015)

    def start(self):
        self.rows = []; self._run = True
        self._th = threading.Thread(target=self._loop, daemon=True); self._th.start()

    def stop(self):
        self._run = False; self._th.join()
        out = {}
        for k in self.KEYS:
            v = [r[k] for r in self.rows if r.get(k) is not None]
            out[k] = max(v) if v else None
        return out


def fence(s):
    return s.get("fence_hold_frames"), s.get("fence_black_frames")


def fence_deltas(tag, s0, s1):
    h0, b0 = fence(s0); h1, b1 = fence(s1)
    if None in (h0, b0, h1, b1):
        no(f"{tag}: /api/state has no fence_hold_frames / fence_black_frames (the app predates them)")
        return None, None
    return h1 - h0, b1 - b0


def stall_max(tag, pol_max, s1):
    vals = [v for v in (pol_max.get("peak_message_stall_ms"), s1.get("peak_message_stall_ms")) if v is not None]
    if not vals:
        no(f"{tag}: no peak_message_stall_ms (the app predates the heartbeat)")
        return None
    return max(vals)


# ---------------------------------------------------------------- rows
def m1(tag):
    code_, body = hb_on()
    (ok if code_ == 200 and isinstance(body, dict) and body.get("ok") is True else no)(
        f"{tag}: POST /api/debug/heartbeat answers 200 ok (got {code_} {str(body)[:80]})")
    time.sleep(0.5)
    s = state()
    (ok if s.get("message_heartbeat_on") is True else no)(f"{tag}: message_heartbeat_on true (got {s.get('message_heartbeat_on')})")
    (ok if "peak_message_stall_ms" in s else no)(f"{tag}: peak_message_stall_ms present (got {s.get('peak_message_stall_ms')})")


def m2c(tag):
    L = LID[tag]
    if not load(tag + "_W1", [layer(L, [iclip(1, mpath("warm.png"))])]):
        return
    wait_layer_id(0, L); trig(0, 0); wait_active(0, 0); time.sleep(1.0)
    s0 = state()
    swapped = 0
    for j, name in enumerate(("w2.png", "w3.png", "w4.png")):
        lid = L + 10 * (j + 1)
        if not load(f"{tag}_W{j + 2}", [layer(lid, [iclip(1, mpath(name))])]):
            return
        if wait_layer_id(0, lid) is not None:
            swapped += 1
        time.sleep(0.3)
    s1 = state()
    dh, db = fence_deltas(tag, s0, s1)
    if dh is None:
        return
    print(f"      {tag}: {swapped}/3 loads swapped; fence hold +{dh}, black +{db}; {la()}", flush=True)
    (ok if swapped == 3 else no)(f"{tag}: the 3 loads swapped ({swapped}/3)")
    (ok if db == 0 else no)(f"{tag}: (c) a composition load shows no deck-less black frame (fence_black_frames +{db} == 0)")


def drop_row(tag, files, stall_bound, picture):
    L = LID[tag]
    if not load(tag, [layer(L, [iclip(1, mpath("warm.png"))]), layer(L + 100, [])]):
        return
    wait_layer_id(0, L); trig(0, 0); wait_active(0, 0); time.sleep(1.0)
    if stall_bound is not None and not wait_no_compiler(tag):
        return
    code_, _ = hb_on()
    if code_ != 200:
        no(f"{tag}: POST /api/debug/heartbeat answered {code_} (the app predates it)")
    time.sleep(0.3)
    s0 = state()
    pol = Poller(); pol.start()
    try:
        r = S.post(A + "/api/debug/drop_files", json={"layer": 1, "column": 0, "files": files}, timeout=10)
        dropped = r.status_code
    except Exception as e:  # noqa: BLE001
        dropped = str(e)
    landed, _ = wait_until(lambda: cell_json(1, 0), 5.0)
    time.sleep(0.3)
    pm = pol.stop(); s1 = state()
    if dropped != 200:
        no(f"{tag}: POST /api/debug/drop_files answered {dropped} (the app predates it)"); return
    (ok if landed is not None else no)(f"{tag}: the dropped clip is in layer 1 column 0 (after {landed} s)")
    dh, db = fence_deltas(tag, s0, s1)
    st = stall_max(tag, pm, s1)
    c = cell_json(1, 0) or {}
    print(f"      {tag}: fence hold +{dh}, black +{db}; max message stall {st} ms; peak callback "
          f"{pm.get('peak_callback_ms')} ms; clip {c.get('clipWidth')}x{c.get('clipHeight')} thumb "
          f"{c.get('thumbnailW')}x{c.get('thumbnailH')}; {la()}", flush=True)
    if dh is not None:
        (ok if db == 0 else no)(f"{tag}: (a) no deck-less black frame during the drop (fence_black_frames +{db} == 0)")
        (ok if dh <= int(FIX["holdMaxPerDrop"]) else no)(
            f"{tag}: (b) the fence holds only the setClip (fence_hold_frames +{dh} <= {FIX['holdMaxPerDrop']})")
    if stall_bound is not None and st is not None:
        (ok if st <= float(stall_bound) else no)(f"{tag}: (c) longest message-thread stall {st:.1f} ms <= {stall_bound}")
    if landed is None:
        return
    trig(1, 0); wait_active(1, 0)
    time.sleep(1.0 if picture[0] == "seq" else 0.5)
    x = cap(tag)
    if x is None:
        return
    d = dbox(x, picture[1]())
    (ok if d <= TOL else no)(f"{tag}: (d) the dropped {picture[0]} is on screen (dbox {d:.2f} <= {TOL})")


def m4(tag):
    L = LID[tag]
    if not load(tag + "_E", [layer(L, [iclip(1, mpath("warm.png"))])]):
        return
    wait_layer_id(0, L); trig(0, 0); wait_active(0, 0); time.sleep(1.0)
    if not wait_no_compiler(tag):
        return
    hb_on(); time.sleep(0.3)
    s0 = state()
    pol = Poller(); pol.start()
    good = load(tag + "_S2", [layer(L + 1, [sclip(1, seq_files("seqA", int(FIX["seqFrames"])), 30)]),
                              layer(L + 2, [sclip(2, seq_files("seqB", int(FIX["seqFrames"])), 30)])])
    t = wait_layer_id(1, L + 2) if good else None
    time.sleep(0.3)
    pm = pol.stop(); s1 = state()
    if not good:
        return
    (ok if t is not None else no)(f"{tag}: the two-sequence composition is loaded (after {t} s)")
    dh, db = fence_deltas(tag, s0, s1)
    st = stall_max(tag, pm, s1)
    print(f"      {tag}: max message stall {st} ms; fence hold +{dh}, black +{db}; {la()}", flush=True)
    if st is not None:
        (ok if st <= float(FIX["stallMaxMs"]) else no)(f"{tag}: (a) longest message-thread stall {st:.1f} ms <= {FIX['stallMaxMs']}")
    if db is not None:
        (ok if db == 0 else no)(f"{tag}: (b) no deck-less black frame (fence_black_frames +{db} == 0)")
    trig(0, 0); trig(1, 0); wait_active(1, 0); time.sleep(1.0)
    x = cap(tag)
    if x is not None:
        d = dbox(x, flat_img(FLAT["B"]))
        (ok if d <= TOL else no)(f"{tag}: (c) both sequences play (the top one's frame on screen, dbox {d:.2f} <= {TOL})")


def m5(tag):
    L = LID[tag]
    files = seq_files("seq12m", 12)
    os.remove(files[5])
    if not load(tag, [layer(L, [sclip(1, files, 1.0)])]):
        return
    wait_layer_id(0, L); time.sleep(0.5)
    trig(0, 0); t0 = time.time(); wait_active(0, 0)
    time.sleep(max(0.0, 5.4 - (time.time() - t0)))
    x = cap(tag)
    s = state()
    if x is None:
        return
    k, m = code(x)
    print(f"      {tag}: code {k} (mean rgb {np.round(m, 1).tolist()}); images_pending {s.get('images_pending')}; "
          f"seq_late_frames {s.get('seq_late_frames')}", flush=True)
    (ok if k == 4 else no)(f"{tag}: the missing frame 05 repeats frame 04 (code {k} == 4)")


def m6(tag):
    L = LID[tag]
    if not load(tag, [layer(L, [sclip(1, seq_files("seq12", 12), 1.0, inPoint=0.5)])]):
        return
    wait_layer_id(0, L); time.sleep(0.5)
    trig(0, 0); wait_active(0, 0); time.sleep(1.2)
    trig(0, 0); time.sleep(0.25)
    c = cell_json(0, 0) or {}
    x = cap(tag)
    pos = c.get("playheadPosition")
    k = code(x)[0] if x is not None else None
    print(f"      {tag}: playhead {pos}, code {k}", flush=True)
    (ok if pos is not None and 0.5 <= pos < 0.8 else no)(f"{tag}: the retrigger seeks to the in-point (playhead {pos} in [0.5, 0.8))")
    (ok if k in (6, 7) else no)(f"{tag}: the frame after the seek is 6 or 7 (code {k})")


def m7(tag):
    L = LID[tag]
    present = mpath("present.png")
    if not load(tag, [layer(L, [iclip(1, mpath("base.png"))]), layer(L + 100, [iclip(2, present)])]):
        return
    wait_layer_id(0, L); trig(0, 0); trig(1, 0); wait_active(1, 0); time.sleep(1.0)
    capA = cap(tag + "_A")
    if capA is None:
        return
    dA = dbox(capA, img(mpath("top.png")))
    (ok if dA <= TOL else no)(f"{tag}: the top layer shows first (dbox vs top {dA:.2f} <= {TOL})")
    if "mediaMissing" not in (cell_json(1, 0) or {}):
        no(f"{tag}: /api/composition clips have no mediaMissing (the app predates it)"); return
    os.remove(present)
    tMiss, _ = wait_until(lambda: (cell_json(1, 0) or {}).get("mediaMissing") is True, 2.5, 0.05)
    time.sleep(0.2)
    capB = cap(tag + "_B")
    with open(mpath("top.png"), "rb") as src, open(present, "wb") as dst:
        dst.write(src.read())
    tBack, _ = wait_until(lambda: (cell_json(1, 0) or {}).get("mediaMissing") is False, 2.5, 0.05)
    time.sleep(0.3)
    capC = cap(tag + "_C")
    pmax = float(FIX["presenceMaxS"])
    print(f"      {tag}: missing after {tMiss} s, present again after {tBack} s", flush=True)
    (ok if tMiss is not None and tMiss <= pmax else no)(f"{tag}: a deleted file is missing within {pmax} s ({tMiss})")
    if capB is not None:
        dB = dbox(capB, img(mpath("base.png")))
        (ok if dB <= TOL else no)(f"{tag}: the missing layer is skipped -- the base shows (dbox {dB:.2f} <= {TOL})")
    (ok if tBack is not None and tBack <= pmax else no)(f"{tag}: a restored file is present within {pmax} s ({tBack})")
    if capC is not None:
        dC = dbox(capC, capA)
        (ok if dC <= TOL else no)(f"{tag}: the restored layer shows again (dbox vs capA {dC:.2f} <= {TOL})")


def m8(tag):
    L = LID[tag]
    if not load(tag, [layer(L, [vclip(1, mpath("v1080_m8.mp4"), speed=False)])]):
        return
    wait_layer_id(0, L); time.sleep(0.5)
    trig(0, 0); wait_active(0, 0); time.sleep(1.0)
    c = cell_json(0, 0) or {}
    pos = c.get("playheadPosition")
    lim = float(FIX["speedMinPlayhead"])
    (ok if pos is not None and pos >= lim else no)(
        f"{tag}: a clip saved without \"speed\" plays at 1.0 (playhead {pos} >= {lim} after 1 s)")


def main():
    if MAKE:
        make_fixtures(); return
    s_first = state()
    rows = [("m1_heartbeat", lambda: m1("m1_heartbeat")),
            ("m2c_load_fence", lambda: m2c("m2c_load_fence")),
            ("m3_drop_4k_video", lambda: drop_row("m3_drop_4k_video", [mpath("v4k.mp4")], FIX["dropStall4kMaxMs"],
                                                  ("video", lambda: img(mpath("v4k_ref.png"))))),
            ("m3b_drop_1080_video", lambda: drop_row("m3b_drop_1080_video", [mpath("v1080.mp4")], FIX["dropStall1080MaxMs"],
                                                     ("video", lambda: img(mpath("v1080_ref.png"))))),
            ("m3c_drop_4k_image", lambda: drop_row("m3c_drop_4k_image", [mpath("img4k.png")], None,
                                                   ("image", lambda: img(mpath("img4k.png"))))),
            ("m3d_drop_seq300", lambda: drop_row("m3d_drop_seq300", seq_files("seqD", int(FIX["seqFrames"])),
                                                 FIX["stallMaxMs"], ("seq", lambda: flat_img(FLAT["D"])))),
            ("m4_load_two_seq", lambda: m4("m4_load_two_seq")),
            ("m5_missing_frame_repeats", lambda: m5("m5_missing_frame_repeats")),
            ("m6_retrigger_seek", lambda: m6("m6_retrigger_seek")),
            ("m7_presence", lambda: m7("m7_presence")),
            ("m8_speed_default", lambda: m8("m8_speed_default"))]
    for name, fn in rows:
        if ONLY is None or name in ONLY:
            print(f"--- {name}", flush=True)
            fn()
    if ONLY is None or "end_hold_witness" in ONLY:
        print("--- end_hold_witness", flush=True)
        s_end = state()
        h0, h1 = s_first.get("fence_hold_frames"), s_end.get("fence_hold_frames")
        if h0 is None or h1 is None:
            no("end_hold_witness: /api/state has no fence_hold_frames (the app predates it)")
        else:
            print(f"      end_hold_witness: fence hold +{h1 - h0}, black +{s_end.get('fence_black_frames', 0) - s_first.get('fence_black_frames', 0)} over the run", flush=True)
            (ok if h1 - h0 >= 1 else no)(f"end_hold_witness: the fence held the canvas at least once ({h1 - h0} frames)")
    print(f"\nPY {PASS} PASS / {FAIL} FAIL", flush=True)
    sys.exit(0 if FAIL == 0 else 1)


if __name__ == "__main__":
    main()
