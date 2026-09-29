#!/usr/bin/env python3
"""probe-async-load.py -- REST/pixel half of .harmony/probe-async-load.sh (s-rta-0929 asyncload).

Plan: .harmony/.reports/s-rta-0929/plan-asyncload.md 5.9 + its HARMONY ADOPTION (AL1-AL12). The .sh owns launch /
refuse / quit; this file writes the fixtures (--make-fixtures, BEFORE the app starts) and then talks to the running app
on 7070. Every captured PNG is decoded with PIL+numpy; every render_frame response is checked; the output dir is fresh
per run. Rows may be listed more than once (AL1: "a2_load_16x4k" x5 in one launch).

usage: probe-async-load.py <root> <fresh-outdir> --make-fixtures [row,row,...]
       probe-async-load.py <root> <fresh-outdir> [row,row,...]
rows: a1_witness_fields a2_load_16x4k a2b_load_16x1080 a3_cancel_by_newer_load a3b_explicit_cancel
      a3c_cancel_with_sequences a4_load_then_trigger a5_duplicate_deck a5b_duplicate_twice a6_append_deck
      a7_failure_mid_batch end_hold_witness end_quit_mid_load | end_quit_hung_open (its own launch, see the .sh)

Witnesses: /api/state "load" {opens_pending, open_batches, opens_stale, opens_failed, opens_dropped, staged,
staged_players, queued, timing{...}, TEST-ONLY audio_xruns / audio_callbacks / audio_callback_gap_max_ms (reset on read) /
audio_callback_period_ms / analysis_ring_overruns}; peak_message_stall_ms (TEST-ONLY heartbeat, reset on read, polled
every 15 ms keeping the max); fence_hold_frames / fence_black_frames; video_players / video_uploads / video_late_frames /
video_pending_frames / videos_pending / seq_open. GET /api/debug/ui_text = the file label read ON the message thread.
POST /api/load_composition answers only after the staged swap on the async app ({"ok":false,"reason":"superseded by a
newer load"} for a load a newer one / a cancel retired). /api/composition has no composition name: a composition is
identified by its layer ids (unique per row).

a1_witness_fields: heartbeat on; state.load has every field; ui_text answers. RED on main: absent / 404.
a2_load_16x4k: load W (layer 0 col 0 = a playing 1080p clip, triggered); 1 s; quiet; heartbeat; s0; Poller; post the
  16 x 4K load V16 on a thread; wait for its answer; /api/composition at once; s1. PASS: (a) max stall <= stallMaxMs;
  (b) ok:true within swapMaxS AND the composition read right after the answer is V16 with 16 clips; (c) fence_black +0,
  hold <= holdMaxPerCut; (d) the OLD clip kept uploading (>= uploadsMinPerS x window) and its playhead moved during the
  window (window = answer - post; < 0.3 s = not measurable, printed); (e) every V16 clip clipWidth 3840, thumbnailW > 0;
  (f) trigger (0,0): video_pending_frames +0 and dbox(cap, ref4k) <= boxTol; (g) audio_xruns +0, analysis_ring_overruns
  +0, gap max <= audioGapFactor x period; (h) video_late_frames +<= lateMaxPerLoad; (i) load.timing printed (the commit-1
  gate: opens_ms / total_ms >= 0.8). RED on the commit-1 app: (a) ~1000+, (b) (the answer precedes the load).
a2b_load_16x1080: a2 with 16 x 1080p ((e) clipWidth 1920, ref1080).
a3_cancel_by_newer_load: load W2 (image comp base / top); s0; post V32; at +cancelDelayS post W3 (image comp); PASS:
  (a) V32 answers ok:false "superseded by a newer load" within cancelAnswerMaxS of W3's post; (b) W3 ok:true within
  swapMaxS and on screen; (c) ui_text "Loaded: <W3>"; (d) open_batches +2 (stale / dropped printed); (e) after 1 s
  video_players == 0; (f) max stall <= stallMaxMs.
a3b_explicit_cancel: load W2; s0; post V32; at +cancelDelayS POST /api/debug/cancel_load. PASS: (a) V32 answers
  "superseded..." within cancelAnswerMaxS of the cancel; (b) W2 still on screen; (c) ui_text "Loaded: <W2>" (restored);
  (d) after 1 s video_players == 0; (e) fence_black +0.
a3c_cancel_with_sequences: a3b with V32S = V32 + a layer of two 300-frame sequences; (f) seq_open unchanged after 1 s.
a4_load_then_trigger: load W2 (base col 0 triggered, top col 1); 1 s; s0; post V32; at +triggerDelayS trigger (0, 1)
  from this thread. PASS: precondition V32's window > triggerDelayS (else FAIL "window too short to test"); (a) the
  trigger answered before the load; (b) W2 answered it while V32 was staged (layer 0 activeClipColumn 1 on W2);
  (c) after the load: V32 on screen, every layer activeClipColumn -1; (d) fence_black +0; (e) trigger (0,0) -> v4k on
  screen; (f1) ui_text right after the trigger reads "Loading <V32>..." (AL5); (f2) a second window: post V32b, trigger
  (0,1), cancel_load -> ui_text == the trigger's text (top's file name), not "Loaded: <W2b>".
a5_duplicate_deck: load D (deck "A": 4 x 4K, col 0 triggered); 1 s; quiet; heartbeat; s0; POST duplicate_deck {0};
  +0.05 s ui_text; poll numDecks == 2. PASS: (a) "Loading A copy..." then "Duplicated deck: A copy"; (b) activeDeck 1;
  (c) video_pending_frames +0, videos_pending max 0 in the window, fence_black +0, hold <= holdMaxPerCut; (d) the copy's
  active clip on screen (dbox ref4k); (e) max stall <= stallMaxMs; (f) video_players == 8.
a5b_duplicate_twice (AL7): load D; two duplicate_deck {0} posts 0.05 s apart. PASS: numDecks 3 (deck names = today's:
  "A", "A copy", "A copy" -- compload::duplicateDeck names a copy "<src> copy"), activeDeck 2, video_players 12, label
  "Duplicated deck: A copy".
a6_append_deck: load W2; s0; POST load_deck {deck16.json}; +0.05 s ui_text; poll numDecks == 2. PASS: (a) max stall;
  (b) numDecks 2 within swapMaxS, activeDeck 1, ui_text "Loaded deck: deck16"; (c) deck 1's 16 clips clipWidth 3840;
  (d) fence_black +0; (e) audio as a2 (g).
a7_failure_mid_batch: V16F = V16 with cell 7 = a header-only H.264 (open() fails) and cell 9's file deleted before the
  load. PASS: (a) ok:true within swapMaxS; (b) cell 7 keeps the file defaults (clipWidth 1920, thumbnailW 0), cell 9
  mediaMissing, the other 14 clipWidth 3840; (c) opens_failed +1; (d) trigger (0,0) -> v4k; (e) no crash dialog.
end_hold_witness: fence_hold_frames over the run >= 1.
end_quit_mid_load: post V32 and return after 0.1 s (the .sh quits the app while it is staged).
end_quit_hung_open (AL2, its own launch): a composition whose video cell 0 is a FIFO (open(2) blocks forever) + one 4K
  cell; post it; at +0.3 s post W3 (image comp). PASS: W3 ok:true within swapMaxS; the FIFO load answers "superseded";
  ui_text answers "Loaded: <W3>" (the app works while an open hangs). The .sh then quits (<= 30 s, no .ips, no dialog).
"""
import json, os, shutil, subprocess, sys, threading, time

