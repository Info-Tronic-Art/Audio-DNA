#!/usr/bin/env python3
"""probe-milkdrop.py -- REST/pixel half of .harmony/probe-milkdrop.sh (s-rta-1002b lane bf10: MilkDrop draws into its
own composition-canvas-sized framebuffer, never the Preview panel's window framebuffer).

Pre-registered in .harmony/.reports/s-rta-1002b/plan-bf10.md I4 and ruling-bf10.md (amendments 2, 3, 5, 9, 14, 17 and
the FINAL CONSOLIDATED GATE LIST, G2.1-G2.10 / G3). The .sh owns launch (TEST MODE) / refuse / quit; this file talks to
the running app on 7070 and on the test-mode TestServer ([::1]:8080), and decodes every PNG with PIL+numpy.

usage: probe-milkdrop.py <root> <fresh-outdir> [row,row,...]
rows: m6_panel_bars m1_fill_legacy m2_live_no_stale m3_runtime_change m3_rewarm m4_comp_load_clip m5_preset_switch
      m7_output_tap m9_deck_roundtrip m10_layer_over m8_context_cycle   (default: all of these, in this order)
      perf_md_1080 perf_md_4k perf_md_1080_2l                          (A/B perf rows: only when named)
env:  MILKDROP_MODE  lane (default): every RED / GUARD row must meet its GREEN bar (G2).
                     pre: calibration on the PRE-LANE app (amendment 2): every RED row must FAIL its GREEN bar AND
                     show the box fingerprint (else VOID -> the lane stops); every GUARD row must PASS.
      MILKDROP_ARM   label for the m3_rewarm DATA line (A = pre-lane, B = lane).
      MILKDROP_P1    a bundled preset file name that replaces the pinned P1 (only to calibrate P1 -- plan I4 m2):
                     accepted ONLY with MILKDROP_MODE=pre; in lane (gate) mode the probe exits 2 before any request
                     (Harmony ruling R2). Every run prints "P1 = <preset path>" as its first line.

Capture rules (FINAL GATE LIST): every capture is an 8080 render_frame WITHOUT a width/height override; the canvas is
set through 8080 set_composition_params, then >= 1.5 s (m3: 0.5 s by design); every request uses Connection: close;
no Output window (set_output_tap opens none); no synthetic input. m6 takes WINDOW-ONLY captures of the main window by
Quartz window id (CGWindowListCreateImage, kCGWindowListOptionIncludingWindow) -- never the full screen.

Shared bars:
  uniform      alpha 255 on 100.000 % of pixels; >= 99.99 % of pixels within +-6 of the per-channel median (every
               channel); median max channel >= 64.
  fingerprint  (pre-lane app) the alpha-255 pixels form ONE solid axis-aligned rectangle anchored at the picture's
               bottom-left, >= 99.9 % alpha 255 inside it and none outside; across the run each rectangle is
               min(panel, canvas) per axis (+-2 px), panel = the largest rectangle seen; coverage = rect / canvas
               within +-1 point of that expectation. The ruling's 756x840 table is printed beside it.
Rows (RED = must fail on the pre-lane app WITH the fingerprint; GUARD = must pass on both apps):
  m1_fill_legacy     RED at 1280x720 / 1920x1080 / 3840x2160 / 1080x1920, GUARD at 640x360: load_milkdrop_preset
                     (bf10_solid); PNG == canvas size AND uniform.
  m2_live_no_stale   RED: P1 at 1920x1080 and 1080x1920 (3 s warm); two captures 0.5 s apart: alpha 255 on 100 % in
                     both, every 64x64 tile has >= 20 % of its pixels changed by > 8 levels (max over RGB).
  m3_runtime_change  RED: bf10_solid 1920x1080 -> 2560x1440 -> 1920x1080, capture 0.5 s after each change: new size
                     AND uniform. INFO: 8080 peak_frame_time_ms across each change.
  m3_rewarm          INFO (amendment 17): P1 at 1920x1080, 3 s; reference L0 = mean luminance of the alpha-255
                     pixels; peak read + discarded; canvas 2560x1440; captures at ~+0.1/+0.25/+0.5/+1.0/+2.0 s (real
                     send times printed); peak read. DATA m3_rewarm arm=<A|B> t=<s>:<L/L0> ... peak_frame_ms=<n>.
  m4_comp_load_clip  RED: 7070 load_composition (deck 0 L0 col0 = {mediaType 4, projectm_visualizer}) at 1280x720,
                     trigger_clip, load_milkdrop_preset(bf10_solid); then a second JSON at 1080x1920 (+ trigger):
                     both captures uniform at the canvas size.
  m5_preset_switch   RED: 1920x1080: bf10_solid -> capture A; bf10_solid_b -> 0.5 s -> capture B: B uniform AND
                     |median(B) - median(A)| >= 40 in some channel.
  m6_panel_bars      RED: window captures. CONTROL first, before MilkDrop is ever loaded: per canvas 640x360 /
                     3840x2160 / 1080x1920, an untriggered-layer composition + 7070 load_image of a solid green PNG of
                     the canvas size; the green rectangle = the present rect; the 12-px strips just outside it on its
                     bar sides. Control valid iff strips std <= 3 and max channel <= 40 (else INVALID: one rerun;
                     INVALID again = FAIL, amendment 14). Then bf10_solid per canvas: inside the rect >= 99.5 % of
                     pixels within +-10 of the rect's median; strips std <= 3 and mean within +-6 of the control's.
                     Pre-lane fingerprint: at least one bar strip's mean is more than 6 from the control's.
  m7_output_tap      RED: set_output_tap on, bf10_solid at 1920x1080: output_probe 1920x1080 has >= 99.99 % of
                     pixels within +-6 of the median and median max >= 64. Pre-lane fingerprint: >= 60 % of pixels
                     outside +-6 of the bf10_solid colour.
  m8_context_cycle   RED: /api/debug/gl_context_cycle (the releaseGL / initGL path), then m1's procedure and bar at
                     1920x1080 (the capture right after the cycle, before the reload, is INFO).
  m9_deck_roundtrip  RED (C0): amendment 3 verbatim: C0 and C3 uniform, |median(C3) - median(C0)| <= 6 per channel,
                     the live-phase pair after the return meets m2's bar; C1 / C2 INFO.
  m10_layer_over     GUARD: deck 0 L0 = MilkDrop (bf10_solid), L1 = an opaque quadrant PNG (Normal, opacity 1), both
                     triggered: >= 99.9 % of pixels within +-3 per channel of the PNG, after 1.5 s and after a canvas
                     round trip 1920x1080 -> 1080x1920 -> 1920x1080 (1.5 s each).
  perf_md_1080 / perf_md_4k / perf_md_1080_2l   (A/B, G3; .harmony/probe-milkdrop-ab.py applies the bars): the m4
                     composition with the heavy preset (319) via load_milkdrop_preset, canvas 1920x1080 / 3840x2160
                     (2l: L0 and L1 both MilkDrop clips, 1920x1080); 3 s warm-up; 7070 /api/state 10 x 0.5 s;
                     DATA perf_md_<x> fps=<mean> fps_min=<min> frame_ms=<mean> gpu_ms=<mean> gpu_peak_ms=<max>.
"""
import json, os, subprocess, sys, time

