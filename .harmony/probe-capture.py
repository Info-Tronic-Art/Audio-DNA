#!/usr/bin/env python3
"""probe-capture.py -- REST/pixel half of .harmony/probe-capture.sh (s-rta-0928 renderleft R2 / R3).

The .sh owns launch (TEST MODE: 7070 + 8080) / refuse / quit; this file talks to the running app. Every PNG is
decoded with PIL+numpy; every render_frame response is checked; the output dir is fresh per run.

usage: probe-capture.py <root> <fresh-outdir> <media-dir> [row,row,...]
rows: cr1_concurrent_7070 cr2_concurrent_sizes_8080 cr3_round_trip
env:  CAPT_CALIBRATE=1 -- cr3 also writes its medians to <out>/cr3_calib.json (the calibration run on main).

Fixture: the renderperf capt.py composition -- one Opaque layer at opacity 0.5 showing media/P16_01_baseline.png,
1920x1080 canvas, triggered, 1.5 s settle. A static image at time 0.0 renders the same frame every time.

cr1_concurrent_7070: ref = one solo 7070 render_frame (time 0.0); then 4 threads behind a barrier POST 7070
  render_frame {fresh path, time 0.0}, client timeout 15 s. PASS: all 4 ok:true, each call <= maxCallS, each PNG
  decodes array_equal to ref. RED on main: 3 of 4 fail after ~5 s (a second caller overwrites capturePromise_; the
  first times out and its timeout path clears whatever capture is pending then -- plan F8).
cr2_concurrent_sizes_8080: solo 8080 refs at 640x360 and 320x180 (render_frame width/height = the TEST-ONLY canvas
  lock); then both sizes concurrently (barrier). PASS: both ok, each decodes at its own size and array_equal to its
  ref, each <= maxCallS. RED on main: the two lock set/clear pairs race on the canvas size (TestServer sets and clears
  the lock OUTSIDE captureFrame) and one call times out or returns the other's size.
cr3_round_trip (perf; waits for no compiler, prints the load average): 5 sequential 7070 captures at 1920x1080, then
  the composition reloaded at 3840x2160 and 2 captures there. The client times each round trip; the app's err.log
  line "[Eyes] Captured frame: <path> (WxH) read= convert= png= ms" gives the split. PASS: median 1080p png <=
  pngRatio x calib.png1080_ms AND median 1080p round trip <= rtRatio x calib.rt1080_ms; every PNG non-blank at the
  canvas size. RED on main by construction (main IS the calibration).
"""
import json, os, re, statistics, subprocess, sys, threading, time

import numpy as np
import requests
from PIL import Image

A = "http://127.0.0.1:7070"
E = "http://localhost:8080"   # the TestServer listens on localhost only (IPv6)
ROOT, OUT, MEDIA = sys.argv[1], sys.argv[2], sys.argv[3]
ONLY = set(sys.argv[4].split(",")) if len(sys.argv) > 4 and sys.argv[4] else None
FIX = json.load(open(os.path.join(ROOT, ".harmony", "probe-capture.json")))
MAXCALL = float(FIX["maxCallS"])
PASS = FAIL = 0


def sess():
    s = requests.Session(); s.headers["Connection"] = "close"   # cpp-httplib 5 s keep-alive drop
    return s


S = sess()


def ok(msg):
    global PASS; PASS += 1; print(f"PASS  {msg}", flush=True)


def no(msg):
    global FAIL; FAIL += 1; print(f"FAIL  {msg}", flush=True)


def load(tag, size):
    comp = {"name": "capt-" + tag, "activeDeckIndex": 0, "masterOpacity": 1.0, "globalTransitionSpeed": 0.0,
            "outputWidth": size[0], "outputHeight": size[1],
            "decks": [{"name": "D0", "id": 0, "numColumns": 1, "layers": [
                {"name": "L0", "id": 0, "opacity": 0.5, "visible": True, "blendMode": 0, "type": 0,
                 "transitionSpeed": 0.0, "layerEffects": [],
                 "clips": [{"name": "c1", "id": 1, "mediaType": 1, "mediaFile": os.path.join(MEDIA, "P16_01_baseline.png"),
                            "effects": []}]}]}]}
    path = os.path.join(OUT, f"comp_{tag}.json")
    json.dump(comp, open(path, "w"), indent=1)
    r = S.post(A + "/api/load_composition", json={"path": path}, timeout=20)
    good = r.ok and r.json().get("ok") is True
    if not good:
        no(f"{tag}: load_composition rejected: {r.text[:160]}"); return False
    time.sleep(1.0)
    S.post(A + "/api/trigger_clip", json={"layer": 0, "column": 0}, timeout=6)
    time.sleep(1.5)
    return True