import numpy as np
import requests
from PIL import Image

A = "http://127.0.0.1:7070"
ROOT, OUT = sys.argv[1], sys.argv[2]
MAKE = len(sys.argv) > 3 and sys.argv[3] == "--make-fixtures"
_rows_arg = (sys.argv[4] if len(sys.argv) > 4 else "") if MAKE else (sys.argv[3] if len(sys.argv) > 3 else "")
ONLY = [r for r in _rows_arg.split(",") if r] if _rows_arg else None
FIX = json.load(open(os.path.join(ROOT, ".harmony", "probe-async-load.json")))
TOL = float(FIX["boxTol"]); LID = FIX["layers"]; CW, CH = FIX["canvas"]
STALL = float(FIX["stallMaxMs"]); SWAP = float(FIX["swapMaxS"]); CANCEL = float(FIX["cancelAnswerMaxS"])
HOLD = int(FIX["holdMaxPerCut"])
MEDIA = os.path.join(OUT, "media")
PASS = FAIL = 0
S = requests.Session()
S.headers["Connection"] = "close"   # one fresh connection per request (cpp-httplib 5 s keep-alive drop)
LOAD_KEYS = ("opens_pending", "open_batches", "opens_stale", "opens_failed", "opens_dropped", "staged", "staged_players",
             "queued", "timing", "audio_xruns", "audio_callbacks", "audio_callback_gap_max_ms", "audio_callback_period_ms",
             "analysis_ring_overruns")
N4K, N1080 = 32, 16


def ok(msg):
    global PASS; PASS += 1; print(f"PASS  {msg}", flush=True)


def no(msg):
    global FAIL; FAIL += 1; print(f"FAIL  {msg}", flush=True)


def info(msg):
    print(f"      {msg}", flush=True)


def la():
    return "load avg %.2f %.2f %.2f" % os.getloadavg()


# ---------------------------------------------------------------- fixtures (written before the app starts)
def mpath(*name):
    return os.path.join(MEDIA, *name)


def write_png(path, arr):
    Image.fromarray(arr, "RGBA").save(path, compress_level=1)


def noisy(w, h, k, base_b=128):
    rng = np.random.default_rng(929 + k)
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


def clone(src, dst):
    # an APFS clone (cp -c): a distinct path, no copy of the data
    r = subprocess.run(["cp", "-c", mpath(src), mpath(dst)], capture_output=True, text=True)
    if r.returncode != 0:
        shutil.copyfile(mpath(src), mpath(dst))


def v4k(k):
    return mpath(f"v4k_{k:02d}.mp4")


def v1080(k):
    return mpath(f"v1080_{k:02d}.mp4")


def seq_files(name, n):
    return [os.path.join(mpath(name), f"f{j:03d}.png") for j in range(n)]


def make_seq300(name, k, rgb):
    os.makedirs(mpath(name), exist_ok=True)
    files = seq_files(name, int(FIX["seqFrames"]))
    write_png(files[0], noisy(1920, 1080, 40 + k))
    write_png(files[1], flat(1920, 1080, rgb))
    for f in files[2:]:
        shutil.copyfile(files[1], f)


def make_fixtures():
    os.makedirs(MEDIA, exist_ok=True)
    t0 = time.time()
    make_video("v4k.mp4", "v4k")
    make_video("v1080.mp4", "v1080")
    for k in range(N4K):
        clone("v4k.mp4", f"v4k_{k:02d}.mp4")
    for k in range(N1080):
        clone("v1080.mp4", f"v1080_{k:02d}.mp4")
    for k in range(4):
        clone("v4k.mp4", f"deckA_{k}.mp4")
    clone("v1080.mp4", "w_play.mp4")
    for name, rgb in (("base.png", (30, 60, 200)), ("top.png", (220, 220, 60)), ("w3.png", (30, 90, 30))):
        write_png(mpath(name), flat(256, 144, rgb))
    make_seq300("seqA", 0, (40, 200, 90))
    make_seq300("seqB", 1, (200, 40, 170))
    shutil.copyfile(os.path.join(ROOT, "tests", "fixtures", "video_h264_cut_header.mp4"), mpath("cut_header.mp4"))
    L = LID["a6_append_deck"] * 10 + 5
    deck16 = {"name": "deck16", "id": 0, "numColumns": 16,
              "layers": [layer(L, [vclip(k + 1, v4k(k)) for k in range(16)])]}
    json.dump(deck16, open(mpath("deck16.json"), "w"), indent=1)
    print(f"fixtures written to {MEDIA} in {time.time() - t0:.1f} s", flush=True)


# ---------------------------------------------------------------- REST helpers
def iclip(cid, path):
    return {"name": f"c{cid}", "id": cid, "mediaType": 1, "mediaFile": path, "effects": []}


def vclip(cid, path):
    return {"name": f"v{cid}", "id": cid, "mediaType": 2, "mediaFile": path, "transportMode": 0, "loopMode": 0,
            "speed": 1.0, "effects": []}


def sclip(cid, paths, fps):
    return {"name": f"s{cid}", "id": cid, "mediaType": 5, "mediaFile": "", "sequenceFiles": paths,
            "sequenceFps": float(fps), "speed": 1.0, "transportMode": 0, "loopMode": 0, "effects": []}


def layer(lid, clips):
    return {"name": f"L{lid}", "id": lid, "opacity": 1.0, "visible": True, "blendMode": 0, "type": 0,
            "transitionSpeed": 0.0, "layerEffects": [], "clips": clips}


def comp_file(tag, layers, ncols=1, deck_name="D0"):
    comp = {"name": tag, "activeDeckIndex": 0, "masterOpacity": 1.0, "globalTransitionSpeed": 0.0,
            "outputWidth": CW, "outputHeight": CH,
            "decks": [{"name": deck_name, "id": 0, "numColumns": ncols, "layers": layers}]}
    path = os.path.join(OUT, f"{tag}.json")
    json.dump(comp, open(path, "w"), indent=1)
    return path