import numpy as np
import requests
from PIL import Image

A = "http://127.0.0.1:7070"
T8 = "http://[::1]:8080"
ROOT, OUT = sys.argv[1], sys.argv[2]
ONLY = [r for r in sys.argv[3].split(",") if r] if len(sys.argv) > 3 and sys.argv[3] else None
MODE = os.environ.get("MILKDROP_MODE", "lane")
ARM = os.environ.get("MILKDROP_ARM", "?")
P1_OVERRIDE = os.environ.get("MILKDROP_P1", "")
if P1_OVERRIDE and MODE != "pre":   # R2: a gate run never silently swaps its content proxy
    print("REFUSE: MILKDROP_P1 overrides the pinned P1 only in calibration mode (MILKDROP_MODE=pre); lane / gate runs "
          "use the pinned P1", flush=True)
    sys.exit(2)
FIX = json.load(open(os.path.join(ROOT, ".harmony", "probe-milkdrop.json")))
SOLID = os.path.join(ROOT, FIX["fixtures"]["solid"])
SOLID_B = os.path.join(ROOT, FIX["fixtures"]["solid_b"])
P1 = os.path.join(ROOT, FIX["presetDir"], P1_OVERRIDE or FIX["p1"])
print(f"P1 = {P1}" + ("  (OVERRIDE via MILKDROP_P1 -- calibration only)" if P1_OVERRIDE else ""), flush=True)
HEAVY = os.path.join(ROOT, FIX["presetDir"], FIX["heavy"])
CA = np.array(FIX["colourA"], float)
U = FIX["uniform"]; FP = FIX["fingerprint"]
S = requests.Session()
S.headers["Connection"] = "close"   # one fresh connection per request (cpp-httplib keep-alive race, s-rta-0927)
RESULTS = []      # (row, kind, green_ok, fp_ok, detail)
BOXES = []        # (row, W, H, box_w, box_h, coverage) for the cross-canvas fingerprint check
ERRORS = []


def say(msg):
    print(msg, flush=True)


def err(msg):
    ERRORS.append(msg); say(f"ERROR {msg}")


def record(row, kind, green_ok, fp_ok, detail):
    RESULTS.append((row, kind, bool(green_ok), bool(fp_ok), detail))
    if MODE == "pre" and kind == "RED":
        v = "RED-OK" if (not green_ok and fp_ok) else "VOID"
        why = "" if v == "RED-OK" else (" (the GREEN bar PASSED)" if green_ok else " (failed WITHOUT the fingerprint)")
        say(f"{v}  {row}: {detail}{why}")
    elif MODE == "pre" and kind == "GUARD":
        say(f"{'PASS' if green_ok else 'GUARD-FAIL'}  {row}: {detail}")
    else:
        say(f"{'PASS' if green_ok else 'FAIL'}  {row}: {detail}")


# ------------------------------------------------------------------ REST
def post(base, path, body=None, timeout=15):
    try:
        r = S.post(base + path, json=body if body is not None else {}, timeout=timeout)
        try:
            return r.status_code, r.json()
        except ValueError:
            return r.status_code, {"text": r.text[:200]}
    except Exception as e:  # noqa: BLE001
        return -1, {"error": str(e)}


def get(base, path, timeout=8):
    try:
        return S.get(base + path, timeout=timeout).json()
    except Exception as e:  # noqa: BLE001
        err(f"GET {base}{path}: {e}")
        return {}


def set_size(w, h, wait=1.5):
    code, body = post(T8, "/api/set_composition_params", {"outputWidth": int(w), "outputHeight": int(h)})
    if code != 200 or not body.get("ok"):
        err(f"set_composition_params {w}x{h}: HTTP {code} {body}")
    if wait:
        time.sleep(wait)