def render(base, name, size=None, s=None):
    """-> (array or None, seconds, response)"""
    p = os.path.join(OUT, name + ".png")
    if os.path.exists(p):
        os.remove(p)
    req = {"output_path": p, "time": 0.0}
    if size:
        req.update(width=int(size[0]), height=int(size[1]))
    t0 = time.perf_counter()
    try:
        r = (s or sess()).post(base + "/api/render_frame", json=req, timeout=15)
        body = r.json()
    except Exception as e:  # noqa: BLE001
        return None, time.perf_counter() - t0, str(e)
    dt = time.perf_counter() - t0
    if not body.get("ok") or not os.path.isfile(p):
        return None, dt, body
    return np.asarray(Image.open(p).convert("RGBA")), dt, body


def concurrent(calls):
    """calls = [(base, name, size)] -> results in order, all started behind one barrier."""
    bar = threading.Barrier(len(calls)); res = [None] * len(calls)

    def run(i, c):
        s = sess(); bar.wait(); res[i] = render(c[0], c[1], c[2], s)
    th = [threading.Thread(target=run, args=(i, c)) for i, c in enumerate(calls)]
    for t in th: t.start()
    for t in th: t.join()
    return res


def cr1(tag):
    if not load(tag, (1920, 1080)):
        return
    ref, rt, body = render(A, tag + "_ref")
    if ref is None:
        no(f"{tag}: solo capture failed: {body}"); return
    res = concurrent([(A, f"{tag}_c{i}", None) for i in range(4)])
    for i, (a, dt, body) in enumerate(res):
        eq = a is not None and a.shape == ref.shape and np.array_equal(a, ref)
        print(f"      {tag}: call {i}: ok={a is not None} {dt:.2f} s equal_to_solo={eq} {'' if a is not None else body}", flush=True)
        (ok if a is not None and dt <= MAXCALL and eq else no)(
            f"{tag}: concurrent 7070 capture {i} answered with the right picture in {dt:.2f} s (<= {MAXCALL})")


def cr2(tag):
    if not load(tag, (1920, 1080)):
        return
    sizes = [tuple(x) for x in FIX["cr2Sizes"]]
    refs = {}
    for sz in sizes:
        a, dt, body = render(E, f"{tag}_ref_{sz[0]}", sz)
        if a is None:
            no(f"{tag}: solo 8080 capture at {sz} failed: {body}"); return
        refs[sz] = a
    res = concurrent([(E, f"{tag}_c_{sz[0]}", sz) for sz in sizes])
    for sz, (a, dt, body) in zip(sizes, res):
        shape_ok = a is not None and a.shape[1] == sz[0] and a.shape[0] == sz[1]
        eq = shape_ok and np.array_equal(a, refs[sz])
        print(f"      {tag}: {sz}: ok={a is not None} {dt:.2f} s shape={None if a is None else a.shape[:2]} "
              f"equal_to_solo={eq} {'' if a is not None else body}", flush=True)
        (ok if a is not None and dt <= MAXCALL and eq else no)(
            f"{tag}: concurrent 8080 capture at {sz[0]}x{sz[1]} answered at its own size with its own picture in "
            f"{dt:.2f} s (<= {MAXCALL})")


LINE = re.compile(r"Captured frame: (\S+) \((\d+)x(\d+)\) read=([\d.]+) convert=([\d.]+) png=([\d.]+) ms(.*)")


def log_split(path):
    try:
        for ln in open(os.path.join(OUT, "err.log"), errors="replace"):
            m = LINE.search(ln)
            if m and m.group(1) == path:
                return float(m.group(4)), float(m.group(5)), float(m.group(6)), m.group(7).strip()
    except OSError:
        pass
    return None


