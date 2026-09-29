#!/usr/bin/env python3
"""probe-idle-paint.py -- the launching / REST / pixel half of .harmony/probe-idle-paint.sh (s-rta-0928b idlepaint).

Plan: .harmony/.reports/s-rta-0928b/plan-idlepaint.md section 3.2 + HARMONY ADOPTION I1-I8. The .sh owns the rig gate
and the final quit; THIS file owns launching (one `open -g` per launch, health wait, quit, wait for exit), because every
gate row is a >= 5-launch arm and the identity rows alternate two bundles.

usage: probe-idle-paint.py <root> <fresh-outdir> [row,row,...]   (env IDLEPAINT_APP, IDLEPAINT_APP_BEFORE)
rows (run order; default = all but x1 / v1b): c0_preflight i1_idle_card i2_idle_many16 g2_strip_playhead g4_routine
      g5_driven x1_within_build_off v0_capture_teeth v1_identity_test_mode v1b_native_vs_inpeer v2_identity_fallback
      v2b_fallback_frames v3_identity_production_masked
Gate rows (PASS = window max median <= winMaxMedMs AND main-thread CPU <= cpuMainMsPerS, medians over `launches`):
  i1 card fixture (probe-routines.json), i2 many16 (4 layers x 4 Image clips over 16 PIL-made 3840x2160 JPEGs),
  g4 a loop routine playing on 3 layers at a manual 120 BPM (adoption I3): the window max only -- its main-thread CPU is
  an INFO line, BEFORE (main, 5 more launches) vs AFTER (Harmony ruling J2). g1 (animation rates: waveform / signal-bar
  layer draws per s, TopBar paints per s, modes native, 0 fallbacks) and g3 (INFO: MainComponent paints/s, ClipInspector
  repaints/s) ride on i1's launches. g5 (REPORT-ONLY, I6): the card at a manual 120 BPM (beat wheel, bar display and
  beat-phase meters move) with the live input driving the waveform -- the panels' paint cost does not depend on the
  values they show (INFERRED), so this is the real-show CPU. x1 (INFO): the same build with ADNA_UI_NATIVE_LAYERS=0.
Idle window: settleS after the last trigger, POST /api/debug/heartbeat {on, period_ms}, `ps -M` sample, idleS during
which the ONLY traffic is one GET /api/state + one GET /api/debug/ui_paint every pollMs (both read atomics on the HTTP
thread), `ps -M` sample. win_max_med = median of the per-poll peak_message_stall_ms; cpu_main = main-thread
(utime + stime) delta / wall (first thread row of ps -M = the main thread). A compiler (clang) seen during a window
taints the launch: re-run (at most launches + 3 attempts); a compiler running BEFORE a launch stops the row (TAINTED --
the lock is never held while waiting for quiet). Every launch prints the load average.
g2: seq24 (one layer, a 24-frame 320x180 PNG sequence, 30 fps loop) triggered, playingS: >= minTransportRepaints
  transport repaints and (adoption I2) strip paints whose painted playhead pixel moved / ticks whose playhead pixel moved
  >= minPaintAdvanceRatio; then the card loaded + triggered, settleS, playingS: <= idleMaxTransportRepaints.
Identity (Harmony ruling K2, fix round 2 -- it replaces J3's anti-aliased-edge clause; rows v1 / v1b / v2 / v3 / v3p /
v4): every differing pixel differs by <= identity.maxDelta (1/255) per channel, ANYWHERE; a pixel differing by more,
outside the row's declared live-content masks, is a violation. Each line prints the differing count, the max delta and
the violating count. v0 proves the rule rejects a 1-px shift.
Pixel rows (window-only captures `screencapture -x -o -l <Quartz window id>` of the largest on-screen Audio-DNA window,
decoded with PIL + numpy; MainComponent -> capture mapping from GET /api/debug/ui_paint: the title-bar height is the
window's Quartz height minus MainComponent's height, measured every run -- adoption I8; the precondition
peer_layer_backed == 1 is asserted in c0):
  v0 teeth: AFTER with ADNA_UI_NATIVE_LAYERS_TEETH=shift vs AFTER (test mode, card): the comparator must find diffs in
     BOTH panel rects and nowhere else outside the allowed set.
  v1 BEFORE vs AFTER, --test-mode (meters and waveform static, except the SignalBar's 'Mod 1' oscillator -- live even
     in test mode, masked on S2): S1 default, S2 card, S3 many16, S4 = S3 after POST /api/debug/ui_repaint_all on AFTER
     (the full-window pass, adoption I7). Masked: the TopBar row's right fpsMaskRightPt (fps / DSP labels), and on S2
     the whole SignalBar (Mod 1's sine meter, fix round 2); the rest K2 identity (a diff PNG saved). S4 is scoped to the
     waveform rect + 8 px (I7's subject); the whole-window S4 count is INFO: the window's first display pass draws
     emoji / slider-thumb / text edges a little differently from every later pass, in both builds (fix round 1, J1).
  v1n (INFO) the comparator's noise floor: the BEFORE app against itself, two launches, S1-S3.
  v1b (adoption I5) AFTER, test mode, card, inject_features non-trivial meters (+ the TEMPORARY waveform-freeze hook
     ADNA_TEMP_WAVE_FREEZE=1 when the build carries it -- else the waveform half is SKIP): native capture, forced in-peer
     (POST /api/debug/ui_native_fallback) capture, native again: the panel rects pixel-equal.
  v2 AFTER, test mode, card: A; an in-peer overlay parented to the top-level (kind "panel": a background app's PopupMenu
     is dismissed by JUCE within ~50 ms) at MainComponent (menuX, menuY) inside the SignalBar; signalbar_mode == 2
     within modePollS (waveform stays 0); B; off; signalbar_mode == 0; C. PASS: every A/B diff inside the panel's rect
     (+10 px) AND some inside the SignalBar; A == C outside the fps mask. Then kind "menu": the real parented PopupMenu
     comes and goes (layer_fallbacks + 1, mode back to 0).
  v2b (adoptions I1 / I4) production, g4's routine playing, ADNA_UI_OVERLAY_WITNESS=1 (a vblank witness): v2b.cycles
     panel + menu open/close cycles; ui_overlay_covered_frames stays 0 (a layer never showed while an overlay crossed
     it) and ui_restore_frames_max <= restoreFramesMax; teeth arm ADNA_UI_NATIVE_LAYERS_TEETH=asynchide (the plan
     body's late hide) must read covered > 0.
  v3 (SHOULD) BEFORE vs AFTER, production, card: masks SignalBar, WaveformDisplay, TopBar row; the rest K2 identity.
  v3p (Harmony ruling K3) production, card, v3p.runs launch pairs: BEFORE's (main's) idle look vs AFTER's idle look,
     masked only where production draws live content -- the fps mask, the SignalBar, the WaveformDisplay and the TopBar's
     beat wheel / bar readout / tempo + tracker-state labels (topbar_rect x + v3p.topbarLiveFromLeftPt, v3p.
     topbarLiveWidthPt wide: TopBar::resized lays them out at fixed offsets 326..512 pt from its left); the rest K2.
  v4 (INFO only, Harmony addendum 3 L2) AFTER, test mode, card, v4.runs launches: the idle look vs the look after POST
     /api/debug/ui_repaint_all (a whole-MainComponent pass, as a resize or leaving binding / MIDI-learn mode draws),
     outside the fps mask: K2 numbers printed, never a verdict. The window's FIRST display pass draws the Files grid's
     folder emoji (<= 66/255) and three TopBar slider thumbs (<= 29/255) differently from every later pass (fix round 1,
     J1), so this row documents that first-display-pass class (identical in main) instead of gating on it; v3p is the
     identity gate for ruling K3.
Exit 0 iff no FAIL (SKIP / TAINTED are not PASS: they exit 2).
"""
import json, os, re, statistics as st, subprocess, sys, time

import numpy as np
import requests
from PIL import Image, ImageDraw

A = "http://127.0.0.1:7070"
ROOT, OUT = sys.argv[1], sys.argv[2]
ALL_ROWS = ["c0_preflight", "i1_idle_card", "i2_idle_many16", "g2_strip_playhead", "g4_routine", "g5_driven",
            "x1_within_build_off", "v0_capture_teeth", "v1_identity_test_mode", "v1n_noise_floor", "v1b_native_vs_inpeer",
            "v2_identity_fallback", "v2b_fallback_frames", "v3_identity_production_masked",
            "v3p_production_idle_identity", "v4_full_pass_identity"]