def load_md(path):
    code, body = post(T8, "/api/load_milkdrop_preset", {"preset_path": path}, timeout=20)
    if code != 200 or body.get("ok") is not True:
        err(f"load_milkdrop_preset {os.path.basename(path)}: HTTP {code} {body}")


def load_source(kind):
    code, body = post(T8, "/api/load_source", {"source_type": kind})
    if code != 200:
        err(f"load_source {kind}: HTTP {code} {body}")


def trig(li, col):
    code, body = post(A, "/api/trigger_clip", {"layer": li, "column": col})
    if code != 200:
        err(f"trigger_clip {li}/{col}: HTTP {code} {body}")


def switch(dk):
    code, body = post(A, "/api/switch_deck", {"deck": dk})
    if code != 200:
        err(f"switch_deck {dk}: HTTP {code} {body}")


def peak():
    return get(T8, "/api/state").get("peak_frame_time_ms")


def src_clip(cid):
    return {"name": f"md{cid}", "id": cid, "mediaType": 4, "sourceType": "projectm_visualizer", "effects": []}


def img_clip(cid, path):
    return {"name": f"img{cid}", "id": cid, "mediaType": 1, "mediaFile": path, "effects": []}


def layer(lid, clips):
    return {"name": f"L{lid}", "id": lid, "opacity": 1.0, "visible": True, "blendMode": 0, "type": 0,
            "transitionSpeed": 0.0, "layerEffects": [], "clips": clips}


def deck(did, layers):
    return {"name": f"D{did}", "id": did, "numColumns": 1, "layers": layers}


def load_comp(tag, decks, size, **extra):
    comp = {"name": "milkdrop-" + tag, "activeDeckIndex": 0, "masterOpacity": 1.0, "globalTransitionSpeed": 0.0,
            "decks": decks, "outputWidth": int(size[0]), "outputHeight": int(size[1])}
    comp.update(extra)
    path = os.path.join(OUT, f"comp_{tag}.json")
    json.dump(comp, open(path, "w"), indent=1)
    code, body = post(A, "/api/load_composition", {"path": path}, timeout=15)
    if code != 200 or body.get("ok") is not True:
        err(f"{tag}: load_composition rejected: HTTP {code} {body}")
        return False
    time.sleep(1.0)
    on = [e.get("name") for e in get(A, "/api/state").get("effects", []) if e.get("enabled")]
    if on:
        err(f"{tag}: rig -- the global effect chain has enabled effects nobody in this probe set: {on[:6]}")
    return True


def solid_png(name, size, rgb):
    p = os.path.join(OUT, name)
    Image.new("RGBA", (int(size[0]), int(size[1])), tuple(int(c) for c in rgb) + (255,)).save(p)
    return p


def neutral(size=(1920, 1080)):
    """An untriggered-layer composition: compositeDeck draws nothing, so the legacy single source shows (R-E13)."""
    holder = solid_png("holder.png", (64, 64), (0, 0, 0))
    return load_comp("neutral", [deck(0, [layer(0, [img_clip(1, holder)])])], size)


# ------------------------------------------------------------------ captures + metrics
def decode(p):
    return np.asarray(Image.open(p).convert("RGBA")).astype(np.int16)


def cap(name):
    """8080 render_frame WITHOUT a width/height override into the fresh dir; requires ok, the file, a fresh mtime."""
    p = os.path.join(OUT, name + ".png")
    if os.path.exists(p):
        os.remove(p)
    t0 = time.time()
    code, body = post(T8, "/api/render_frame", {"output_path": p}, timeout=40)
    if code != 200 or not body.get("ok"):
        err(f"render_frame {name}: HTTP {code} {body}")
        return None
    if not os.path.isfile(p) or os.path.getmtime(p) < t0 - 0.01:
        err(f"render_frame {name}: no fresh file at {p}")
        return None
    return decode(p)


def size_of(f):
    return (int(f.shape[1]), int(f.shape[0]))


def uniform(f):
    rgb = f[..., :3]
    med = np.median(rgb.reshape(-1, 3), axis=0)
    a255 = 100.0 * float((f[..., 3] == 255).mean())
    w6 = 100.0 * float((np.abs(rgb - med) <= U["tol"]).all(axis=2).mean())
    mm = float(med.max())
    ok = a255 >= U["alpha255Pct"] and w6 >= U["within6Pct"] and mm >= U["medianMaxMin"]
    return ok, {"alpha255": a255, "within6": w6, "median": tuple(int(x) for x in med), "medmax": mm}


def ustr(st):
    return (f"alpha255={st['alpha255']:.4f}% within6={st['within6']:.4f}% median={st['median']} "
            f"medmax={st['medmax']:.0f}")


