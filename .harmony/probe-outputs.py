#!/usr/bin/env python3
"""probe-outputs.py -- REST/pixel half of .harmony/probe-outputs.sh (s-rta-0927 outputs-c1 = plan5 slice C1: the
composition canvas is copied once per frame into shared IOSurface frames that every output window presents;
.harmony/.reports/s-rta-0926b/plan5-final.md section 10.2).

SCREEN-SAFE BY CONSTRUCTION: no row opens an output window. The frame path is exercised offscreen -- the main
renderer publishes with the tap forced on (8080 /api/set_output_tap, test mode only) and 8080 /api/output_probe
presents the newest frame through a PRIVATE CGL context of the app (no window) with the SAME presentSharedFrame()
the output window uses, then writes a PNG (alpha forced to 255 = what a display shows). Every PNG is decoded with
PIL+numpy; every probe/capture response is checked (a failed one is a FAIL). The output dir is fresh per run.

usage: probe-outputs.py <root> <fresh-outdir> <media-dir> [row,row,...]
rows: o_probe_matches_canvas o_probe_portrait_target o_probe_tracks_change o_probe_survives_resolution_change
      o_probe_repeat_stable o_state_outputs o_state_displays o_restore_empty o_poll_idle o_no_window_opened o_tap_cost

Metric: d(X, Y) = mean |X - Y| over RGB, 0..255. Fixtures A = media/P16_01_baseline.png, B =
media/P16_02_Screen_Split_2x2.png (d(A, B) = 29.4 at 1920x1080). Composition: 1920x1080, deck 0 L0 (Opaque, no
effects) col0 = A, col1 = B. Fixture B is ITSELF a 2x2 grid of four copies on a (15,15,15) frame (a Screen Split
still): a frame of B shows that grid and grey edge at every canvas size -- compare a B frame only with a B reference.

o_probe_matches_canvas (RED on the base: 404): before the tap is ever on, output_probe answers 409 (nothing
  published). Trigger col0; 1 s; f0 = 7070 render_frame (1920x1080); set_output_tap on; 0.3 s; output_probe
  1920x1080 -> a 1920x1080 PNG with d(probe, f0) <= matchTol (2).
o_probe_portrait_target (RED): output_probe 1080x1920 -> the picture sits in fitCanvas(1920,1080,1080,1920) =
  {0,656,1080,607} (GL, bottom-up) = PNG rows 657..1263: every other row has mean RGB <= barTol (2), and
  d(inner, f0 resized to 1080x607 BILINEAR) <= innerTol (6).
o_probe_tracks_change (RED): trigger col1 (B); 0.5 s; f_B = render_frame; output_probe -> d(probe, f0) > 20 AND
  d(probe, f_B) <= 2 (LIVE, not a stale slot).
o_probe_survives_resolution_change (RED): trigger col0 (A; the previous row left B showing); 1 s; 8080
  set_composition_params 1280x720; 0.4 s; f_720 = render_frame (must be 1280x720, and d(f_720, fixture A stretched
  to 1280x720 BILINEAR) <= contentTol (6): one full-bleed A, like with like); output_probe 1920x1080 -> response
  canvas 1280x720 and gen > the previous probe's gen; d(probe, f_720 resized to 1920x1080 BILINEAR) <= 6.
  Restores 1920x1080.
o_probe_repeat_stable (guard/soak): col0 again; 60 output_probes 1920x1080 back to back while the main renders
  static A: every PNG decodes, none blank, all d <= 2 vs a fresh f0 (the cross-thread/cross-context race soak).
  Prints the elapsed time.
o_state_outputs (RED): 8080 and 7070 /api/state carry outputs {live == 0, tap == true, frame_gen >= 1,
  frame_serial >= 1, canvas_w == 1920, canvas_h == 1080}; after set_output_tap off, tap == false.
o_state_displays (RED on a pre-C2 app: no `displays`; s-rta-0927 outputs-c2 = plan5 C2): 8080 and 7070
  /api/state.outputs.displays is the Output menu's display list (OutputManager, built from buildOutputMenu):
  non-empty, exactly one `main`, index 0..n-1, every entry has x/y/w/h/scale/main/live/label, nothing live
  (this probe never opens an output; the count of live entries == outputs.live == 0), label == "Display <i+1>
  (<w>x<h>[, main])". Independent oracle (CoreGraphics, not JUCE): the entry count == CGGetActiveDisplayList's and
  the main entry's w x h == CGDisplayBounds(CGMainDisplayID()) in points. Then the level probe's OWN discovery
  (tests/visual/test_output_window_level.py `_pick_main_fullscreen_item`, imported -- never run: its main() is
  replaced by a raise before use) must pick the main entry's label: the Boris-supervised level probe can find the
  item it opens.
o_restore_empty (RED on a pre-C3 app: no settings override, no outputs.manager, no route; s-rta-0927 outputs-c3 =
  plan5 C3): the app reads its settings from the run's scratch file (err.log carries the test-mode
  AUDIODNA_SETTINGS_FILE line -- never the user's real settings.json); 8080 /api/state.outputs.manager reports
  saved == the scratch file's target count (0 when absent) and restorable == 0; 8080 POST /api/output_restore_last
  (test mode; it REFUSES with 409 whenever restoring could open a window, and the app re-checks on the message
  thread) runs the "Restore Last Outputs" menu action's own handler: restore_calls advances by exactly 1, outputs.live
  stays 0 on 8080 and 7070, saved and settings_writes are unchanged, the scratch settings file is byte-identical
  (sha256, or still absent), and the window sampler has seen no Output window. NEVER run with a seeded target that
  matches a connected display (the route would refuse; no row may ever open a window).
o_poll_idle (RED on a pre-C3 app: no set_output_poll / outputs.manager): the 30 Hz display poll with ZERO outputs
  changes nothing. With the tap off and no compiler running (load average printed): 5 s with the poll paused
  (8080 set_output_poll false), then 5 s with it running. Poll ticks: 0 while paused, >= minPollHz per second while
  running (the poll is the first line of every UI timer tick); reconciles and settings writes do not move;
  outputs.live == 0 on every 7070 and 8080 sample; the displays list never changes; and 7070 frame_time_ms (mean)
  with the poll running is within frameTol (0.3 ms) of the mean with it paused.
o_no_window_opened (guard, THE LAW): a Quartz window-list sampler (every 0.25 s for the whole run) never sees an
  Audio-DNA window named like the Output window, and never more than ONE on-screen Audio-DNA window at layer 0
  (the main window) -- the second check needs no window-name permission. The .sh adds the after-quit census.
o_tap_cost (REPORT, never FAILs; OPT-IN -- runs only when named in the row list): at 1920x1080 and 3840x2160, 7070
  /api/state frame_time_ms / peak_frame_time_ms / gpu_time_ms sampled every 0.25 s for 5 s with the tap off, then
  on. Waits until no clang/clang++ is running and prints the load average next to the numbers.

Calibration / RED lines: .harmony/.reports/s-rta-0927/outputs-c1.md.
"""
import json, os, subprocess, sys, threading, time