def post_load(path, timeout=75):
    """POST /api/load_composition; returns (ok, reason, status, t_post, t_answer)."""
    t_post = time.time()
    try:
        r = S.post(A + "/api/load_composition", json={"path": path}, timeout=timeout)
        t_ans = time.time()
        body = r.json() if r.headers.get("content-type", "").startswith("application/json") else {}
        return body.get("ok") is True, body.get("reason"), r.status_code, t_post, t_ans
    except Exception as e:  # noqa: BLE001
        return False, str(e), None, t_post, time.time()


def load(tag, layers, ncols=1, deck_name="D0", wait_lid=None):
    """A synchronous setup load: waits for the answer (after the swap on the async app) and, on an app that answers
    at once, for the composition to show (wait_lid)."""
    good, reason, _, _, _ = post_load(comp_file(tag, layers, ncols, deck_name))
    if not good:
        no(f"{tag}: load_composition rejected: {reason}")
        return False
    if wait_lid is not None and wait_layer_id(0, wait_lid, 15.0) is None:
        no(f"{tag}: the composition never showed (layer id {wait_lid})")
        return False
    return True


class AsyncLoad:
    """POST /api/load_composition on a thread: t_post / t_answer / ok / reason."""
    def __init__(self, tag, layers, ncols=1, deck_name="D0"):
        self.tag = tag; self.path = comp_file(tag, layers, ncols, deck_name)
        self.good = None; self.reason = None; self.status = None; self.t_post = None; self.t_answer = None

    def _run(self):
        self.good, self.reason, self.status, self.t_post, self.t_answer = post_load(self.path)

    def start(self):
        self.t_post = time.time()
        self._th = threading.Thread(target=self._run, daemon=True); self._th.start()
        return self

    def join(self, limit):
        self._th.join(limit)
        return self.t_answer is not None

    def wall(self):
        return None if self.t_answer is None else self.t_answer - self.t_post


def trig(li, col):
    try:
        S.post(A + "/api/trigger_clip", json={"layer": li, "column": col}, timeout=6)
    except Exception:  # noqa: BLE001
        pass


def post(route, body=None, timeout=6):
    try:
        r = S.post(A + route, json=body or {}, timeout=timeout)
        return r.status_code
    except Exception as e:  # noqa: BLE001
        return str(e)


def state():
    try:
        return S.get(A + "/api/state", timeout=6).json()
    except Exception as e:  # noqa: BLE001
        no(f"/api/state: {e}")
        return {}


def lstate(s):
    v = s.get("load")
    return v if isinstance(v, dict) else {}


def ui_text():
    try:
        r = S.get(A + "/api/debug/ui_text", timeout=6)
        if r.status_code != 200:
            return None, r.status_code
        b = r.json()
        return (b.get("file_label") if b.get("ok") else None), b.get("reason", 200)
    except Exception as e:  # noqa: BLE001
        return None, str(e)


def comp_state():
    try:
        return S.get(A + "/api/composition", timeout=6).json()
    except Exception:  # noqa: BLE001
        return None


def layer_json(li, deck=0, c=None):
    c = c if c is not None else comp_state()
    try:
        return c["decks"][deck]["layers"][li]
    except Exception:  # noqa: BLE001
        return None


def clips_of(L):
    return sorted((L or {}).get("clips", []), key=lambda c: c.get("column", 0))


def cell_json(li, col, deck=0, c=None):
    for cl in clips_of(layer_json(li, deck, c)):
        if cl.get("column") == col:
            return cl
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


def num_decks():
    return (comp_state() or {}).get("numDecks")


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


def box(a, b=16):
    h, w = a.shape[0] // b * b, a.shape[1] // b * b
    return a[:h, :w, :3].reshape(h // b, b, w // b, b, 3).mean(axis=(1, 3))


def dbox(x, y):
    if x.shape[:2] != y.shape[:2]:
        y = np.asarray(Image.fromarray(y.astype(np.uint8), "RGBA").resize((x.shape[1], x.shape[0]), Image.BOX)).astype(np.float32)
    return float(np.abs(box(x) - box(y)).mean())


def wait_no_compiler(tag, limit_s=1800):
    t0 = time.time()
    while True:
        busy = [n for n in ("clang", r"clang\+\+") if subprocess.run(["pgrep", "-x", n], capture_output=True).stdout.strip()]
        if not busy:
            return True
        if time.time() - t0 > limit_s:
            no(f"{tag}: a compiler ({busy}) kept running for {limit_s} s -- perf not measured"); return False
        info(f"{tag}: waiting for {busy} to finish ({la()})")
        time.sleep(20)


def hb_on():
    try:
        r = S.post(A + "/api/debug/heartbeat", json={"on": True, "period_ms": int(FIX["heartbeatPeriodMs"])}, timeout=6)
        return r.status_code, (r.json() if r.status_code == 200 else None)
    except Exception as e:  # noqa: BLE001
        return None, str(e)


def unc_windows():
    try:
        import Quartz  # noqa: WPS433
        wl = Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionOnScreenOnly, Quartz.kCGNullWindowID)
        return len([w for w in wl if "UserNotificationCenter" in str(w.get("kCGWindowOwnerName", ""))])
    except Exception:  # noqa: BLE001
        return None


class Poller:
    """Reads /api/state every pollMs in a thread (and /api/composition every 4th poll); peak_message_stall_ms and the
    audio gap reset on read, so the max over reads is the max over the window."""
    def __init__(self):
        self.rows = []; self.comps = []; self._run = False

    def _loop(self):
        s = requests.Session(); s.headers["Connection"] = "close"
        n = 0
        while self._run:
            try:
                d = s.get(A + "/api/state", timeout=6).json()
                ld = d.get("load") if isinstance(d.get("load"), dict) else {}
                self.rows.append({"t": time.time(), "stall": d.get("peak_message_stall_ms"),
                                  "uploads": d.get("video_uploads"), "late": d.get("video_late_frames"),
                                  "pending_frames": d.get("video_pending_frames"), "pending_now": d.get("videos_pending"),
                                  "gap": ld.get("audio_callback_gap_max_ms")})
                n += 1
                if n % 4 == 0:
                    c = s.get(A + "/api/composition", timeout=6).json()
                    L = layer_json(0, 0, c) or {}
                    col = L.get("activeClipColumn")
                    pos = None
                    for cl in L.get("clips", []):
                        if cl.get("column") == col:
                            pos = cl.get("playheadPosition")
                    self.comps.append({"t": time.time(), "lid": L.get("id"), "pos": pos})
            except Exception:  # noqa: BLE001
                pass
            time.sleep(float(FIX["pollMs"]) / 1000.0)

    def start(self):
        self.rows = []; self.comps = []; self._run = True
        self._th = threading.Thread(target=self._loop, daemon=True); self._th.start()
        return self

    def stop(self):
        self._run = False; self._th.join()
        return self

    def max(self, key):
        v = [r[key] for r in self.rows if r.get(key) is not None]
        return max(v) if v else None

    def at(self, t, key, after=True):
        rows = [r for r in self.rows if r.get(key) is not None and ((r["t"] >= t) if after else (r["t"] <= t))]
        if not rows:
            return None
        return rows[0][key] if after else rows[-1][key]