def fingerprint(row, f):
    """The pre-registered box fingerprint on one capture (amendment 2)."""
    H, W = f.shape[0], f.shape[1]
    m = f[..., 3] == 255
    n = int(m.sum())
    if n == 0:
        return False, f"fp: no alpha-255 pixel at all ({W}x{H})"
    ys, xs = np.where(m)
    x0, x1, y0, y1 = int(xs.min()), int(xs.max()), int(ys.min()), int(ys.max())
    bw, bh = x1 - x0 + 1, y1 - y0 + 1
    inside = 100.0 * float(m[y0:y1 + 1, x0:x1 + 1].mean())
    outside = n - int(m[y0:y1 + 1, x0:x1 + 1].sum())
    anch = x0 == 0 and y1 == H - 1          # PNG rows are top-down: bottom-left = x0 0, last row H-1
    cov = 100.0 * bw * bh / (W * H)
    ok = anch and inside >= FP["insideAlphaPct"] and outside == 0
    BOXES.append((row, W, H, bw, bh, cov, ok))
    pw, ph = FP["panelDiag"]
    diag = 100.0 * min(pw, W) * min(ph, H) / (W * H)
    return ok, (f"fp: box {bw}x{bh} at ({x0},{y0})-({x1},{y1}) bottom-left={anch} inside_alpha255={inside:.3f}% "
                f"outside={outside} coverage={cov:.2f}% (756x840 table {diag:.2f}%)")


def lum(f):
    m = f[..., 3] == 255
    if not m.any():
        return float("nan")
    rgb = f[..., :3].astype(float)
    y = 0.2126 * rgb[..., 0] + 0.7152 * rgb[..., 1] + 0.0722 * rgb[..., 2]
    return float(y[m].mean())


def tiles(a, b, cfg):
    t, lv, need = cfg["tile"], cfg["levels"], cfg["minChangedPct"]
    H, W = a.shape[0], a.shape[1]
    ch = (np.abs(a[..., :3] - b[..., :3]).max(axis=2) > lv)
    n = bad = 0; mn = 100.0
    for y in range(0, H, t):
        for x in range(0, W, t):
            fr = 100.0 * float(ch[y:y + t, x:x + t].mean()); n += 1; mn = min(mn, fr)
            bad += fr < need
    a1 = 100.0 * float((a[..., 3] == 255).mean()); b1 = 100.0 * float((b[..., 3] == 255).mean())
    ok = a1 >= 100.0 and b1 >= 100.0 and bad == 0
    return ok, f"alpha255 {a1:.4f}% / {b1:.4f}%, tiles {n}, tiles < {need:.0f}% changed: {bad}, min tile {mn:.1f}%"


def pair(tag, gap):
    t0 = time.time(); a = cap(tag + "_a")
    time.sleep(max(0.0, gap - (time.time() - t0)))
    b = cap(tag + "_b")
    return a, b


# ------------------------------------------------------------------ window capture (m6)
def main_window():
    import Quartz
    wl = Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionAll, Quartz.kCGNullWindowID)
    ws = [w for w in wl if w.get("kCGWindowOwnerName", "") == "Audio-DNA" and w.get("kCGWindowLayer", 1) == 0
          and "Output" not in str(w.get("kCGWindowName", ""))]
    ws.sort(key=lambda w: -w["kCGWindowBounds"]["Width"] * w["kCGWindowBounds"]["Height"])
    return ws[0] if ws else None


def winshot(name):
    import Quartz
    from Foundation import NSURL
    w = main_window()
    if w is None:
        err(f"winshot {name}: no main window")
        return None
    wid = w["kCGWindowNumber"]
    img = Quartz.CGWindowListCreateImage(Quartz.CGRectNull, Quartz.kCGWindowListOptionIncludingWindow, wid,
                                         Quartz.kCGWindowImageBoundsIgnoreFraming)
    if img is None:
        err(f"winshot {name}: capture failed (window {wid})")
        return None
    p = os.path.join(OUT, name + ".png")
    dest = Quartz.CGImageDestinationCreateWithURL(NSURL.fileURLWithPath_(p), "public.png", 1, None)
    Quartz.CGImageDestinationAddImage(dest, img, None)
    Quartz.CGImageDestinationFinalize(dest)
    b = w["kCGWindowBounds"]
    say(f"      winshot {name}: window {wid} bounds {int(b['X'])},{int(b['Y'])} {int(b['Width'])}x{int(b['Height'])} "
        f"onscreen={w.get('kCGWindowIsOnscreen')} image {Quartz.CGImageGetWidth(img)}x{Quartz.CGImageGetHeight(img)}")
    return np.asarray(Image.open(p).convert("RGB")).astype(np.int16)


def green_rect(f):
    # The window capture is colour-managed: the probe's (0, 255, 0) PNG shows as about (117, 251, 76) on this display,
    # so the locator keys on green DOMINANCE, never on raw channel values.
    m = (f[..., 1] >= 150) & (f[..., 1] - f[..., 0] >= 80) & (f[..., 1] - f[..., 2] >= 80)
    if m.sum() < 1000:
        return None
    ys, xs = np.where(m)
    x0, x1, y0, y1 = int(xs.min()), int(xs.max()), int(ys.min()), int(ys.max())
    fill = float(m[y0:y1 + 1, x0:x1 + 1].mean())
    return (x0, y0, x1, y1, fill)


def strips(f, rect, sides, n):
    x0, y0, x1, y1 = rect[:4]
    out = {}
    for s in sides:
        if s == "top":
            r = f[max(0, y0 - n):y0, x0:x1 + 1]
        elif s == "bottom":
            r = f[y1 + 1:y1 + 1 + n, x0:x1 + 1]
        elif s == "left":
            r = f[y0:y1 + 1, max(0, x0 - n):x0]
        else:
            r = f[y0:y1 + 1, x1 + 1:x1 + 1 + n]
        px = r.reshape(-1, 3).astype(float)
        out[s] = (px.mean(axis=0), float(px.std(axis=0).max()), float(px.max()) if px.size else -1.0, px.shape[0])
    return out


