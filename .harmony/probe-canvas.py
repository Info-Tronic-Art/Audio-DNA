#!/usr/bin/env python3
"""probe-canvas.py -- REST/pixel half of .harmony/probe-canvas.sh (s-rta-0926b plan4 item 1: the composition
canvas drives the picture's shape; .harmony/.reports/s-rta-0926b/plan4-final.md sections 2.6 / 2.8).

The .sh owns launch (TEST MODE) / refuse / quit; this file talks to the running app on 7070 and on the test-mode
TestServer ([::1]:8080), and decodes every captured PNG with PIL+numpy (never file hashes). Every render_frame
response is checked; a failed capture is a FAIL. The output dir is fresh per run.

usage: probe-canvas.py <root> <fresh-outdir> <media-dir> [row,row,...]
rows: c_default_shape c_4k_shape c_custom_4x3 c_runtime_change_keeps_history c_legacy_image_fit
      c_capture_deterministic c_capture_cost c_perf_1080 c_perf_4k
RETIRED (lane bf9b, s-rta-1002b; Harmony ruling R-S2, rulings-bf9b-merge.md): f2_deck_transition (plan4 F2, "a deck
switch fades from the OUTGOING deck"). The deck-to-deck fade no longer exists: a deck switch changes nothing on screen
(BORIS_DECISIONS.md "Decks are boxes of clips": "when I switch between decks, do not change the clips playing in the
layers or how they are playing"). Successors: probe-boxes k1a / k1c / k1t (a switch shows no transition at all).

Metric: d(X, Y) = mean |X - Y| over RGB, 0..255. non-blank = alpha>0 fraction >= 0.5 and RGB std >= 3.
Fixtures A = media/P16_01_baseline.png, B = media/P16_02_Screen_Split_2x2.png (both 756x878).

c_default_shape (RED on the base): composition JSON WITHOUT outputWidth/outputHeight, deck 0 L0 col0 = A.
  PASS: the render_frame PNG is exactly 1920x1080, non-blank, and d(f, A resized to 1920x1080 BILINEAR)
  <= shape.contentTol (the deck path stretches a clip to the canvas). The SIZE carries the RED.
c_4k_shape (RED): outputWidth 3840 / outputHeight 2160. PASS: PNG 3840x2160 and d(f BOX-downsized to
  1920x1080, the default frame) <= contentTol. Prints /api/state fps after 3 s (informational).
c_custom_4x3 (RED): 1024x768. PASS: PNG 1024x768, d(f, A resized to 1024x768) <= contentTol.
c_runtime_change_keeps_history (RED -- the teeth row for the rescale-blit): 1080p, L0 col0 = a probe-made BRIGHT
  smooth-ramp image (mean ~130; A/B average only ~20) + [Freeze 0.98];
  after 2.5 s f0; 8080 /api/set_composition_params {outputWidth 2560, outputHeight 1440}; 0.1 s; f1; back to
  1920x1080; 0.1 s; f2. PASS: f0 non-blank; f1 is 2560x1440, non-blank, d(f1 resized to 1920x1080, f0) <=
  runtime.tolChange; f2 is 1920x1080, d(f2, f0) <= runtime.tolBack. Prints 8080 peak_frame_time_ms across each
  change (the hitch -- report only). A history recreated black instead of rescaled leaves 0.98^n of the picture
  missing n frames later -- 0.1 s + the capture's own latency is ~15-30 frames at 60-120 Hz: 0.55-0.75 missing.
  (Calibrated on the lane: the plan's Freeze 0.95 / 0.4 s had NO teeth on this 120 Hz rig -- a no-rescale mutant
  passed with d 4.31 / 2.63 because 0.95^~78 frames is only 2% -- so the row was re-tuned and re-proven on the
  mutant; see the lane report.)
c_legacy_image_fit (RED): deck 0 has a layer whose clip is never triggered (compositeShow returns 0), and 7070
  /api/load_image shows a probe-made 600x600 solid red PNG (the legacy fallback). PASS: PNG 1920x1080, columns
  x in [0, 400) mean RGB <= 2 (black bars INSIDE the canvas), x in [440, 1480) mean R >= 250 (the image fitted
  1080x1080 at x 420..1500, never stretched).
c_capture_deterministic (guard, GREEN on both): two render_frame captures with the same `time` decode to the
  identical pixel array.
c_capture_cost (REPORT, never FAILs): at 1080p and at 4K, wall time of one render_frame and the lowest /api/state
  fps sampled every 0.25 s over the 1.5 s after it (the capture is one long frame on the GL thread: the
  synchronous glReadPixels + per-pixel copy + PNG encode, plan4 1F).
c_perf_1080 (GATE) / c_perf_4k (REPORT): the plan4 2.8 composition (probe-canvas.json "perf"), all three layers
  triggered, 3 s warm-up, 7070 /api/state sampled 10 x 0.5 s. Waits until no clang/clang++ is running and prints
  the load average next to the numbers. 1080p PASS: mean fps >= 58 AND mean frame_time_ms <= 12; if
  CANVAS_BASE_FPS is set and below 58 (the base binary itself cannot reach the bar on this rig), fps >= base - 2.
  Also prints gpu_time_ms / peak_gpu_time_ms (plan4 A-opt; absent on the base binary) and the temporal_buffers /
  frame_rings counts.

Calibration: see the lane report .harmony/.reports/s-rta-0926b/canvas.md (RED lines on the pre-change app,
GREEN lines on the lane build, numbers quoted there).
"""
import json, os, subprocess, sys, time