def fence_deltas(tag, s0, s1):
    h0, b0, h1, b1 = s0.get("fence_hold_frames"), s0.get("fence_black_frames"), s1.get("fence_hold_frames"), s1.get("fence_black_frames")
    if None in (h0, b0, h1, b1):
        no(f"{tag}: /api/state has no fence_hold_frames / fence_black_frames")
        return None, None
    return h1 - h0, b1 - b0


def stall_max(tag, pol, s1):
    vals = [v for v in (pol.max("stall") if pol else None, s1.get("peak_message_stall_ms")) if v is not None]
    if not vals:
        no(f"{tag}: no peak_message_stall_ms (the app predates the heartbeat)")
        return None
    return max(vals)


def delta(s0, s1, key, sub=None):
    a = (lstate(s0) if sub else s0).get(key); b = (lstate(s1) if sub else s1).get(key)
    return None if a is None or b is None else b - a


def audio_checks(tag, s0, s1, pol):
    l1 = lstate(s1)
    dx = delta(s0, s1, "audio_xruns", True)
    dov = delta(s0, s1, "analysis_ring_overruns", True)
    gaps = [g for g in ((pol.max("gap") if pol else None), l1.get("audio_callback_gap_max_ms")) if g is not None]
    gap = max(gaps) if gaps else None
    period = l1.get("audio_callback_period_ms")
    info(f"{tag}: audio_xruns {lstate(s0).get('audio_xruns')} -> {l1.get('audio_xruns')}, ring overruns +{dov}, "
         f"callback gap max {gap} ms, period {period} ms, callbacks {l1.get('audio_callbacks')}")
    (ok if dx == 0 and l1.get("audio_xruns", -1) >= 0 else no)(f"{tag}: (g) CoreAudio overloads +{dx} == 0")
    (ok if dov == 0 else no)(f"{tag}: (g) analysis ring overruns +{dov} == 0")
    lim = None if period in (None, 0) else float(FIX["audioGapFactor"]) * float(period)
    (ok if gap is not None and lim is not None and gap <= lim else no)(
        f"{tag}: (g) audio callback gap max {gap} ms <= {FIX['audioGapFactor']} x period ({lim})")


def comp_layer_ids(c):
    try:
        return [L.get("id") for L in c["decks"][c.get("activeDeck", 0)]["layers"]]
    except Exception:  # noqa: BLE001
        return []


# ---------------------------------------------------------------- rows
def a1(tag):
    code_, body = hb_on()
    (ok if code_ == 200 else no)(f"{tag}: POST /api/debug/heartbeat 200 (got {code_})")
    time.sleep(0.5)
    s = state()
    ld = s.get("load")
    if not isinstance(ld, dict):
        no(f"{tag}: /api/state has no 'load' object (got {str(ld)[:60]})")
    else:
        missing = [k for k in LOAD_KEYS if k not in ld]
        (ok if not missing else no)(f"{tag}: state.load has every witness field (missing {missing})")
        tm = ld.get("timing") if isinstance(ld.get("timing"), dict) else {}
        info(f"{tag}: timing keys {sorted(tm.keys())}; period {ld.get('audio_callback_period_ms')} ms; "
             f"xruns {ld.get('audio_xruns')}; callbacks {ld.get('audio_callbacks')}")
        (ok if isinstance(ld.get("audio_xruns"), int) and ld.get("audio_xruns") >= 0 else no)(
            f"{tag}: audio_xruns >= 0 (the device reports its overloads; got {ld.get('audio_xruns')})")
        gaps = []
        for _ in range(5):
            time.sleep(0.2)
            gaps.append(lstate(state()).get("audio_callback_gap_max_ms"))
        info(f"{tag}: 5 idle callback-gap readings (ms): {gaps}")
    t, why = ui_text()
    (ok if t is not None else no)(f"{tag}: GET /api/debug/ui_text answers the file label (got {t!r} / {why})")