import numpy as np
import requests
from PIL import Image

A = "http://127.0.0.1:7070"
T8 = "http://[::1]:8080"
ROOT, OUT, MEDIA = sys.argv[1], sys.argv[2], sys.argv[3]
ONLY = set(sys.argv[4].split(",")) if len(sys.argv) > 4 and sys.argv[4] else None
FIX = json.load(open(os.path.join(ROOT, ".harmony", "probe-outputs.json")))
IMG_A = FIX["imageA"].replace("@MEDIA@", MEDIA)
IMG_B = FIX["imageB"].replace("@MEDIA@", MEDIA)
CW, CH = FIX["canvas"]
TOL = float(FIX["matchTol"])
PASS = FAIL = 0
S = requests.Session()
# One fresh connection per request, never a pooled keep-alive one (s-rta-0927 c1-state-fix). The app's cpp-httplib
# server closes a connection idle for 5 s (CPPHTTPLIB_KEEPALIVE_TIMEOUT_SECOND) -- shutdown, then it drains and
# DISCARDS whatever request arrives in that instant. A reused connection can then fail with RemoteDisconnected
# (~1 run in 3, on the pre-C1 app too), and requests never retries it. Evidence:
# .harmony/.reports/s-rta-0927/c1-state-fix.md.
S.headers["Connection"] = "close"
KEEP = {}


def ok(msg):
    global PASS; PASS += 1; print(f"PASS  {msg}", flush=True)


def no(msg):
    global FAIL; FAIL += 1; print(f"FAIL  {msg}", flush=True)


# ------------------------------------------------------------------ window sampler (o_no_window_opened)
class WindowWatch(threading.Thread):
    def __init__(self):
        super().__init__(daemon=True)
        self.stop_ev = threading.Event()
        self.samples = 0
        self.max_layer0 = 0
        self.named_output = []
        self.names_seen = set()
        self.err = None

    def run(self):
        try:
            import Quartz
        except Exception as e:  # noqa: BLE001
            self.err = f"no pyobjc Quartz: {e}"
            return
        want = FIX["outputWindowName"]
        while not self.stop_ev.is_set():
            wl = Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionOnScreenOnly, Quartz.kCGNullWindowID)
            n0 = 0
            for w in wl:
                if str(w.get("kCGWindowOwnerName", "")) != "Audio-DNA":
                    continue
                name = str(w.get("kCGWindowName", "") or "")
                if name:
                    self.names_seen.add(name)
                if name == want or "Output" in name:
                    self.named_output.append(name)
                b = w.get("kCGWindowBounds", {})
                if int(w.get("kCGWindowLayer", 0)) == 0 and float(b.get("Width", 0)) >= 50 and float(b.get("Height", 0)) >= 50:
                    n0 += 1
            self.max_layer0 = max(self.max_layer0, n0)
            self.samples += 1
            self.stop_ev.wait(0.25)