def wait_no_compiler(tag, limit_s=1800):
    t0 = time.time()
    while True:
        busy = [n for n in ("clang", r"clang\+\+") if subprocess.run(["pgrep", "-x", n], capture_output=True).stdout.strip()]
        if not busy:
            return True
        if time.time() - t0 > limit_s:
            no(f"{tag}: a compiler kept running -- perf not measured"); return False
        print(f"      {tag}: waiting for {busy} (load avg %.2f %.2f %.2f)" % os.getloadavg(), flush=True)
        time.sleep(20)


def cr3(tag):
    if not wait_no_compiler(tag):
        return
    table = {"1080": [], "4k": []}
    for key, size, n in (("1080", (1920, 1080), 5), ("4k", (3840, 2160), 2)):
        if not load(f"{tag}_{key}", size):
            return
        for i in range(n):
            name = f"{tag}_{key}_{i}"
            a, dt, body = render(A, name, s=S)
            sp = log_split(os.path.join(OUT, name + ".png"))
            nb = a is not None and a.shape[1] == size[0] and a.shape[0] == size[1] and float(a[..., :3].std()) >= 3.0
            table[key].append((dt * 1000.0, sp, nb))
            print(f"      {tag}: {key} #{i}: round trip {dt * 1000:.1f} ms; read/convert/png = "
                  f"{sp[:3] if sp else None} {sp[3] if sp else ''}; non-blank at {size} {nb}", flush=True)
    print(f"      {tag}: load avg %.2f %.2f %.2f" % os.getloadavg(), flush=True)
    med = {}
    for key in table:
        rows = table[key]
        med[key] = {"rt_ms": statistics.median(r[0] for r in rows),
                    "png_ms": statistics.median(r[1][2] for r in rows if r[1]) if all(r[1] for r in rows) else None,
                    "convert_ms": statistics.median(r[1][1] for r in rows if r[1]) if all(r[1] for r in rows) else None,
                    "read_ms": statistics.median(r[1][0] for r in rows if r[1]) if all(r[1] for r in rows) else None}
        print(f"      {tag}: median {key}: {med[key]}", flush=True)
    if os.environ.get("CAPT_CALIBRATE") == "1":
        json.dump({"png1080_ms": med["1080"]["png_ms"], "rt1080_ms": med["1080"]["rt_ms"],
                   "png4k_ms": med["4k"]["png_ms"], "rt4k_ms": med["4k"]["rt_ms"],
                   "loadavg": os.getloadavg(), "date": time.strftime("%Y-%m-%d %H:%M:%S")},
                  open(os.path.join(OUT, "cr3_calib.json"), "w"), indent=1)
        print(f"      {tag}: calibration written to {OUT}/cr3_calib.json", flush=True)
    allnb = all(r[2] for k in table for r in table[k])
    (ok if allnb else no)(f"{tag}: every capture is non-blank at its canvas size")
    cal = FIX.get("cr3", {}).get("calib")
    if not cal:
        no(f"{tag}: no calibration in probe-capture.json cr3.calib (run CAPT_CALIBRATE=1 on main first)"); return
    pr, rr = float(FIX["cr3"]["pngRatio"]), float(FIX["cr3"]["rtRatio"])
    png, rt = med["1080"]["png_ms"], med["1080"]["rt_ms"]
    if png is None:
        no(f"{tag}: no split line for the 1080p captures"); return
    (ok if png <= pr * cal["png1080_ms"] else no)(
        f"{tag}: median 1080p png {png:.1f} ms <= {pr} x calibration {cal['png1080_ms']:.1f} ms")
    (ok if rt <= rr * cal["rt1080_ms"] else no)(
        f"{tag}: median 1080p round trip {rt:.1f} ms <= {rr} x calibration {cal['rt1080_ms']:.1f} ms")


def main():
    rows = [("cr1_concurrent_7070", lambda: cr1("cr1_concurrent_7070")),
            ("cr2_concurrent_sizes_8080", lambda: cr2("cr2_concurrent_sizes_8080")),
            ("cr3_round_trip", lambda: cr3("cr3_round_trip"))]
    for name, fn in rows:
        if ONLY is None or name in ONLY:
            print(f"--- {name}", flush=True)
            fn()
    print(f"\nPY {PASS} PASS / {FAIL} FAIL", flush=True)
    sys.exit(0 if FAIL == 0 else 1)


if __name__ == "__main__":
    main()