def sstr(st):
    return "; ".join(f"{s} mean=({m[0]:.1f},{m[1]:.1f},{m[2]:.1f}) std={sd:.2f} max={mx:.0f} n={k}"
                     for s, (m, sd, mx, k) in st.items())


# ------------------------------------------------------------------ rows
def m6_panel_bars():
    cfg = FIX["m6"]; n = int(cfg["strip"])
    if not neutral():
        return
    ctl = {}
    for (W, H) in cfg["sizes"]:
        for attempt in (1, 2):
            set_size(W, H)
            g = solid_png(f"green_{W}x{H}.png", (W, H), (0, 255, 0))
            code, body = post(A, "/api/load_image", {"filepath": g})
            if code != 200 or not body.get("ok"):
                err(f"m6 control load_image {W}x{H}: HTTP {code} {body}")
            time.sleep(1.5)
            f = winshot(f"m6_ctl_{W}x{H}_{attempt}")
            r = green_rect(f) if f is not None else None
            say(f"      m6 control {W}x{H} attempt {attempt}: present rect {r}")
            if r is not None and r[4] >= 0.99:
                ctl[(W, H)] = (f, r)
                break
            ctl[(W, H)] = (f, None)
    land = [v[1] for k, v in ctl.items() if v[1] is not None and k[0] > k[1]]
    port = [v[1] for k, v in ctl.items() if v[1] is not None and k[0] < k[1]]
    if not land or not port:
        for (W, H) in cfg["sizes"]:
            record(f"m6_panel_bars@{W}x{H}", "RED", False, False, "INVALID: no green present rect found in the control")
        return
    pw = land[0][2] - land[0][0] + 1; ph = port[0][3] - port[0][1] + 1
    say(f"      m6: panel estimate {pw}x{ph} px (landscape rect width x portrait rect height), aspect {pw / ph:.3f}")
    valid = {}
    for (W, H) in cfg["sizes"]:
        f, r = ctl[(W, H)]
        sides = ("top", "bottom") if W / H > pw / ph else ("left", "right")
        if r is None:
            valid[(W, H)] = None; continue
        st = strips(f, r, sides, n)
        good = all(sd <= cfg["ctlStdMax"] and mx <= cfg["ctlMaxChan"] and k > 0 for (_, sd, mx, k) in st.values())
        say(f"      m6 control {W}x{H} rect {r[:4]} bar sides {sides}: {sstr(st)} -> {'VALID' if good else 'INVALID'}")
        valid[(W, H)] = (r, sides, st) if good else None
    load_md(SOLID)
    for (W, H) in cfg["sizes"]:
        row = f"m6_panel_bars@{W}x{H}"
        set_size(W, H)
        f = winshot(f"m6_md_{W}x{H}")
        cap(f"m6_md_canvas_{W}x{H}")
        if valid[(W, H)] is None or f is None:
            record(row, "RED", False, False, "INVALID control (after one rerun) -> FAIL (amendment 14)")
            continue
        r, sides, cst = valid[(W, H)]
        x0, y0, x1, y1 = r[:4]
        ins = f[y0:y1 + 1, x0:x1 + 1].reshape(-1, 3)
        med = np.median(ins, axis=0)
        inpct = 100.0 * float((np.abs(ins - med) <= cfg["insideTol"]).all(axis=1).mean())
        st = strips(f, r, sides, n)
        dm = {s: float(np.abs(st[s][0] - cst[s][0]).max()) for s in sides}
        sok = all(st[s][1] <= cfg["stripStdMax"] and dm[s] <= cfg["stripMeanTol"] for s in sides)
        green = inpct >= cfg["insidePct"] and sok
        fp = any(dm[s] > cfg["stripMeanTol"] for s in sides)
        record(row, "RED", green, fp,
               f"inside rect {inpct:.3f}% within +-{cfg['insideTol']} of median {tuple(int(x) for x in med)}; strips "
               f"{sstr(st)}; |mean - control| per side {', '.join(f'{s} {v:.1f}' for s, v in dm.items())}")


def m1_fill_legacy():
    cfg = FIX["m1"]
    if not neutral():
        return
    load_md(SOLID)
    for kind, sizes in (("RED", cfg["red"]), ("GUARD", cfg["guard"])):
        for (W, H) in sizes:
            set_size(W, H)
            f = cap(f"m1_{W}x{H}")
            row = f"m1_fill_legacy@{W}x{H}"
            if f is None:
                record(row, kind, False, False, "capture failed"); continue
            ok, st = uniform(f)
            green = ok and size_of(f) == (W, H)
            fpok, fps = fingerprint(row, f) if kind == "RED" else (True, "")
            record(row, kind, green, fpok, f"PNG {size_of(f)} {ustr(st)} {fps}")


def m2_live_no_stale():
    cfg = FIX["m2"]
    if not neutral():
        return
    load_md(P1)
    for (W, H) in cfg["sizes"]:
        set_size(W, H, wait=cfg["warm"])
        a, b = pair(f"m2_{W}x{H}", cfg["gap"])
        row = f"m2_live_no_stale@{W}x{H}"
        if a is None or b is None:
            record(row, "RED", False, False, "capture failed"); continue
        ok, txt = tiles(a, b, cfg)
        fa, ta = fingerprint(row, a); fb, tb = fingerprint(row, b)
        record(row, "RED", ok and size_of(a) == (W, H), fa and fb, f"PNG {size_of(a)} {txt}; A {ta}; B {tb}")


