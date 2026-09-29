#!/usr/bin/env python3
"""probe-seq-vram.py -- REST/pixel half of .harmony/probe-seq-vram.sh (s-rta-0928b seqvram).

Live witness that an image sequence plays through a BOUNDED window of recycled GL textures (SeqVram.h): all
sequences together hold at most kBudgetBytes (1 GiB) unless their 8-frame floors alone exceed it, playback stays
smooth (no late / pending frames on a quiet machine), and every capture shows the frame the playhead says.
Plan + Harmony adoption: .harmony/.reports/s-rta-0928b/plan-seqvram.md (section 4.5 + H7-H11, H15).

The .sh owns launch / refuse / quit; this file writes the fixtures (--make-fixtures, BEFORE the app starts) and then
talks to the running app on 7070 (production mode). Every captured PNG is decoded with PIL+numpy.

usage: probe-seq-vram.py <root> <fresh-outdir> --make-fixtures [row,row,...]
       probe-seq-vram.py <root> <fresh-outdir> [row,row,...]
rows (run order, H15 + fix round F6 / F8): v1_memory_1080 v2_smooth_loop_1080 v4_retrigger_hit v6_budget_share
      v8_crossfade_two_long v9_three_decoding v7_deck_keeps_time v7b_trim_spike v7c_one_per_deck v11_retire
      v3_smooth_pingpong_4k v10_floors_win
Fix round (plan-seqvram.md HARMONY ADOPTION ADDENDUM F1-F9): v8 gains (b') the longest render callback over the fade
(the frame-top scan runs outside the frame timer) and (f) the incoming grows >= incomingMinTextures textures
(seq_drawn_textures: the sequences drawn within kIdleFrames) within 3 s after the fade completes, then 0 late frames
for 2 s; v7c_one_per_deck = one 300-frame sequence per deck, a deck switch when deck 0's window is full: the longest
render callback over the 2 s after it (the idle trim within the per-frame delete budget); v11_retire is REPORT-ONLY
(INFO lines: the retire drain of a full window at a composition swap). Every row prints seq_upload_deferred,
seq_deletes and seq_drawn_textures in its state lines ("absent" on an app that predates them).

Fixtures (<out>/media, PIL compress_level 1): frame k of a set with code c: R = 255x/W, G = 255y/H, B = (37c) % 256,
alpha 255, plus a CODE BAND at the bottom (64 px at 1080 rows, scaled with the height): 10 cells across, cell b white
iff bit b of c. Sets: L300 = 300 x 1920x1080 (codes 0..299), M300 = 300 x 1920x1080 (codes 400..699), Q40 = 40 x
3840x2160 (codes 800..839), warm.png (256x144 flat). code(cap, rect) = threshold (> 127) the mean of each cell's inner
50 %. A sequence clip carries speed 1.0, transportMode 0, loopMode m, reverse false (Clip::fromVar reads them
unconditionally: a clip without "speed" plays at 0).
Expected frame: the playhead (/api/composition) read right BEFORE and right AFTER the capture brackets it: the code's
frame index must lie in [int(p0 * n) - tol, int(p1 * n) + tol] (mod n; PingPong: either direction).
Several layers in one capture (v9, v7b): each layer is scaled to 1/3 about anchor (0 / 0.5 / 1, 0.5) -- side by side,
the rest of each layer transparent (layer_transform clears to alpha 0); every layer above the first is Transparent
(type 1, keying Alpha), since an Opaque layer replaces everything below it.
Counters: /api/state seq_* (a missing key FAILs the row: "the app predates it"). Perf rows wait until no compiler
runs and print the load average.
"""
import json, os, subprocess, sys, threading, time
from multiprocessing import Pool

import numpy as np
import requests
from PIL import Image

A = "http://127.0.0.1:7070"
ROOT, OUT = sys.argv[1], sys.argv[2]
MAKE = len(sys.argv) > 3 and sys.argv[3] == "--make-fixtures"
_rows_arg = sys.argv[4] if MAKE else (sys.argv[3] if len(sys.argv) > 3 else "")
ONLY = set(_rows_arg.split(",")) if _rows_arg else None
FIX = json.load(open(os.path.join(ROOT, ".harmony", "probe-seq-vram.json")))
PEAK = float(FIX["peakMaxMs"]); TOL = float(FIX["boxTol"])
BUDGET = float(FIX["budgetMB"]); SLACK = float(FIX["slackMB"])
F1080 = float(FIX["frameMB1080"]); F4K = float(FIX["frameMB4k"]); FLOOR = int(FIX["minWindowFrames"])
SHOWN_TOL = int(FIX["shownTol"]); LATE_MAX = int(FIX["lateMaxAfterJump"])
INCOMING_MIN = int(FIX["incomingMinTextures"])
LID = FIX["layers"]
MEDIA = os.path.join(OUT, "media")
PASS = FAIL = 0
S = requests.Session()
S.headers["Connection"] = "close"   # one fresh connection per request (cpp-httplib 5 s keep-alive drop)
SEQ_KEYS = ("seq_open", "seq_textures", "seq_texture_mb", "seq_over_budget", "seq_frames_shown", "seq_late_frames",
            "seq_pending_frames", "seq_uploads", "seq_slot_reuses", "seq_evictions", "seq_stale_drops")


def ok(msg):
    global PASS; PASS += 1; print(f"PASS  {msg}", flush=True)


def no(msg):
    global FAIL; FAIL += 1; print(f"FAIL  {msg}", flush=True)


# ---------------------------------------------------------------- fixtures (written before the app starts)
SETS = {"L300": (300, 1920, 1080, 0), "M300": (300, 1920, 1080, 400), "Q40": (40, 3840, 2160, 800)}
ROW_SETS = {"v1_memory_1080": ("L300",), "v2_smooth_loop_1080": ("L300",), "v4_retrigger_hit": ("L300",),
            "v6_budget_share": ("L300",), "v8_crossfade_two_long": ("L300", "M300"),
            "v9_three_decoding": ("L300", "M300"), "v7_deck_keeps_time": ("L300",),
            "v7b_trim_spike": ("L300", "M300"), "v7c_one_per_deck": ("L300", "M300"), "v11_retire": ("L300",),
            "v3_smooth_pingpong_4k": ("Q40",), "v10_floors_win": ("Q40",)}