def a2(tag, width, vid, ref):
    L = LID[tag]; lw, lv = L * 10 + 1, L * 10 + 2
    if not load(tag + "_W", [layer(lw, [vclip(1, mpath("w_play.mp4"))])], wait_lid=lw):
        return
    trig(0, 0); wait_active(0, 0); time.sleep(1.0)
    if not wait_no_compiler(tag):
        return
    hb_on(); time.sleep(0.3)
    s0 = state()
    pol = Poller().start()
    time.sleep(0.1)
    j = AsyncLoad(tag + "_V16", [layer(lv, [vclip(k + 1, vid(k)) for k in range(int(FIX["w16"]))])], int(FIX["w16"])).start()
    j.join(SWAP + 1.0)
    c_at_answer = comp_state() if j.t_answer is not None else None
    j.join(75)
    shown = wait_layer_id(0, lv, 15.0)
    time.sleep(0.3)
    pol.stop(); s1 = state()
    w = j.wall()
    info(f"{tag}: answer {j.good} {j.reason!r} after {w if w is None else round(w, 3)} s; V16 on screen "
         f"{shown is not None}; {la()}")
    st = stall_max(tag, pol, s1)
    info(f"{tag}: max message stall {st} ms")
    if st is not None:
        (ok if st <= STALL else no)(f"{tag}: (a) longest message-thread stall {st:.1f} ms <= {STALL}")
    L_at = layer_json(0, 0, c_at_answer) if c_at_answer else None
    after_cut = L_at is not None and L_at.get("id") == lv and len(L_at.get("clips", [])) == int(FIX["w16"])
    (ok if j.good and w is not None and w <= SWAP and after_cut else no)(
        f"{tag}: (b) ok:true within {SWAP} s ({w}) AND the composition read right after the answer is V16 "
        f"(layer id {None if L_at is None else L_at.get('id')} == {lv}, {len((L_at or {}).get('clips', []))} clips)")
    dh, db = fence_deltas(tag, s0, s1)
    if dh is not None:
        (ok if db == 0 else no)(f"{tag}: (c) no deck-less black frame (fence_black +{db} == 0)")
        (ok if dh <= HOLD else no)(f"{tag}: (c) the cut's fence holds <= {HOLD} frames (+{dh})")
    if w is not None and w >= 0.3:
        u0 = pol.at(j.t_post, "uploads"); u1 = pol.at(j.t_answer, "uploads", after=False)
        du = None if u0 is None or u1 is None else u1 - u0
        poss = [r["pos"] for r in pol.comps if j.t_post <= r["t"] <= j.t_answer and r["lid"] == lw and r["pos"] is not None]
        need = float(FIX["uploadsMinPerS"]) * w
        info(f"{tag}: window {w:.3f} s: old clip uploads +{du} (need >= {need:.1f}); old playheads {poss[:3]}..{poss[-2:]}")
        (ok if du is not None and du >= need else no)(f"{tag}: (d) the OLD show kept uploading frames (+{du} >= {need:.1f})")
        (ok if len(poss) >= 2 and len(set(poss)) >= 2 else no)(f"{tag}: (d) the OLD clip's playhead moved in the window ({len(poss)} reads)")
    else:
        info(f"{tag}: (d) window {w} s < 0.3 s: the old show's animation is not measurable (the answer came at once)")
    c = comp_state()
    cl = clips_of(layer_json(0, 0, c))
    good_e = len(cl) == int(FIX["w16"]) and all(x.get("clipWidth") == width and (x.get("thumbnailW") or 0) > 0 for x in cl)
    (ok if good_e else no)(f"{tag}: (e) every V16 clip has clipWidth {width} and a thumbnail "
                           f"({[(x.get('clipWidth'), x.get('thumbnailW')) for x in cl][:3]}...)")
    sp = state()
    trig(0, 0); wait_active(0, 0); time.sleep(0.4)
    x = cap(tag)
    sq = state()
    dp = delta(sp, sq, "video_pending_frames")
    (ok if dp == 0 else no)(f"{tag}: (f) the first trigger after the cut draws at once (video_pending_frames +{dp} == 0)")
    if x is not None:
        d = dbox(x, img(ref))
        (ok if d <= TOL else no)(f"{tag}: (f) the triggered clip is on screen (dbox {d:.2f} <= {TOL})")
    audio_checks(tag, s0, s1, pol)
    dl = delta(s0, s1, "video_late_frames")
    (ok if dl is not None and dl <= int(FIX["lateMaxPerLoad"]) else no)(
        f"{tag}: (h) the old clip's late frames +{dl} <= {FIX['lateMaxPerLoad']}")
    tm = lstate(s1).get("timing") if isinstance(lstate(s1).get("timing"), dict) else None
    if tm:
        r = (tm.get("opens_ms", 0) / tm["total_ms"]) if tm.get("total_ms") else None
        info(f"{tag}: (i) timing {json.dumps(tm)}; opens_ms / total_ms = {None if r is None else round(r, 3)}")
    else:
        info(f"{tag}: (i) no load.timing")


def image_comp(tag, lid, top=True):
    clips = [iclip(1, mpath("base.png"))] + ([iclip(2, mpath("top.png"))] if top else [])
    return [layer(lid, clips)], len(clips)


def a3(tag):
    L = LID[tag]; lw2, lv, lw3 = L * 10 + 1, L * 10 + 2, L * 10 + 3
    lay, n = image_comp(tag, lw2)
    if not load(tag + "_W2", lay, n, wait_lid=lw2):
        return
    trig(0, 0); wait_active(0, 0); time.sleep(0.5)
    hb_on(); time.sleep(0.3)
    s0 = state()
    pol = Poller().start()
    j1 = AsyncLoad(tag + "_V32", [layer(lv, [vclip(k + 1, v4k(k)) for k in range(int(FIX["w32"]))])], int(FIX["w32"])).start()
    time.sleep(float(FIX["cancelDelayS"]))
    j2 = AsyncLoad(tag + "_W3", [layer(lw3, [iclip(1, mpath("w3.png"))])], 1).start()
    j1.join(10); j2.join(SWAP + 1.0)
    c = comp_state() if j2.t_answer is not None else None
    j2.join(75)
    wait_layer_id(0, lw3, 15.0)
    ut, why = ui_text()
    time.sleep(1.0)
    pol.stop(); s1 = state()
    dt = None if j1.t_answer is None else j1.t_answer - j2.t_post
    info(f"{tag}: V32 {j1.good} {j1.reason!r} ({dt if dt is None else round(dt, 3)} s after W3's post); "
         f"W3 {j2.good} {j2.reason!r} after {j2.wall()} s; label {ut!r}")
    (ok if j1.good is False and j1.reason == "superseded by a newer load" and dt is not None and dt <= CANCEL else no)(
        f"{tag}: (a) V32 answers 'superseded by a newer load' within {CANCEL} s of W3's post (got {j1.good} {j1.reason!r}, {dt})")
    on = (layer_json(0, 0, c) or {}).get("id") == lw3 if c else False
    (ok if j2.good and j2.wall() is not None and j2.wall() <= SWAP and on else no)(
        f"{tag}: (b) W3 ok:true within {SWAP} s ({j2.wall()}) and on screen when it answered ({on})")
    want = "Loaded: " + tag + "_W3"
    (ok if ut == want else no)(f"{tag}: (c) the label reads {want!r} (got {ut!r} / {why})")
    db_ = delta(s0, s1, "open_batches", True)
    info(f"{tag}: open_batches +{db_}, opens_stale +{delta(s0, s1, 'opens_stale', True)}, "
         f"opens_dropped +{delta(s0, s1, 'opens_dropped', True)}")
    (ok if db_ == 2 else no)(f"{tag}: (d) two batches began (open_batches +{db_} == 2)")
    vp = s1.get("video_players")
    (ok if vp == 0 else no)(f"{tag}: (e) the cancelled batch's players are retired (video_players {vp} == 0)")
    st = stall_max(tag, pol, s1)
    if st is not None:
        (ok if st <= STALL else no)(f"{tag}: (f) longest message-thread stall {st:.1f} ms <= {STALL}")