def m3_runtime_change():
    cfg = FIX["m3"]
    if not neutral():
        return
    load_md(SOLID)
    set_size(*cfg["seq"][0])
    peak()
    for i, (W, H) in enumerate(cfg["seq"]):
        if i > 0:
            set_size(W, H, wait=cfg["after"])
        pk = peak()
        f = cap(f"m3_{i}_{W}x{H}")
        row = f"m3_runtime_change[{i}]@{W}x{H}"
        if f is None:
            record(row, "RED", False, False, "capture failed"); continue
        ok, st = uniform(f)
        fpok, fps = fingerprint(row, f)
        record(row, "RED", ok and size_of(f) == (W, H), fpok,
               f"PNG {size_of(f)} {ustr(st)} {fps} (INFO peak_frame_time_ms {pk})")


def m3_rewarm():
    cfg = FIX["m3"]["rewarm"]
    if not neutral():
        return
    load_md(P1)
    set_size(*cfg["from"], wait=cfg["warm"])
    ref = cap("m3rw_ref")
    if ref is None:
        err("m3_rewarm: reference capture failed"); return
    L0 = lum(ref)
    peak()
    code, body = post(T8, "/api/set_composition_params", {"outputWidth": cfg["to"][0], "outputHeight": cfg["to"][1]})
    t0 = time.time()
    pts = []
    for at in cfg["at"]:
        time.sleep(max(0.0, at - (time.time() - t0)))
        ts = time.time() - t0
        f = cap(f"m3rw_{at:.2f}")
        tr = time.time() - t0
        if f is None:
            continue
        pts.append((ts, tr, lum(f) / L0 if L0 == L0 and L0 > 0 else float("nan"), size_of(f),
                    100.0 * float((f[..., 3] == 255).mean())))
    pk = peak()
    say("      m3_rewarm: L0=%.2f (alpha-255 mean luminance at 1920x1080); per capture: %s" % (
        L0, "; ".join(f"send +{a:.3f}s recv +{b:.3f}s L/L0={c:.3f} PNG {d} alpha255={e:.2f}%" for a, b, c, d, e in pts)))
    say("DATA m3_rewarm arm=%s %s peak_frame_ms=%s" % (ARM, " ".join(f"t={a:.3f}:{c:.3f}" for a, b, c, d, e in pts), pk))
    record("m3_rewarm", "INFO", True, True, f"INFO (amendment 17; no gate) set_composition_params HTTP {code}")


def m4_comp_load_clip():
    for i, (W, H) in enumerate(FIX["m4"]["sizes"]):
        if not load_comp(f"m4_{W}x{H}", [deck(0, [layer(0, [src_clip(1)])])], (W, H)):
            record(f"m4_comp_load_clip@{W}x{H}", "RED", False, False, "load_composition failed"); continue
        trig(0, 0)
        if i == 0:
            load_md(SOLID)
        time.sleep(1.5)
        f = cap(f"m4_{W}x{H}")
        row = f"m4_comp_load_clip@{W}x{H}"
        if f is None:
            record(row, "RED", False, False, "capture failed"); continue
        ok, st = uniform(f)
        fpok, fps = fingerprint(row, f)
        record(row, "RED", ok and size_of(f) == (W, H), fpok, f"PNG {size_of(f)} {ustr(st)} {fps}")


def m5_preset_switch():
    cfg = FIX["m5"]
    if not neutral(tuple(cfg["size"])):
        return
    load_md(SOLID)
    set_size(*cfg["size"])
    fa = cap("m5_A")
    load_md(SOLID_B)
    time.sleep(cfg["after"])
    fb = cap("m5_B")
    row = "m5_preset_switch"
    if fa is None or fb is None:
        record(row, "RED", False, False, "capture failed"); return
    oka, sta = uniform(fa); okb, stb = uniform(fb)
    diff = float(np.abs(np.array(stb["median"]) - np.array(sta["median"])).max())
    fpok, fps = fingerprint(row, fb)
    record(row, "RED", okb and diff >= cfg["minDiff"], fpok,
           f"A {ustr(sta)}; B PNG {size_of(fb)} {ustr(stb)}; max |median B - median A| = {diff:.0f}; {fps}")


def m7_output_tap():
    cfg = FIX["m7"]; W, H = cfg["size"]
    if not neutral((W, H)):
        return
    code, body = post(T8, "/api/set_output_tap", {"enabled": True})
    if code != 200:
        err(f"set_output_tap on: HTTP {code} {body}")
    load_md(SOLID)
    set_size(W, H)
    p = os.path.join(OUT, "m7_output_probe.png")
    if os.path.exists(p):
        os.remove(p)
    code, body = post(T8, "/api/output_probe", {"output_path": p, "width": W, "height": H}, timeout=30)
    post(T8, "/api/set_output_tap", {"enabled": False})
    row = "m7_output_tap"
    if code != 200 or not os.path.isfile(p):
        record(row, "RED", False, False, f"output_probe HTTP {code} {body}"); return
    f = decode(p)
    rgb = f[..., :3]
    med = np.median(rgb.reshape(-1, 3), axis=0)
    w6 = 100.0 * float((np.abs(rgb - med) <= 6).all(axis=2).mean())
    green = w6 >= 99.99 and float(med.max()) >= 64
    outA = 100.0 * float((np.abs(rgb - CA) > 6).any(axis=2).mean())
    record(row, "RED", green, outA >= cfg["redOutsidePct"],
           f"output_probe {size_of(f)} (canvas {body.get('canvas_w')}x{body.get('canvas_h')}) within6={w6:.4f}% "
           f"median={tuple(int(x) for x in med)}; outside +-6 of bf10_solid {outA:.2f}% (RED bar >= "
           f"{cfg['redOutsidePct']:.0f}%, expected ~69.4%)")


