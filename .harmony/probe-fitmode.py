#!/usr/bin/env python3
"""probe-fitmode.py -- REST/pixel half of .harmony/probe-fitmode.sh (s-rta-0926b plan-fitmode: a clip whose
picture is not the composition's shape is shown Stretch (default) / Bars / Crop; plan section 2.9).

The .sh owns launch / refuse / quit; this file talks to the already-running app on port 7070 (and OSC on UDP
8000) and decodes every captured PNG with PIL+numpy (never file hashes). Every render_frame response is
checked; a failed capture is a FAIL. The output dir is fresh per run. Every capture passes `time` (fixture
"time") so same-time captures are byte-comparable.

usage: probe-fitmode.py <root> <fresh-outdir> <media-dir> [row,row,...]
rows: f_stretch_identity f_bars_geometry f_bars_transparent_lower_shows f_crop_geometry f_transform_after_fit
      f_layer_transform_no_double_fit f_stretch_transform_identity f_same_aspect_identity f_source_unaffected
      f_rest_osc_set f_perf
env:  FIT_SHOTS=<dir> -- also write the composition frames (BOX-downscaled to 960x540, RGB as the app shows
      them) + the PIL oracles there.

Metric: d(X, Y) = mean |X - Y| over RGB, 0..255. Fixture A = media/P16_01_baseline.png (756x878 portrait,
opaque), B = media/P16_02_Screen_Split_2x2.png (756x878, opaque). Canvas = outputWidth/outputHeight 1920x1080,
so every frame must be 1920x1080 (a pre-canvas build renders at the panel size -- those rows FAIL on size).
Oracles (PIL BILINEAR): stretch = A.resize((1920,1080)); bars = black canvas with A.resize((929,1080)) at
x=495 (ClipFit::barsRect); crop = A.crop((0,226,756,651)).resize((1920,1080)) (ClipFit::cropRect).
Re-derived on the fixture: d(stretch, bars) = 17.32, d(stretch, crop) = 22.39 -- the RED margins; tol 6.

f_stretch_identity (guard, GREEN on both): L0 Opaque = A with NO fitMode key vs the same with fitMode 0:
  byte-identical (np.array_equal) -- the must-not-change witness.
f_bars_geometry (RED): L0 Opaque = A fitMode 1. PASS: d(f, bars) <= tol; side bands x < 480 and x >= 1440
  mean RGB <= bandTol (mean alpha printed); centre columns [495, 1424) d(., A.resize((929,1080))) <= tol.
f_bars_transparent_lower_shows (RED): L0 Opaque = B (Stretch), L1 Transparent Normal = A fitMode 1; reference
  = B alone. PASS: both side bands equal the reference EXACTLY (RGB) and centre d(., reference) >= centreMin.
f_crop_geometry (RED): L0 Opaque = A fitMode 2. PASS: d(f, crop) <= tol.
f_transform_after_fit (RED): L0 Opaque = B, L1 Transparent = A fitMode 1 + clip scale 0.5 (fit first, then
  the transform). PASS: outside [728,1193) x [270,810) grown by a 12 px margin equals B-only exactly; inside,
  d(., A.resize((465,540))) <= tol.
f_layer_transform_no_double_fit (RED; teeth for the shared-program trap -- applyLayerTransform must reset
  u_fitEnabled): L0 Opaque = A fitMode 1, LAYER layerScale 0.5 with layerAnchorX/Y 0.5 (centre; the Layer
  anchor default is 0 = the corner). expected = black with A.resize((465,540)) at (728,270). PASS: d(f, expected)
  <= tol AND inside [728,1193) x [270,810) d(., A 465x540) <= tol AND outside the box (12 px margin) mean RGB <=
  bandTol. (Whole-frame d alone has no teeth on this dark fixture: 4.32 on the pre-fit build.) A double fit
  squeezes the picture to ~225 px wide (black inside the box).
f_stretch_transform_identity (guard): A fitMode 0 + clip scale 0.5 vs the same without the key: byte-identical.
f_same_aspect_identity (guard): a probe-made 1920x1080 PNG (A resized) with fitMode 1 and 2 vs 0:
  byte-identical (same aspect -> ClipFit::scale() == {1,1} -> no pass).
f_source_unaffected (guard): a checkerboard Source clip fitMode 1 vs 0 at the same time. Two fitMode-0
  captures are compared first: identical -> byte-identical required; else d <= 2 x that noise floor (said).
f_rest_osc_set (RED): deck 0 layer 0, col 0 = A fitMode 0 (triggered), col 1 = B. POST /api/set_clip_param
  {layer 0, column 0, param fitMode, value 1} -> /api/composition readback 1 and d(f1, bars) <= tol; value 0
  -> readback 0 and f2 byte-identical to f0; bad requests (value 3, param "bogus") answer ok:false and change
  nothing. OSC (hand-encoded ,i on UDP 8000): /audiodna/clip/0/1/fit 1 -> col 1 readback 1 AND the layer's
  activeClipColumn stays 0 (the bare /audiodna/clip/{l}/{c} trigger branch did NOT fire); /audiodna/clip/0/0/fit 2
  -> readback 2 and d(f3, crop) <= tol; /audiodna/clip/0/0/fit 0 -> readback 0.
f_perf (REPORT, never FAILs; FINDING if a delta > 1 ms): three Transparent layers each = A, fitMode 1 vs 0;
  /api/state frame_time_ms and gpu_time_ms after 3 s, 10 x 0.5 s; both means, the deltas and the load average.
  Waits until no clang/clang++ is running.

Calibration: the lane report .harmony/.reports/s-rta-0926b/canvas.md, section "Fit mode".
"""
import json, os, socket, struct, subprocess, sys, time

