"""T3 live rows (plan-source-defects.md T3) + the A5 crystal_cavern diagnosis sweep. argv[1] = OUT dir.
Every source is loaded with its FULL explicit default dict from /api/sources of THIS app (H1)."""
import sys, os, json
from lc import *
OUT = sys.argv[1]; P = OUT + "/png"; os.makedirs(P, exist_ok=True)
IMG = "/Users/boriskarpman/projects/RealTimeAudio/tests/fixtures/test_card.png"
srcs = sources(); log("sources:", len(srcs), " health:", get("/api/health"))
log("composition_params:", get("/api/composition_params"))
rows = []
def row(sid, params, w, h, t, tag=""):
    post("/api/load_source", {"source_type": sid, "params": params})
    name = f"{sid}{tag}_{w}x{h}_t{t:g}.png"
    im = render(f"{P}/{name}", t=t, w=w, h=h); mt = metrics(im)
    log(f"ROW {sid:15s} {tag:14s} {w}x{h} t={t:<5g} {fmt(mt)} visible={visible(mt)}")
    rows.append({"id": sid, "tag": tag, "w": w, "h": h, "t": t, **mt, "visible": visible(mt), "png": name})
    return im
post("/api/reset", {})
for sid in ("julia_set", "burning_ship"):
    d = defaults(srcs, sid); log(f"{sid} defaults: {d}")
    for (w, h) in ((256, 256), (1920, 1080)):
        for t in (0.0, 1.13, 10.0): row(sid, d, w, h, t)
d = defaults(srcs, "newton_3d"); log(f"newton_3d defaults: {d}")
for (w, h) in ((256, 256), (1920, 1080)):
    for t in (0.0, 1.0, 1.13, 30.0, 45.0): row("newton_3d", d, w, h, t)
d = defaults(srcs, "sierpinski"); log(f"sierpinski defaults: {d}")
for (w, h) in ((256, 256), (512, 512), (1024, 1024), (1920, 1080)): row("sierpinski", d, w, h, 1.13)
row("sierpinski", {**d, "u_src_iterations": 0.75}, 1024, 1024, 1.13, "_iter0.75")
row("sierpinski", {**d, "u_src_iterations": 0.75}, 1920, 1080, 1.13, "_iter0.75")
log("\n== A5 crystal_cavern diagnosis: explicit defaults, lit% by time (prediction: onset 2.0-3.0 s at Speed 0.3, 4-6 s at Speed 0.0)")
d = defaults(srcs, "crystal_cavern"); log(f"crystal_cavern defaults: {d}")
for t in (0, 0.5, 1, 1.5, 2, 2.5, 3, 4, 5, 10, 30, 60): row("crystal_cavern", d, 256, 256, float(t))
for t in (0, 3, 4, 5, 6, 8): row("crystal_cavern", {**d, "u_src_speed": 0.0}, 256, 256, float(t), "_speed0")
for t in (1.0, 5.0, 10.0): row("crystal_cavern", d, 1920, 1080, t)
log("\n== Dot Field on the test card (legacy effect chain via set_effect; /api/reset restores the registered defaults)")
def fx(name, params, w, h, tag):
    post("/api/reset", {}); post("/api/load_image", {"filepath": IMG})
    import time as _t; _t.sleep(0.3)
    if name: post("/api/set_effect", {"name": name, "enabled": True, "params": params})
    fn = f"fx_{(name or 'none').replace(' ', '_')}{tag}_{w}x{h}.png"
    im = render(f"{P}/{fn}", t=1.13, w=w, h=h); mt = metrics(im)
    log(f"FX  {name or 'none':12s} {tag:12s} {w}x{h} {fmt(mt)}")
    rows.append({"id": "fx:" + (name or "none"), "tag": tag, "w": w, "h": h, "t": 1.13, **mt, "png": fn})
    return im
for (w, h) in ((256, 256), (1920, 1080)):
    base = fx(None, {}, w, h, "")
    inv = fx("Invert", {}, w, h, "")
    log(f"    Pitfall-28 check: Invert vs none PSNR={psnr(base, inv):.1f} (finite = render_frame applies the chain)")
    dflt = fx("Dot Field", {}, w, h, "_default")
    dep0 = fx("Dot Field", {"depth": 0.0}, w, h, "_depth0")
    dep1 = fx("Dot Field", {"depth": 1.0}, w, h, "_depth1")
    s10 = fx("Dot Field", {"size": 1.0}, w, h, "_size1")
    log(f"    Dot Field {w}x{h}: PSNR(default, depth 0)={psnr(dflt, dep0):.1f} PSNR(default, depth 1)={psnr(dflt, dep1):.1f} PSNR(default, size 1)={psnr(dflt, s10):.1f}")
post("/api/reset", {})
log("\n== twisted_torus / torus family registered params")
for sid in ("twisted_torus", "striped_torus", "spiral_vortex", "checker_torus", "ribbed_vortex", "wormhole_tunnel", "wormhole", "torus_hole",
            "mandelbulb", "newton_3d", "band_tower", "structural_landscape", "julia_set_3d"):
    log(f"PARAMS {sid:20s} n={len(srcs[sid]['params'])}: {[p['name'] for p in srcs[sid]['params']]}")
log("TOTAL source params:", sum(len(s["params"]) for s in srcs.values()), "sources:", len(srcs))
json.dump(rows, open(OUT + "/t3_rows.json", "w"), indent=1)
log("t3 done; rows:", len(rows))