DEFAULT_ROWS = [r for r in ALL_ROWS if r not in ("x1_within_build_off", "v1b_native_vs_inpeer")]
ROWS = [r for r in (sys.argv[3].split(",") if len(sys.argv) > 3 and sys.argv[3] else DEFAULT_ROWS) if r]
for r in ROWS:
    if r not in ALL_ROWS:
        raise SystemExit(f"unknown row {r}")
CFG = json.load(open(os.path.join(ROOT, ".harmony", "probe-idle-paint.json")))
APP = os.environ["IDLEPAINT_APP"]
APPB = os.environ.get("IDLEPAINT_APP_BEFORE", "")
MEDIA = os.path.join(OUT, "media")
N = int(os.environ.get("IDLEPAINT_LAUNCHES") or CFG["launches"])   # the env override is for A/B attribution runs only
PASS = FAIL = SKIP = 0
SUMMARY = {}
S = requests.Session()
S.headers["Connection"] = "close"   # one fresh connection per request (cpp-httplib 5 s keep-alive drop)


def ok(msg):
    global PASS; PASS += 1; print(f"PASS  {msg}", flush=True)


def no(msg):
    global FAIL; FAIL += 1; print(f"FAIL  {msg}", flush=True)


def skip(msg):
    global SKIP; SKIP += 1; print(f"SKIP  {msg}", flush=True)


def info(msg):
    print(f"INFO  {msg}", flush=True)


def load_avg():
    return "load %.2f %.2f %.2f" % os.getloadavg()


# ---------------------------------------------------------------- REST
def get(path, timeout=5):
    try:
        r = S.get(A + path, timeout=timeout)
        return r.json() if r.status_code == 200 else None
    except Exception:
        return None


def post(path, body=None, timeout=6):
    try:
        r = S.post(A + path, json=body if body is not None else {}, timeout=timeout)
        try:
            return r.json()
        except Exception:
            return {"ok": False, "status": r.status_code}
    except Exception as e:
        return {"ok": False, "error": str(e)}


# ---------------------------------------------------------------- launching
def adna_pids():
    out = subprocess.run(["ps", "-eo", "pid=,ucomm="], capture_output=True, text=True).stdout
    return [int(l.split()[0]) for l in out.splitlines() if len(l.split()) >= 2 and l.split()[1] == "Audio-DNA"]


def compilers():
    n = 0
    for name in ("clang", "clang++"):
        n += len(subprocess.run(["pgrep", "-x", name], capture_output=True, text=True).stdout.split())
    return n


def listener(port):
    out = subprocess.run(["lsof", "-nP", f"-iTCP:{port}", "-sTCP:LISTEN"], capture_output=True, text=True).stdout
    rows = out.splitlines()[1:]
    return rows[0].split()[0] if rows else ""


def quit_app():
    if adna_pids():
        subprocess.run(["osascript", "-e", 'tell application "Audio-DNA" to quit'], capture_output=True)
    for _ in range(30):
        if not adna_pids():
            break
        time.sleep(1)
    if adna_pids():
        print("  app still running after 30 s -- kill", flush=True)
        for p in adna_pids():
            try:
                os.kill(p, 15)
            except OSError:
                pass
        time.sleep(2)


def launch(app, tag, env=(), test=False):
    """-> (pid, dir) or (None, dir). Refuses when an Audio-DNA or a 7070 / 8080 listener exists."""
    d = os.path.join(OUT, tag)
    os.makedirs(d, exist_ok=True)
    if adna_pids() or listener(7070) or listener(8080):
        print(f"  REFUSE launch {tag}: Audio-DNA running or a port is taken", flush=True)
        return None, d
    for f in ("out.log", "err.log"):
        open(os.path.join(d, f), "w").close()   # open --stdout/--stderr APPEND: truncate first
    cmd = ["open", "-g", "--stdout", os.path.join(d, "out.log"), "--stderr", os.path.join(d, "err.log")]
    for kv in env:
        cmd += ["--env", kv]
    cmd.append(app)
    if test:
        cmd += ["--args", "--test-mode"]
    subprocess.run(cmd, check=False)
    up = False
    for _ in range(60):
        if get("/api/health", timeout=1) is not None:
            up = True
            break
        time.sleep(1)
    time.sleep(2)
    pids = adna_pids()
    who = listener(7070)
    print(f"  launched {tag}: {os.path.basename(os.path.dirname(os.path.dirname(os.path.dirname(app))))} "
          f"env={list(env)} test={test} pid={pids} 7070={who} {load_avg()}", flush=True)
    if not up or who != "Audio-DNA" or len(pids) != 1:
        return None, d
    return pids[0], d


# ---------------------------------------------------------------- fixtures
def card_fixture():
    return json.loads(open(os.path.join(ROOT, ".harmony", "probe-routines.json")).read().replace("@ROOT@", ROOT))


def img_clip(cid, path):
    return {"name": f"img{cid}", "id": cid, "mediaType": 1, "mediaFile": path}


def make_many16():
    os.makedirs(MEDIA, exist_ok=True)
    w, h = CFG["many16"]["w"], CFG["many16"]["h"]
    paths = []
    for k in range(16):
        p = os.path.join(MEDIA, f"img4k_{k + 1:02d}.jpg")
        if not os.path.exists(p):
            x = np.linspace(0, 255, w, dtype=np.float32)[None, :]
            y = np.linspace(0, 255, h, dtype=np.float32)[:, None]
            a = np.zeros((h, w, 3), np.uint8)
            a[..., 0] = x.astype(np.uint8)
            a[..., 1] = y.astype(np.uint8)
            a[..., 2] = (37 * k) % 256
            im = Image.fromarray(a, "RGB")
            ImageDraw.Draw(im).text((100, 100), f"many16 #{k + 1}", fill=(255, 255, 255))
            im.save(p, quality=90)
        paths.append(p)
    return paths


def many16_fixture():
    fx = card_fixture()
    imgs = make_many16()
    dk = fx["decks"][0]
    dk["numColumns"] = 4
    base = dk["layers"][0]
    layers, cid = [], 100
    for li in range(4):
        L = {k: v for k, v in base.items() if k != "clips"}
        L["name"], L["id"], L["clips"] = f"L{li + 1}", li, []
        for c in range(4):
            L["clips"].append(img_clip(cid, imgs[li * 4 + c])); cid += 1
        layers.append(L)
    dk["layers"] = layers
    return fx


def seq24_fixture():
    c = CFG["seq24"]
    os.makedirs(MEDIA, exist_ok=True)
    paths = []
    for k in range(c["frames"]):
        p = os.path.join(MEDIA, f"seq24_{k:03d}.png")
        if not os.path.exists(p):
            a = np.zeros((c["h"], c["w"], 4), np.uint8)
            a[..., 0] = (10 * k) % 256; a[..., 1] = 128; a[..., 2] = 255 - (10 * k) % 256; a[..., 3] = 255
            Image.fromarray(a, "RGBA").save(p, compress_level=1)
        paths.append(p)
    fx = card_fixture()
    dk = fx["decks"][0]
    base = dk["layers"][0]
    L = {k: v for k, v in base.items() if k != "clips"}
    L["name"], L["id"] = "SEQ", 0
    L["clips"] = [{"name": "seq24", "id": 50, "mediaType": 5, "mediaFile": "", "sequenceFiles": paths,
                   "sequenceFps": float(c["fps"]), "speed": 1.0, "transportMode": 0, "loopMode": 0, "reverse": False,
                   "effects": []}, None]
    dk["layers"] = [L]
    return fx