import numpy as np
import requests
from PIL import Image

A = "http://127.0.0.1:7070"
ROOT, OUT, MEDIA = sys.argv[1], sys.argv[2], sys.argv[3]
ONLY = set(sys.argv[4].split(",")) if len(sys.argv) > 4 and sys.argv[4] else None
SHOTS = os.environ.get("FIT_SHOTS", "")
FIX = json.load(open(os.path.join(ROOT, ".harmony", "probe-fitmode.json")))
IMG_A = FIX["imageA"].replace("@MEDIA@", MEDIA)
IMG_B = FIX["imageB"].replace("@MEDIA@", MEDIA)
CW, CH = (int(v) for v in FIX["canvas"])
T = float(FIX["time"])
TOL = float(FIX["tol"])
BAND_TOL = float(FIX["bandTol"])
CENTRE_MIN = float(FIX["centreMin"])
PASS = FAIL = 0
S = requests.Session()
KEEP = {}


def ok(msg):
    global PASS; PASS += 1; print(f"PASS  {msg}", flush=True)


def no(msg):
    global FAIL; FAIL += 1; print(f"FAIL  {msg}", flush=True)


def clip(cid, img, **extra):
    c = {"name": f"c{cid}", "id": cid, "mediaType": 1, "mediaFile": img, "effects": []}
    c.update(extra)
    return c


def source_clip(cid, **extra):
    src = FIX["source"]
    c = {"name": f"s{cid}", "id": cid, "mediaType": 4, "sourceType": src["sourceType"], "effects": [],
         "sourceParams": [{"name": n, "uniform": u, "value": v, "default": v} for n, u, v in src["sourceParams"]]}
    c.update(extra)
    return c


def layer(lid, clips, ltype=0, **extra):
    """blendMode 0 = Normal (the Layer default is Additive); ltype 0 Opaque, 1 Transparent."""
    l = {"name": f"L{lid}", "id": lid, "opacity": 1.0, "visible": True, "blendMode": 0, "type": ltype,
         "transitionSpeed": 0.0, "layerEffects": [], "clips": clips}
    l.update(extra)
    return l


def deck(did, layers, ncols=1):
    return {"name": f"D{did}", "id": did, "numColumns": ncols, "layers": layers}


def load(tag, decks):
    comp = {"name": "fit-" + tag, "activeDeckIndex": 0, "masterOpacity": 1.0, "globalTransitionSpeed": 0.0,
            "outputWidth": CW, "outputHeight": CH, "decks": decks}
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


def cap(name):
    """render_frame (7070) into the fresh dir at the fixture time; deletes a pre-existing file first, then
    requires ok:true, the file, and an mtime at/after the request (never decode a PNG this call did not write)."""
    p = os.path.join(OUT, name + ".png")
    if os.path.exists(p):
        os.remove(p)
    t0 = time.time()
    try:
        body = S.post(A + "/api/render_frame", json={"output_path": p, "time": T}, timeout=30).json()
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