def m8_context_cycle():
    W, H = FIX["m8"]["size"]
    if not neutral((W, H)):
        return
    load_md(SOLID)
    set_size(W, H)
    code, body = post(T8, "/api/debug/gl_context_cycle", {})
    if code != 200:
        record("m8_context_cycle", "RED", False, False, f"gl_context_cycle HTTP {code} {body}"); return
    time.sleep(2.0)
    fi = cap("m8_after_cycle")
    if fi is not None:
        oki, sti = uniform(fi)
        say(f"      m8 INFO right after the cycle (before the reload): PNG {size_of(fi)} {ustr(sti)}")
    load_md(SOLID)
    time.sleep(1.5)
    f = cap("m8")
    row = "m8_context_cycle"
    if f is None:
        record(row, "RED", False, False, "capture failed"); return
    ok, st = uniform(f)
    fpok, fps = fingerprint(row, f)
    record(row, "RED", ok and size_of(f) == (W, H), fpok,
           f"gl_context_cycle gen_before {body.get('gen_before')}; PNG {size_of(f)} {ustr(st)} {fps}")


def m9_deck_roundtrip():
    cfg = FIX["m9"]; W, H = cfg["size"]
    blue = solid_png("blue_1920x1080.png", (W, H), (0, 0, 255))
    decks = [deck(0, [layer(0, [src_clip(1)])]), deck(1, [layer(10, [img_clip(2, blue)])]), deck(2, [layer(20, [])])]
    if not load_comp("m9", decks, (W, H), globalTransitionSpeed=float(cfg["transition"])):
        record("m9_deck_roundtrip", "RED", False, False, "load_composition failed"); return
    trig(0, 0)
    load_md(SOLID)
    load_source("plasma")
    time.sleep(1.5)
    c0 = cap("m9_C0")
    switch(1); time.sleep(1.0); c1 = cap("m9_C1")
    switch(2); time.sleep(1.0); c2 = cap("m9_C2")
    switch(0); time.sleep(1.5); c3 = cap("m9_C3")
    for nm, f in (("C1", c1), ("C2", c2)):
        if f is not None:
            say(f"      m9 INFO {nm} (another deck shown): {ustr(uniform(f)[1])}")
    load_md(P1)
    load_source("plasma")
    time.sleep(3.0)
    switch(1); time.sleep(1.0); switch(2); time.sleep(1.0); switch(0); time.sleep(1.5)
    la, lb = pair("m9_live", FIX["m2"]["gap"])
    row = "m9_deck_roundtrip"
    if c0 is None or c3 is None or la is None or lb is None:
        record(row, "RED", False, False, "capture failed"); return
    ok0, s0 = uniform(c0); ok3, s3 = uniform(c3)
    md = float(np.abs(np.array(s3["median"]) - np.array(s0["median"])).max())
    okl, tl = tiles(la, lb, FIX["m2"])
    fpok, fps = fingerprint(row, c0)
    record(row, "RED", ok0 and ok3 and md <= cfg["medTol"] and okl, fpok,
           f"C0 {ustr(s0)}; C3 {ustr(s3)}; max |median C3 - C0| = {md:.0f}; live pair: {tl}; C0 {fps}")