def routine_fixture():
    """Deck A: 3 layers each lit with the test card; one LOOP routine (16 beats) whose three lanes move the three layers'
    opacity -- a band on every layer it plays on (adoption I3's 'a routine spanning >= 3 layers')."""
    fx = card_fixture()
    dk = fx["decks"][0]
    base = dk["layers"][0]
    card = base["clips"][0]
    layers = []
    for li in range(3):
        L = {k: v for k, v in base.items() if k != "clips"}
        L["name"], L["id"] = f"L{li + 1}", li
        c = dict(card); c["id"] = 10 + li
        L["clips"] = [c, None]
        layers.append(L)
    dk["layers"] = layers

    def key(li):
        return {"scope": "layer", "deck": {"i": 0, "rel": True, "name": "A"},
                "layer": {"i": li, "id": li, "name": f"L{li + 1}"}, "control": "scalar", "scalar": "opacity"}
    lanes = [{"key": key(li), "kind": "continuous",
              "gestures": [{"grip": "held", "curve": [{"x": 0.0, "y": 0.4, "interp": "linear"},
                                                      {"x": 16.0, "y": 1.0, "interp": "linear"}],
                            "stamps": [], "bpmAtBegin": 120.0, "origin": "human"}]} for li in range(3)]
    fx["routines"] = [{"uuid": "idle-sweep", "name": "Sweep", "lengthBeats": 16.0, "quantize": "off", "loop": True,
                       "restoreState": False, "restoreStyle": "jump", "deckRelative": True,
                       "source": {"takeFolder": "", "fromBeat": 0.0, "toBeat": 16.0}, "preamble": [], "lanes": lanes}]
    fx["routineBank"] = [{"slot": 0, "uuid": "idle-sweep"}]
    return fx


def write_fixture(fx, name):
    p = os.path.join(OUT, f"fixture-{name}.json")
    json.dump(fx, open(p, "w"), indent=1)
    return p


def setup(name, bpm=None):
    """load + trigger column 0 of every layer that has a clip there. -> True when the load answered ok."""
    if name == "default":
        return True
    fx = {"card": card_fixture, "many16": many16_fixture, "seq24": seq24_fixture, "routine": routine_fixture}[name]()
    r = post("/api/load_composition", {"path": write_fixture(fx, name)})
    if not (r or {}).get("ok"):
        print(f"  load_composition {name}: {r}", flush=True)
        return False
    time.sleep(1.5)
    if bpm is not None:
        post("/api/set_bpm", {"bpm": bpm})
    for li, L in enumerate(fx["decks"][0]["layers"]):
        if L["clips"] and L["clips"][0] is not None:
            post("/api/trigger_clip", {"layer": li, "column": 0})
            time.sleep(0.2)
    if name == "routine":
        post("/api/routine/fire", {"slot": 0})
        for _ in range(40):
            b = (get("/api/routine/status") or {}).get("bank") or [{}]
            if b[0].get("state") == "running":
                break
            time.sleep(0.1)
        b = (get("/api/routine/status") or {}).get("bank") or [{}]
        print(f"  routine state {b[0].get('state')} layers {b[0].get('layers')}", flush=True)
    return True


# ---------------------------------------------------------------- the idle window
def t2ms(s):
    m = re.match(r"(?:(\d+):)?(\d+)\.(\d+)", s)
    return ((int(m[1] or 0) * 60 + int(m[2])) * 1000 + int(m[3].ljust(3, "0")[:3])) if m else 0


def ps_main_ms(pid):
    out = subprocess.run(["ps", "-M", "-p", str(pid)], capture_output=True, text=True).stdout.splitlines()
    for ln in out[1:]:
        tm = [x for x in ln.split() if re.match(r"^\d+:\d+\.\d+$", x)]
        if len(tm) >= 2:
            return t2ms(tm[0]) + t2ms(tm[1])   # the first thread row = the main thread
    return None


def ui():
    return get("/api/debug/ui_paint")


def idle_window(pid, settle=None, dur=None):
    settle = CFG["settleS"] if settle is None else settle
    dur = CFG["idleS"] if dur is None else dur
    poll = CFG["pollMs"] / 1000.0
    time.sleep(settle)
    hb = post("/api/debug/heartbeat", {"on": True, "period_ms": CFG["heartbeatPeriodMs"]})
    time.sleep(0.5)
    get("/api/state")   # discard the peak of the switch-on
    u0 = ui()
    tainted = compilers() > 0
    c0, t0 = ps_main_ms(pid), time.monotonic()
    peaks = []
    for i in range(int(round(dur / poll))):
        target = t0 + (i + 1) * poll
        time.sleep(max(0.0, target - time.monotonic()))
        s = get("/api/state") or {}
        ui()
        if "peak_message_stall_ms" in s:
            peaks.append(float(s["peak_message_stall_ms"]))
        if i % 4 == 0 and compilers() > 0:
            tainted = True
    c1, t1 = ps_main_ms(pid), time.monotonic()
    u1 = ui()
    post("/api/debug/heartbeat", {"on": False})
    wall = t1 - t0
    res = {"hb_ok": bool((hb or {}).get("ok")), "win_max_med": st.median(peaks) if peaks else None,
           "win_max_max": max(peaks) if peaks else None, "n_windows": len(peaks),
           "cpu_main": (c1 - c0) / wall if c0 is not None and c1 is not None else None, "wall": wall,
           "tainted": tainted, "load": load_avg()}
    if u0 and u1:
        rate = lambda k: (u1[k] - u0[k]) / wall
        res.update(wf_draws=rate("waveform_layer_draws"), sb_draws=rate("signalbar_layer_draws"),
                   top_paints=rate("top_bar_paints"), main_paints=rate("main_component_paints"),
                   insp_repaints=rate("clip_inspector_repaints"), strip_repaints=rate("layer_strip_transport_repaints"),
                   band_repaints=rate("layer_strip_band_repaints"),
                   modes=[u1["waveform_mode"], u1["signalbar_mode"]], fallbacks=u1["layer_fallbacks"])
    return res


def fmt(v, f="%.1f"):
    return "absent" if v is None else (f % v)


def run_arm(row, app, fixture, n, env=(), bpm=None):
    """n clean launches -> list of idle_window results; None when a compiler ran before a launch (TAINTED)."""
    got, attempt = [], 0
    while len(got) < n and attempt < n + 3:
        if compilers() > 0:
            print(f"  {row}: a compiler is running before launch {len(got) + 1} -- stop (never wait holding the lock)",
                  flush=True)
            return None
        attempt += 1
        pid, _ = launch(app, f"{row}/r{attempt}", env=env)
        if pid is None:
            quit_app()
            no(f"{row}: launch {attempt} did not come up")
            return got
        if not setup(fixture, bpm=bpm):
            quit_app()
            no(f"{row}: fixture {fixture} did not load")
            return got
        r = idle_window(pid)
        quit_app()
        print(f"  {row} r{attempt}: win_max_med {fmt(r['win_max_med'])} ms (max {fmt(r['win_max_max'])}, "
              f"{r['n_windows']} windows) cpu_main {fmt(r['cpu_main'])} ms/s"
              + (f" | wf {r['wf_draws']:.1f}/s sb {r['sb_draws']:.1f}/s top {r['top_paints']:.1f}/s main "
                 f"{r['main_paints']:.1f}/s insp {r['insp_repaints']:.1f}/s strip {r['strip_repaints']:.1f}/s band "
                 f"{r['band_repaints']:.1f}/s modes {r['modes']} fallbacks {r['fallbacks']}" if "wf_draws" in r else "")
              + f" | {r['load']}" + (" TAINTED (compiler seen) -- re-run" if r["tainted"] else ""), flush=True)
        json.dump(r, open(os.path.join(OUT, row, f"r{attempt}", "idle.json"), "w"), indent=1)
        if not r["tainted"]:
            got.append(r)
    return got


def gate(row, runs, label, cpu_gate=True):
    """PASS = window max median <= winMaxMedMs AND (cpu_gate) main CPU median <= cpuMainMsPerS. g4 passes
    cpu_gate=False (Harmony ruling J2): its CPU is an INFO line (BEFORE vs AFTER), the window max stays the gate."""
    if runs is None:
        skip(f"{row}: TAINTED (a compiler ran) -- re-run on a quiet machine"); return
    if len(runs) < N:
        no(f"{row}: only {len(runs)} clean launches of {N}"); return
    wm = [r["win_max_med"] for r in runs]
    cpu = [r["cpu_main"] for r in runs if r["cpu_main"] is not None]
    med_cpu = st.median(cpu) if cpu else None
    SUMMARY[row] = {"win_max_med": wm, "cpu_main": cpu}
    if any(v is None for v in wm):
        no(f"{row} ({label}): window max absent (app predates the heartbeat) | main CPU median {fmt(med_cpu)} ms/s "
           f"[{fmt(min(cpu) if cpu else None)}-{fmt(max(cpu) if cpu else None)}]")
        return
    med_wm = st.median(wm)
    if not cpu_gate:
        (ok if med_wm <= CFG["winMaxMedMs"] else no)(
            f"{row} ({label}): window max median {med_wm:.1f} ms [{min(wm):.1f}-{max(wm):.1f}] (<= {CFG['winMaxMedMs']}) "
            f"| main CPU median {fmt(med_cpu)} ms/s [{fmt(min(cpu) if cpu else None)}-{fmt(max(cpu) if cpu else None)}] "
            f"(INFO, ruling J2)")
        return
    line = (f"{row} ({label}): window max median {med_wm:.1f} ms [{min(wm):.1f}-{max(wm):.1f}] (<= {CFG['winMaxMedMs']}) "
            f"AND main CPU median {fmt(med_cpu)} ms/s [{fmt(min(cpu))}-{fmt(max(cpu))}] (<= {CFG['cpuMainMsPerS']})")
    (ok if med_wm <= CFG["winMaxMedMs"] and med_cpu is not None and med_cpu <= CFG["cpuMainMsPerS"] else no)(line)


