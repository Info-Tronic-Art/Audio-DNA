"""Fix-round live rows: crystal_cavern at its explicit registered defaults (from THIS app's /api/sources), decoded PNGs,
plus the grazed-crystal box dark fraction at the two pinned 1920x1080 frames (same boxes as the T2 case). argv[1] = OUT."""
import sys, os, json
from lc import *
OUT = sys.argv[1]; P = OUT + "/png"; os.makedirs(P, exist_ok=True)
srcs = sources(); log("sources:", len(srcs), " health:", get("/api/health"))
post("/api/reset", {})
d = defaults(srcs, "crystal_cavern"); log("crystal_cavern defaults:", d)
post("/api/load_source", {"source_type": "crystal_cavern", "params": d})
BOX = {10.0: (1560, 440, 1900, 800), 24.0: (1300, 340, 1600, 620)}
rows = []
for (w, h) in ((256, 256), (1920, 1080)):
    for t in (1.0, 5.0, 10.0, 24.0):
        name = f"crystal_cavern_{w}x{h}_t{t:g}.png"
        im = render(f"{P}/{name}", t=t, w=w, h=h); mt = metrics(im)
        extra = ""
        if w == 1920 and t in BOX:
            x0, y0, x1, y1 = BOX[t]; m = im[y0:y1, x0:x1].max(axis=2)
            mt["box_dark"] = float((m < 40).mean()); extra = f" grazed-crystal box dark={mt['box_dark']*100:.1f}%"
        log(f"ROW crystal_cavern {w}x{h} t={t:<5g} {fmt(mt)}{extra}")
        rows.append({"w": w, "h": h, "t": t, **mt, "png": name})
log("\n== 20 s sweep 96x54 every 0.25 s (visible + not flat)")
bad = 0
for i in range(81):
    t = 0.25 * i; im = render(f"{P}/sweep.png", t=t, w=96, h=54); mt = metrics(im)
    b = not visible(mt); fl = (not b) and mt["sd"] < 4.0
    bad += b or fl
    if b or fl: log(f"  BAD t={t} {fmt(mt)}")
log(f"SWEEP 81 frames: {bad} black-or-flat")
json.dump(rows, open(OUT + "/cav_rows.json", "w"), indent=1)
log("cav done")