def show(tag, decks, trigs=((0, 0),), settle=1.5):
    if not load(tag, decks):
        return None
    for li, col in trigs:
        trig(li, col)
    time.sleep(settle)
    return cap(tag)


def size_of(f):
    return (int(f.shape[1]), int(f.shape[0]))


def on_canvas(tag, f):
    if f is None:
        no(f"{tag}: capture failed"); return False
    if size_of(f) != (CW, CH):
        no(f"{tag}: frame is {size_of(f)[0]}x{size_of(f)[1]}, not the {CW}x{CH} canvas (a pre-canvas build renders "
           f"at the preview panel's size -- the fit geometry cannot be measured)")
        return False
    return True


def d(x, y):
    return float(np.abs(x[..., :3] - y[..., :3]).mean())


def same_rgb(x, y):
    return x.shape == y.shape and bool(np.array_equal(x[..., :3], y[..., :3]))


def nonblank(a):
    return float((a[..., 3] > 0).mean()) >= 0.5 and float(a[..., :3].std()) >= 3.0


def pil(path_or_arr):
    if isinstance(path_or_arr, str):
        return Image.open(path_or_arr).convert("RGBA")
    return Image.fromarray(np.clip(path_or_arr, 0, 255).astype(np.uint8), "RGBA")


def arr(im):
    return np.asarray(im).astype(float)


def pasted(base, img, box_size, at):
    """base (array or None = transparent black canvas) with img resized to box_size pasted at `at`."""
    canvas = pil(base) if base is not None else Image.new("RGBA", (CW, CH), (0, 0, 0, 0))
    canvas = canvas.copy()
    canvas.paste(pil(img).resize(box_size, Image.BILINEAR), at)
    return arr(canvas)


def shot(name, f):
    if not SHOTS or f is None:
        return
    os.makedirs(SHOTS, exist_ok=True)
    im = pil(f).convert("RGB")   # RGB as the app shows it (the present pass / recorder ignore alpha)
    im.resize((960, 540), Image.BOX).save(os.path.join(SHOTS, name + ".png"))


IM_A = pil(IMG_A)
BX, BW = int(FIX["bars"]["x"]), int(FIX["bars"]["w"])
BAND_L, BAND_R = int(FIX["bars"]["bandL"]), int(FIX["bars"]["bandR"])
STRETCH = arr(IM_A.resize((CW, CH), Image.BILINEAR))
BARS = pasted(None, IMG_A, (BW, CH), (BX, 0))
A_BARS = arr(IM_A.resize((BW, CH), Image.BILINEAR))
CROP = arr(IM_A.crop(tuple(FIX["crop"]["box"])).resize((CW, CH), Image.BILINEAR))
SC = FIX["scaled"]
SX, SY, SW, SH, SM = int(SC["x"]), int(SC["y"]), int(SC["w"]), int(SC["h"]), int(SC["margin"])
A_SMALL = arr(IM_A.resize((SW, SH), Image.BILINEAR))


def one_a(**extra):
    return [deck(0, [layer(0, [clip(1, IMG_A, **extra)])])]


def b_ref():
    if "b_ref" not in KEEP:
        KEEP["b_ref"] = show("b_only", [deck(0, [layer(0, [clip(1, IMG_B)])])])
    return KEEP["b_ref"]


def a_over_b(tag, **extra):
    return show(tag, [deck(0, [layer(0, [clip(1, IMG_B)]), layer(1, [clip(2, IMG_A, **extra)], ltype=1)])],
                trigs=((0, 0), (1, 0)))


# ------------------------------------------------------------------ rows
def f_stretch_identity():
    fa = show("stretch_nokey", one_a())
    fb = show("stretch_key0", one_a(fitMode=0))
    if fa is None or fb is None:
        no("f_stretch_identity: capture failed"); return
    info = f"d(f, stretch oracle)={d(fa, STRETCH):.2f}" if size_of(fa) == (CW, CH) else f"frame {size_of(fa)}"
    shot("stretch", fa)
    (ok if same_rgb(fa, fb) and np.array_equal(fa, fb) else no)(
        f"f_stretch_identity: A with no fitMode key vs fitMode 0 -- byte-identical={np.array_equal(fa, fb)} "
        f"(max |diff| {float(np.abs(fa - fb).max()) if fa.shape == fb.shape else 'shape differs'}; {info})")