def cancel_row(tag, with_seq):
    L = LID[tag]; lw2, lv = L * 10 + 1, L * 10 + 2
    lay, n = image_comp(tag, lw2)
    if not load(tag + "_W2", lay, n, wait_lid=lw2):
        return
    trig(0, 0); wait_active(0, 0); time.sleep(0.5)
    hb_on(); time.sleep(0.3)
    s0 = state()
    layers = [layer(lv, [vclip(k + 1, v4k(k)) for k in range(int(FIX["w32"]))])]
    if with_seq:
        nf = int(FIX["seqFrames"])
        layers.append(layer(lv + 1, [sclip(101, seq_files("seqA", nf), 30), sclip(102, seq_files("seqB", nf), 30)]))
    j = AsyncLoad(tag + "_V32", layers, int(FIX["w32"])).start()
    time.sleep(float(FIX["cancelDelayS"]))
    t_c = time.time()
    code_ = post("/api/debug/cancel_load")
    j.join(10)
    time.sleep(1.0)
    ut, why = ui_text()
    s1 = state()
    c = comp_state()
    dt = None if j.t_answer is None else j.t_answer - t_c
    info(f"{tag}: cancel_load {code_}; V32 {j.good} {j.reason!r} ({dt if dt is None else round(dt, 3)} s after the "
         f"cancel); label {ut!r}; staged_players {lstate(s1).get('staged_players')}")
    (ok if j.good is False and j.reason == "superseded by a newer load" and dt is not None and dt <= CANCEL else no)(
        f"{tag}: (a) V32 answers 'superseded by a newer load' within {CANCEL} s of the cancel (got {j.good} {j.reason!r}, {dt})")
    lid = (layer_json(0, 0, c) or {}).get("id")
    (ok if lid == lw2 else no)(f"{tag}: (b) nothing swapped: W2 still on screen (layer id {lid} == {lw2})")
    want = "Loaded: " + tag + "_W2"
    (ok if ut == want else no)(f"{tag}: (c) the label is restored to {want!r} (got {ut!r} / {why})")
    vp = s1.get("video_players")
    (ok if vp == 0 else no)(f"{tag}: (d) the cancelled batch's players are retired (video_players {vp} == 0)")
    dh, db = fence_deltas(tag, s0, s1)
    if db is not None:
        (ok if db == 0 else no)(f"{tag}: (e) no deck-less black frame (fence_black +{db} == 0)")
    if with_seq:
        (ok if s1.get("seq_open") == s0.get("seq_open") else no)(
            f"{tag}: (f) no sequence left open by the cancelled batch (seq_open {s0.get('seq_open')} -> {s1.get('seq_open')})")
    j.join(75)
    if j.good:   # a synchronous app loaded V32 after all: wait for it before the next row
        wait_layer_id(0, lv, 15.0)


def a4(tag):
    L = LID[tag]; lw2, lv, lw2b, lvb = L * 10 + 1, L * 10 + 2, L * 10 + 3, L * 10 + 4
    lay, n = image_comp(tag, lw2)
    if not load(tag + "_W2", lay, n, wait_lid=lw2):
        return
    trig(0, 0); wait_active(0, 0); time.sleep(1.0)
    s0 = state()
    j = AsyncLoad(tag + "_V32", [layer(lv, [vclip(k + 1, v4k(k)) for k in range(int(FIX["w32"]))])], int(FIX["w32"])).start()
    time.sleep(float(FIX["triggerDelayS"]))
    t_tp = time.time(); trig(0, 1); t_ta = time.time()
    tb, _ = wait_until(lambda: ((layer_json(0) or {}).get("id") == lw2 and (layer_json(0) or {}).get("activeClipColumn") == 1), 0.5, 0.01)
    t_seen = None if tb is None else t_ta + tb
    ui1, _ = ui_text()
    j.join(75)
    wait_layer_id(0, lv, 15.0)
    time.sleep(0.3)
    c = comp_state(); s1 = state()
    w = j.wall()
    info(f"{tag}: V32 {j.good} {j.reason!r} after {w} s; trigger answered {t_ta - j.t_post:.3f} s after the post; "
         f"W2 col 1 active seen {t_seen if t_seen is None else round(t_seen - j.t_post, 3)} s; label then {ui1!r}")
    if w is None or w <= float(FIX["triggerDelayS"]):
        no(f"{tag}: window too short to test (the load answered after {w} s <= triggerDelayS {FIX['triggerDelayS']})")
    else:
        (ok if t_ta < j.t_answer else no)(f"{tag}: (a) the trigger answered before the load")
        (ok if t_seen is not None and t_seen < j.t_answer else no)(
            f"{tag}: (b) the OLD show answered the trigger while the load was staged (W2 layer 0 col 1 active)")
    lays = ((c or {}).get("decks") or [{}])[0].get("layers", [])
    acts = [L_.get("activeClipColumn") for L_ in lays]
    (ok if (layer_json(0, 0, c) or {}).get("id") == lv and all(a == -1 for a in acts) else no)(
        f"{tag}: (c) V32 on screen with no active clip (the trigger did not leak into it; activeClipColumn {acts})")
    dh, db = fence_deltas(tag, s0, s1)
    if db is not None:
        (ok if db == 0 else no)(f"{tag}: (d) no deck-less black frame (fence_black +{db} == 0)")
    trig(0, 0); wait_active(0, 0); time.sleep(0.4)
    x = cap(tag)
    if x is not None:
        d = dbox(x, img(mpath("v4k_ref.png")))
        (ok if d <= TOL else no)(f"{tag}: (e) the new show works: the triggered 4K clip is on screen (dbox {d:.2f} <= {TOL})")
    want1 = "Loading " + tag + "_V32..."
    (ok if ui1 == want1 else no)(f"{tag}: (f1) the label keeps {want1!r} through the trigger (got {ui1!r})")
    # (f2) AL5: a cancel after a trigger restores the TRIGGER's text
    lay, n = image_comp(tag + "b", lw2b)
    if not load(tag + "_W2b", lay, n, wait_lid=lw2b):
        return
    trig(0, 0); wait_active(0, 0); time.sleep(0.5)
    j2 = AsyncLoad(tag + "_V32b", [layer(lvb, [vclip(k + 1, v4k(k)) for k in range(int(FIX["w32"]))])], int(FIX["w32"])).start()
    time.sleep(float(FIX["triggerDelayS"]))
    trig(0, 1); time.sleep(0.05)
    ui_mid, _ = ui_text()
    post("/api/debug/cancel_load")
    j2.join(10); time.sleep(0.3)
    ui_end, _ = ui_text()
    want_end = os.path.basename(mpath("top.png"))
    info(f"{tag}: (f2) V32b {j2.good} {j2.reason!r}; label mid {ui_mid!r}, after the cancel {ui_end!r}")
    (ok if ui_mid == "Loading " + tag + "_V32b..." and ui_end == want_end else no)(
        f"{tag}: (f2) after a trigger + cancel the label shows the trigger's text {want_end!r} (mid {ui_mid!r}, end {ui_end!r})")
    j2.join(75)
    if j2.good:
        wait_layer_id(0, lvb, 15.0)


def deck_a(tag, lid):
    return [layer(lid, [vclip(k + 1, mpath(f"deckA_{k}.mp4")) for k in range(4)])]