# ---------------------------------------------------------------- captures
def main_window():
    import Quartz
    wl = Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionOnScreenOnly, Quartz.kCGNullWindowID)
    best = None
    for w in wl:
        if str(w.get("kCGWindowOwnerName", "")) != "Audio-DNA" or int(w.get("kCGWindowLayer", 0)) != 0:
            continue
        b = w.get("kCGWindowBounds", {})
        area = float(b.get("Width", 0)) * float(b.get("Height", 0))
        if best is None or area > best[0]:
            best = (area, int(w["kCGWindowNumber"]), float(b["Width"]), float(b["Height"]))
    return best


def capture(name):
    """-> (RGB uint8 array, window pts (w, h)) or (None, None)."""
    mw = main_window()
    if mw is None:
        return None, None
    p = os.path.join(OUT, "shots", name + ".png")
    os.makedirs(os.path.dirname(p), exist_ok=True)
    subprocess.run(["screencapture", "-x", "-o", "-l", str(mw[1]), p], check=False)
    if not os.path.exists(p):
        return None, None
    return np.asarray(Image.open(p).convert("RGB")).copy(), (mw[2], mw[3])


class Geo:
    """MainComponent coordinates -> capture pixels. Title bar = window height - MainComponent height (measured per run)."""
    def __init__(self, u, win_pts, img):
        self.scale = img.shape[1] / win_pts[0]
        self.title = win_pts[1] - u["main_h"]
        self.u = u
        self.ok = abs(win_pts[0] - u["main_w"]) <= 1 and 0 <= self.title <= 80

    def px(self, r):
        x, y, w, h = r
        s = self.scale
        return (int(round(x * s)), int(round((y + self.title) * s)), int(round(w * s)), int(round(h * s)))

    def desc(self):
        return f"scale {self.scale:.2f}, title bar {self.title:.0f} pt, main {self.u['main_w']}x{self.u['main_h']}"


def in_rect(r, x, y):
    return r[0] <= x < r[0] + r[2] and r[1] <= y < r[1] + r[3]


def clusters(mask):
    """8-connected components of a boolean mask -> list of (x0, y0, x1, y1, n). Bails out past 50k pixels."""
    ys, xs = np.nonzero(mask)
    if len(xs) > 50000:
        return [(int(xs.min()), int(ys.min()), int(xs.max()), int(ys.max()), int(len(xs)))], True
    pts = set(zip(xs.tolist(), ys.tolist()))
    out = []
    while pts:
        sx, sy = pts.pop()
        stack, comp = [(sx, sy)], [(sx, sy)]
        while stack:
            x, y = stack.pop()
            for dx in (-1, 0, 1):
                for dy in (-1, 0, 1):
                    q = (x + dx, y + dy)
                    if q in pts:
                        pts.remove(q); stack.append(q); comp.append(q)
        cx = [c[0] for c in comp]; cy = [c[1] for c in comp]
        out.append((min(cx), min(cy), max(cx), max(cy), len(comp)))
    return out, False