import numpy as np
import requests
from PIL import Image

A = "http://127.0.0.1:7070"
T8 = "http://[::1]:8080"
ROOT, OUT, MEDIA = sys.argv[1], sys.argv[2], sys.argv[3]
ONLY = set(sys.argv[4].split(",")) if len(sys.argv) > 4 and sys.argv[4] else None
FIX = json.load(open(os.path.join(ROOT, ".harmony", "probe-canvas.json")))
IMG_A = FIX["imageA"].replace("@MEDIA@", MEDIA)
IMG_B = FIX["imageB"].replace("@MEDIA@", MEDIA)
PASS = FAIL = 0
S = requests.Session()
# One fresh connection per request, never a pooled keep-alive one (s-rta-0927 c1-state-fix). The app's cpp-httplib
# server closes a connection idle for 5 s (CPPHTTPLIB_KEEPALIVE_TIMEOUT_SECOND) -- shutdown, then it drains and
# DISCARDS whatever request arrives in that instant. A reused connection can then fail with RemoteDisconnected
# (~1 run in 3, on the pre-C1 app too), and requests never retries it. Evidence:
# .harmony/.reports/s-rta-0927/c1-state-fix.md.
S.headers["Connection"] = "close"
KEEP = {}   # frames shared between rows (c_default_shape's frame)


def ok(msg):
    global PASS; PASS += 1; print(f"PASS  {msg}", flush=True)


def no(msg):
    global FAIL; FAIL += 1; print(f"FAIL  {msg}", flush=True)


def fx(e):
    return {"name": e[0], "enabled": True, "bypassed": False, "dryWet": 1.0, "params": list(e[1:])}


def clip(cid, img, effects=(), **extra):
    c = {"name": f"c{cid}", "id": cid, "mediaType": 1, "mediaFile": img, "effects": [fx(e) for e in effects]}
    c.update(extra)
    return c


def layer(lid, clips, ltype=0, speed=0.0, **extra):
    l = {"name": f"L{lid}", "id": lid, "opacity": 1.0, "visible": True, "blendMode": 0, "type": ltype,
         "transitionSpeed": speed, "layerEffects": [], "clips": clips}
    l.update(extra)
    return l


def deck(did, layers, ncols=1):
    return {"name": f"D{did}", "id": did, "numColumns": ncols, "layers": layers}


def load(tag, decks, size=None, **comp_extra):
    """size=None writes NO outputWidth/outputHeight keys (the S4 case: every older probe fixture)."""
    comp = {"name": "canvas-" + tag, "activeDeckIndex": 0, "masterOpacity": 1.0, "globalTransitionSpeed": 0.0,
            "decks": decks}
    if size:
        comp["outputWidth"], comp["outputHeight"] = int(size[0]), int(size[1])
    comp.update(comp_extra)
    path = os.path.join(OUT, f"comp_{tag}.json")
    json.dump(comp, open(path, "w"), indent=1)
    r = S.post(A + "/api/load_composition", json={"path": path}, timeout=10)
    good = r.ok and r.json().get("ok") is True
    if not good:
        no(f"{tag}: load_composition rejected: {r.text[:160]}")
    time.sleep(1.0)
    # rig guard (probe-render-state.py): nothing here touches the global effect chain.
    try:
        on = [e.get("name") for e in S.get(A + "/api/state", timeout=6).json().get("effects", []) if e.get("enabled")]
    except Exception:  # noqa: BLE001
        on = ["<state unreadable>"]
    if on:
        no(f"{tag}: rig -- the global effect chain has enabled effects nobody in this probe set: {on[:6]}")
    return good