def mpath(name):
    return os.path.join(MEDIA, name)


def fname(set_name, k):
    return f"{set_name}_{k:03d}.png"


def frames(set_name, lo=0, hi=None):
    n = SETS[set_name][0]
    return [mpath(fname(set_name, k)) for k in range(lo, n if hi is None else hi)]


def band_h(h):
    return max(8, round(64 * h / 1080))


def frame_array(w, h, code):
    x = np.linspace(0, 255, w, endpoint=False, dtype=np.float32)[None, :]
    y = np.linspace(0, 255, h, endpoint=False, dtype=np.float32)[:, None]
    a = np.empty((h, w, 4), np.uint8)
    a[..., 0] = np.broadcast_to(x, (h, w)).astype(np.uint8)
    a[..., 1] = np.broadcast_to(y, (h, w)).astype(np.uint8)
    a[..., 2] = (37 * code) % 256
    a[..., 3] = 255
    bh = band_h(h); cw = w // 10
    a[h - bh:, :, :3] = 0
    for b in range(10):
        if (code >> b) & 1:
            a[h - bh:, b * cw:(b + 1) * cw, :3] = 255
    return a


def _write(job):
    path, w, h, code = job
    Image.fromarray(frame_array(w, h, code), "RGBA").save(path, compress_level=1)


def make_fixtures():
    os.makedirs(MEDIA, exist_ok=True)
    t0 = time.time()
    need = set()
    for r, sets in ROW_SETS.items():
        if ONLY is None or r in ONLY:
            need.update(sets)
    jobs = []
    for sname in sorted(need):
        n, w, h, off = SETS[sname]
        jobs += [(mpath(fname(sname, k)), w, h, off + k) for k in range(n)]
    with Pool(6) as p:
        p.map(_write, jobs, chunksize=4)
    a = np.zeros((144, 256, 4), np.uint8); a[..., :3] = (200, 120, 40); a[..., 3] = 255
    Image.fromarray(a, "RGBA").save(mpath("warm.png"), compress_level=1)
    mb = sum(os.path.getsize(j[0]) for j in jobs) / 1e6
    print(f"fixtures: {len(jobs) + 1} written to {MEDIA} ({mb:.0f} MB, sets {sorted(need)}) in {time.time() - t0:.1f} s",
          flush=True)


# ---------------------------------------------------------------- REST helpers
def seq_clip(cid, paths, fps, loop_mode=0):
    return {"name": f"s{cid}", "id": cid, "mediaType": 5, "mediaFile": "", "sequenceFiles": paths,
            "sequenceFps": float(fps), "speed": 1.0, "transportMode": 0, "loopMode": loop_mode, "reverse": False,
            "effects": []}


def img_clip(cid, path):
    return {"name": f"c{cid}", "id": cid, "mediaType": 1, "mediaFile": path, "effects": []}


def layer(lid, clips, speed=0.0, **extra):
    l = {"name": f"L{lid}", "id": lid, "opacity": 1.0, "visible": True, "blendMode": 0, "type": 0, "keyingMode": 0,
         "transitionSpeed": speed, "layerEffects": [], "clips": clips}
    l.update(extra); return l


def third(i, stacked):
    """layer transform: 1/3 size about anchor (0 / 0.5 / 1, 0.5) -> the i-th third of the canvas, centred vertically.
    A layer above another is Transparent (type 1, keying Alpha): an Opaque layer replaces everything below it, even where
    its transform left alpha 0."""
    return {"layerScale": 1.0 / 3.0, "layerAnchorX": [0.0, 0.5, 1.0][i], "layerAnchorY": 0.5, "type": 1 if stacked else 0}


def third_rect(i, W, H):
    return (round(i * W / 3), round(H / 3), round(W / 3), round(H / 3))


def deck(did, layers, ncols=None):
    n = ncols or max(len(l["clips"]) for l in layers)
    return {"name": f"D{did}", "id": did, "numColumns": n, "layers": layers}


def load(tag, decks, size):
    comp = {"name": "seqvram-" + tag, "activeDeckIndex": 0, "masterOpacity": 1.0, "globalTransitionSpeed": 0.0,
            "outputWidth": size[0], "outputHeight": size[1], "decks": decks}
    path = os.path.join(OUT, f"comp_{tag}.json")
    json.dump(comp, open(path, "w"), indent=1)
    try:
        r = S.post(A + "/api/load_composition", json={"path": path}, timeout=30)
        good = r.ok and r.json().get("ok") is True
    except Exception as e:  # noqa: BLE001
        good = False; r = e
    if not good:
        no(f"{tag}: load_composition rejected: {str(getattr(r, 'text', r))[:160]}")
    return good


def trig(li, col):
    S.post(A + "/api/trigger_clip", json={"layer": li, "column": col}, timeout=6)


def switch(dk):
    S.post(A + "/api/switch_deck", json={"deck": dk}, timeout=6)


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


def wait_active(dk, li, col, limit=10.0):
    t0 = time.time()
    while time.time() - t0 < limit:
        c = comp_state()
        try:
            if c["decks"][dk]["layers"][li]["activeClipColumn"] == col:
                return time.time() - t0
        except Exception:  # noqa: BLE001
            pass
        time.sleep(0.005)
    return None


def wait_deck(dk, limit=10.0):
    t0 = time.time()
    while time.time() - t0 < limit:
        c = comp_state()
        if c is not None and c.get("activeDeck") == dk:
            return time.time() - t0
        time.sleep(0.005)
    return None