def f_bars_geometry():
    f = show("bars", one_a(fitMode=1))
    if not on_canvas("f_bars_geometry", f):
        return
    shot("bars", f)
    de = d(f, BARS)
    bl, br = float(f[:, :BAND_L, :3].mean()), float(f[:, BAND_R:, :3].mean())
    al, ar = float(f[:, :BAND_L, 3].mean()), float(f[:, BAND_R:, 3].mean())
    dc = d(f[:, BX:BX + BW], A_BARS)
    good = de <= TOL and bl <= BAND_TOL and br <= BAND_TOL and dc <= TOL
    (ok if good else no)(
        f"f_bars_geometry: A fitMode 1 = its own shape, centred, bars beside it -- d(f, bars)={de:.2f} (tol {TOL}), "
        f"band RGB L={bl:.2f} R={br:.2f} (<= {BAND_TOL}; alpha L={al:.1f} R={ar:.1f}), centre d={dc:.2f} "
        f"[d(f, stretch)={d(f, STRETCH):.2f}]")


def f_bars_transparent_lower_shows():
    ref = b_ref()
    f = a_over_b("bars_over_lower", fitMode=1)
    if not on_canvas("f_bars_transparent_lower_shows", f) or not on_canvas("f_bars_transparent_lower_shows ref", ref):
        return
    shot("bars-over-lower-layer", f)
    eqL = same_rgb(f[:, :BAND_L], ref[:, :BAND_L])
    eqR = same_rgb(f[:, BAND_R:], ref[:, BAND_R:])
    dL, dR = d(f[:, :BAND_L], ref[:, :BAND_L]), d(f[:, BAND_R:], ref[:, BAND_R:])
    dc = d(f[:, BX:BX + BW], ref[:, BX:BX + BW])
    (ok if eqL and eqR and dc >= CENTRE_MIN else no)(
        f"f_bars_transparent_lower_shows: the bars are transparent -- the lower layer (B) shows beside A: "
        f"bands == B-only exactly L={eqL} R={eqR} (d L={dL:.2f} R={dR:.2f}), centre d(f, B-only)={dc:.2f} "
        f"(>= {CENTRE_MIN})")


def f_crop_geometry():
    f = show("crop", one_a(fitMode=2))
    if not on_canvas("f_crop_geometry", f):
        return
    shot("crop", f)
    de = d(f, CROP)
    (ok if de <= TOL else no)(
        f"f_crop_geometry: A fitMode 2 fills the canvas, top and bottom cut -- d(f, crop)={de:.2f} (tol {TOL}) "
        f"[d(f, stretch)={d(f, STRETCH):.2f}]")


def outside_mask():
    m = np.ones((CH, CW), dtype=bool)
    m[max(0, SY - SM):SY + SH + SM, max(0, SX - SM):SX + SW + SM] = False
    return m


def f_transform_after_fit():
    ref = b_ref()
    f = a_over_b("bars_clip_scale", fitMode=1, scale=0.5)
    if not on_canvas("f_transform_after_fit", f) or not on_canvas("f_transform_after_fit ref", ref):
        return
    shot("bars-clip-scale-0.5", f)
    m = outside_mask()
    eq_out = bool(np.array_equal(f[m][:, :3], ref[m][:, :3]))
    d_out = float(np.abs(f[m][:, :3] - ref[m][:, :3]).mean())
    d_in = d(f[SY:SY + SH, SX:SX + SW], A_SMALL)
    (ok if eq_out and d_in <= TOL else no)(
        f"f_transform_after_fit: fit first, then the clip's scale 0.5 -- outside == B-only exactly={eq_out} "
        f"(d {d_out:.2f}), inside d(., A {SW}x{SH})={d_in:.2f} (tol {TOL})")