def trig(li, col):
    S.post(A + "/api/trigger_clip", json={"layer": li, "column": col}, timeout=6)


def decode(p):
    return np.asarray(Image.open(p).convert("RGBA")).astype(float)


def cap(name, t=None):
    """render_frame (7070) into the fresh dir; deletes a pre-existing file first, then requires ok:true, the
    file, and an mtime at/after the request (never decode a PNG this call did not write)."""
    p = os.path.join(OUT, name + ".png")
    if os.path.exists(p):
        os.remove(p)
    req = {"output_path": p}
    if t is not None:
        req["time"] = float(t)
    t0 = time.time()
    try:
        body = S.post(A + "/api/render_frame", json=req, timeout=30).json()
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


def size_of(f):
    return (int(f.shape[1]), int(f.shape[0]))


def resized(img, size, how=Image.BILINEAR):
    if isinstance(img, str):
        im = Image.open(img).convert("RGBA")
    else:
        im = Image.fromarray(np.clip(img, 0, 255).astype(np.uint8), "RGBA")
    return np.asarray(im.resize(size, how)).astype(float)


def d(x, y):
    return float(np.abs(x[..., :3] - y[..., :3]).mean())


def nonblank(a):
    return float((a[..., 3] > 0).mean()) >= 0.5 and float(a[..., :3].std()) >= 3.0


def state(base=A):
    try:
        return S.get(base + "/api/state", timeout=6).json()
    except Exception as e:  # noqa: BLE001
        no(f"{base}/api/state: {e}")
        return None


def loadavg():
    return "load avg %.2f %.2f %.2f" % os.getloadavg()


def wait_no_compiler(tag, limit_s=1800):
    """Rig rule: perf rows run only when no compiler is running."""
    t0 = time.time()
    while True:
        # macOS pgrep takes a regex: a bare "clang++" is an invalid pattern (error, empty stdout = never "busy").
        busy = [n for n in ("clang", r"clang\+\+") if subprocess.run(["pgrep", "-x", n], capture_output=True).stdout.strip()]
        if not busy:
            return True
        if time.time() - t0 > limit_s:
            no(f"{tag}: a compiler ({busy}) kept running for {limit_s} s -- perf not measured")
            return False
        print(f"      {tag}: waiting for {busy} to finish ({loadavg()})", flush=True)
        time.sleep(20)


def a_comp(tag, size=None, fxs=()):
    return load(tag, [deck(0, [layer(0, [clip(1, IMG_A, fxs)])])], size=size)


# ------------------------------------------------------------------ rows
def c_default_shape():
    tol = float(FIX["shape"]["contentTol"])
    if not a_comp("default"):
        return
    trig(0, 0); time.sleep(1.0)
    f = cap("default")
    if f is None:
        return
    KEEP["default"] = f
    sz = size_of(f)
    (ok if sz == (1920, 1080) else no)(f"c_default_shape: a composition without outputWidth/outputHeight renders "
                                       f"a 1920x1080 canvas (render_frame PNG {sz[0]}x{sz[1]})")
    de = d(f if sz == (1920, 1080) else resized(f, (1920, 1080)), resized(IMG_A, (1920, 1080)))
    (ok if nonblank(f) and de <= tol else no)(
        f"c_default_shape: the clip fills the canvas (non-blank={nonblank(f)}, d(f, A stretched to 1920x1080)="
        f"{de:.2f}, tol {tol})")