def a5(tag):
    L = LID[tag]; lw = L * 10 + 1
    if not load(tag + "_D", deck_a(tag, lw), 4, deck_name="A", wait_lid=lw):
        return
    trig(0, 0); wait_active(0, 0); time.sleep(1.0)
    if not wait_no_compiler(tag):
        return
    hb_on(); time.sleep(0.3)
    s0 = state()
    pol = Poller().start()
    time.sleep(0.1)
    t0 = time.time()
    code_ = post("/api/debug/duplicate_deck", {"deck": 0})
    time.sleep(0.05)
    ui_mid, _ = ui_text()
    t, _ = wait_until(lambda: num_decks() == 2, SWAP + 5.0)
    t_dup = None if t is None else time.time() - t0
    time.sleep(0.3)
    ui_end, _ = ui_text()
    pol.stop(); s1 = state()
    c = comp_state()
    x = cap(tag)
    info(f"{tag}: duplicate_deck {code_}; 2 decks after {t_dup} s; label mid {ui_mid!r}, end {ui_end!r}; {la()}")
    (ok if ui_mid == "Loading A copy..." and ui_end == "Duplicated deck: A copy" else no)(
        f"{tag}: (a) the label reads 'Loading A copy...' then 'Duplicated deck: A copy' (got {ui_mid!r} -> {ui_end!r})")
    (ok if (c or {}).get("activeDeck") == 1 and t_dup is not None and t_dup <= SWAP else no)(
        f"{tag}: (b) the copy is the active deck within {SWAP} s (activeDeck {(c or {}).get('activeDeck')}, {t_dup} s)")
    dp = delta(s0, s1, "video_pending_frames"); pn = pol.max("pending_now")
    dh, db = fence_deltas(tag, s0, s1)
    (ok if dp == 0 and (pn or 0) == 0 else no)(
        f"{tag}: (c) the copy's frame 0 is ready at the cut (video_pending_frames +{dp}, videos_pending max {pn})")
    if dh is not None:
        (ok if db == 0 and dh <= HOLD else no)(f"{tag}: (c) fence black +{db} == 0, hold +{dh} <= {HOLD}")
    if x is not None:
        d = dbox(x, img(mpath("v4k_ref.png")))
        (ok if d <= TOL else no)(f"{tag}: (d) the copy's active clip is on screen (dbox {d:.2f} <= {TOL})")
    st = stall_max(tag, pol, s1)
    if st is not None:
        (ok if st <= STALL else no)(f"{tag}: (e) longest message-thread stall {st:.1f} ms <= {STALL}")
    vp = s1.get("video_players")
    (ok if vp == 8 else no)(f"{tag}: (f) the copy has its own players (video_players {vp} == 8)")


def a5b(tag):
    L = LID[tag]; lw = L * 10 + 1
    if not load(tag + "_D", deck_a(tag, lw), 4, deck_name="A", wait_lid=lw):
        return
    trig(0, 0); wait_active(0, 0); time.sleep(0.5)
    c1 = post("/api/debug/duplicate_deck", {"deck": 0})
    time.sleep(0.05)
    c2 = post("/api/debug/duplicate_deck", {"deck": 0})
    t, _ = wait_until(lambda: num_decks() == 3, 2 * SWAP + 5.0)
    time.sleep(0.5)
    c = comp_state(); s1 = state(); ut, _ = ui_text()
    names = [d.get("name") for d in (c or {}).get("decks", [])]
    info(f"{tag}: posts {c1}/{c2}; 3 decks after {t} s; names {names}; activeDeck {(c or {}).get('activeDeck')}; "
         f"video_players {s1.get('video_players')}; label {ut!r}")
    (ok if names == ["A", "A copy", "A copy"] else no)(f"{tag}: two Duplicate clicks = two copies, in order (names {names})")
    (ok if (c or {}).get("activeDeck") == 2 else no)(f"{tag}: the second copy is active ({(c or {}).get('activeDeck')} == 2)")
    (ok if s1.get("video_players") == 12 else no)(f"{tag}: each copy has its own players (video_players {s1.get('video_players')} == 12)")
    (ok if ut == "Duplicated deck: A copy" else no)(f"{tag}: the label reads 'Duplicated deck: A copy' (got {ut!r})")


def a6(tag):
    L = LID[tag]; lw2 = L * 10 + 1
    lay, n = image_comp(tag, lw2)
    if not load(tag + "_W2", lay, n, wait_lid=lw2):
        return
    trig(0, 0); wait_active(0, 0); time.sleep(0.5)
    if not wait_no_compiler(tag):
        return
    hb_on(); time.sleep(0.3)
    s0 = state()
    pol = Poller().start()
    time.sleep(0.1)
    t0 = time.time()
    code_ = post("/api/debug/load_deck", {"path": mpath("deck16.json")})
    time.sleep(0.05)
    ui_mid, _ = ui_text()
    t, _ = wait_until(lambda: num_decks() == 2, SWAP + 5.0)
    t_app = None if t is None else time.time() - t0
    time.sleep(0.3)
    ui_end, _ = ui_text()
    pol.stop(); s1 = state()
    c = comp_state()
    info(f"{tag}: load_deck {code_}; 2 decks after {t_app} s; label mid {ui_mid!r}, end {ui_end!r}; {la()}")
    st = stall_max(tag, pol, s1)
    if st is not None:
        (ok if st <= STALL else no)(f"{tag}: (a) longest message-thread stall {st:.1f} ms <= {STALL}")
    (ok if t_app is not None and t_app <= SWAP and (c or {}).get("activeDeck") == 1 and ui_end == "Loaded deck: deck16" else no)(
        f"{tag}: (b) 2 decks within {SWAP} s ({t_app}), activeDeck 1 ({(c or {}).get('activeDeck')}), label 'Loaded deck: deck16' ({ui_end!r})")
    cl = clips_of(layer_json(0, 1, c))
    (ok if len(cl) == 16 and all(x.get("clipWidth") == 3840 for x in cl) else no)(
        f"{tag}: (c) the appended deck's 16 clips have clipWidth 3840 ({len(cl)} clips)")
    dh, db = fence_deltas(tag, s0, s1)
    if db is not None:
        (ok if db == 0 else no)(f"{tag}: (d) no deck-less black frame (fence_black +{db} == 0)")
    audio_checks(tag, s0, s1, pol)


