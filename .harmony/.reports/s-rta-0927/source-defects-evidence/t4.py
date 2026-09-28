"""T4 must-not-change sweep: every source at explicit defaults and every effect (reset -> registered defaults) on the
test card, captured at 256x256 and 1920x1080, t=1.13. argv[1] = OUT dir. PNGs only; compare offline (t4cmp.py)."""
import sys, os, time, json
from lc import *
OUT = sys.argv[1]; P = OUT + "/png"; os.makedirs(P, exist_ok=True)
IMG = "/Users/boriskarpman/projects/RealTimeAudio/tests/fixtures/test_card.png"
srcs = sources()
post("/api/reset", {})
fxnames = [e["name"] for e in get("/api/state")["effects"]]
log("sources:", len(srcs), "effects:", len(fxnames), time.strftime("%H:%M:%S"))
SIZES = ((256, 256), (1920, 1080))
for sid in sorted(srcs):
    for (w, h) in SIZES:
        post("/api/load_source", {"source_type": sid, "params": defaults(srcs, sid)})
        render(f"{P}/src_{sid}_{w}x{h}.png", t=1.13, w=w, h=h)
log("sources done", time.strftime("%H:%M:%S"))
for name in fxnames:
    for (w, h) in SIZES:
        post("/api/reset", {}); post("/api/load_image", {"filepath": IMG}); time.sleep(0.15)
        post("/api/set_effect", {"name": name, "enabled": True})
        render(f"{P}/fx_{name.replace(' ', '_').replace('/', '_')}_{w}x{h}.png", t=1.13, w=w, h=h)
post("/api/reset", {})
json.dump({"sources": sorted(srcs), "effects": fxnames, "params": {s: [p["uniform"] for p in srcs[s]["params"]] for s in srcs}}, open(OUT + "/t4_index.json", "w"))
log("t4 done", time.strftime("%H:%M:%S"))