def fade_done(dk, li, c=None):
    """the layer's crossfade has completed (previousClipColumn -1 / crossfadeProgress 1)."""
    c = c if c is not None else comp_state()
    try:
        l = c["decks"][dk]["layers"][li]
        return l["previousClipColumn"] == -1 or float(l["crossfadeProgress"]) >= 1.0
    except Exception:  # noqa: BLE001
        return False


def playhead(dk, li, col, c=None):
    c = c if c is not None else comp_state()
    try:
        for cl in c["decks"][dk]["layers"][li]["clips"]:
            if cl["column"] == col:
                return float(cl["playheadPosition"])
    except Exception:  # noqa: BLE001
        pass
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


def cap_with_playheads(name, heads):
    """heads = [(dk, li, col)]. Returns (capture, [(p_before, p_after)] per head)."""
    c0 = comp_state()
    before = [playhead(dk, li, col, c0) for dk, li, col in heads]
    f = cap(name)
    c1 = comp_state()
    after = [playhead(dk, li, col, c1) for dk, li, col in heads]
    return f, list(zip(before, after))


def box(a, b=16):
    h, w = a.shape[0] // b * b, a.shape[1] // b * b
    return a[:h, :w, :3].reshape(h // b, b, w // b, b, 3).mean(axis=(1, 3))


def dbox(x, y):
    if x.shape[:2] != y.shape[:2]:
        y = np.asarray(Image.fromarray(y.astype(np.uint8), "RGBA").resize((x.shape[1], x.shape[0]), Image.BOX)).astype(np.float32)
    return float(np.abs(box(x) - box(y)).mean())


def code(f, rect=None):
    """The 10-bit code in the band at the bottom of rect (x, y, w, h) of capture f (whole frame when None)."""
    H, W = f.shape[:2]
    x0, y0, w, h = rect if rect is not None else (0, 0, W, H)
    bh = h * 64.0 / 1080.0; cw = w / 10.0
    ya, yb = int(round(y0 + h - bh * 0.75)), int(round(y0 + h - bh * 0.25))
    c = 0
    for b in range(10):
        xa, xb = int(round(x0 + (b + 0.25) * cw)), int(round(x0 + (b + 0.75) * cw))
        if f[ya:yb, xa:xb, :3].mean() > 127:
            c |= 1 << b
    return c


def fixture(set_name, k):
    return np.asarray(Image.open(mpath(fname(set_name, k))).convert("RGBA")).astype(np.float32)


def in_bracket(idx, pb, n, tol, pingpong=False):
    """frame idx lies in [int(p0 n) - tol, int(p1 n) + tol] mod n (PingPong: the frames between, either direction)."""
    p0, p1 = pb
    if p0 is None or p1 is None:
        return False, "playhead unavailable"
    e0, e1 = min(int(p0 * n), n - 1), min(int(p1 * n), n - 1)
    if pingpong:
        lo, hi = min(e0, e1) - tol, max(e0, e1) + tol
        return lo <= idx <= hi, f"expected {e0}..{e1} +- {tol}"
    span = (e1 - e0) % n
    off = (idx - (e0 - tol)) % n
    return off <= span + 2 * tol, f"expected {e0}..{e1} +- {tol}"


def check_code(tag, label, f, pb, set_name, lo, n, tol, rect=None, pingpong=False, check_pixels=True):
    """code in the capture -> frame index of the set slice [lo, lo + n); within the playhead bracket; pixels decoded."""
    if f is None:
        return
    off = SETS[set_name][3]
    c = code(f, rect)
    idx = c - off - lo
    good, exp = in_bracket(idx, pb, n, tol, pingpong) if 0 <= idx < n else (False, f"code {c} not in {set_name}[{lo}:{lo + n}]")
    (ok if good else no)(f"{tag}: {label} code {c} = frame {idx} of {n}, {exp} (playheads {pb[0]}, {pb[1]})")
    if check_pixels and 0 <= idx < n:
        d = dbox(f, fixture(set_name, lo + idx))
        (ok if d <= TOL else no)(f"{tag}: {label} pixels are that frame's (dbox {d:.2f} <= {TOL})")


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
    the window. The last seq_* counters are kept too."""
    KEYS = ("peak_frame_time_ms", "peak_callback_ms")

    def __init__(self):
        self.rows = []; self._run = False

    def _loop(self):
        s = requests.Session(); s.headers["Connection"] = "close"
        while self._run:
            try:
                d = s.get(A + "/api/state", timeout=6).json()
                self.rows.append((time.time(), d))
            except Exception:  # noqa: BLE001
                pass
            time.sleep(0.015)

    def start(self):
        self.rows = []; self._run = True
        self._th = threading.Thread(target=self._loop, daemon=True); self._th.start()

    def stop(self):
        self._run = False; self._th.join(); return self.maxes()

    def window(self, sec):
        self.start(); time.sleep(sec); return self.stop()

    def argmax(self, k):
        """(time, value) of the largest k in the window (None when absent)."""
        v = [(r[0], r[1][k]) for r in self.rows if r[1].get(k) is not None]
        return max(v, key=lambda x: x[1]) if v else None

    def steps(self, k, t0):
        """[(seconds after t0, new value)] each time counter k changed during the window."""
        out, last = [], None
        for t, d in self.rows:
            v = d.get(k)
            if v is not None and last is not None and v != last:
                out.append((round(t - t0, 3), v))
            if v is not None:
                last = v
        return out

    def maxes(self):
        out = {}
        for k in self.KEYS:
            v = [r[1][k] for r in self.rows if r[1].get(k) is not None]
            out[k] = max(v) if v else None
        return out


def has_seq(tag, *snaps):
    for s in snaps:
        if s is None:
            no(f"{tag}: /api/state unavailable"); return False
        miss = [k for k in SEQ_KEYS if k not in s]
        if miss:
            no(f"{tag}: /api/state has no {', '.join(miss[:3])}{'...' if len(miss) > 3 else ''} (the app predates them)")
            return False
    return True


def d(a, b, k):
    return b[k] - a[k]


def la():
    return "load avg %.2f %.2f %.2f" % os.getloadavg()


def show(tag, s):
    if s is None or "seq_textures" not in s:
        return "seq_* absent"
    return (f"textures {s['seq_textures']} / {s['seq_texture_mb']:.1f} MB / open {s['seq_open']} / over {s['seq_over_budget']} / "
            f"uploads {s['seq_uploads']} / evictions {s['seq_evictions']} / reuses {s['seq_slot_reuses']} / "
            f"stale {s['seq_stale_drops']} / late {s['seq_late_frames']} / pending {s['seq_pending_frames']} / "
            f"shown {s['seq_frames_shown']} / deferred {s.get('seq_upload_deferred', 'absent')} / "
            f"deletes {s.get('seq_deletes', 'absent')} / drawn {s.get('seq_drawn_textures', 'absent')}")


def wait_window_full(tag, base, min_textures=125, limit=10.0):
    """Until this row's evictions (seq_evictions - base: the counter is cumulative over the run) > 0 and seq_textures >=
    min_textures (a full window in lap 1); None on timeout (e.g. an app that never evicts)."""
    t0 = time.time(); s = None
    while time.time() - t0 < limit:
        s = state()
        if s is None or "seq_evictions" not in s:
            return None
        if s["seq_evictions"] - base > 0 and s["seq_textures"] >= min_textures:
            print(f"      {tag}: window full after {time.time() - t0:.1f} s ({show(tag, s)})", flush=True)
            return time.time() - t0
        time.sleep(0.05)
    print(f"      {tag}: window not full after {limit} s ({show(tag, s)})", flush=True)
    return None


W1 = (1920, 1080); W4 = (3840, 2160)
MB_CAP = BUDGET + SLACK


# ---------------------------------------------------------------- rows
def v1(tag):
    lid = LID[tag]; n = 300
    if not load(tag, [deck(0, [layer(lid, [seq_clip(1, frames("L300"), 30.0)])])], W1):
        return
    time.sleep(1.0); trig(0, 0); wait_active(0, 0, 0)
    time.sleep(21.0)
    s1 = state()
    f, pbs = cap_with_playheads(tag + "_A", [(0, 0, 0)])
    if not has_seq(tag, s1):
        return
    print(f"      {tag}: {show(tag, s1)}; {la()}", flush=True)
    cap_tex = int(BUDGET / F1080) + 1
    (ok if s1["seq_textures"] <= cap_tex else no)(f"{tag}: (a) textures after 2 laps {s1['seq_textures']} <= {cap_tex}")
    (ok if s1["seq_texture_mb"] <= MB_CAP else no)(f"{tag}: (b) sequence texture MB {s1['seq_texture_mb']:.1f} <= {MB_CAP}")
    (ok if s1["seq_over_budget"] == 0 else no)(f"{tag}: (c) seq_over_budget {s1['seq_over_budget']} == 0")
    check_code(tag, "(d)", f, pbs[0], "L300", 0, n, 1)


def v2(tag):
    lid = LID[tag]; n = 300
    if not load(tag, [deck(0, [layer(lid, [seq_clip(1, frames("L300"), 10.0)])])], W1):
        return
    time.sleep(1.0); trig(0, 0); wait_active(0, 0, 0); time.sleep(1.0)
    if not wait_no_compiler(tag):
        return
    s0 = state(); pol = Poller(); m = pol.window(8.0); s1 = state()
    caps = []
    for j in range(3):
        caps.append(cap_with_playheads(f"{tag}_{j}", [(0, 0, 0)]))
        if j < 2:
            time.sleep(1.0)
    if not has_seq(tag, s0, s1):
        return
    pf = m.get("peak_frame_time_ms")
    late, pend, shown = d(s0, s1, "seq_late_frames"), d(s0, s1, "seq_pending_frames"), d(s0, s1, "seq_frames_shown")
    print(f"      {tag}: 8 s window: late {late}, pending {pend}, shown {shown}, max frame {pf}, max callback "
          f"{m.get('peak_callback_ms')}; {show(tag, s1)}; {la()}", flush=True)
    (ok if late == 0 else no)(f"{tag}: (a) late frames over 8 s at 10 fps {late} == 0")
    (ok if pend == 0 else no)(f"{tag}: (b) pending frames {pend} == 0")
    (ok if abs(shown - 80) <= SHOWN_TOL else no)(f"{tag}: (c) frames shown {shown} in [{80 - SHOWN_TOL}, {80 + SHOWN_TOL}]")
    if pf is None:
        no(f"{tag}: (d) peak_frame_time_ms absent")
    else:
        (ok if pf <= PEAK else no)(f"{tag}: (d) longest frame during steady playback {pf:.2f} ms <= {PEAK}")
    codes = []
    for j, (f, pbs) in enumerate(caps):
        check_code(tag, f"(e) cap {j}", f, pbs[0], "L300", 0, n, 1)
        codes.append(None if f is None else code(f))
    if None not in codes:
        inc = all(0 < (codes[j + 1] - codes[j]) % n < n // 2 for j in range(2))
        (ok if inc else no)(f"{tag}: (e) the 3 codes strictly increase mod {n}: {codes}")


def v4(tag):
    lid = LID[tag]; n = 300
    if not load(tag, [deck(0, [layer(lid, [seq_clip(1, frames("L300"), 30.0)])])], W1):
        return
    base = state()
    time.sleep(1.0); trig(0, 0); wait_active(0, 0, 0)
    wait_window_full(tag, 0 if base is None else base.get("seq_evictions", 0), 125, 10.0)
    pol = Poller(); pol.start(); time.sleep(0.1)
    s0 = state(); p0 = playhead(0, 0, 0); tt = time.time()
    trig(0, 0)   # the SAME column: a retrigger -> seekTo(inPoint) = frame 0
    time.sleep(0.5)
    s1 = state()
    pol.stop()
    f, pbs = cap_with_playheads(tag, [(0, 0, 0)])
    if not has_seq(tag, s0, s1):
        return
    late, pend = d(s0, s1, "seq_late_frames"), d(s0, s1, "seq_pending_frames")
    print(f"      {tag}: retrigger at frame {None if p0 is None else int(p0 * n)}, textures {s0['seq_textures']}: late {late}, "
          f"pending {pend}; late steps (s after the trigger, total) {pol.steps('seq_late_frames', tt)}; uploads "
          f"{pol.steps('seq_uploads', tt)}; stale {pol.steps('seq_stale_drops', tt)}; after: {show(tag, s1)}; {la()}",
          flush=True)
    (ok if pend == 0 else no)(f"{tag}: (a) pending frames after the retrigger {pend} == 0")
    (ok if late == 0 else no)(f"{tag}: (b) late frames after the retrigger {late} == 0 (the in-point frames were kept)")
    check_code(tag, "(c)", f, pbs[0], "L300", 0, n, 2)
    (ok if s1["seq_textures"] <= int(BUDGET / F1080) + 1 else no)(
        f"{tag}: (d) textures {s1['seq_textures']} <= {int(BUDGET / F1080) + 1}")


def v6(tag):
    la_, lb = LID[tag]
    A_ = seq_clip(1, frames("L300", 0, 120), 30.0); B_ = seq_clip(2, frames("L300", 120, 300), 10.0)
    if not load(tag, [deck(0, [layer(la_, [A_]), layer(lb, [B_])])], W1):
        return
    time.sleep(1.0); trig(0, 0); wait_active(0, 0, 0)
    time.sleep(5.0); s1 = state(); time.sleep(4.0); s2 = state()
    trig(1, 0); wait_active(0, 1, 0)
    time.sleep(8.0); s3 = state()
    if not has_seq(tag, s1, s2, s3):
        return
    print(f"      {tag}: s1 {show(tag, s1)}\n      {tag}: s2 {show(tag, s2)}\n      {tag}: s3 {show(tag, s3)}; {la()}",
          flush=True)
    up = d(s1, s2, "seq_uploads")
    (ok if up == 0 and s2["seq_textures"] == 120 else no)(
        f"{tag}: (a) a 120-frame sequence that fits re-decodes nothing on its second lap (uploads {up} == 0, "
        f"textures {s2['seq_textures']} == 120)")
    (ok if s3["seq_texture_mb"] <= MB_CAP else no)(f"{tag}: (b) both sequences {s3['seq_texture_mb']:.1f} MB <= {MB_CAP}")
    late = d(s2, s3, "seq_late_frames")
    (ok if late == 0 else no)(f"{tag}: (c) late frames over the 8 s both play {late} == 0")
    (ok if s3["seq_over_budget"] == 0 else no)(f"{tag}: (d) seq_over_budget {s3['seq_over_budget']} == 0")


def v8(tag):
    lid = LID[tag]; n = 300
    c0 = seq_clip(1, frames("L300"), 30.0); c1 = seq_clip(2, frames("M300"), 30.0)
    if not load(tag, [deck(0, [layer(lid, [c0, c1], speed=2.0)])], W1):
        return
    base = state()
    time.sleep(1.0); trig(0, 0); wait_active(0, 0, 0)
    wait_window_full(tag, 0 if base is None else base.get("seq_evictions", 0), 125, 10.0)
    if not wait_no_compiler(tag):
        return
    s0 = state(); pol = Poller(); pol.start(); tt = time.time()
    trig(0, 1); wait_active(0, 0, 1)
    time.sleep(1.0)
    tc = time.time(); fm = cap(tag + "_mid"); tce = time.time()
    sm = state()
    # F7: the fade's end, then until the incoming holds INCOMING_MIN textures (seq_drawn_textures) or 3 s pass
    td = None
    while time.time() - tt < 8.0:
        if fade_done(0, 0):
            td = time.time(); break
        time.sleep(0.01)
    tg = None; sg = None
    if td is not None:
        while time.time() - td < 3.0:
            sg = state()
            if sg is not None and (sg.get("seq_drawn_textures") or 0) >= INCOMING_MIN:
                tg = time.time() - td; break
            time.sleep(0.05)
        else:
            sg = state()
    sa = state(); time.sleep(2.0); sb = state()
    m = pol.stop()
    s1 = state()
    fa, pbs = cap_with_playheads(tag + "_after", [(0, 0, 1)])
    if not has_seq(tag, s0, sm, s1, sa, sb):
        return
    pend, pend_after = d(s0, s1, "seq_pending_frames"), d(sm, s1, "seq_pending_frames")
    late = d(s0, s1, "seq_late_frames")
    pf, pc = m.get("peak_frame_time_ms"), m.get("peak_callback_ms")
    amc = pol.argmax("peak_callback_ms")
    tdr = None if td is None else round(td - tt, 3)
    print(f"      {tag}: fade: pending {pend} ({pend_after} after mid-fade), late {late}, max frame {pf}, max callback {pc} "
          f"at {None if amc is None else round(amc[0] - tt, 3)} s after the trigger (mid capture {tc - tt:.3f}-{tce - tt:.3f} s, "
          f"fade done at {tdr} s); callback>10 {[(round(r[0] - tt, 3), round(r[1]['peak_callback_ms'], 1)) for r in pol.rows if (r[1].get('peak_callback_ms') or 0) > 10]}; "
          f"evictions {pol.steps('seq_evictions', tt)[-3:]}; late steps {pol.steps('seq_late_frames', tt)}; "
          f"s0 {show(tag, s0)}; s1 {show(tag, s1)}; {la()}", flush=True)
    print(f"      {tag}: after the fade: drawn textures {None if sg is None else sg.get('seq_drawn_textures', 'absent')} "
          f"(reached {INCOMING_MIN} at {None if tg is None else round(tg, 3)} s; seq_textures - 2 = "
          f"{None if sg is None else sg['seq_textures'] - 2}); drawn steps "
          f"{[x for x in pol.steps('seq_drawn_textures', tt)][:12]}; deletes steps {pol.steps('seq_deletes', tt)[:10]}; "
          f"sa {show(tag, sa)}; sb {show(tag, sb)}", flush=True)
    (ok if pend <= 6 and pend_after == 0 else no)(
        f"{tag}: (a) pending frames over the fade {pend} <= 6 and {pend_after} == 0 after the incoming chain shows")
    if pf is None:
        no(f"{tag}: (b) peak_frame_time_ms absent")
    else:
        (ok if pf <= PEAK else no)(f"{tag}: (b) longest frame over the fade {pf:.2f} ms <= {PEAK} (callback {pc})")
    if pc is None:
        no(f"{tag}: (b') peak_callback_ms absent")
    else:
        (ok if pc <= PEAK else no)(
            f"{tag}: (b') longest render callback over the fade {pc:.2f} ms <= {PEAK} (the frame-top scan and trim)")
    (ok if s1["seq_texture_mb"] <= BUDGET + 16 else no)(
        f"{tag}: (c) after the fade {s1['seq_texture_mb']:.1f} MB <= {BUDGET + 16}")
    if fm is not None:
        luma = float((0.299 * fm[..., 0] + 0.587 * fm[..., 1] + 0.114 * fm[..., 2]).mean())
        (ok if luma > 20 else no)(f"{tag}: (d) mid-fade capture is not black (mean luma {luma:.1f} > 20)")
    check_code(tag, "(d) after the fade", fa, pbs[0], "M300", 0, n, 2)
    (ok if late <= LATE_MAX else no)(f"{tag}: (e) late frames over the fade {late} <= {LATE_MAX}")
    if td is None:
        no(f"{tag}: (f) the fade never completed within 8 s")
    elif sg is None or "seq_drawn_textures" not in sg:
        no(f"{tag}: (f) /api/state has no seq_drawn_textures (the app predates it; seq_textures - 2 = "
           f"{None if sg is None else sg.get('seq_textures', 0) - 2})")
    else:
        (ok if tg is not None else no)(
            f"{tag}: (f) the incoming holds {sg['seq_drawn_textures']} >= {INCOMING_MIN} textures within 3 s after the fade "
            f"({None if tg is None else round(tg, 3)} s)")
        lg = d(sa, sb, "seq_late_frames")
        (ok if lg == 0 else no)(f"{tag}: (f) late frames over the next 2 s {lg} == 0")


def v9(tag):
    lids = LID[tag]; n = 150
    subsets = [("L300", 0), ("L300", 150), ("M300", 0)]
    lay = [layer(lids[i], [seq_clip(i + 1, frames(sn, lo, lo + n), 15.0)], **third(i, i > 0))
           for i, (sn, lo) in enumerate(subsets)]
    if not load(tag, [deck(0, lay)], W1):
        return
    time.sleep(1.0)
    for i in range(3):
        trig(i, 0)
    for i in range(3):
        wait_active(0, i, 0)
    time.sleep(5.0)
    if not wait_no_compiler(tag):
        return
    s0 = state(); time.sleep(20.0); s1 = state()
    f, pbs = cap_with_playheads(tag, [(0, i, 0) for i in range(3)])
    if not has_seq(tag, s0, s1):
        return
    late, pend = d(s0, s1, "seq_late_frames"), d(s0, s1, "seq_pending_frames")
    print(f"      {tag}: last 20 s: late {late}, pending {pend}; {show(tag, s1)}; {la()}", flush=True)
    (ok if late == 0 else no)(f"{tag}: (a) late frames over 20 s, three sequences decoding {late} == 0")
    (ok if pend == 0 else no)(f"{tag}: (b) pending frames {pend} == 0")
    (ok if s1["seq_texture_mb"] <= BUDGET + 16 else no)(f"{tag}: (c) {s1['seq_texture_mb']:.1f} MB <= {BUDGET + 16}")
    for i, (sn, lo) in enumerate(subsets):
        check_code(tag, f"(d) layer {i}", f, pbs[i], sn, lo, n, 2, rect=third_rect(i, *W1), check_pixels=False)


def v7(tag):
    l0, l1 = LID[tag]; n = 300
    if not load(tag, [deck(0, [layer(l0, [seq_clip(1, frames("L300"), 10.0)])]),
                      deck(1, [layer(l1, [img_clip(2, mpath("warm.png"))])])], W1):
        return
    time.sleep(1.0); trig(0, 0); wait_active(0, 0, 0)
    time.sleep(2.0)
    p0 = playhead(0, 0, 0); s0 = state()
    switch(1); wait_deck(1)
    time.sleep(3.0)
    switch(0); p1 = playhead(0, 0, 0); wait_deck(0)
    time.sleep(1.0)
    s1 = state()
    f, pbs = cap_with_playheads(tag, [(0, 0, 0)])
    if not has_seq(tag, s0, s1):
        return
    late, pend = d(s0, s1, "seq_late_frames"), d(s0, s1, "seq_pending_frames")
    ran = None if p0 is None or p1 is None else (p1 - p0) * 30.0
    print(f"      {tag}: clock ran {ran} s while off screen; late {late}, pending {pend}; {show(tag, s1)}; {la()}", flush=True)
    (ok if ran is not None and 2.6 <= ran <= 3.4 else no)(f"{tag}: (a) the clock ran off screen: {ran} s in [2.6, 3.4]")
    (ok if pend == 0 else no)(f"{tag}: (b) pending frames since s0 {pend} == 0")
    (ok if late <= LATE_MAX else no)(f"{tag}: (c) late frames on the return {late} <= {LATE_MAX}")
    (ok if late >= 1 else no)(f"{tag}: (c') the return holds the shown frame for a decode: late {late} >= 1")
    check_code(tag, "(d)", f, pbs[0], "L300", 0, n, 1)


def v7b(tag):
    ids = LID[tag]; n = 300
    d0 = deck(0, [layer(ids[i], [seq_clip(i + 1, frames(sn), 30.0)]) for i, sn in enumerate(("L300", "M300", "L300"))])
    d1 = deck(1, [layer(ids[3 + i], [seq_clip(10 + i, frames(sn), 30.0)], **third(2 * i, i > 0))
                  for i, sn in enumerate(("M300", "L300"))])
    if not load(tag, [d0, d1], W1):
        return
    time.sleep(1.0)
    for i in range(3):
        trig(i, 0)
    for i in range(3):
        wait_active(0, i, 0)
    t0 = time.time(); s = None
    while time.time() - t0 < 10.0:
        s = state()
        if s is None or "seq_texture_mb" not in s or s["seq_texture_mb"] >= BUDGET - 3 * F1080:
            break
        time.sleep(0.05)
    print(f"      {tag}: deck 0 after {time.time() - t0:.1f} s: {show(tag, s)}", flush=True)
    time.sleep(1.0)
    if not wait_no_compiler(tag):
        return
    s0 = state(); pol = Poller(); pol.start()
    switch(1); wait_deck(1)
    trig(0, 0); trig(1, 0)
    time.sleep(5.0)
    m = pol.stop(); s1 = state()
    f, pbs = cap_with_playheads(tag, [(1, 0, 0), (1, 1, 0)])
    if not has_seq(tag, s0, s1):
        return
    pf, pc = m.get("peak_frame_time_ms"), m.get("peak_callback_ms")
    print(f"      {tag}: switch window: max frame {pf}, max callback {pc}; s0 {show(tag, s0)}; s1 {show(tag, s1)}; {la()}",
          flush=True)
    if pf is None or pc is None:
        no(f"{tag}: (a) peak_frame_time_ms / peak_callback_ms absent")
    else:
        (ok if pf <= PEAK else no)(f"{tag}: (a) longest frame over the switch (idle trim) {pf:.2f} ms <= {PEAK}")
        (ok if pc <= PEAK else no)(f"{tag}: (a') longest render callback over the switch (the frame-top trim) {pc:.2f} ms <= {PEAK}")
    (ok if s1["seq_texture_mb"] <= BUDGET + 16 else no)(f"{tag}: (b) after 5 s {s1['seq_texture_mb']:.1f} MB <= {BUDGET + 16}")
    for i, sn in enumerate(("M300", "L300")):
        check_code(tag, f"(c) deck 1 layer {i}", f, pbs[i], sn, 0, n, 2, rect=third_rect(2 * i, *W1), check_pixels=False)


def v7c(tag):
    """fix round F6: one 300-frame 1080p sequence per deck; switch when deck 0's window is full. The idle trim of deck 0's
    sequence (F1: 60 frames after the switch) deletes within the per-frame budget (F2)."""
    l0, l1 = LID[tag]
    d0 = deck(0, [layer(l0, [seq_clip(1, frames("L300"), 30.0)])])
    d1 = deck(1, [layer(l1, [seq_clip(10, frames("M300"), 30.0)])])
    if not load(tag, [d0, d1], W1):
        return
    base = state(); time.sleep(1.0); trig(0, 0); wait_active(0, 0, 0)
    wait_window_full(tag, 0 if base is None else base.get("seq_evictions", 0), 125, 10.0)
    time.sleep(0.5)
    if not wait_no_compiler(tag):
        return
    s0 = state(); pol = Poller(); pol.start(); tt = time.time()
    switch(1); wait_deck(1); trig(0, 0)
    time.sleep(max(0.0, tt + 2.0 - time.time()))
    m = pol.stop(); s2 = state()
    time.sleep(max(0.0, tt + 5.0 - time.time()))
    s5 = state()
    if not has_seq(tag, s0, s2, s5):
        return
    pf, pc = m.get("peak_frame_time_ms"), m.get("peak_callback_ms")
    amc = pol.argmax("peak_callback_ms")
    ev = [(round(t - tt, 3), dd["seq_evictions"]) for t, dd in pol.rows if dd.get("seq_evictions") is not None]
    jumps = [(ev[k][0], ev[k][1] - ev[k - 1][1]) for k in range(1, len(ev)) if ev[k][1] - ev[k - 1][1] >= 100]
    print(f"      {tag}: switch: max frame {pf}, max callback {pc} at {None if amc is None else round(amc[0] - tt, 3)} s; "
          f"eviction jumps >= 100 {jumps}; textures {[x for x in pol.steps('seq_textures', tt) if x[0] < 2.0][:10]} "
          f"deletes {pol.steps('seq_deletes', tt)[:10]} callback>10 "
          f"{[(round(r[0] - tt, 3), round(r[1]['peak_callback_ms'], 1)) for r in pol.rows if (r[1].get('peak_callback_ms') or 0) > 10]}; "
          f"s0 {show(tag, s0)}; s2 {show(tag, s2)}; s5 {show(tag, s5)}; {la()}", flush=True)
    if pc is None:
        no(f"{tag}: (a) peak_callback_ms absent")
    else:
        (ok if pc <= PEAK else no)(f"{tag}: (a) longest render callback over the 2 s after the switch {pc:.2f} ms <= {PEAK}")
    (ok if s5["seq_texture_mb"] <= BUDGET + 16 else no)(f"{tag}: (b) after 5 s {s5['seq_texture_mb']:.1f} MB <= {BUDGET + 16}")
    (ok if jumps else no)(f"{tag}: (c) the idle trim of deck 0 ran inside the 2 s window (eviction jumps >= 100: {jumps})")


def v11(tag):
    """fix round F8, REPORT-ONLY: a full window, then an empty composition -- the retire drain (INFO lines, no verdict)."""
    lid = LID[tag]
    if not load(tag, [deck(0, [layer(lid, [seq_clip(1, frames("L300"), 30.0)])])], W1):
        return
    base = state(); time.sleep(1.0); trig(0, 0); wait_active(0, 0, 0)
    wait_window_full(tag, 0 if base is None else base.get("seq_evictions", 0), 125, 10.0)
    if not wait_no_compiler(tag):
        return
    s0 = state(); pol = Poller(); pol.start(); tt = time.time()
    load(tag + "_empty", [deck(0, [layer(lid, [])], ncols=1)], W1)
    time.sleep(2.0)
    m = pol.stop(); s1 = state()
    if not has_seq(tag, s0, s1):
        return
    amc = pol.argmax("peak_callback_ms")
    dl = None if "seq_deletes" not in s1 else s1["seq_deletes"] - s0.get("seq_deletes", 0)
    print(f"INFO  {tag}: over 2 s after loading an empty composition: max callback {m.get('peak_callback_ms')} at "
          f"{None if amc is None else round(amc[0] - tt, 3)} s, max frame {m.get('peak_frame_time_ms')}, seq_deletes "
          f"+{dl if dl is not None else 'absent'}, textures {s0['seq_textures']} -> {s1['seq_textures']} "
          f"({s1['seq_texture_mb']:.1f} MB)", flush=True)
    print(f"INFO  {tag}: textures steps {pol.steps('seq_textures', tt)[:20]}; deletes steps {pol.steps('seq_deletes', tt)[:20]}; "
          f"callback>10 {[(round(r[0] - tt, 3), round(r[1]['peak_callback_ms'], 1)) for r in pol.rows if (r[1].get('peak_callback_ms') or 0) > 10]}; "
          f"s1 {show(tag, s1)}; {la()}", flush=True)


def v3(tag):
    lid = LID[tag]; n = 40
    if not load(tag, [deck(0, [layer(lid, [seq_clip(1, frames("Q40"), 10.0, loop_mode=1)])])], W4):
        return
    time.sleep(1.0); trig(0, 0); wait_active(0, 0, 0); time.sleep(1.0)
    if not wait_no_compiler(tag):
        return
    s0 = state(); pol = Poller(); m = pol.window(12.0); s1 = state()
    caps = [cap_with_playheads(f"{tag}_0", [(0, 0, 0)])]
    time.sleep(1.0)
    caps.append(cap_with_playheads(f"{tag}_1", [(0, 0, 0)]))
    if not has_seq(tag, s0, s1):
        return
    late, pend, shown = d(s0, s1, "seq_late_frames"), d(s0, s1, "seq_pending_frames"), d(s0, s1, "seq_frames_shown")
    print(f"      {tag}: 12 s window: late {late}, pending {pend}, shown {shown}, max frame {m.get('peak_frame_time_ms')}, "
          f"max callback {m.get('peak_callback_ms')}; {show(tag, s1)}; {la()}", flush=True)
    (ok if late == 0 else no)(f"{tag}: (a) late frames over 12 s (4K PingPong at 10 fps) {late} == 0")
    (ok if pend == 0 else no)(f"{tag}: (b) pending frames {pend} == 0")
    (ok if 120 - 2 * SHOWN_TOL <= shown <= 120 + SHOWN_TOL else no)(
        f"{tag}: (c) frames shown {shown} in [{120 - 2 * SHOWN_TOL}, {120 + SHOWN_TOL}] (3 bounces)")
    cap_tex = int(BUDGET / F4K) + 1
    (ok if s1["seq_texture_mb"] <= MB_CAP and s1["seq_textures"] <= cap_tex else no)(
        f"{tag}: (d) {s1['seq_texture_mb']:.1f} MB <= {MB_CAP} and textures {s1['seq_textures']} <= {cap_tex}")
    for j, (f, pbs) in enumerate(caps):
        check_code(tag, f"(e) cap {j}", f, pbs[0], "Q40", 0, n, 1, pingpong=True)


def v10(tag):
    lids = LID[tag]; n = 20
    lay = [layer(lids[i], [seq_clip(i + 1, frames("Q40", 5 * i, 5 * i + n), 3.0)]) for i in range(5)]
    if not load(tag, [deck(0, lay)], W4):
        return
    time.sleep(1.0)
    for i in range(5):
        trig(i, 0)
    for i in range(5):
        wait_active(0, i, 0)
    time.sleep(10.0); s10 = state(); time.sleep(5.0); s1 = state()
    if not has_seq(tag, s10, s1):
        return
    print(f"      {tag}: s10 {show(tag, s10)}\n      {tag}: s15 {show(tag, s1)}; late over the last 5 s "
          f"{d(s10, s1, 'seq_late_frames')} (reported, not asserted: 4K decode-bound); {la()}", flush=True)
    (ok if s1["seq_open"] == 5 and s1["seq_over_budget"] == 1 else no)(
        f"{tag}: (a) seq_open {s1['seq_open']} == 5 and seq_over_budget {s1['seq_over_budget']} == 1 (floors win)")
    lim = 5 * FLOOR * F4K + 32
    (ok if s1["seq_texture_mb"] <= lim else no)(f"{tag}: (b) {s1['seq_texture_mb']:.1f} MB <= 5 x {FLOOR} x {F4K} + 32 = {lim:.1f}")
    pend = d(s10, s1, "seq_pending_frames")
    (ok if pend == 0 else no)(f"{tag}: (c) no pending frame after 10 s ({pend} == 0)")


def main():
    if MAKE:
        make_fixtures(); return
    rows = [("v1_memory_1080", v1), ("v2_smooth_loop_1080", v2), ("v4_retrigger_hit", v4), ("v6_budget_share", v6),
            ("v8_crossfade_two_long", v8), ("v9_three_decoding", v9), ("v7_deck_keeps_time", v7),
            ("v7b_trim_spike", v7b), ("v7c_one_per_deck", v7c), ("v11_retire", v11), ("v3_smooth_pingpong_4k", v3),
            ("v10_floors_win", v10)]
    for name, fn in rows:
        if ONLY is None or name in ONLY:
            print(f"--- {name} ({time.strftime('%H:%M:%S')})", flush=True)
            fn(name)
    print(f"\nPY {PASS} PASS / {FAIL} FAIL", flush=True)
    sys.exit(0 if FAIL == 0 else 1)


if __name__ == "__main__":
    main()