def c_4k_shape():
    tol = float(FIX["shape"]["contentTol"]); W, H = FIX["shape"]["fourK"]
    ref = KEEP.get("default")
    if ref is None:
        if not a_comp("4k_ref"):
            return
        trig(0, 0); time.sleep(1.0); ref = cap("4k_ref")
    if not a_comp("4k", size=(W, H)):
        return
    trig(0, 0); time.sleep(1.5)
    f = cap("4k")
    if f is None or ref is None:
        return
    sz = size_of(f)
    (ok if sz == (W, H) else no)(f"c_4k_shape: outputWidth {W} / outputHeight {H} renders a {W}x{H} canvas "
                                 f"(render_frame PNG {sz[0]}x{sz[1]})")
    de = d(resized(f, (1920, 1080), Image.BOX), ref if size_of(ref) == (1920, 1080) else resized(ref, (1920, 1080)))
    (ok if de <= tol else no)(f"c_4k_shape: the 4K picture is the 1080p picture at 4x the pixels "
                              f"(d(4K box-downsized, 1080p)={de:.2f}, tol {tol})")
    time.sleep(3.0)
    st = state()
    if st:
        print(f"      c_4k_shape: /api/state fps {st.get('fps', 0):.1f} frame_time_ms "
              f"{st.get('frame_time_ms', 0):.2f} gpu_time_ms {st.get('gpu_time_ms', 'n/a')} ({loadavg()})",
              flush=True)


def c_custom_4x3():
    tol = float(FIX["shape"]["contentTol"]); W, H = FIX["shape"]["custom"]
    if not a_comp("4x3", size=(W, H)):
        return
    trig(0, 0); time.sleep(1.0)
    f = cap("4x3")
    if f is None:
        return
    sz = size_of(f)
    (ok if sz == (W, H) else no)(f"c_custom_4x3: a {W}x{H} composition renders a {W}x{H} canvas "
                                 f"(render_frame PNG {sz[0]}x{sz[1]})")
    de = d(f if sz == (W, H) else resized(f, (W, H)), resized(IMG_A, (W, H)))
    (ok if nonblank(f) and de <= tol else no)(f"c_custom_4x3: the clip fills the 4:3 canvas "
                                              f"(d(f, A stretched to {W}x{H})={de:.2f}, tol {tol})")


def set_size8(w, h):
    try:
        r = S.post(T8 + "/api/set_composition_params", json={"outputWidth": int(w), "outputHeight": int(h)},
                   timeout=6)
        return r.status_code, r.text[:160]
    except Exception as e:  # noqa: BLE001
        return -1, str(e)


def make_bright():
    """A smooth, BRIGHT probe image (mean ~130; A/B average ~20, which would leave a recreated-black history
    only ~6 levels off -- no margin). Smooth ramps, so GL bilinear vs PIL resampling cannot matter."""
    p = os.path.join(OUT, "bright.png")
    if not os.path.exists(p):
        W, H = 756, 878
        x = np.linspace(0.0, 1.0, W)[None, :]; y = np.linspace(0.0, 1.0, H)[:, None]
        rgba = np.zeros((H, W, 4), np.uint8)
        rgba[..., 0] = (40 + 180 * x).astype(np.uint8)
        rgba[..., 1] = (40 + 180 * y).astype(np.uint8)
        rgba[..., 2] = 160
        rgba[..., 3] = 255
        Image.fromarray(rgba, "RGBA").save(p)
    return p