def a7(tag):
    L = LID[tag]; lv = L * 10 + 2
    gone = mpath("a7_gone.mp4")
    subprocess.run(["cp", "-c", mpath("v4k.mp4"), gone])
    files = [v4k(k) for k in range(16)]
    files[7] = mpath("cut_header.mp4"); files[9] = gone
    os.remove(gone)
    s0 = state()
    j = AsyncLoad(tag + "_V16F", [layer(lv, [vclip(k + 1, f) for k, f in enumerate(files)])], 16).start()
    j.join(75)
    wait_layer_id(0, lv, 15.0); time.sleep(0.3)
    s1 = state(); c = comp_state()
    cl = clips_of(layer_json(0, 0, c))
    info(f"{tag}: answer {j.good} {j.reason!r} after {j.wall()} s; cells {[(x.get('column'), x.get('clipWidth'), x.get('thumbnailW'), x.get('mediaMissing')) for x in cl]}")
    (ok if j.good and j.wall() is not None and j.wall() <= SWAP else no)(f"{tag}: (a) ok:true within {SWAP} s ({j.wall()})")
    by = {x.get("column"): x for x in cl}
    c7, c9 = by.get(7, {}), by.get(9, {})
    others = [by.get(k, {}) for k in range(16) if k not in (7, 9)]
    good_b = (c7.get("clipWidth") == 1920 and (c7.get("thumbnailW") or 0) == 0 and c9.get("mediaMissing") is True
              and len(others) == 14 and all(o.get("clipWidth") == 3840 for o in others))
    (ok if good_b else no)(f"{tag}: (b) the failed open keeps the file defaults, the missing file is missing, 14 opened")
    df = delta(s0, s1, "opens_failed", True)
    (ok if df == 1 else no)(f"{tag}: (c) one failed open counted (opens_failed +{df} == 1)")
    trig(0, 0); wait_active(0, 0); time.sleep(0.4)
    x = cap(tag)
    if x is not None:
        d = dbox(x, img(mpath("v4k_ref.png")))
        (ok if d <= TOL else no)(f"{tag}: (d) the batch still loaded: the 4K clip is on screen (dbox {d:.2f} <= {TOL})")
    u = unc_windows()
    (ok if u == 0 else no)(f"{tag}: (e) no crash dialog on screen (UserNotificationCenter windows {u})")


def end_quit_mid_load(tag):
    L = LID[tag]; lv = L * 10 + 2
    j = AsyncLoad(tag + "_V32", [layer(lv, [vclip(k + 1, v4k(k)) for k in range(int(FIX["w32"]))])], int(FIX["w32"])).start()
    time.sleep(0.1)
    info(f"{tag}: V32 posted {time.time() - j.t_post:.2f} s ago, answered {j.t_answer is not None}: the .sh quits now")


def end_quit_hung_open(tag):
    L = LID[tag]; lv, lw3 = L * 10 + 2, L * 10 + 3
    fifo = mpath("hung.mp4")
    if not os.path.exists(fifo):
        os.mkfifo(fifo)
    j = AsyncLoad(tag + "_H", [layer(lv, [vclip(1, fifo), vclip(2, v4k(0))])], 2).start()
    time.sleep(0.3)
    j2 = AsyncLoad(tag + "_W3", [layer(lw3, [iclip(1, mpath("w3.png"))])], 1).start()
    j2.join(SWAP + 1.0)
    j.join(3.0)
    time.sleep(0.2)
    ut, why = ui_text()
    info(f"{tag}: FIFO load {j.good} {j.reason!r}; W3 {j2.good} {j2.reason!r} after {j2.wall()} s; label {ut!r} / {why}")
    (ok if j2.good and j2.wall() is not None and j2.wall() <= SWAP else no)(
        f"{tag}: a second load answers ok while an open hangs ({j2.good}, {j2.wall()} s)")
    (ok if j.good is False and j.reason == "superseded by a newer load" else no)(
        f"{tag}: the hung load answers 'superseded by a newer load' (got {j.good} {j.reason!r})")
    want = "Loaded: " + tag + "_W3"
    (ok if ut == want else no)(f"{tag}: the UI answers while an open hangs (label {ut!r} == {want!r})")
    info(f"{tag}: the .sh quits now (a pool thread is inside open(2) of a FIFO)")


def main():
    if MAKE:
        make_fixtures(); return
    s_first = state()
    table = {
        "a1_witness_fields": lambda: a1("a1_witness_fields"),
        "a2_load_16x4k": lambda: a2("a2_load_16x4k", 3840, v4k, mpath("v4k_ref.png")),
        "a2b_load_16x1080": lambda: a2("a2b_load_16x1080", 1920, v1080, mpath("v1080_ref.png")),
        "a3_cancel_by_newer_load": lambda: a3("a3_cancel_by_newer_load"),
        "a3b_explicit_cancel": lambda: cancel_row("a3b_explicit_cancel", False),
        "a3c_cancel_with_sequences": lambda: cancel_row("a3c_cancel_with_sequences", True),
        "a4_load_then_trigger": lambda: a4("a4_load_then_trigger"),
        "a5_duplicate_deck": lambda: a5("a5_duplicate_deck"),
        "a5b_duplicate_twice": lambda: a5b("a5b_duplicate_twice"),
        "a6_append_deck": lambda: a6("a6_append_deck"),
        "a7_failure_mid_batch": lambda: a7("a7_failure_mid_batch"),
    }
    order = ["a1_witness_fields", "a2_load_16x4k", "a2b_load_16x1080", "a3_cancel_by_newer_load", "a3b_explicit_cancel",
             "a3c_cancel_with_sequences", "a4_load_then_trigger", "a5_duplicate_deck", "a5b_duplicate_twice",
             "a6_append_deck", "a7_failure_mid_batch", "end_hold_witness", "end_quit_mid_load"]
    rows = ONLY if ONLY is not None else order
    for name in rows:
        print(f"--- {name}  ({time.strftime('%H:%M:%S')})", flush=True)
        if name in table:
            table[name]()
        elif name == "end_hold_witness":
            s_end = state()
            h0, h1 = s_first.get("fence_hold_frames"), s_end.get("fence_hold_frames")
            if h0 is None or h1 is None:
                no("end_hold_witness: /api/state has no fence_hold_frames")
            else:
                info(f"end_hold_witness: fence hold +{h1 - h0}, black +{s_end.get('fence_black_frames', 0) - s_first.get('fence_black_frames', 0)} over the run")
                (ok if h1 - h0 >= 1 else no)(f"end_hold_witness: the fence held the canvas at least once ({h1 - h0} frames)")
        elif name == "end_quit_mid_load":
            end_quit_mid_load(name)
        elif name == "end_quit_hung_open":
            end_quit_hung_open(name)
        else:
            no(f"unknown row {name}")
    print(f"\nPY {PASS} PASS / {FAIL} FAIL", flush=True)
    sys.exit(0 if FAIL == 0 else 1)


if __name__ == "__main__":
    main()