WATCH = WindowWatch()


# ------------------------------------------------------------------ helpers
def fx(e):
    return {"name": e[0], "enabled": True, "bypassed": False, "dryWet": 1.0, "params": list(e[1:])}


def load(tag, size=(CW, CH)):
    clips = [{"name": "cA", "id": 1, "mediaType": 1, "mediaFile": IMG_A, "effects": []},
             {"name": "cB", "id": 2, "mediaType": 1, "mediaFile": IMG_B, "effects": []}]
    layer = {"name": "L0", "id": 0, "opacity": 1.0, "visible": True, "blendMode": 0, "type": 0,
             "transitionSpeed": 0.0, "layerEffects": [], "clips": clips}
    comp = {"name": "outputs-" + tag, "activeDeckIndex": 0, "masterOpacity": 1.0, "globalTransitionSpeed": 0.0,
            "outputWidth": int(size[0]), "outputHeight": int(size[1]),
            "decks": [{"name": "D0", "id": 0, "numColumns": 2, "layers": [layer]}]}
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


def trig(col):
    S.post(A + "/api/trigger_clip", json={"layer": 0, "column": col}, timeout=6)


def decode(p):
    return np.asarray(Image.open(p).convert("RGBA")).astype(float)


def fresh(p):
    if os.path.exists(p):
        os.remove(p)
    return p


def cap(name):
    """7070 render_frame into the fresh dir (never decode a PNG this call did not write)."""
    p = fresh(os.path.join(OUT, name + ".png"))
    t0 = time.time()
    try:
        body = S.post(A + "/api/render_frame", json={"output_path": p}, timeout=30).json()
    except Exception as e:  # noqa: BLE001
        no(f"render_frame {name}: {e}")
        return None
    if not body.get("ok") or not os.path.isfile(p) or os.path.getmtime(p) < t0 - 0.01:
        no(f"render_frame {name}: response {body} / file written={os.path.isfile(p)}")
        return None
    return decode(p)


def tap(on):
    try:
        r = S.post(T8 + "/api/set_output_tap", json={"enabled": bool(on)}, timeout=6)
        return r.status_code, (r.json() if r.headers.get("content-type", "").startswith("application/json") else r.text[:120])
    except Exception as e:  # noqa: BLE001
        return -1, str(e)


def probe(name, w, h, expect_ok=True, row="", quiet=False):
    """8080 output_probe -> (frame array | None, response dict | None, http status). quiet: the caller reports."""
    def fail(m):
        if not quiet:
            no(f"{row}: {m}")
    p = fresh(os.path.join(OUT, name + ".png"))
    t0 = time.time()
    try:
        r = S.post(T8 + "/api/output_probe", json={"width": int(w), "height": int(h), "output_path": p}, timeout=30)
    except Exception as e:  # noqa: BLE001
        fail(f"output_probe {name}: {e}")
        return None, None, -1
    try:
        body = r.json()
    except Exception:  # noqa: BLE001
        body = {"raw": r.text[:120]}
    if not expect_ok:
        return None, body, r.status_code
    if r.status_code != 200 or not body.get("ok"):
        fail(f"output_probe {name}: HTTP {r.status_code} {str(body)[:160]}")
        return None, body, r.status_code
    if not os.path.isfile(p) or os.path.getmtime(p) < t0 - 0.01:
        fail(f"output_probe {name}: ok:true but no fresh file at {p}")
        return None, body, r.status_code
    return decode(p), body, r.status_code


def size_of(f):
    return (int(f.shape[1]), int(f.shape[0]))


def resized(img, size):
    im = Image.fromarray(np.clip(img, 0, 255).astype(np.uint8), "RGBA")
    return np.asarray(im.resize(size, Image.BILINEAR)).astype(float)


def d(x, y):
    return float(np.abs(x[..., :3] - y[..., :3]).mean())


def nonblank(a):
    return float(a[..., :3].std()) >= 3.0