def c_runtime_change_keeps_history():
    cfg = FIX["runtime"]; W2, H2 = cfg["to"]; settle = float(cfg["settle"])
    img = make_bright()
    if not load("runtime", [deck(0, [layer(0, [clip(1, img, cfg["fx"])])])], size=(1920, 1080)):
        return
    trig(0, 0); time.sleep(2.5)
    f0 = cap("rt_f0")
    peaks = []
    s = state(T8); peaks.append(("before 1440p", s.get("peak_frame_time_ms") if s else None))
    code1, body1 = set_size8(W2, H2)
    time.sleep(settle)
    f1 = cap("rt_f1")
    s = state(T8); peaks.append(("across 1080p -> 1440p", s.get("peak_frame_time_ms") if s else None))
    print(f"      c_runtime_change: set_composition_params {W2}x{H2} -> HTTP {code1} {body1}", flush=True)
    code2, body2 = set_size8(1920, 1080)
    time.sleep(settle)
    f2 = cap("rt_f2")
    s = state(T8); peaks.append(("across 1440p -> 1080p", s.get("peak_frame_time_ms") if s else None))
    print(f"      c_runtime_change: set_composition_params 1920x1080 -> HTTP {code2} {body2}", flush=True)
    print(f"      c_runtime_change: peak_frame_time_ms {peaks}", flush=True)
    if f0 is None or f1 is None or f2 is None:
        no("c_runtime_change_keeps_history: capture failed"); return
    dA = d(f0 if size_of(f0) == (1920, 1080) else resized(f0, (1920, 1080)), resized(img, (1920, 1080)))
    print(f"      c_runtime_change: f0 {size_of(f0)} d(f0, image)={dA:.2f} mean(image) "
          f"{resized(img, (1920, 1080))[..., :3].mean():.1f} (Freeze 0.98's 8-bit fixed point sits up to ~25 levels "
          f"under the image -- report only)", flush=True)
    (ok if nonblank(f0) else no)(f"c_runtime_change_keeps_history: the Freeze 0.98 picture f0 is non-blank")
    s1 = size_of(f1)
    d1 = d(resized(f1, size_of(f0)), f0)
    (ok if s1 == (W2, H2) and nonblank(f1) and d1 <= float(cfg["tolChange"]) else no)(
        f"c_runtime_change_keeps_history: {settle} s after 1080p -> {W2}x{H2} the frame is {s1[0]}x{s1[1]} and "
        f"keeps the Freeze history (d(f1 resized, f0)={d1:.2f}, tol {cfg['tolChange']})")
    s2 = size_of(f2)
    d2 = d(f2 if s2 == size_of(f0) else resized(f2, size_of(f0)), f0)
    (ok if s2 == (1920, 1080) and d2 <= float(cfg["tolBack"]) else no)(
        f"c_runtime_change_keeps_history: {settle} s after going back to 1920x1080 the frame is {s2[0]}x{s2[1]} "
        f"and still the same picture (d(f2, f0)={d2:.2f}, tol {cfg['tolBack']})")


def c_legacy_image_fit():
    cfg = FIX["legacy"]; n = int(cfg["size"])
    red = os.path.join(OUT, "red600.png")
    Image.new("RGBA", (n, n), (255, 0, 0, 255)).save(red)
    if not a_comp("legacy"):        # the clip is never triggered: compositeShow returns 0
        return
    r = S.post(A + "/api/load_image", json={"filepath": red}, timeout=10)
    if not (r.ok and r.json().get("ok")):
        no(f"c_legacy_image_fit: load_image rejected: {r.text[:160]}"); return
    time.sleep(0.8)
    f = cap("legacy")
    if f is None:
        return
    sz = size_of(f)
    if sz != (1920, 1080):
        no(f"c_legacy_image_fit: the legacy image frame is the 1920x1080 canvas (render_frame PNG {sz[0]}x{sz[1]}; "
           f"mean R of the whole frame {f[..., 0].mean():.1f})"); return
    b0, b1 = cfg["barCols"]; i0, i1 = cfg["imgCols"]
    bar = float(f[:, b0:b1, :3].mean()); img = float(f[:, i0:i1, 0].mean())
    (ok if bar <= 2.0 and img >= 250.0 else no)(
        f"c_legacy_image_fit: a 600x600 image is fitted inside the 1920x1080 canvas over black, never stretched "
        f"(x {b0}..{b1} mean RGB {bar:.2f} <= 2; x {i0}..{i1} mean R {img:.1f} >= 250)")


def c_capture_deterministic():
    if not a_comp("determ"):
        return
    trig(0, 0); time.sleep(1.0)
    f1, f2 = cap("determ_1", t=1.25), cap("determ_2", t=1.25)
    if f1 is None or f2 is None:
        return
    same = f1.shape == f2.shape and bool(np.array_equal(f1, f2))
    (ok if same and nonblank(f1) else no)(f"c_capture_deterministic: two captures with the same time decode to "
                                          f"identical pixels ({size_of(f1)}, identical={same})")