def f_layer_transform_no_double_fit():
    f = show("bars_layer_scale", [deck(0, [layer(0, [clip(1, IMG_A, fitMode=1)], layerScale=0.5,
                                                   layerAnchorX=0.5, layerAnchorY=0.5)])])
    if not on_canvas("f_layer_transform_no_double_fit", f):
        return
    shot("bars-layer-scale-0.5", f)
    exp = pasted(None, IMG_A, (SW, SH), (SX, SY))
    stretched_half = pasted(None, IMG_A, (CW // 2, CH // 2), (CW // 4, CH // 4))
    # A is dark (mean ~20), so the whole-frame d alone cannot tell "fitted once" from "not fitted" (calibrated on
    # the pre-fit build: 4.32 < tol). The row therefore also needs the picture region to match A at 465x540 and
    # everything outside it (12 px margin) to be black: no fit leaves A beside the box, a double fit leaves
    # black bars INSIDE it.
    de = d(f, exp)
    d_in = d(f[SY:SY + SH, SX:SX + SW], A_SMALL)
    m = outside_mask()
    out_rgb = float(f[m][:, :3].mean())
    (ok if de <= TOL and d_in <= TOL and out_rgb <= BAND_TOL else no)(
        f"f_layer_transform_no_double_fit: Bars clip under a layer scale 0.5 is fitted ONCE -- d(f, expected)={de:.2f}, "
        f"inside d(., A {SW}x{SH})={d_in:.2f} (tol {TOL}), outside mean RGB={out_rgb:.2f} (<= {BAND_TOL}) "
        f"[d(f, stretched picture at half size)={d(f, stretched_half):.2f}]")


def f_stretch_transform_identity():
    fa = show("stretch_scale_nokey", one_a(scale=0.5))
    fb = show("stretch_scale_key0", one_a(scale=0.5, fitMode=0))
    if fa is None or fb is None:
        no("f_stretch_transform_identity: capture failed"); return
    (ok if fa.shape == fb.shape and np.array_equal(fa, fb) else no)(
        f"f_stretch_transform_identity: A scale 0.5 with no key vs fitMode 0 -- byte-identical="
        f"{fa.shape == fb.shape and bool(np.array_equal(fa, fb))} (frame {size_of(fa)})")


def f_same_aspect_identity():
    p = os.path.join(OUT, "a_1920x1080.png")
    IM_A.resize((CW, CH), Image.BILINEAR).save(p)
    fr = {}
    for m in (0, 1, 2):
        fr[m] = show(f"same_aspect_{m}", [deck(0, [layer(0, [clip(1, p, fitMode=m)])])])
        if fr[m] is None:
            no("f_same_aspect_identity: capture failed"); return
    e1 = fr[1].shape == fr[0].shape and bool(np.array_equal(fr[1], fr[0]))
    e2 = fr[2].shape == fr[0].shape and bool(np.array_equal(fr[2], fr[0]))
    (ok if e1 and e2 else no)(
        f"f_same_aspect_identity: a {CW}x{CH} picture is untouched by Bars / Crop -- Bars==Stretch {e1}, "
        f"Crop==Stretch {e2} (frame {size_of(fr[0])})")


def f_source_unaffected():
    if not load("source_0", [deck(0, [layer(0, [source_clip(1, fitMode=0)])])]):
        return
    trig(0, 0); time.sleep(1.5)
    s0a, s0b = cap("source_0a"), cap("source_0b")
    s1 = show("source_1", [deck(0, [layer(0, [source_clip(1, fitMode=1)])])])
    if s0a is None or s0b is None or s1 is None:
        no("f_source_unaffected: capture failed"); return
    floor = d(s0a, s0b)
    nb = nonblank(s0a)
    if floor == 0.0:
        good = s1.shape == s0a.shape and bool(np.array_equal(s1, s0a))
        how = "byte-identical required (two fitMode-0 captures are identical)"
    else:
        good = d(s1, s0a) <= 2.0 * floor
        how = f"DOWNGRADED to d <= 2 x noise floor {floor:.3f} (two fitMode-0 captures differ)"
    (ok if good and nb else no)(
        f"f_source_unaffected: a {FIX['source']['sourceType']} Source ignores fitMode -- {how}: "
        f"d(fitMode 1, fitMode 0)={d(s1, s0a):.3f}, non-blank={nb} (frame {size_of(s0a)})")


def comp_clip(layer_idx, column):
    """(fitMode or None, activeClipColumn) from 7070 /api/composition, deck 0."""
    try:
        c = S.get(A + "/api/composition", timeout=6).json()
        lay = c["decks"][0]["layers"][layer_idx]
        for cl in lay.get("clips", []):
            if int(cl.get("column", -1)) == column:
                return cl.get("fitMode"), lay.get("activeClipColumn")
        return None, lay.get("activeClipColumn")
    except Exception as e:  # noqa: BLE001
        print(f"      /api/composition unreadable: {e}", flush=True)
        return None, None


def set_fit(layer_idx, column, value, param="fitMode"):
    try:
        r = S.post(A + "/api/set_clip_param",
                   json={"layer": layer_idx, "column": column, "param": param, "value": value}, timeout=6)
        return r.status_code, r.text.strip()[:120]
    except Exception as e:  # noqa: BLE001
        return -1, str(e)[:120]


def osc(address, value):
    def pad(b):
        return b + b"\0" * (4 - len(b) % 4)
    msg = pad(address.encode()) + pad(b",i") + struct.pack(">i", int(value))
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        s.sendto(msg, (FIX["osc"]["host"], int(FIX["osc"]["port"])))
    finally:
        s.close()


def f_rest_osc_set():
    tag = "f_rest_osc_set"
    if not load("rest", [deck(0, [layer(0, [clip(1, IMG_A, fitMode=0), clip(2, IMG_B)])], ncols=2)]):
        return
    trig(0, 0); time.sleep(1.5)
    f0 = cap("rest_f0")
    if f0 is None:
        no(f"{tag}: capture failed"); return
    bad = []
    px = size_of(f0) == (CW, CH)   # the REST / OSC readback checks run on any build; the pixel checks need the canvas
    if not px:
        bad.append(f"frame {size_of(f0)} is not the {CW}x{CH} canvas -- pixel checks skipped")
    st, body = set_fit(0, 0, 1); time.sleep(0.5)
    rb, act = comp_clip(0, 0)
    f1 = cap("rest_f1")
    d1 = d(f1, BARS) if f1 is not None and px else 999.0
    print(f"      REST value 1: HTTP {st} {body} -> readback fitMode={rb} activeClipColumn={act}; d(f1, bars)={d1:.2f}",
          flush=True)
    if rb != 1: bad.append(f"REST 1 readback {rb}")
    if px and d1 > TOL: bad.append(f"REST 1 d(f1, bars) {d1:.2f}")
    for v, p in ((3, "fitMode"), (1, "bogus")):
        st, body = set_fit(0, 0, v, p); time.sleep(0.3)
        rbx, _ = comp_clip(0, 0)
        refused = st == 200 and '"ok":false' in body.replace(" ", "")
        print(f"      REST bad request param={p} value={v}: HTTP {st} {body} -> readback {rbx}", flush=True)
        if not refused or rbx != 1: bad.append(f"bad request {p}={v} not refused (HTTP {st}, readback {rbx})")
    st, body = set_fit(0, 0, 0); time.sleep(0.5)
    rb, act = comp_clip(0, 0)
    f2 = cap("rest_f2")
    eq = f2 is not None and f2.shape == f0.shape and bool(np.array_equal(f2, f0))
    print(f"      REST value 0: HTTP {st} {body} -> readback {rb}; f2 == f0 byte-identical {eq}", flush=True)
    if rb != 0: bad.append(f"REST 0 readback {rb}")
    if not eq: bad.append("REST 0 frame != f0")
    # OSC: /clip/0/1/fit on the NOT-active column first -- the bare trigger branch would switch the layer to col 1.
    osc("/audiodna/clip/0/1/fit", 1); time.sleep(0.6)
    rb1, act = comp_clip(0, 1)
    print(f"      OSC /audiodna/clip/0/1/fit 1 -> col 1 fitMode={rb1}, activeClipColumn={act}", flush=True)
    if rb1 != 1: bad.append(f"OSC col1 readback {rb1}")
    if act != 0: bad.append(f"OSC fit address TRIGGERED the clip (activeClipColumn {act})")
    osc("/audiodna/clip/0/0/fit", 2); time.sleep(0.6)
    rb, act = comp_clip(0, 0)
    f3 = cap("rest_f3")
    d3 = d(f3, CROP) if f3 is not None and px else 999.0
    print(f"      OSC /audiodna/clip/0/0/fit 2 -> fitMode={rb}, activeClipColumn={act}; d(f3, crop)={d3:.2f}", flush=True)
    if rb != 2: bad.append(f"OSC col0 readback {rb}")
    if px and d3 > TOL: bad.append(f"OSC 2 d(f3, crop) {d3:.2f}")
    if act != 0: bad.append(f"activeClipColumn {act}")
    osc("/audiodna/clip/0/0/fit", 0); time.sleep(0.6)
    rb, act = comp_clip(0, 0)
    print(f"      OSC /audiodna/clip/0/0/fit 0 -> fitMode={rb}, activeClipColumn={act}", flush=True)
    if rb != 0: bad.append(f"OSC 0 readback {rb}")
    if act != 0: bad.append(f"activeClipColumn {act}")
    (ok if not bad else no)(f"f_rest_osc_set: /api/set_clip_param and /audiodna/clip/{{l}}/{{c}}/fit flip the fit live, "
                            f"/api/composition reads it back, the OSC address never triggers (failures {bad[:10]})")


def loadavg():
    return "load avg %.2f %.2f %.2f" % os.getloadavg()


def wait_no_compiler(tag, limit_s=1800):
    """Rig rule: perf rows run only when no compiler is running."""
    t0 = time.time()
    while True:
        busy = [n for n in ("clang", "clang++") if subprocess.run(["pgrep", "-x", n], capture_output=True).stdout.strip()]
        if not busy:
            return True
        if time.time() - t0 > limit_s:
            print(f"REPORT {tag}: a compiler ({busy}) kept running for {limit_s} s -- perf not measured", flush=True)
            return False
        print(f"      {tag}: waiting for {busy} to finish ({loadavg()})", flush=True)
        time.sleep(20)


def f_perf():
    cfg = FIX["perf"]
    if not wait_no_compiler("f_perf"):
        return
    means = {}
    for m in (1, 0):
        n = int(cfg["layers"])
        if not load(f"perf_{m}", [deck(0, [layer(i, [clip(10 + i, IMG_A, fitMode=m)], ltype=1) for i in range(n)])]):
            return
        for i in range(n):
            trig(i, 0)
        time.sleep(float(cfg["warm"]))
        ft, fps, gp, s = [], [], [], {}
        for _ in range(int(cfg["samples"])):
            time.sleep(float(cfg["every"]))
            try:
                s = S.get(A + "/api/state", timeout=6).json()
                ft.append(float(s.get("frame_time_ms", 0.0))); fps.append(float(s.get("fps", 0.0)))
                if s.get("gpu_time_ms") is not None:
                    gp.append(float(s["gpu_time_ms"]))
            except Exception:  # noqa: BLE001
                pass
        means[m] = (float(np.mean(ft)) if ft else float("nan"), float(np.mean(fps)) if fps else float("nan"),
                    float(np.mean(gp)) if gp else float("nan"))
        print(f"      perf fitMode {m}: frame_time_ms mean {means[m][0]:.3f} (n={len(ft)}), fps mean {means[m][1]:.1f}, "
              f"gpu_time_ms mean {means[m][2]:.3f} (n={len(gp)}) ({loadavg()})", flush=True)
    delta = means[1][0] - means[0][0]
    gdelta = means[1][2] - means[0][2]
    tag = "FINDING" if delta > float(cfg["findingMs"]) or gdelta > float(cfg["findingMs"]) else "ok"
    print(f"REPORT f_perf: three Bars layers vs three Stretch layers -- frame_time_ms {means[1][0]:.3f} vs "
          f"{means[0][0]:.3f} (delta {delta:+.3f} ms), gpu_time_ms {means[1][2]:.3f} vs {means[0][2]:.3f} "
          f"(delta {gdelta:+.3f} ms) ({tag}; FINDING above {cfg['findingMs']} ms); {loadavg()}", flush=True)


def oracles():
    if not SHOTS:
        return
    for name, a in (("expected-stretch", STRETCH), ("expected-bars", BARS), ("expected-crop", CROP)):
        shot(name, a)


def main():
    rows = [("f_stretch_identity", f_stretch_identity),
            ("f_bars_geometry", f_bars_geometry),
            ("f_bars_transparent_lower_shows", f_bars_transparent_lower_shows),
            ("f_crop_geometry", f_crop_geometry),
            ("f_transform_after_fit", f_transform_after_fit),
            ("f_layer_transform_no_double_fit", f_layer_transform_no_double_fit),
            ("f_stretch_transform_identity", f_stretch_transform_identity),
            ("f_same_aspect_identity", f_same_aspect_identity),
            ("f_source_unaffected", f_source_unaffected),
            ("f_rest_osc_set", f_rest_osc_set),
            ("f_perf", f_perf)]
    oracles()
    for name, fn in rows:
        if ONLY is None or name in ONLY:
            print(f"--- {name}", flush=True)
            fn()
    print(f"\nPY {PASS} PASS / {FAIL} FAIL", flush=True)
    sys.exit(0 if FAIL == 0 else 1)


if __name__ == "__main__":
    main()
