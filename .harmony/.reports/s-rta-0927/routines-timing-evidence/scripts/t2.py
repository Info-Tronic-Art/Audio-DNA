# T2 measure: frame times around a layer's FIRST crossfade (the spare frame ring is created), then a
# second crossfade (no creation) for comparison. Polls /api/state peak_frame_time_ms (reset on read).
import json, os, sys, time, requests
A = "http://127.0.0.1:7070"
OUT, LID, W, H = sys.argv[1], int(sys.argv[2]), int(sys.argv[3]), int(sys.argv[4])
MEDIA = "/Users/boriskarpman/projects/RealTimeAudio/media"
IMG_A, IMG_B = MEDIA + "/P16_01_baseline.png", MEDIA + "/P16_02_Screen_Split_2x2.png"
S = requests.Session()
fx = {"name": "Screen Split", "enabled": True, "bypassed": False, "dryWet": 1.0, "params": [0.15, 0.15, 0.25, 0.0]}
clips = [{"name": "c%d" % k, "id": 10 + k, "mediaType": 1, "mediaFile": IMG_A if k % 2 == 0 else IMG_B, "effects": [fx]} for k in range(6)]
comp = {"name": "t2", "activeDeckIndex": 0, "masterOpacity": 1.0, "globalTransitionSpeed": 0.0,
        "outputWidth": W, "outputHeight": H,
        "decks": [{"name": "D0", "id": 0, "numColumns": 6, "layers": [
            {"name": "L", "id": LID, "opacity": 1.0, "visible": True, "blendMode": 0, "type": 0,
             "transitionSpeed": 1.0, "layerEffects": [], "clips": clips}]}]}
path = os.path.join(OUT, "t2comp_%d.json" % LID); json.dump(comp, open(path, "w"))
print("load", S.post(A + "/api/load_composition", json={"path": path}, timeout=10).json())
time.sleep(1.0)
def st():
    d = S.get(A + "/api/state", timeout=6).json()
    return (time.time(), d.get("peak_frame_time_ms"), d.get("frame_rings"), d.get("temporal_buffers"), d.get("frame_time_ms"), d.get("gpu_time_ms"))
def poll(sec, tag, t0):
    rows = []; end = time.time() + sec
    while time.time() < end:
        r = st(); rows.append((tag, round(r[0] - t0, 3)) + r[1:]); time.sleep(0.015)
    return rows
log = []
st()
t = time.time(); log += poll(0.6, "pre-first-use", t)
S.post(A + "/api/trigger_clip", json={"layer": 0, "column": 0}, timeout=6); tU = time.time()
log += poll(1.5, "first-use", tU)
log += poll(1.0, "steady", tU)
S.post(A + "/api/trigger_clip", json={"layer": 0, "column": 1}, timeout=6); tF = time.time()
log += poll(1.6, "fade1", tF)
S.post(A + "/api/trigger_clip", json={"layer": 0, "column": 2}, timeout=6); tF2 = time.time()
log += poll(1.6, "fade2", tF2)
json.dump(log, open(os.path.join(OUT, "t2_%d_%dx%d.json" % (LID, W, H)), "w"))
def summ(tag):
    rows = [r for r in log if r[0] == tag]
    pk = max(rows, key=lambda r: r[2] or 0)
    return "%s: n=%d max peak %.2f ms at +%.3f s, frame_rings %s->%s, temporal %s->%s, median peak %.2f" % (
        tag, len(rows), pk[2], pk[1], rows[0][3], rows[-1][3], rows[0][4], rows[-1][4], sorted(r[2] for r in rows)[len(rows)//2])
for tg in ("pre-first-use", "first-use", "steady", "fade1", "fade2"):
    print(summ(tg))
print("fade1 timeline (t, peak):", [(r[1], round(r[2], 1)) for r in log if r[0] == "fade1" and r[1] < 0.3])