def quadrant_png(size):
    W, H = size
    a = np.zeros((H, W, 4), np.uint8); a[..., 3] = 255
    a[:H // 2, :W // 2, :3] = (230, 40, 40); a[:H // 2, W // 2:, :3] = (40, 200, 60)
    a[H // 2:, :W // 2, :3] = (50, 70, 220); a[H // 2:, W // 2:, :3] = (235, 235, 235)
    p = os.path.join(OUT, f"quadrant_{W}x{H}.png"); Image.fromarray(a, "RGBA").save(p)
    return p, a.astype(np.int16)


def m10_layer_over():
    cfg = FIX["m10"]; W, H = cfg["size"]
    q, qa = quadrant_png((W, H))
    if not load_comp("m10", [deck(0, [layer(0, [src_clip(1)]), layer(1, [img_clip(2, q)])])], (W, H)):
        record("m10_layer_over", "GUARD", False, True, "load_composition failed"); return
    trig(0, 0)
    load_md(SOLID)
    trig(1, 0)
    time.sleep(1.5)
    shots = [("steady", cap("m10_steady"))]
    set_size(*cfg["via"]); set_size(W, H)
    shots.append(("after round trip", cap("m10_roundtrip")))
    parts, good = [], True
    for nm, f in shots:
        if f is None or size_of(f) != (W, H):
            good = False; parts.append(f"{nm}: capture failed or wrong size"); continue
        pct = 100.0 * float((np.abs(f[..., :3] - qa[..., :3]) <= cfg["tol"]).all(axis=2).mean())
        good &= pct >= cfg["pct"]
        parts.append(f"{nm}: {pct:.4f}% within +-{cfg['tol']} of the quadrant PNG")
    record("m10_layer_over", "GUARD", good, True, "; ".join(parts))


def top_cpu():
    try:
        out = subprocess.run("ps -Ao pcpu=,etime=,comm= | sort -rn | head -5", shell=True, capture_output=True,
                             text=True).stdout
        return " | ".join(" ".join(l.split()[:2]) + " " + os.path.basename(l.split(None, 2)[2]) for l in
                          out.strip().splitlines() if len(l.split()) >= 3)
    except Exception as e:  # noqa: BLE001
        return f"ps failed: {e}"


def perf(tag, size, two):
    cfg = FIX["perf"]
    layers = [layer(0, [src_clip(1)])] + ([layer(1, [src_clip(2)])] if two else [])
    if not load_comp(tag, [deck(0, layers)], size):
        return
    trig(0, 0)
    if two:
        trig(1, 0)
    load_md(HEAVY)
    time.sleep(float(cfg["warmup"]))
    get(A, "/api/state")
    fps, ft, gpu, pg = [], [], [], []
    for _ in range(int(cfg["samples"])):
        time.sleep(float(cfg["interval"]))
        st = get(A, "/api/state")
        if not st:
            continue
        fps.append(float(st.get("fps", 0.0))); ft.append(float(st.get("frame_time_ms", 0.0)))
        if "gpu_time_ms" in st:
            gpu.append(float(st["gpu_time_ms"])); pg.append(float(st.get("peak_gpu_time_ms", 0.0)))
    if not fps:
        err(f"{tag}: no /api/state samples"); return
    f = cap(f"{tag}_look")
    look = f"PNG {size_of(f)} alpha255={100.0 * float((f[..., 3] == 255).mean()):.2f}%" if f is not None else "no PNG"
    g = f"gpu_ms={np.mean(gpu):.3f} gpu_peak_ms={max(pg):.3f}" if gpu else "gpu_ms=nan gpu_peak_ms=nan"
    say(f"DATA {tag} fps={np.mean(fps):.2f} fps_min={min(fps):.2f} frame_ms={np.mean(ft):.3f} {g}")
    say(f"      {tag}: {look}; load avg {' '.join(f'{x:.2f}' for x in os.getloadavg())}; top cpu: {top_cpu()}")
    record(tag, "INFO", True, True, "perf DATA above (G3: .harmony/probe-milkdrop-ab.py applies the bars)")


ROWS = [("m6_panel_bars", m6_panel_bars), ("m1_fill_legacy", m1_fill_legacy), ("m2_live_no_stale", m2_live_no_stale),
        ("m3_runtime_change", m3_runtime_change), ("m3_rewarm", m3_rewarm), ("m4_comp_load_clip", m4_comp_load_clip),
        ("m5_preset_switch", m5_preset_switch), ("m7_output_tap", m7_output_tap),
        ("m9_deck_roundtrip", m9_deck_roundtrip), ("m10_layer_over", m10_layer_over),
        ("m8_context_cycle", m8_context_cycle)]
PERF = [("perf_md_1080", lambda: perf("perf_md_1080", (1920, 1080), False)),
        ("perf_md_4k", lambda: perf("perf_md_4k", (3840, 2160), False)),
        ("perf_md_1080_2l", lambda: perf("perf_md_1080_2l", (1920, 1080), True))]


def fingerprint_consistency():
    """Amendment 2: the box is the panel's physical size, clipped by the canvas, at every RED canvas of the run."""
    good = [b for b in BOXES if b[6]]
    if not good:
        return True
    pw = max(b[3] for b in good); ph = max(b[4] for b in good)
    tol = FP["sizeTolPx"]; ok = True
    say(f"      fingerprint consistency: panel {pw}x{ph} px (largest box of the run; diag 756x840)")
    for row, W, H, bw, bh, cov, _ in good:
        ew, eh = min(pw, W), min(ph, H)
        ecov = 100.0 * ew * eh / (W * H)
        this = abs(bw - ew) <= tol and abs(bh - eh) <= tol and abs(cov - ecov) <= FP["covTolPts"]
        ok &= this
        say(f"      {'ok' if this else 'MISMATCH'} {row}: box {bw}x{bh} vs expected {ew}x{eh}; coverage {cov:.2f}% vs "
            f"{ecov:.2f}%")
    return ok


def main():
    names = [n for n, _ in ROWS] if ONLY is None else ONLY
    table = dict(ROWS + PERF)
    for n in names:
        if n not in table:
            err(f"unknown row {n}"); continue
        say(f"--- {n}")
        table[n]()
    say("")
    if MODE == "pre":
        cons = fingerprint_consistency()
        red = [r for r in RESULTS if r[1] == "RED"]
        void = [r for r in red if r[2] or not r[3]]
        gfail = [r for r in RESULTS if r[1] == "GUARD" and not r[2]]
        say(f"PRE {len(red) - len(void)} RED-OK / {len(void)} VOID; GUARD {sum(1 for r in RESULTS if r[1] == 'GUARD') - len(gfail)} "
            f"PASS / {len(gfail)} GUARD-FAIL; fingerprint consistency {'ok' if cons else 'MISMATCH'}; errors {len(ERRORS)}")
        sys.exit(0 if not void and not gfail and cons and not ERRORS else 1)
    gated = [r for r in RESULTS if r[1] in ("RED", "GUARD")]
    fails = [r for r in gated if not r[2]]
    say(f"PY {len(gated) - len(fails)} PASS / {len(fails)} FAIL; errors {len(ERRORS)}")
    sys.exit(0 if not fails and not ERRORS else 1)


if __name__ == "__main__":
    main()