def c_capture_cost():
    for (W, H) in ((1920, 1080), tuple(FIX["shape"]["fourK"])):
        if not a_comp(f"cost_{W}", size=(W, H)):
            return
        trig(0, 0); time.sleep(2.0)
        state()
        t0 = time.time(); f = cap(f"cost_{W}"); wall = (time.time() - t0) * 1000.0
        fps = []
        for _ in range(6):
            time.sleep(0.25)
            st = state()
            if st:
                fps.append(float(st.get("fps", 0.0)))
        print(f"      c_capture_cost {W}x{H}: render_frame wall {wall:.0f} ms, PNG "
              f"{size_of(f) if f is not None else None}; /api/state fps over the next 1.5 s: min "
              f"{min(fps) if fps else float('nan'):.1f} {[round(x, 1) for x in fps]} ({loadavg()})", flush=True)
    ok("c_capture_cost: REPORT row (numbers above; never FAILs)")


def perf(tag, size, gate):
    cfg = FIX["perf"]
    if not wait_no_compiler(tag):
        return
    imgs = {"A": IMG_A, "B": IMG_B}
    layers = [layer(i, [clip(10 + i, imgs[s["img"]], s["fx"])], ltype=int(s["type"])) for i, s in enumerate(cfg["layers"])]
    if not load(tag, [deck(0, layers)], size=size):
        return
    for i in range(len(layers)):
        trig(i, 0)
    time.sleep(float(cfg["warmup"]))
    state()   # resets the peaks
    fps, ft, gpu, pk, pg = [], [], [], [], []
    for _ in range(int(cfg["samples"])):
        time.sleep(float(cfg["interval"]))
        st = state()
        if not st:
            continue
        fps.append(float(st.get("fps", 0.0))); ft.append(float(st.get("frame_time_ms", 0.0)))
        pk.append(float(st.get("peak_frame_time_ms", 0.0)))
        if "gpu_time_ms" in st:
            gpu.append(float(st["gpu_time_ms"])); pg.append(float(st.get("peak_gpu_time_ms", 0.0)))
    if not fps:
        no(f"{tag}: no /api/state samples"); return
    mf, mt = float(np.mean(fps)), float(np.mean(ft))
    gtxt = (f"gpu_time_ms mean {np.mean(gpu):.2f} peak {max(pg):.2f}" if gpu and max(gpu) > 0
            else ("gpu_time_ms n/a (driver reported 0)" if gpu else "gpu_time_ms n/a (field absent)"))
    st = state() or {}
    print(f"      {tag} {size[0]}x{size[1]}: fps mean {mf:.1f} min {min(fps):.1f}; frame_time_ms mean {mt:.2f} "
          f"max-peak {max(pk):.2f}; {gtxt}; temporal_buffers {st.get('temporal_buffers', 'n/a')} frame_rings "
          f"{st.get('frame_rings', 'n/a')} ({loadavg()})", flush=True)
    if not gate:
        ok(f"{tag}: REPORT row (numbers above; never FAILs){'' if mf >= 30 else ' -- BELOW 30 fps'}")
        return
    base = os.environ.get("CANVAS_BASE_FPS")
    min_fps = float(cfg["minFps"])
    if base and float(base) < min_fps:
        min_fps = float(base) - 2.0
        print(f"      {tag}: base binary reached only {float(base):.1f} fps on this rig -> gate fps >= {min_fps:.1f}",
              flush=True)
    (ok if mf >= min_fps and mt <= float(cfg["maxFrameMs"]) else no)(
        f"{tag}: the plan4 2.8 composition holds mean fps {mf:.1f} >= {min_fps:.1f} and mean frame_time_ms "
        f"{mt:.2f} <= {cfg['maxFrameMs']}")


def main():
    rows = [("c_default_shape", c_default_shape),
            ("c_4k_shape", c_4k_shape),
            ("c_custom_4x3", c_custom_4x3),
            ("c_runtime_change_keeps_history", c_runtime_change_keeps_history),
            ("c_legacy_image_fit", c_legacy_image_fit),
            ("c_capture_deterministic", c_capture_deterministic),
            ("c_capture_cost", c_capture_cost),
            ("c_perf_1080", lambda: perf("c_perf_1080", (1920, 1080), True)),
            ("c_perf_4k", lambda: perf("c_perf_4k", tuple(FIX["shape"]["fourK"]), False))]
    for name, fn in rows:
        if ONLY is None or name in ONLY:
            print(f"--- {name}", flush=True)
            fn()
    print(f"\nPY {PASS} PASS / {FAIL} FAIL", flush=True)
    sys.exit(0 if FAIL == 0 else 1)


if __name__ == "__main__":
    main()