def compare(a, b, geo, name, masks=()):
    """-> (n diff px outside masks, clusters, max delta, clusters). Saves a diff PNG. Exact comparison (v0 / v1b's
    meters-moved / v2's in-bar count); the identity rows use identity() (ruling K2)."""
    if a.shape != b.shape:
        return None
    d = np.abs(a.astype(np.int16) - b.astype(np.int16)).max(axis=2)
    m = d > 0
    for r in masks:
        x, y, w, h = geo.px(r)
        m[max(0, y):y + h, max(0, x):x + w] = False
    cl, big = clusters(m)
    if m.any():
        vis = np.zeros_like(a); vis[m] = (255, 0, 255)
        Image.fromarray(np.where(m[..., None], vis, (a // 3)).astype(np.uint8)).save(
            os.path.join(OUT, "shots", f"diff-{name}.png"))
    for c in cl:
        x0, y0, x1, y1, n = c
        md = int(d[y0:y1 + 1, x0:x1 + 1][m[y0:y1 + 1, x0:x1 + 1]].max())
        print(f"    {name}: cluster bbox ({x0},{y0})-({x1},{y1}) px {n} max delta {md}", flush=True)
    return int(m.sum()), cl, (int(d[m].max()) if m.any() else 0), cl


def identity(a, b, geo, name, masks=()):
    """Harmony ruling K2 (rows v1 / v1b / v2 / v3 / v3p / v4): identical = every differing pixel (outside masks) differs
    by <= identity.maxDelta per channel, anywhere. -> (n differing px, max delta, n violating px, violation clusters).
    Saves a diff PNG: violations magenta, tolerated pixels yellow."""
    if a.shape != b.shape:
        return None
    d = np.abs(a.astype(np.int16) - b.astype(np.int16)).max(axis=2)
    m = d > 0
    for r in masks:
        x, y, w, h = geo.px(r)
        m[max(0, y):y + h, max(0, x):x + w] = False
    viol = m & (d > CFG["identity"]["maxDelta"])
    if m.any():
        vis = (a // 3).astype(np.uint8)
        vis[m & ~viol] = (255, 255, 0)
        vis[viol] = (255, 0, 255)
        Image.fromarray(vis).save(os.path.join(OUT, "shots", f"diff-{name}.png"))
    cl, big = clusters(viol)
    for c in cl[:20]:
        x0, y0, x1, y1, n = c
        print(f"    {name}: K2 violation cluster bbox ({x0},{y0})-({x1},{y1}) px {n} max delta "
              f"{int(d[y0:y1 + 1, x0:x1 + 1][viol[y0:y1 + 1, x0:x1 + 1]].max())}", flush=True)
    if len(cl) > 20:
        print(f"    {name}: ... {len(cl) - 20} more violation clusters", flush=True)
    return int(m.sum()), (int(d[m].max()) if m.any() else 0), int(viol.sum()), cl


def k2(res):
    n, md, nv, cl = res
    return f"{n} px differ (max delta {md}), {nv} violate K2 (> {CFG['identity']['maxDelta']}/255) in {len(cl)} cluster(s)"


def fps_mask(u):
    t = u["topbar_rect"]
    w = CFG["v"]["fpsMaskRightPt"]
    return (t[0] + t[2] - w, t[1], w, t[3])


def wait_mode(key, want, secs):
    end = time.monotonic() + secs
    while time.monotonic() < end:
        u = ui() or {}
        if u.get(key) == want:
            return True
        time.sleep(0.02)
    return False


# ---------------------------------------------------------------- rows
def row_c0():
    pid, _ = launch(APP, "c0/r1")
    if pid is None:
        quit_app(); no("c0_preflight: app never answered /api/health"); return False
    hb = post("/api/debug/heartbeat", {"on": True, "period_ms": 4})
    (ok if (hb or {}).get("ok") else no)(f"c0_preflight: POST /api/debug/heartbeat answers ok ({hb})"
                                         + ("" if (hb or {}).get("ok") else " -- app predates mediaopen's heartbeat"))
    post("/api/debug/heartbeat", {"on": False})
    time.sleep(1.0)
    u = ui()
    if u is None:
        no("c0_preflight: GET /api/debug/ui_paint absent (app predates idlepaint)")
    else:
        (ok if u.get("peer_layer_backed") == 1 else no)(
            f"c0_preflight: the peer NSView is layer-backed (peer_layer_backed {u.get('peer_layer_backed')}, adoption I8)")
    img, win = capture("c0-preflight")
    cap_ok = img is not None and len(np.unique(img.reshape(-1, 3), axis=0)) > 1
    (ok if cap_ok else no)(f"c0_preflight: window-only capture of the main window is not blank "
                           f"({'window ' + str(win) if win else 'no window'}) -- else grant Screen Recording to the host")
    quit_app()
    return cap_ok


def row_i(row, fixture, label):
    runs = run_arm(row, APP, fixture, N)
    gate(row, runs, label)
    return runs


def row_g1_g3(runs):
    if not runs:
        return
    if "wf_draws" not in runs[0]:
        no("g1_anim_rates: ui_paint counters absent (app predates idlepaint)")
        info("g3_peer_quiet: counters absent"); return
    lo, hi = CFG["g1"]["layerDrawsPerS"]
    tlo, thi = CFG["g1"]["topBarPaintsPerS"]
    bad = []
    for i, r in enumerate(runs):
        if not (lo <= r["wf_draws"] <= hi and lo <= r["sb_draws"] <= hi and tlo <= r["top_paints"] <= thi
                and r["modes"] == [0, 0] and r["fallbacks"] == 0):
            bad.append(i + 1)
    desc = "; ".join(f"r{i + 1} wf {r['wf_draws']:.1f} sb {r['sb_draws']:.1f} top {r['top_paints']:.1f} modes "
                     f"{r['modes']} fallbacks {r['fallbacks']}" for i, r in enumerate(runs))
    (ok if not bad else no)(f"g1_anim_rates: waveform / signal-bar layer draws in [{lo}, {hi}]/s, TopBar paints in "
                            f"[{tlo}, {thi}]/s, modes native, 0 fallbacks ({desc})" + (f" -- bad {bad}" if bad else ""))
    info("g3_peer_quiet: MainComponent paints/s " + ", ".join(f"{r['main_paints']:.1f}" for r in runs)
         + " | ClipInspector repaints/s " + ", ".join(f"{r['insp_repaints']:.2f}" for r in runs))


def row_g2():
    if compilers() > 0:
        skip("g2_strip_playhead: TAINTED (a compiler ran)"); return
    pid, _ = launch(APP, "g2/r1")
    if pid is None:
        quit_app(); no("g2_strip_playhead: launch"); return
    g = CFG["g2"]
    if not setup("seq24"):
        quit_app(); no("g2_strip_playhead: seq24 did not load"); return
    time.sleep(1.0)
    u0 = ui()
    if u0 is None:
        quit_app(); no("g2_strip_playhead: ui_paint counters absent (app predates idlepaint)"); return
    time.sleep(g["playingS"])
    u1 = ui()
    rep = u1["layer_strip_transport_repaints"] - u0["layer_strip_transport_repaints"]
    ticks = u1["layer_strip_playhead_ticks"] - u0["layer_strip_playhead_ticks"]
    paints = u1["layer_strip_playhead_paints"] - u0["layer_strip_playhead_paints"]
    comp = get("/api/composition") or {}
    (ok if rep >= g["minTransportRepaints"] else no)(
        f"g2_strip_playhead: seq24 playing {g['playingS']} s -> {rep} transport repaints (>= {g['minTransportRepaints']})")
    ratio = paints / ticks if ticks else 0.0
    (ok if ticks > 0 and ratio >= g["minPaintAdvanceRatio"] else no)(
        f"g2_strip_playhead (I2): painted playhead advances {paints} / ticks whose playhead pixel moved {ticks} = "
        f"{ratio:.3f} (>= {g['minPaintAdvanceRatio']})")
    if not setup("card"):
        quit_app(); no("g2_strip_playhead: card did not load"); return
    time.sleep(CFG["settleS"])
    u2 = ui()
    time.sleep(g["playingS"])
    u3 = ui()
    idle = u3["layer_strip_transport_repaints"] - u2["layer_strip_transport_repaints"]
    (ok if idle <= g["idleMaxTransportRepaints"] else no)(
        f"g2_strip_playhead: card (Image clips) idle {g['playingS']} s -> {idle} transport repaints "
        f"(<= {g['idleMaxTransportRepaints']})")
    quit_app()


def row_g5():
    runs = run_arm("g5_driven", APP, "card", N, bpm=CFG["g4"]["bpm"])
    if runs is None:
        skip("g5_driven: TAINTED"); return
    cpu = [r["cpu_main"] for r in runs if r["cpu_main"] is not None]
    wm = [r["win_max_med"] for r in runs if r["win_max_med"] is not None]
    SUMMARY["g5_driven"] = {"win_max_med": wm, "cpu_main": cpu}
    info(f"g5_driven (REPORT-ONLY, I6): card at a manual {CFG['g4']['bpm']} BPM + live input: main CPU median "
         f"{fmt(st.median(cpu) if cpu else None)} ms/s {['%.1f' % c for c in cpu]} | window max median "
         f"{fmt(st.median(wm) if wm else None)} ms {['%.1f' % w for w in wm]}")


def row_x1():
    runs = run_arm("x1_within_build_off", APP, "card", int(CFG["x1"]["launches"]), env=("ADNA_UI_NATIVE_LAYERS=0",))
    if not runs:
        skip("x1_within_build_off: no clean launch"); return
    cpu = [r["cpu_main"] for r in runs]
    wm = [r["win_max_med"] for r in runs]
    info(f"x1_within_build_off (INFO): ADNA_UI_NATIVE_LAYERS=0 -> window max median {fmt(st.median(wm))} ms, "
         f"main CPU median {fmt(st.median(cpu))} ms/s")


GEO = {}   # the last good geometry of this run (v1n reuses it)


def geo_for(img, win):
    u = ui()
    if u is None or img is None:
        return None, u
    g = Geo(u, win, img)
    print(f"    geometry: {g.desc()} ({'ok' if g.ok else 'MISMATCH'})", flush=True)
    if g.ok:
        GEO.update(geo=g, u=u)
    return (g if g.ok else None), u


def shoot_states(app, tag, env=(), repaint=False):
    """test mode: S1 default, S2 card, S3 many16 (+ S4 after a full repaint). -> {state: (img, win)}, geo, u."""
    pid, _ = launch(app, tag, env=env, test=True)
    shots, geo, u = {}, None, None
    if pid is None:
        quit_app(); return shots, geo, u
    v = CFG["v"]
    time.sleep(v["settleS"])
    shots["S1"] = capture(f"{tag}-S1")
    for s, fx, wait in (("S2", "card", v["settleS"]), ("S3", "many16", v["settleManyS"])):
        setup(fx)
        time.sleep(wait)
        shots[s] = capture(f"{tag}-{s}")
    if repaint:
        r = post("/api/debug/ui_repaint_all")
        time.sleep(1.0)
        shots["S4"] = capture(f"{tag}-S4")
        print(f"    ui_repaint_all: {r}", flush=True)
    else:
        time.sleep(1.0)
        shots["S4"] = capture(f"{tag}-S4")
    geo, u = geo_for(*shots["S3"])
    quit_app()
    return shots, geo, u


def row_v0():
    if compilers() > 0:
        skip("v0_capture_teeth: TAINTED"); return
    pid, _ = launch(APP, "v0/teeth", env=("ADNA_UI_NATIVE_LAYERS_TEETH=shift",), test=True)
    if pid is None:
        quit_app(); no("v0_capture_teeth: launch"); return
    setup("card"); time.sleep(CFG["v"]["settleS"])
    t = capture("v0-teeth")
    quit_app()
    pid, _ = launch(APP, "v0/plain", test=True)
    if pid is None:
        quit_app(); no("v0_capture_teeth: launch"); return
    setup("card"); time.sleep(CFG["v"]["settleS"])
    p = capture("v0-plain")
    geo, u = geo_for(*p)
    quit_app()
    if t[0] is None or p[0] is None or geo is None:
        skip("v0_capture_teeth: no capture / geometry"); return
    res = compare(t[0], p[0], geo, "v0", masks=(fps_mask(u),))
    n, cl, md, bad = res
    grow = lambda r: (r[0] - 2, r[1] - 2, r[2] + 4, r[3] + 4)   # a 1-pt shift moves an edge up to 2 capture px
    sb, wf = grow(geo.px(u["signalbar_rect"])), grow(geo.px(u["waveform_rect"]))
    in_sb = [c for c in cl if in_rect(sb, c[0], c[1]) and in_rect(sb, c[2], c[3])]
    in_wf = [c for c in cl if in_rect(wf, c[0], c[1]) and in_rect(wf, c[2], c[3])]
    outside = [c for c in cl if c not in in_sb and c not in in_wf]
    (ok if in_sb and in_wf and not outside else no)(
        f"v0_capture_teeth: the comparator catches a 1-px shift of the layers: {len(in_sb)} cluster(s) in the SignalBar, "
        f"{len(in_wf)} in the waveform, {len(outside)} elsewhere ({n} px, max delta {md})")
    r3 = identity(t[0], p[0], geo, "v0-k2", masks=(fps_mask(u),))
    (ok if r3[2] > 0 else no)(f"v0_capture_teeth (K2): the K2 identity rule rejects the 1-px shift -- {k2(r3)} (> 0)")


def row_v1():
    if not APPB or not os.path.isdir(APPB):
        skip("v1_identity_test_mode: no BEFORE app"); return
    if compilers() > 0:
        skip("v1_identity_test_mode: TAINTED"); return
    b, _, _ = shoot_states(APPB, "v1/before")
    a, geo, u = shoot_states(APP, "v1/after", repaint=True)
    if geo is None or any(b.get(s, (None,))[0] is None or a.get(s, (None,))[0] is None for s in ("S1", "S2", "S3", "S4")):
        skip("v1_identity_test_mode: missing capture or geometry"); return
    for s in ("S1", "S2", "S3"):
        # S2's SignalBar 'Mod 1' oscillator meter is LIVE content even in test mode (a sine of beatPhase,
        # SignalRegistry.cpp:65 / OscillatorSignal.h:59-71) -- mask it like the fps readout (fix round 2, 160 px @ max
        # delta 119 seen between BEFORE and AFTER captures otherwise).
        masks = (fps_mask(u), u["signalbar_rect"]) if s == "S2" else (fps_mask(u),)
        r3 = identity(b[s][0], a[s][0], geo, f"v1-{s}", masks=masks)
        what = {"S1": "default", "S2": "card", "S3": "many16"}[s]
        mask_desc = "the fps mask and the SignalBar (Mod 1 is live)" if s == "S2" else "the fps mask"
        (ok if r3[2] == 0 else no)(f"v1_identity_test_mode {s} ({what}): BEFORE vs AFTER outside {mask_desc} -- {k2(r3)}")
        live_meters_info("v1", s, b[s][0], a[s][0], geo, u, r3)
    # S4 (adoption I7): after POST /api/debug/ui_repaint_all (a whole-MainComponent pass) the waveform's corners are
    # still right. Scoped to the waveform rect + 8 px. The whole-window line is INFO: the window's FIRST display pass
    # draws the Files grid's folder emoji (<= 66/255), three TopBar slider thumbs (<= 29/255) and text (1/255) a little
    # differently from every later pass, in BOTH builds; a later whole-window pass (a resize, leaving binding / MIDI-learn
    # mode) replaces them in both, and main's 30 Hz union never covered those regions (fix round 1, J1 -- the BEFORE app
    # cannot be driven to such a pass, so BEFORE S4 still shows its first-pass pixels there).
    wr = u["waveform_rect"]
    around = (wr[0] - 8, wr[1] - 8, wr[2] + 16, wr[3] + 16)
    outside = [(0, 0, u["main_w"], around[1]), (0, around[1] + around[3], u["main_w"], u["main_h"] + 100),
               (0, around[1], around[0], around[3]), (around[0] + around[2], around[1], u["main_w"], around[3])]
    r3 = identity(b["S4"][0], a["S4"][0], geo, "v1-S4-waveform", masks=outside)
    (ok if r3[2] == 0 else no)(f"v1_identity_test_mode S4 (many16 after a full-window repaint, I7): the waveform rect + 8 px "
                               f"BEFORE vs AFTER -- {k2(r3)}")
    r3 = identity(b["S4"][0], a["S4"][0], geo, "v1-S4-window", masks=(fps_mask(u),))
    info(f"v1_identity_test_mode S4 whole window (BEFORE's first-pass pixels vs AFTER's full pass, J1): {k2(r3)}")


def row_v1n():
    """INFO: the comparator's noise floor -- the BEFORE app against itself (two launches, the same state sequence)."""
    if not APPB or not os.path.isdir(APPB):
        skip("v1n_noise_floor: no BEFORE app"); return
    if compilers() > 0:
        skip("v1n_noise_floor: TAINTED"); return
    geo, u = GEO.get("geo"), GEO.get("u")
    b1, _, _ = shoot_states(APPB, "v1n/before-1")
    b2, _, _ = shoot_states(APPB, "v1n/before-2")
    if geo is None:
        pid, _ = launch(APP, "v1n/geometry", test=True)
        time.sleep(CFG["v"]["settleS"])
        geo, u = geo_for(*capture("v1n-geometry"))
        quit_app()
    if geo is None:
        skip("v1n_noise_floor: no geometry"); return
    for s in ("S1", "S2", "S3"):
        if b1.get(s, (None,))[0] is None or b2.get(s, (None,))[0] is None:
            skip(f"v1n_noise_floor {s}: missing capture"); continue
        r3 = identity(b1[s][0], b2[s][0], geo, f"v1n-{s}", masks=(fps_mask(u),))
        info(f"v1n_noise_floor {s}: BEFORE vs BEFORE (two launches) outside the fps mask -- {k2(r3)}")


def row_v1b():
    if compilers() > 0:
        skip("v1b_native_vs_inpeer: TAINTED"); return
    pid, _ = launch(APP, "v1b/r1", env=("ADNA_TEMP_WAVE_FREEZE=1",), test=True)
    if pid is None:
        quit_app(); no("v1b_native_vs_inpeer: launch"); return
    setup("card"); time.sleep(2.0)
    n0 = capture("v1b-before-inject")
    feats = {"rms": 0.61, "peak": 0.83, "rmsDB": -4.3, "bpm": 128.0, "beatPhase": 0.37, "barPhase": 0.59}
    r = post("/api/inject_features", feats)
    time.sleep(2.0)
    n1 = capture("v1b-native-1")
    geo, u = geo_for(*n1)
    fb = post("/api/debug/ui_native_fallback", {"on": True})
    went = wait_mode("signalbar_mode", 2, 2.0) and wait_mode("waveform_mode", 2, 2.0)
    time.sleep(0.5)
    f = capture("v1b-inpeer")
    post("/api/debug/ui_native_fallback", {"on": False})
    back = wait_mode("signalbar_mode", 0, 2.0) and wait_mode("waveform_mode", 0, 2.0)
    time.sleep(0.5)
    n2 = capture("v1b-native-2")
    quit_app()
    print(f"    inject {r} fallback {fb} went {went} back {back}", flush=True)
    if geo is None or any(x[0] is None for x in (n0, n1, f, n2)):
        skip("v1b_native_vs_inpeer: no capture / geometry"); return
    sb, wf = u["signalbar_rect"], u["waveform_rect"]

    def region_only(keep):
        return [r_ for r_ in ((0, 0, u["main_w"], keep[1]), (0, keep[1] + keep[3], u["main_w"], u["main_h"]),
                              (0, keep[1], keep[0], keep[3]), (keep[0] + keep[2], keep[1], u["main_w"], keep[3]))]
    moved = compare(n0[0], n1[0], geo, "v1b-meters-moved", masks=region_only(sb))
    (ok if moved[0] > 0 else no)(f"v1b_native_vs_inpeer: the injected features moved the meters ({moved[0]} px changed "
                                 f"in the SignalBar)")
    x, y, w, h = geo.px(wf)
    patch = n1[0][y + 6:y + h - 6, x + 6:x + w - 6].reshape(-1, 3)
    wave_driven = len(np.unique(patch, axis=0)) > 12
    for name, rect in (("SignalBar", sb), ("waveform", wf)):
        if name == "waveform" and not wave_driven:
            skip("v1b_native_vs_inpeer waveform: not driven (the TEMPORARY freeze hook is absent from this build)")
            continue
        r3 = identity(n1[0], f[0], geo, f"v1b-{name}", masks=region_only(rect))
        (ok if went and r3[2] == 0 else no)(f"v1b_native_vs_inpeer {name}: native layer vs forced in-peer at a frozen "
                                            f"driven state -- {k2(r3)}")
        r3 = identity(n1[0], n2[0], geo, f"v1b-{name}-again", masks=region_only(rect))
        (ok if back and r3[2] == 0 else no)(f"v1b_native_vs_inpeer {name}: native again after the round trip -- {k2(r3)}")


def row_v2():
    if compilers() > 0:
        skip("v2_identity_fallback: TAINTED"); return
    v = CFG["v"]
    pid, _ = launch(APP, "v2/r1", test=True, env=("ADNA_UI_OVERLAY_WITNESS=1",))
    if pid is None:
        quit_app(); no("v2_identity_fallback: launch"); return
    setup("card"); time.sleep(v["settleS"])
    a = capture("v2-A")
    geo, u = geo_for(*a)
    if geo is None:
        quit_app(); skip("v2_identity_fallback: no capture / geometry"); return
    x, y = v["menuX"], v["menuY"]
    post("/api/debug/ui_test_menu", {"on": True, "x": x, "y": y, "kind": "panel"})
    went = wait_mode("signalbar_mode", 2, v["modePollS"])
    wf_mode = (ui() or {}).get("waveform_mode")
    time.sleep(0.3)
    b = capture("v2-B")
    post("/api/debug/ui_test_menu", {"on": False})
    back = wait_mode("signalbar_mode", 0, v["modePollS"])
    time.sleep(0.3)
    c = capture("v2-C")
    u2 = ui() or {}
    (ok if went and wf_mode == 0 else no)(f"v2_identity_fallback: the panel over the SignalBar -> signalbar_mode 2 within "
                                          f"{v['modePollS']} s ({went}), waveform stays native ({wf_mode})")
    (ok if back else no)(f"v2_identity_fallback: overlay gone -> signalbar_mode 0 within {v['modePollS']} s ({back})")
    R = (x - 10, y - 10, v["panelW"] + 20, v["panelH"] + 20)
    r3 = identity(a[0], b[0], geo, "v2-AB", masks=(fps_mask(u), R))
    n_in, cl_in, _, _ = compare(a[0], b[0], geo, "v2-AB-in-bar",
                                masks=(fps_mask(u), (0, 0, u["main_w"], u["signalbar_rect"][1]),
                                       (0, u["signalbar_rect"][1] + u["signalbar_rect"][3], u["main_w"], u["main_h"])))
    (ok if r3[2] == 0 and n_in > 0 else no)(f"v2_identity_fallback: A vs B outside the overlay's rect -- {k2(r3)}; the "
                                            f"overlay IS visible over the SignalBar ({n_in} px changed there)")
    r3 = identity(a[0], c[0], geo, "v2-AC", masks=(fps_mask(u),))
    (ok if r3[2] == 0 else no)(f"v2_identity_fallback: A vs C after the overlay closed -- {k2(r3)}")
    (ok if u2.get("ui_overlay_covered_frames") == 0 else no)(
        f"v2_identity_fallback: ui_overlay_covered_frames {u2.get('ui_overlay_covered_frames')} (== 0), restore frames "
        f"last {u2.get('ui_restore_frames_last')} max {u2.get('ui_restore_frames_max')}")
    f0 = u2.get("layer_fallbacks", 0)
    post("/api/debug/ui_test_menu", {"on": True, "x": x, "y": y, "kind": "menu"})
    t0 = time.monotonic(); saw2 = False
    while time.monotonic() - t0 < 2.0:
        uu = ui() or {}
        if uu.get("layer_fallbacks", 0) > f0:
            saw2 = True
        if saw2 and uu.get("signalbar_mode") == 0:
            break
        time.sleep(0.01)
    post("/api/debug/ui_test_menu", {"on": False})
    time.sleep(0.5)
    uu = ui() or {}
    (ok if saw2 and uu.get("signalbar_mode") == 0 and uu.get("ui_overlay_covered_frames") == 0 else no)(
        f"v2_identity_fallback (menu): a real parented PopupMenu over the bar -> fallback seen {saw2}, mode back "
        f"{uu.get('signalbar_mode')}, covered frames {uu.get('ui_overlay_covered_frames')}")
    quit_app()


def v2b_arm(tag, env):
    pid, _ = launch(APP, tag, env=env)
    if pid is None:
        quit_app(); return None
    setup("routine", bpm=CFG["g4"]["bpm"])
    time.sleep(CFG["settleS"])
    v = CFG["v"]
    u0 = ui() or {}
    cyc = []
    for k in range(CFG["v2b"]["cycles"]):
        for kind in ("panel", "menu"):
            post("/api/debug/ui_test_menu", {"on": True, "x": v["menuX"], "y": v["menuY"], "kind": kind})
            time.sleep(0.6)
            post("/api/debug/ui_test_menu", {"on": False})
            time.sleep(0.6)
            uu = ui() or {}
            cyc.append((kind, uu.get("ui_overlay_covered_frames"), uu.get("ui_restore_frames_last"),
                        uu.get("layer_fallbacks"), uu.get("signalbar_mode")))
    u1 = ui() or {}
    st_ = ((get("/api/routine/status") or {}).get("bank") or [{}])[0].get("state")
    quit_app()
    print(f"    {tag}: cycles (kind, covered, restore last, fallbacks, mode) {cyc} routine {st_}", flush=True)
    return {"covered": (u1.get("ui_overlay_covered_frames") or 0) - (u0.get("ui_overlay_covered_frames") or 0),
            "restore_max": u1.get("ui_restore_frames_max"), "fallbacks": (u1.get("layer_fallbacks") or 0)
            - (u0.get("layer_fallbacks") or 0), "mode_end": u1.get("signalbar_mode"), "routine": st_}


def row_v2b():
    if compilers() > 0:
        skip("v2b_fallback_frames: TAINTED"); return
    r = v2b_arm("v2b/fix", ("ADNA_UI_OVERLAY_WITNESS=1",))
    if r is None:
        no("v2b_fallback_frames: launch"); return
    lim = CFG["v2b"]["restoreFramesMax"]
    (ok if r["covered"] == 0 and r["fallbacks"] >= CFG["v2b"]["cycles"] and r["mode_end"] == 0 else no)(
        f"v2b_fallback_frames (I1, under g4's routine: {r['routine']}): {r['fallbacks']} fallbacks over "
        f"{CFG['v2b']['cycles']} panel + menu cycles, a layer showed over an overlay in {r['covered']} vblank(s) (== 0)")
    rm = r["restore_max"]
    (ok if rm is not None and 0 <= rm <= lim else no)(
        f"v2b_fallback_frames (I4): the layer is back <= {lim} vblanks after the overlay closed (max {rm})")
    t = v2b_arm("v2b/teeth", ("ADNA_UI_OVERLAY_WITNESS=1", "ADNA_UI_NATIVE_LAYERS_TEETH=asynchide"))
    if t is None:
        no("v2b_fallback_frames teeth: launch"); return
    (ok if t["covered"] > 0 else no)(f"v2b_fallback_frames teeth: the plan body's late hide (asynchide) is caught -- "
                                     f"covered {t['covered']} vblank(s) (> 0)")


def row_v3():
    if not APPB or not os.path.isdir(APPB):
        skip("v3_identity_production_masked: no BEFORE app"); return
    if compilers() > 0:
        skip("v3_identity_production_masked: TAINTED"); return
    shots, geo, u = {}, None, None
    for tag, app in (("before", APPB), ("after", APP)):
        pid, _ = launch(app, f"v3/{tag}")
        if pid is None:
            quit_app(); no(f"v3: launch {tag}"); return
        setup("card"); time.sleep(CFG["v"]["settleS"])
        shots[tag] = capture(f"v3-{tag}")
        if tag == "after":
            geo, u = geo_for(*shots[tag])
        quit_app()
    if geo is None or shots["before"][0] is None:
        skip("v3_identity_production_masked: no capture / geometry"); return
    t = u["topbar_rect"]
    r3 = identity(shots["before"][0], shots["after"][0], geo, "v3",
                  masks=(u["signalbar_rect"], u["waveform_rect"], (0, 0, u["main_w"], t[1] + t[3])))
    (ok if r3[2] == 0 else no)(f"v3_identity_production_masked: BEFORE vs AFTER outside the SignalBar / waveform / TopBar "
                               f"row -- {k2(r3)}")


def live_meters_info(row, tag, a, b, geo, u, res):
    """INFO only (never a verdict): when a test-mode identity line has violations, the same pair again with the SignalBar
    and the WaveformDisplay masked too. Their meters are live even in test mode -- 'Mod 1' is a sine of the snapshot's
    beatPhase (SignalRegistry.cpp:65, OscillatorSignal.h:59-71) and its fill edge moved by up to 119/255 between two
    captures of one launch (fix round 2) -- so this separates that class from the rest of the window."""
    if res[2] == 0:
        return
    r = identity(a, b, geo, f"{row}-{tag}-meters-masked", masks=(fps_mask(u), u["signalbar_rect"], u["waveform_rect"]))
    info(f"{row} {tag} with the live meters masked too (SignalBar, waveform): {k2(r)}")


def topbar_live_mask(u):
    """Production-only live content inside the TopBar: the beat wheel, the bar readout and the tempo + tracker-state
    labels (TopBar::resized: fixed offsets 326..512 pt from the TopBar's left), full TopBar height."""
    t, c = u["topbar_rect"], CFG["v3p"]
    return (t[0] + c["topbarLiveFromLeftPt"], t[1], c["topbarLiveWidthPt"], t[3])


def row_v3p():
    """Harmony ruling K3: PRODUCTION, card -- main's idle look vs the lane's idle look, masked only where production
    draws live content (fps readout, SignalBar, WaveformDisplay, the TopBar's beat / tempo readouts): K2 identity."""
    if not APPB or not os.path.isdir(APPB):
        skip("v3p_production_idle_identity: no BEFORE app"); return
    if compilers() > 0:
        skip("v3p_production_idle_identity: TAINTED"); return
    runs = int(CFG["v3p"]["runs"])
    res = []
    for k in range(1, runs + 1):
        shots, geo, u = {}, None, None
        for tag, app in (("before", APPB), ("after", APP)):
            pid, _ = launch(app, f"v3p/r{k}-{tag}")
            if pid is None:
                quit_app(); no(f"v3p_production_idle_identity: launch r{k} {tag}"); return
            setup("card"); time.sleep(CFG["v"]["settleS"])
            shots[tag] = capture(f"v3p-r{k}-{tag}")
            if tag == "after":
                geo, u = geo_for(*shots[tag])
            quit_app()
        if geo is None or shots["before"][0] is None:
            skip(f"v3p_production_idle_identity r{k}: no capture / geometry"); return
        r3 = identity(shots["before"][0], shots["after"][0], geo, f"v3p-r{k}",
                      masks=(fps_mask(u), u["signalbar_rect"], u["waveform_rect"], topbar_live_mask(u)))
        print(f"    v3p r{k}: BEFORE idle vs AFTER idle outside the live-content masks -- {k2(r3)}", flush=True)
        res.append(r3)
    (ok if all(r[2] == 0 for r in res) else no)(
        f"v3p_production_idle_identity (K3): production, card -- main's idle look vs this build's idle look outside the "
        f"fps / SignalBar / waveform / TopBar beat+tempo masks, {runs} launch pairs: "
        + " | ".join(f"r{i + 1} {k2(r)}" for i, r in enumerate(res)))


def row_v4():
    """Harmony ruling K3: test mode, card -- this build's idle look vs its look after POST /api/debug/ui_repaint_all (a
    whole-MainComponent pass), outside the fps mask: K2 identity, v4.runs launches."""
    # Harmony addendum 3 L2: v4 documents the first-display-pass class (identical in main); v3p is the identity gate.
    if compilers() > 0:
        skip("v4_full_pass_identity: TAINTED"); return
    runs = int(CFG["v4"]["runs"])
    res = []
    for k in range(1, runs + 1):
        pid, _ = launch(APP, f"v4/r{k}", test=True)
        if pid is None:
            quit_app(); no(f"v4_full_pass_identity: launch r{k}"); return
        setup("card"); time.sleep(CFG["v"]["settleS"])
        a = capture(f"v4-r{k}-idle")
        geo, u = geo_for(*a)
        rp = post("/api/debug/ui_repaint_all")
        time.sleep(1.0)
        b = capture(f"v4-r{k}-fullpass")
        quit_app()
        if geo is None or a[0] is None or b[0] is None:
            skip(f"v4_full_pass_identity r{k}: no capture / geometry"); return
        r3 = identity(a[0], b[0], geo, f"v4-r{k}", masks=(fps_mask(u),))
        print(f"    v4 r{k}: ui_repaint_all {rp} | idle vs after the full pass outside the fps mask -- {k2(r3)}", flush=True)
        live_meters_info("v4", f"r{k}", a[0], b[0], geo, u, r3)
        res.append(r3)
    info(
        f"v4_full_pass_identity (INFO, Harmony addendum 3 L2): test mode, card -- idle look vs after a whole-MainComponent "
        f"pass outside the fps mask, {runs} launches: " + " | ".join(f"r{i + 1} {k2(r)}" for i, r in enumerate(res)))


# ---------------------------------------------------------------- main
print(f"rows: {','.join(ROWS)} | app {APP} | before {APPB or '-'} | {load_avg()}", flush=True)
for row in ROWS:
    print(f"== {row} ({time.strftime('%H:%M:%S')}, {load_avg()})", flush=True)
    if row == "c0_preflight":
        row_c0()
    elif row == "i1_idle_card":
        runs = row_i(row, "card", "IDLE-HB-card")
        row_g1_g3(runs)
    elif row == "i2_idle_many16":
        row_i(row, "many16", "IDLE-HB-many16")
    elif row == "g4_routine":
        runs = run_arm(row, APP, "routine", N, bpm=CFG["g4"]["bpm"])
        gate(row, runs, "a loop routine on 3 layers, adoption I3", cpu_gate=False)
        # Ruling J2: g4's main-thread CPU is an INFO line, BEFORE (main) vs AFTER, both measured here. The lever (the
        # SignalBar repaints only the strips whose value changed; the routine's band / V-fader repaints leave the
        # TopBar pass) is filed, not built.
        before = run_arm("g4_routine_before", APPB, "routine", N, bpm=CFG["g4"]["bpm"]) \
            if APPB and os.path.isdir(APPB) else None
        med = lambda rs: fmt(st.median([r["cpu_main"] for r in rs if r["cpu_main"] is not None])) if rs else "-"
        rng = lambda rs: ("[" + fmt(min(r["cpu_main"] for r in rs)) + "-" + fmt(max(r["cpu_main"] for r in rs)) + "]") \
            if rs and all(r["cpu_main"] is not None for r in rs) else ""
        info(f"g4_routine CPU (INFO, ruling J2): main-thread CPU median BEFORE {med(before)} ms/s {rng(before)} | AFTER "
             f"{med(runs)} ms/s {rng(runs)} (i1's {CFG['cpuMainMsPerS']} ms/s is not applied to g4)")
        if runs and "band_repaints" in runs[0]:
            info("g4_routine: band repaints/s " + ", ".join(f"{r['band_repaints']:.1f}" for r in runs)
                 + " | strip transport repaints/s " + ", ".join(f"{r['strip_repaints']:.1f}" for r in runs)
                 + " | MainComponent paints/s " + ", ".join(f"{r['main_paints']:.1f}" for r in runs))
    elif row == "g2_strip_playhead":
        row_g2()
    elif row == "g5_driven":
        row_g5()
    elif row == "x1_within_build_off":
        row_x1()
    elif row == "v0_capture_teeth":
        row_v0()
    elif row == "v1_identity_test_mode":
        row_v1()
    elif row == "v1n_noise_floor":
        row_v1n()
    elif row == "v1b_native_vs_inpeer":
        row_v1b()
    elif row == "v2_identity_fallback":
        row_v2()
    elif row == "v2b_fallback_frames":
        row_v2b()
    elif row == "v3_identity_production_masked":
        row_v3()
    elif row == "v3p_production_idle_identity":
        row_v3p()
    elif row == "v4_full_pass_identity":
        row_v4()
quit_app()
json.dump({"pass": PASS, "fail": FAIL, "skip": SKIP, "summary": SUMMARY}, open(os.path.join(OUT, "summary.json"), "w"),
          indent=1)
print(f"\n{PASS} PASS / {FAIL} FAIL / {SKIP} SKIP ({time.strftime('%H:%M:%S')}, {load_avg()})", flush=True)
sys.exit(1 if FAIL else (2 if SKIP else 0))
