# plan-renderperf C0/C2/C3: byte-identity fixture + capture timing. Talks to the running app on 7070.
# 7070's render_frame has no width/height (ApiServer::handleRenderFrame): the canvas size is the composition's
# outputWidth x outputHeight (Pitfall 37), so each size is its own load of the same fixture.
# usage: capt.py OUT TAG  -> OUT/<TAG>_*.png, sha256, HTTP round trips; the app's err.log lines are parsed by the caller.
import hashlib, json, os, sys, time
import numpy as np, requests
from PIL import Image
A = "http://127.0.0.1:7070"
OUT, TAG = sys.argv[1], sys.argv[2]
MEDIA = "/Users/boriskarpman/projects/RealTimeAudio/media"
S = requests.Session(); S.headers["Connection"] = "close"
def load(size):
    comp = {"name": "renderperf-ab", "activeDeckIndex": 0, "masterOpacity": 1.0, "globalTransitionSpeed": 0.0,
            "decks": [{"name": "D0", "id": 0, "numColumns": 1, "layers": [
                {"name": "L0", "id": 0, "opacity": 0.5, "visible": True, "blendMode": 0, "type": 0, "transitionSpeed": 0.0,
                 "layerEffects": [], "clips": [{"name": "c1", "id": 1, "mediaType": 1,
                                                "mediaFile": MEDIA + "/P16_01_baseline.png", "effects": []}]}]}]}
    if size: comp.update(outputWidth=size[0], outputHeight=size[1])
    path = os.path.join(OUT, f"ab_comp_{size[0] if size else 'default'}.json"); json.dump(comp, open(path, "w"), indent=1)
    r = S.post(A + "/api/load_composition", json={"path": path}, timeout=10).json()
    time.sleep(1.0)
    S.post(A + "/api/trigger_clip", json={"layer": 0, "column": 0}, timeout=6); time.sleep(2.0)
    st = S.get(A + "/api/state", timeout=6).json()
    print(f"load {size or 'default'}: {r}; enabled global effects: {[e.get('name') for e in st.get('effects', []) if e.get('enabled')]}", flush=True)
def cap(name):
    p = os.path.join(OUT, f"{TAG}_{name}.png")
    if os.path.exists(p): os.remove(p)
    t0 = time.perf_counter(); body = S.post(A + "/api/render_frame", json={"output_path": p, "time": 0.0}, timeout=20).json()
    rt = (time.perf_counter() - t0) * 1000
    ok = body.get("ok") is True and os.path.isfile(p)
    h = hashlib.sha256(open(p, "rb").read()).hexdigest() if ok else None
    wh = Image.open(p).size if ok else None
    print(f"CAP {TAG}_{name} png={wh} ok={ok} http_rt_ms={rt:.1f} bytes={os.path.getsize(p) if ok else 0} sha256={h}", flush=True)
    return p if ok else None
def arr(p): return np.asarray(Image.open(p).convert("RGBA"))
load(None)
d1, d2 = cap("def1"), cap("def2")                       # byte-identity A/B at the default 1920x1080 canvas
for k in (1, 2, 3): cap(f"t1080_{k}")                  # timing, 1080p
load((1280, 720)); a1, a2 = cap("720_1"), cap("720_2")  # byte-identity A/B at 1280x720
load((3840, 2160)); cap("t4k_1")                        # timing, 4K
for x, y, n in ((d1, d2, "def"), (a1, a2, "720")):
    if x and y:
        a, b = arr(x), arr(y)
        print(f"PIL {n}1 vs {n}2: shape {a.shape} array_equal={np.array_equal(a, b)} mean RGBA "
              f"{a.reshape(-1,4).mean(0).round(2).tolist()} alpha min/max {a[...,3].min()}/{a[...,3].max()}", flush=True)