def fit_canvas(cw, ch, vw, vh):
    """RenderGeometry::fitCanvas (GL coords, y from the bottom)."""
    if cw <= 0 or ch <= 0 or vw <= 0 or vh <= 0:
        return (0, 0, 0, 0)
    if vw * ch <= vh * cw:
        w, h = vw, vw * ch // cw
    else:
        h, w = vh, vh * cw // ch
    return ((vw - w) // 2, (vh - h) // 2, w, h)


def state(base):
    try:
        return S.get(base + "/api/state", timeout=6).json()
    except Exception as e:  # noqa: BLE001
        no(f"{base}/api/state: {e}")
        return None


def loadavg():
    return "load avg %.2f %.2f %.2f" % os.getloadavg()


def wait_no_compiler(tag, limit_s=3600):
    t0 = time.time()
    while True:
        # macOS pgrep takes a regex: a bare "clang++" is an invalid pattern (error, empty stdout = never "busy").
        busy = [n for n in ("clang", r"clang\+\+") if subprocess.run(["pgrep", "-x", n], capture_output=True).stdout.strip()]
        if not busy:
            return True
        if time.time() - t0 > limit_s:
            print(f"      {tag}: a compiler ({busy}) kept running for {limit_s} s -- perf NOT measured", flush=True)
            return False
        print(f"      {tag}: waiting for {busy} to finish ({loadavg()})", flush=True)
        time.sleep(20)


# ------------------------------------------------------------------ rows
def o_probe_matches_canvas():
    if not load("matches"):
        return
    code, body = tap(False)
    _, pb, st = probe("probe_before_tap", CW, CH, expect_ok=False)
    (ok if st == 409 and isinstance(pb, dict) and pb.get("ok") is False else no)(
        f"o_probe_matches_canvas: before any publish output_probe answers 409 ok:false (HTTP {st}, {str(pb)[:100]})")
    trig(0); time.sleep(float(FIX["settleAfterTrigger"]))
    f0 = cap("f0_render_frame")
    if f0 is None:
        return
    KEEP["f0"] = f0
    code, body = tap(True)
    if code != 200:
        no(f"o_probe_matches_canvas: set_output_tap on -> HTTP {code} {str(body)[:120]}")
        return
    time.sleep(float(FIX["settleAfterTap"]))
    pf, pb, st = probe("probe_1920x1080", CW, CH, row="o_probe_matches_canvas")
    if pf is None:
        return
    KEEP["gen"] = int(pb.get("gen", 0))
    sz = size_of(pf)
    de = d(pf, f0) if sz == size_of(f0) else 999.0
    (ok if sz == (CW, CH) and de <= TOL and nonblank(pf) else no)(
        f"o_probe_matches_canvas: output_probe {sz[0]}x{sz[1]} == render_frame {size_of(f0)[0]}x{size_of(f0)[1]} "
        f"(d {de:.3f} <= {TOL}; gen {pb.get('gen')} serial {pb.get('serial')} slot {pb.get('slot')} canvas "
        f"{pb.get('canvas_w')}x{pb.get('canvas_h')})")


def o_probe_portrait_target():
    f0 = KEEP.get("f0")
    if f0 is None:
        no("o_probe_portrait_target: no f0 (o_probe_matches_canvas did not produce one)")
        return
    W, H = FIX["portrait"]["size"]
    pf, pb, st = probe("probe_portrait_1080x1920", W, H, row="o_probe_portrait_target")
    if pf is None:
        return
    x, y, w, h = fit_canvas(CW, CH, W, H)
    top = H - (y + h)   # PNG rows are top-down
    inner = pf[top:top + h, x:x + w]
    bars = np.concatenate([pf[:top], pf[top + h:]], axis=0)
    bar_mean = float(bars[..., :3].mean()) if bars.size else 0.0
    di = d(inner, resized(f0, (w, h)))
    (ok if size_of(pf) == (W, H) and bar_mean <= float(FIX["portrait"]["barTol"])
     and di <= float(FIX["portrait"]["innerTol"]) else no)(
        f"o_probe_portrait_target: {size_of(pf)[0]}x{size_of(pf)[1]}, picture rect {w}x{h} at PNG row {top} "
        f"(fitCanvas {x},{y},{w},{h}); bars mean RGB {bar_mean:.3f} <= {FIX['portrait']['barTol']}; "
        f"d(inner, f0 resized) {di:.3f} <= {FIX['portrait']['innerTol']}")


def o_probe_tracks_change():
    f0 = KEEP.get("f0")
    if f0 is None:
        no("o_probe_tracks_change: no f0")
        return
    trig(1); time.sleep(float(FIX["change"]["settle"]))
    fB = cap("fB_render_frame")
    pf, pb, st = probe("probe_after_B", CW, CH, row="o_probe_tracks_change")
    if pf is None or fB is None:
        return
    d0, dB = d(pf, f0), d(pf, fB)
    (ok if d0 > float(FIX["change"]["minDiff"]) and dB <= TOL else no)(
        f"o_probe_tracks_change: after col1 = B the probe follows: d(probe, f0) {d0:.3f} > {FIX['change']['minDiff']} "
        f"and d(probe, f_B) {dB:.3f} <= {TOL} (serial {pb.get('serial')})")


def o_probe_survives_resolution_change():
    gen0 = KEEP.get("gen")
    RW, RH = FIX["resolution"]["to"]
    trig(0); time.sleep(float(FIX["settleAfterTrigger"]))   # A, not the B the previous row left showing
    r = S.post(T8 + "/api/set_composition_params", json={"outputWidth": RW, "outputHeight": RH}, timeout=6)
    if not r.ok:
        no(f"o_probe_survives_resolution_change: set_composition_params -> HTTP {r.status_code} {r.text[:120]}")
        return
    time.sleep(float(FIX["resolution"]["settle"]))
    f720 = cap("f720_render_frame")
    pf, pb, st = probe("probe_1920x1080_of_720p", CW, CH, row="o_probe_survives_resolution_change")
    S.post(T8 + "/api/set_composition_params", json={"outputWidth": CW, "outputHeight": CH}, timeout=6)
    if pf is None or f720 is None:
        return
    de = d(pf, resized(f720, (CW, CH)))
    refA = resized(decode(IMG_A), (RW, RH))
    da = d(f720, refA) if size_of(f720) == (RW, RH) else 999.0
    ctol = float(FIX["resolution"]["contentTol"])
    gen = int(pb.get("gen", 0))
    (ok if size_of(f720) == (RW, RH) and (pb.get("canvas_w"), pb.get("canvas_h")) == (RW, RH)
     and gen0 is not None and gen > gen0 and de <= float(FIX["resolution"]["tol"]) and da <= ctol else no)(
        f"o_probe_survives_resolution_change: render_frame {size_of(f720)[0]}x{size_of(f720)[1]} of col0 = A, "
        f"d(f_720, fixture A {os.path.basename(IMG_A)} stretched to {RW}x{RH}) {da:.3f} <= {ctol}; probe canvas "
        f"{pb.get('canvas_w')}x{pb.get('canvas_h')}, gen {gen0} -> {gen}, d(probe, f_720 upscaled) {de:.3f} <= "
        f"{FIX['resolution']['tol']}")


def o_probe_repeat_stable():
    trig(0); time.sleep(1.0)
    ref = cap("f0b_render_frame")
    if ref is None:
        return
    W, H = FIX["soak"]["size"]
    n = int(FIX["soak"]["count"])
    worst, bad, t0 = 0.0, [], time.time()
    for i in range(n):
        pf, pb, st = probe(f"soak_{i:02d}", W, H, quiet=True)
        if pf is None:
            bad.append(f"#{i} no frame (HTTP {st})")
            continue
        di = d(pf, ref)
        worst = max(worst, di)
        if not nonblank(pf) or di > TOL:
            bad.append(f"#{i} d {di:.2f} nonblank {nonblank(pf)}")
        if i not in (0, n - 1):
            os.remove(os.path.join(OUT, f"soak_{i:02d}.png"))   # keep the first and the last only
    el = time.time() - t0
    (ok if not bad else no)(
        f"o_probe_repeat_stable: {n} probes in {el:.2f} s, every PNG decoded, none blank, worst d {worst:.3f} "
        f"<= {TOL} ({len(bad)} bad: {bad[:4]})")


def o_state_outputs():
    for base in (T8, A):
        s = state(base)
        o = (s or {}).get("outputs")
        good = isinstance(o, dict) and o.get("live") == 0 and o.get("tap") is True and int(o.get("frame_gen", 0)) >= 1 \
            and int(o.get("frame_serial", 0)) >= 1 and (o.get("canvas_w"), o.get("canvas_h")) == (CW, CH)
        (ok if good else no)(f"o_state_outputs: {base} /api/state.outputs = {o}")
    tap(False)
    time.sleep(0.2)
    o = (state(T8) or {}).get("outputs")
    (ok if isinstance(o, dict) and o.get("tap") is False else no)(
        f"o_state_outputs: after set_output_tap off, 8080 outputs.tap == false ({o})")
    tap(True)


def expected_display_label(i, x):
    return f"Display {i + 1} ({x.get('w')}x{x.get('h')}" + (", main)" if x.get("main") is True else ")")


def level_probe_picker():
    """The Boris-supervised level probe's own item picker, imported (never run: main() is replaced first)."""
    sys.dont_write_bytecode = True   # no __pycache__ in tests/visual
    vis = os.path.join(ROOT, "tests", "visual")
    if vis not in sys.path:
        sys.path.insert(0, vis)
    import importlib
    L = importlib.import_module("test_output_window_level")

    def _never(*_a, **_k):
        raise RuntimeError("probe-outputs never runs the level probe (it opens the Output window)")
    L.main = L._spawn_app = L._click_output_item = L._open_output_window = L._osascript = _never
    return L


def o_state_displays():
    try:
        import Quartz
        _err, _ids, n = Quartz.CGGetActiveDisplayList(32, None, None)
        mb = Quartz.CGDisplayBounds(Quartz.CGMainDisplayID())
        q_count, q_main = int(n), (int(mb.size.width), int(mb.size.height))
    except Exception as e:  # noqa: BLE001
        q_count, q_main = None, None
        no(f"o_state_displays: CoreGraphics display list unavailable ({e})")
    main_label = None
    for base in (T8, A):
        o = (state(base) or {}).get("outputs") or {}
        d = o.get("displays")
        bad = []
        if not isinstance(d, list) or not d:
            bad.append(f"displays missing or empty ({d!r})")
            d = []
        mains = [x for x in d if isinstance(x, dict) and x.get("main") is True]
        if d and len(mains) != 1:
            bad.append(f"{len(mains)} entries flagged main (want 1)")
        for i, x in enumerate(d):
            miss = [k for k in ("index", "x", "y", "w", "h", "scale", "main", "live", "label") if k not in x]
            if miss:
                bad.append(f"entry {i} lacks {miss}")
                continue
            if x["index"] != i:
                bad.append(f"entry {i} has index {x['index']}")
            if x["live"] is not False:
                bad.append(f"entry {i} live={x['live']} (this probe never opens an output)")
            if x["label"] != expected_display_label(i, x):
                bad.append(f"entry {i} label {x['label']!r} != {expected_display_label(i, x)!r}")
        live_n = sum(1 for x in d if x.get("live") is True)
        if d and live_n != o.get("live"):
            bad.append(f"{live_n} live entries != outputs.live {o.get('live')}")
        if d and q_count is not None and len(d) != q_count:
            bad.append(f"{len(d)} entries != CGGetActiveDisplayList {q_count}")
        if mains and q_main is not None and (mains[0].get("w"), mains[0].get("h")) != q_main:
            bad.append(f"main entry {mains[0].get('w')}x{mains[0].get('h')} != CGDisplayBounds(main) {q_main[0]}x{q_main[1]}")
        if mains:
            main_label = mains[0].get("label")
        (ok if not bad else no)(
            f"o_state_displays: {base} outputs.displays = {d} (CoreGraphics: {q_count} display(s), main "
            f"{q_main}){' -- ' + '; '.join(bad) if bad else ''}")
    if main_label is None:
        no("o_state_displays: no main display label to hand to the level probe's picker")
        return
    try:
        L = level_probe_picker()
        picked = L._pick_main_fullscreen_item([x for x in [main_label] if x])
        (ok if picked == main_label else no)(
            f"o_state_displays: the level probe's _pick_main_fullscreen_item (FULLSCREEN_PREFIX {L.FULLSCREEN_PREFIX!r}, "
            f"MAIN_SUFFIX {L.MAIN_SUFFIX!r}) picks {picked!r} from the app's labels; close item DISABLED_ITEM "
            f"{L.DISABLED_ITEM!r}")
    except Exception as e:  # noqa: BLE001 -- ProbeBlocked = the level probe would REFUSE
        no(f"o_state_displays: the level probe's picker REFUSES the app's main label {main_label!r}: "
           f"{type(e).__name__}: {e}")


def sha256_of(path):
    import hashlib
    if not path or not os.path.isfile(path):
        return None
    with open(path, "rb") as f:
        return hashlib.sha256(f.read()).hexdigest()


def manager():
    """8080 /api/state.outputs.manager (test mode, plan5 C3) -> (dict | None, outputs dict)."""
    o = (state(T8) or {}).get("outputs") or {}
    m = o.get("manager")
    return (m if isinstance(m, dict) else None), o


def o_restore_empty():
    sf = os.environ.get("OUTP_SETTINGS_FILE", "")
    before = sha256_of(sf)
    try:
        err = open(os.path.join(OUT, "err.log"), errors="replace").read()
    except OSError:
        err = ""
    honoured = bool(sf) and f"[Settings] test mode: AUDIODNA_SETTINGS_FILE = {sf}" in err
    (ok if honoured else no)(
        f"o_restore_empty: the app reads and writes its settings in the run's scratch file {sf} "
        f"({'err.log: AUDIODNA_SETTINGS_FILE honoured' if honoured else 'NOT honoured: no test-mode settings line in err.log'})")
    seeded = 0
    if before is not None:
        try:
            seeded = len(((json.load(open(sf)).get("outputs") or {}).get("targets")) or [])
        except Exception:  # noqa: BLE001
            seeded = -1
    m0, o0 = manager()
    if m0 is None:
        no(f"o_restore_empty: 8080 /api/state.outputs.manager missing (outputs = {o0})")
        return
    (ok if m0.get("saved") == seeded and m0.get("restorable") == 0 and o0.get("live") == 0 else no)(
        f"o_restore_empty: before: manager {m0}, outputs.live {o0.get('live')} (scratch settings "
        f"{'absent' if before is None else 'seeded, ' + str(seeded) + ' target(s)'}: want saved == {seeded}, "
        f"restorable == 0, live == 0)")
    try:
        r = S.post(T8 + "/api/output_restore_last", json={}, timeout=6)
        code = r.status_code
        body = r.json() if r.headers.get("content-type", "").startswith("application/json") else r.text[:120]
    except Exception as e:  # noqa: BLE001
        code, body = -1, str(e)
    if code != 200:
        no(f"o_restore_empty: POST /api/output_restore_last -> HTTP {code} {str(body)[:160]}")
        return
    want = int(m0.get("restore_calls", 0)) + 1
    t_end = time.time() + 3.0
    m1 = m0
    while time.time() < t_end:
        m1, _ = manager()
        if m1 is not None and int(m1.get("restore_calls", 0)) >= want:
            break
        time.sleep(0.1)
    time.sleep(0.5)   # a window (there must be none) would be on screen by now
    m1, o1 = manager()
    o7 = (state(A) or {}).get("outputs") or {}
    after = sha256_of(sf)
    good = m1 is not None and int(m1.get("restore_calls", 0)) == want and o1.get("live") == 0 and o7.get("live") == 0 \
        and m1.get("saved") == m0.get("saved") and m1.get("settings_writes") == m0.get("settings_writes") \
        and m1.get("restorable") == 0 and after == before and not WATCH.named_output and WATCH.max_layer0 <= 1
    (ok if good else no)(
        f"o_restore_empty: Restore Last Outputs ran (HTTP {code} {body}; restore_calls {m0.get('restore_calls')} -> "
        f"{(m1 or {}).get('restore_calls')}) and opened NOTHING: outputs.live 8080 {o1.get('live')} / 7070 "
        f"{o7.get('live')}, saved {m0.get('saved')} -> {(m1 or {}).get('saved')}, settings_writes "
        f"{m0.get('settings_writes')} -> {(m1 or {}).get('settings_writes')}, scratch settings "
        f"{'absent -> absent' if before is None and after is None else (before or 'absent')[:12] + ' -> ' + (after or 'absent')[:12]}, "
        f"Output-named windows so far {sorted(set(WATCH.named_output))}, max layer-0 windows {WATCH.max_layer0}")


def o_poll_idle():
    cfg = FIX["pollIdle"]
    m0, o0 = manager()
    if m0 is None:
        no(f"o_poll_idle: 8080 /api/state.outputs.manager missing (outputs = {o0})")
        return

    def poll(on):
        try:
            return S.post(T8 + "/api/set_output_poll", json={"enabled": bool(on)}, timeout=6).status_code
        except Exception:  # noqa: BLE001
            return -1

    tap(False)
    code = poll(False)
    if code != 200:
        no(f"o_poll_idle: POST /api/set_output_poll -> HTTP {code}")
        return
    if not wait_no_compiler("o_poll_idle", int(cfg["compilerWaitS"])):
        no("o_poll_idle: frame time NOT measured (a compiler kept running)")
        poll(True)
        return
    print(f"      o_poll_idle: {loadavg()}", flush=True)

    def window():
        time.sleep(float(cfg["settle"]))
        ma, _ = manager()
        ft, lives, disp = [], set(), set()
        t0 = time.time()
        while time.time() - t0 < float(cfg["seconds"]):
            s7 = state(A) or {}
            ft.append(float(s7.get("frame_time_ms", 0)))
            lives.add((s7.get("outputs") or {}).get("live"))
            o8 = (state(T8) or {}).get("outputs") or {}
            lives.add(o8.get("live"))
            disp.add(json.dumps(o8.get("displays"), sort_keys=True))
            time.sleep(float(cfg["interval"]))
        el = time.time() - t0
        mb, _ = manager()
        return float(np.mean(ft)) if ft else 0.0, ma or {}, mb or {}, el, lives, disp, len(ft)

    f_off, a_off, b_off, el_off, l_off, d_off, n_off = window()
    poll(True)
    f_on, a_on, b_on, el_on, l_on, d_on, n_on = window()
    m_end, _ = manager()
    m_end = m_end or {}
    ticks_off = int(b_off.get("poll_ticks", 0)) - int(a_off.get("poll_ticks", 0))
    ticks_on = int(b_on.get("poll_ticks", 0)) - int(a_on.get("poll_ticks", 0))
    hz = ticks_on / el_on if el_on > 0 else 0.0
    rec = int(m_end.get("reconciles", 0)) - int(m0.get("reconciles", 0))
    wr = int(m_end.get("settings_writes", 0)) - int(m0.get("settings_writes", 0))
    lives = l_off | l_on
    disp = d_off | d_on
    tol, min_hz = float(cfg["frameTol"]), float(cfg["minPollHz"])
    good = ticks_off == 0 and hz >= min_hz and rec == 0 and wr == 0 and lives == {0} and len(disp) == 1 \
        and abs(f_on - f_off) <= tol and b_on.get("poll_enabled") is True
    (ok if good else no)(
        f"o_poll_idle: poll paused {el_off:.2f} s: {ticks_off} ticks, frame_time_ms {f_off:.3f} ({n_off} samples) | "
        f"poll running {el_on:.2f} s: {ticks_on} ticks = {hz:.1f}/s (>= {min_hz}), frame_time_ms {f_on:.3f} "
        f"({n_on} samples) | delta {f_on - f_off:+.3f} ms (|d| <= {tol}); reconciles +{rec}, settings writes +{wr}, "
        f"outputs.live values seen {sorted(lives, key=str)}, distinct displays lists {len(disp)} ({loadavg()})")


def o_no_window_opened():
    WATCH.stop_ev.set()
    WATCH.join(timeout=3)
    if WATCH.err:
        no(f"o_no_window_opened: sampler unavailable ({WATCH.err})")
        return
    (ok if WATCH.samples >= 4 and not WATCH.named_output and WATCH.max_layer0 <= 1 else no)(
        f"o_no_window_opened: {WATCH.samples} Quartz samples over the run: Output-named Audio-DNA windows "
        f"{sorted(set(WATCH.named_output))}, max on-screen Audio-DNA layer-0 windows {WATCH.max_layer0} (<= 1 = the "
        f"main window); window names readable: {sorted(WATCH.names_seen)[:4]}")


def o_tap_cost():
    if not wait_no_compiler("o_tap_cost"):
        return
    print(f"      o_tap_cost: {loadavg()}", flush=True)
    tc = FIX["tapCost"]
    for (W, H) in tc["sizes"]:
        if not load(f"tapcost_{W}x{H}", (W, H)):
            return
        trig(0)
        time.sleep(float(tc["warmup"]))
        res = {}
        for label, on in (("off", False), ("on", True)):
            code, body = tap(on)
            if code != 200:
                print(f"REPORT o_tap_cost {W}x{H}: set_output_tap -> HTTP {code} (n/a on this binary)", flush=True)
                res = None
                break
            time.sleep(1.0)
            state(A)   # reset the peak
            ft, gt, pk = [], [], 0.0
            t_end = time.time() + float(tc["seconds"])
            while time.time() < t_end:
                s = state(A) or {}
                ft.append(float(s.get("frame_time_ms", 0))); gt.append(float(s.get("gpu_time_ms", 0)))
                pk = max(pk, float(s.get("peak_frame_time_ms", 0)))
                time.sleep(float(tc["interval"]))
            res[label] = (float(np.mean(ft)), pk, float(np.mean(gt)), float((state(A) or {}).get("fps", 0)))
        if res:
            (f_off, p_off, g_off, fps_off), (f_on, p_on, g_on, fps_on) = res["off"], res["on"]
            print(f"REPORT o_tap_cost {W}x{H}: tap off frame_time_ms {f_off:.3f} peak {p_off:.2f} gpu_time_ms {g_off:.3f} "
                  f"fps {fps_off:.1f} | tap on frame_time_ms {f_on:.3f} peak {p_on:.2f} gpu_time_ms {g_on:.3f} fps "
                  f"{fps_on:.1f} | delta cpu {f_on - f_off:+.3f} ms gpu {g_on - g_off:+.3f} ms ({loadavg()})", flush=True)
    tap(False)


ROWS = [o_probe_matches_canvas, o_probe_portrait_target, o_probe_tracks_change, o_probe_survives_resolution_change,
        o_probe_repeat_stable, o_state_outputs, o_state_displays, o_restore_empty, o_poll_idle, o_tap_cost,
        o_no_window_opened]

if __name__ == "__main__":
    WATCH.start()
    for row in ROWS:
        if ONLY and row.__name__ not in ONLY and row is not o_no_window_opened:
            continue
        if row is o_tap_cost and not (ONLY and "o_tap_cost" in ONLY):
            continue   # opt-in: it waits for every compiler on the machine to finish
        try:
            row()
        except Exception as e:  # noqa: BLE001
            no(f"{row.__name__}: exception {type(e).__name__}: {e}")
    print(f"\nPY {PASS} PASS / {FAIL} FAIL", flush=True)
    sys.exit(0 if FAIL == 0 else 1)
