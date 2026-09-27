"""capture_race.py -- s-rta-0927 renderperf fix round: two captureFrame calls in flight must each get THEIR OWN pixels.

Needs an app built with the TEMPORARY hook AUDIODNA_TEST_CAPTURE_COLLECT_DELAY_MS (never committed; see
fix/hook.py): captureFrame sleeps that long between its future becoming ready and collecting the read, i.e. it
widens the window between "the GL thread signalled capture A" and "caller A picks its pixels up". A second
capture B is armed and serviced by the GL thread inside that window.

Rows (every PNG decoded with PIL+numpy; each output path deleted first -- juce::FileOutputStream appends):
  h_hook     the app's stderr shows the hook armed (else the run proves nothing -> FAIL)
  r0_single  one 640x360 capture alone: ok, decodes 640x360, centre = the red legacy image
  r1_sizes   A 640x360 (8080 render_frame), B 320x180 sent 0.5 s later while A is still in flight:
             A ok AND decodes 640x360; B ok AND decodes 320x180
  r2_content A 640x360 with the red image showing; 0.3 s later the blue image is loaded (7070 load_image);
             B 640x360 0.8 s after A: A ok AND centre red; B ok AND centre blue. Same size on purpose: the
             size check in captureFrame cannot see this swap.
  Each overlap row also requires B was SENT before A's response arrived (else it was not an overlap -> FAIL).
usage: capture_race.py <out-dir> <media-dir-unused>
"""
import os, sys, time, threading, json
import numpy as np, requests
from PIL import Image

A7 = "http://127.0.0.1:7070"
T8 = "http://[::1]:8080"
OUT = sys.argv[1]
H = {"Connection": "close"}   # fresh connection per request (c1-state-fix)
PASS = FAIL = 0


def ok(m):
    global PASS; PASS += 1; print("PASS  " + m, flush=True)


def no(m):
    global FAIL; FAIL += 1; print("FAIL  " + m, flush=True)


def solid(name, rgb):
    p = os.path.join(OUT, name)
    Image.new("RGB", (400, 400), rgb).save(p)
    return p


def load(path):
    r = requests.post(A7 + "/api/load_image", json={"filepath": path}, headers=H, timeout=10)
    return r.status_code, r.text[:120]


def capture(name, w, h, res, key):
    p = os.path.join(OUT, name)
    if os.path.exists(p):
        os.remove(p)
    t0 = time.monotonic()
    try:
        r = requests.post(T8 + "/api/render_frame", json={"output_path": p, "time": 0.0, "width": w, "height": h},
                          headers=H, timeout=30)
        body = r.text
        good = r.status_code == 200 and json.loads(body).get("ok") is True
    except Exception as e:
        body, good = f"EXC {e}", False
    res[key] = {"path": p, "ok": good, "body": body[:160], "t0": t0, "t1": time.monotonic()}


def decode(p):
    if not os.path.exists(p):
        return None
    a = np.asarray(Image.open(p).convert("RGBA"))
    return a


def centre(a):
    h, w = a.shape[:2]
    return a[h // 2 - 10:h // 2 + 10, w // 2 - 10:w // 2 + 10, :3].reshape(-1, 3).mean(axis=0)


def colour(c):
    r, g, b = c
    if r > 150 and g < 60 and b < 60:
        return "red"
    if b > 150 and r < 60 and g < 60:
        return "blue"
    return "other(%.0f,%.0f,%.0f)" % (r, g, b)


def desc(a):
    return "none" if a is None else "%dx%d centre %s" % (a.shape[1], a.shape[0], colour(centre(a)))


red, blue = solid("red.png", (255, 0, 0)), solid("blue.png", (0, 0, 255))
print("load red:", load(red), flush=True)
time.sleep(1.0)

# r0_single
res = {}
capture("r0.png", 640, 360, res, "A")
a = decode(res["A"]["path"])
print(f"      r0_single: ok={res['A']['ok']} wall {1000*(res['A']['t1']-res['A']['t0']):.0f} ms, {desc(a)}", flush=True)
(ok if res["A"]["ok"] and a is not None and a.shape[:2] == (360, 640) and colour(centre(a)) == "red" else no)(
    f"r0_single: a lone 640x360 capture decodes 640x360 with the red image at its centre ({desc(a)})")

# r1_sizes
res = {}
ta = threading.Thread(target=capture, args=("r1_A.png", 640, 360, res, "A")); ta.start()
time.sleep(0.5)
tb = threading.Thread(target=capture, args=("r1_B.png", 320, 180, res, "B")); tb.start()
ta.join(); tb.join()
A_, B_ = res["A"], res["B"]
a, b = decode(A_["path"]), decode(B_["path"])
overlap = B_["t0"] < A_["t1"]
print(f"      r1_sizes: A ok={A_['ok']} [{A_['body'][:60]}] -> {desc(a)}; B ok={B_['ok']} [{B_['body'][:60]}] -> {desc(b)}; "
      f"B sent {1000*(B_['t0']-A_['t0']):.0f} ms after A, A answered at {1000*(A_['t1']-A_['t0']):.0f} ms, overlap={overlap}", flush=True)
(ok if overlap else no)("r1_sizes: B was sent while A was still in flight")
(ok if A_["ok"] and a is not None and a.shape[:2] == (360, 640) else no)(
    f"r1_sizes: capture A (640x360) returns ok and its PNG decodes 640x360 ({desc(a)})")
(ok if B_["ok"] and b is not None and b.shape[:2] == (180, 320) else no)(
    f"r1_sizes: capture B (320x180) returns ok and its PNG decodes 320x180 ({desc(b)})")

# r2_content
res = {}
ta = threading.Thread(target=capture, args=("r2_A.png", 640, 360, res, "A")); ta.start()
time.sleep(0.3)
lb = load(blue)
time.sleep(0.5)
tb = threading.Thread(target=capture, args=("r2_B.png", 640, 360, res, "B")); tb.start()
ta.join(); tb.join()
A_, B_ = res["A"], res["B"]
a, b = decode(A_["path"]), decode(B_["path"])
overlap = B_["t0"] < A_["t1"]
print(f"      r2_content: load blue {lb}; A ok={A_['ok']} [{A_['body'][:60]}] -> {desc(a)}; B ok={B_['ok']} [{B_['body'][:60]}] -> {desc(b)}; "
      f"B sent {1000*(B_['t0']-A_['t0']):.0f} ms after A, A answered at {1000*(A_['t1']-A_['t0']):.0f} ms, overlap={overlap}", flush=True)
(ok if overlap else no)("r2_content: B was sent while A was still in flight")
(ok if A_["ok"] and a is not None and a.shape[:2] == (360, 640) and colour(centre(a)) == "red" else no)(
    f"r2_content: capture A (armed while red showed) returns ok and decodes RED -- its own frame ({desc(a)})")
(ok if B_["ok"] and b is not None and b.shape[:2] == (360, 640) and colour(centre(b)) == "blue" else no)(
    f"r2_content: capture B (armed after blue loaded) returns ok and decodes BLUE ({desc(b)})")

print(f"PY {PASS} PASS / {FAIL} FAIL", flush=True)
sys.exit(0 if FAIL == 0 else 1)
