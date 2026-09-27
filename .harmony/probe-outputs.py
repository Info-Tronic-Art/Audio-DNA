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
      o_probe_repeat_stable o_state_outputs o_no_window_opened o_tap_cost

Metric: d(X, Y) = mean |X - Y| over RGB, 0..255. Fixtures A = media/P16_01_baseline.png, B =
media/P16_02_Screen_Split_2x2.png (d(A, B) = 29.4 at 1920x1080). Composition: 1920x1080, deck 0 L0 (Opaque, no
effects) col0 = A, col1 = B.

o_probe_matches_canvas (RED on the base: 404): before the tap is ever on, output_probe answers 409 (nothing
  published). Trigger col0; 1 s; f0 = 7070 render_frame (1920x1080); set_output_tap on; 0.3 s; output_probe
  1920x1080 -> a 1920x1080 PNG with d(probe, f0) <= matchTol (2).
o_probe_portrait_target (RED): output_probe 1080x1920 -> the picture sits in fitCanvas(1920,1080,1080,1920) =
  {0,656,1080,607} (GL, bottom-up) = PNG rows 657..1263: every other row has mean RGB <= barTol (2), and
  d(inner, f0 resized to 1080x607 BILINEAR) <= innerTol (6).
o_probe_tracks_change (RED): trigger col1 (B); 0.5 s; f_B = render_frame; output_probe -> d(probe, f0) > 20 AND
  d(probe, f_B) <= 2 (LIVE, not a stale slot).
o_probe_survives_resolution_change (RED): 8080 set_composition_params 1280x720; 0.4 s; f_720 = render_frame
  (must be 1280x720); output_probe 1920x1080 -> response canvas 1280x720 and gen > the previous probe's gen;
  d(probe, f_720 resized to 1920x1080 BILINEAR) <= 6. Restores 1920x1080.
o_probe_repeat_stable (guard/soak): col0 again; 60 output_probes 1920x1080 back to back while the main renders
  static A: every PNG decodes, none blank, all d <= 2 vs a fresh f0 (the cross-thread/cross-context race soak).
  Prints the elapsed time.
o_state_outputs (RED): 8080 and 7070 /api/state carry outputs {live == 0, tap == true, frame_gen >= 1,
  frame_serial >= 1, canvas_w == 1920, canvas_h == 1080}; after set_output_tap off, tap == false.
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
        busy = [n for n in ("clang", "clang++") if subprocess.run(["pgrep", "-x", n], capture_output=True).stdout.strip()]
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
    gen = int(pb.get("gen", 0))
    (ok if size_of(f720) == (RW, RH) and (pb.get("canvas_w"), pb.get("canvas_h")) == (RW, RH)
     and gen0 is not None and gen > gen0 and de <= float(FIX["resolution"]["tol"]) else no)(
        f"o_probe_survives_resolution_change: render_frame {size_of(f720)[0]}x{size_of(f720)[1]}, probe canvas "
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
        o_probe_repeat_stable, o_state_outputs, o_tap_cost, o_no_window_opened]

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
