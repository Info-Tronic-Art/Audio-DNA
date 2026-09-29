#!/usr/bin/env python3
"""drive.py OUTDIR SCENARIO [loads] -- diag-vfps REST driver (7070, production mode, one fresh connection per request).

Scene builders are copies of .harmony/probe-video.py (w1 = 4 x 1080p a1080 on a 1080p canvas, 4 layers; w2 = 4 x 4K).
Every measurement window is stamped in CLOCK_UPTIME_RAW ns (= mach_absolute_time x timebase, the diag file's clock),
so the analyzer cuts the app's own per-frame records by window.

SCENARIO
  w1       : `loads` fresh loads (default 3) of the w1 scene: trigger x4, 2 s, a 5 s window (fps polled every 50 ms)
  w2       : one 4 x 4K STILLS window, then `loads` (default 2) fresh loads of 4 x 4K video, 5 s windows each
  w2v      : `loads` (default 2) 4 x 4K video windows only
  w1col    : as w1, but the 4 layers are triggered by ONE /api/trigger_column (all clocks start on one render frame)
  still1080: 4 x 1080p stills on the 1080p canvas, 5 s (w1's upload-free control)
"""
import json, os, statistics, sys, threading, time, urllib.request

A = "http://127.0.0.1:7070"
OUT, SCEN = sys.argv[1], sys.argv[2]
LOADS = int(sys.argv[3]) if len(sys.argv) > 3 else None
M = "/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/f33bd4ec-5bde-4244-b1b7-b2ce40654c9a/scratchpad/diag-vfps/media"
SIZES = {"1080": (1920, 1080), "4k": (3840, 2160)}
windows = []


def up_ns():
    return time.clock_gettime_ns(time.CLOCK_UPTIME_RAW)


def req(path, body=None, timeout=10):
    data = None if body is None else json.dumps(body).encode()
    r = urllib.request.Request(A + path, data=data, method="GET" if body is None else "POST",
                               headers={"Content-Type": "application/json", "Connection": "close"})
    try:
        with urllib.request.urlopen(r, timeout=timeout) as f:
            return json.loads(f.read().decode())
    except Exception as e:  # noqa: BLE001
        return {"ok": False, "error": str(e)}


def vclip(cid, name, ip=0.0):
    return {"name": f"v{cid}", "id": cid, "mediaType": 2, "mediaFile": os.path.join(M, name), "speed": 1.0, "transportMode": 0,
            "loopMode": 0, "reverse": False, "inPoint": ip, "outPoint": 1.0, "effects": []}


def iclip(cid, path):
    return {"name": f"i{cid}", "id": cid, "mediaType": 1, "mediaFile": path, "effects": []}


def layer(lid, clips):
    return {"name": f"L{lid}", "id": lid, "opacity": 1.0, "visible": True, "blendMode": 0, "type": 0,
            "transitionSpeed": 0.0, "layerEffects": [], "clips": clips}


def deck(did, layers):
    return {"name": f"D{did}", "id": did, "numColumns": max(len(l["clips"]) for l in layers), "layers": layers}


def load(tag, clips, size, players):
    W, H = SIZES[size]
    comp = {"name": "vfps-" + tag, "activeDeckIndex": 0, "masterOpacity": 1.0, "globalTransitionSpeed": 0.0,
            "outputWidth": W, "outputHeight": H,
            "decks": [deck(0, [layer(91 + i, [c]) for i, c in enumerate(clips)])]}
    path = os.path.join(OUT, f"comp_{tag}.json")
    json.dump(comp, open(path, "w"), indent=1)
    r = req("/api/load_composition", {"path": path}, timeout=30)
    if r.get("ok") is not True:
        print(f"LOAD FAIL {tag}: {r}", flush=True); return False
    t0 = time.time()
    while time.time() - t0 < 15.0:
        s = req("/api/state")
        if s.get("video_players") == players:
            break
        time.sleep(0.1)
    time.sleep(0.5)
    return True


def wait_active(li, col, limit=10.0):
    t0 = time.time()
    while time.time() - t0 < limit:
        c = req("/api/composition")
        try:
            if c["decks"][c["activeDeck"]]["layers"][li]["activeClipColumn"] == col:
                return time.time() - t0
        except Exception:  # noqa: BLE001
            pass
        time.sleep(0.005)
    return None


def poll_window(tag, sec=5.0, interval=0.05):
    rows = []
    run = [True]

    def loop():
        while run[0]:
            d = req("/api/state", timeout=6)
            if "fps" in d:
                rows.append((up_ns(), d.get("fps"), d.get("gpu_time_ms"), d.get("peak_callback_ms"), d.get("video_uploads"),
                             d.get("video_late_frames")))
            time.sleep(interval)
    s0 = req("/api/state"); t0 = up_ns()
    th = threading.Thread(target=loop, daemon=True); th.start()
    time.sleep(sec)
    run[0] = False; th.join(); t1 = up_ns(); s1 = req("/api/state")
    fps = [r[1] for r in rows if r[1] is not None]
    med = statistics.median(fps) if fps else None
    la = os.getloadavg()
    w = {"tag": tag, "t0": t0, "t1": t1, "fps_median": med, "polls": len(rows), "load": la,
         "uploads": (s1.get("video_uploads", 0) - s0.get("video_uploads", 0)) if "video_uploads" in s0 else None,
         "late": (s1.get("video_late_frames", 0) - s0.get("video_late_frames", 0)) if "video_late_frames" in s0 else None,
         "gl_decodes": (s1.get("gl_video_decode_calls", 0) - s0.get("gl_video_decode_calls", 0)) if "gl_video_decode_calls" in s0 else None,
         "fps_polls": fps,
         "peak_callback_polls": [r[3] for r in rows if r[3] is not None]}   # reset-on-read max over each 50 ms poll
    windows.append(w)
    print(f"WINDOW {tag}: median fps {med}, polls {len(rows)}, uploads {w['uploads']}, late {w['late']}, "
          f"gl decodes {w['gl_decodes']}, load {la[0]:.2f}", flush=True)
    return w


def steady(tag, clips, size, players):
    if not load(tag, clips, size, players):
        return None
    for i in range(len(clips)):
        req("/api/trigger_clip", {"layer": i, "column": 0}); wait_active(i, 0)
    time.sleep(2.0)
    return poll_window(tag)


def main():
    if SCEN == "w1":
        for k in range(LOADS or 3):
            steady(f"w1_load{k + 1}", [vclip(10 + 10 * k + i, "a1080_g250.mp4") for i in range(4)], "1080", 4)
    elif SCEN in ("w2", "w2v"):
        if SCEN == "w2":
            steady("w2_stills", [iclip(200 + i, os.path.join(M, "still4k_f100.png")) for i in range(4)], "4k", 0)
        for k in range(LOADS or 2):
            steady(f"w2_video{k + 1}", [vclip(300 + 10 * k + i, "a4k_g250.mp4") for i in range(4)], "4k", 4)
    elif SCEN == "w1col":   # the 4 layers triggered by ONE trigger_column (a scene launch: same render frame)
        for k in range(LOADS or 3):
            tag = f"w1col_load{k + 1}"
            if load(tag, [vclip(500 + 10 * k + i, "a1080_g250.mp4") for i in range(4)], "1080", 4):
                req("/api/trigger_column", {"column": 0})
                for i in range(4):
                    wait_active(i, 0)
                time.sleep(2.0)
                poll_window(tag)
    elif SCEN == "still1080":
        steady("still1080", [iclip(400 + i, os.path.join(M, "still1080_f100.png")) for i in range(4)], "1080", 0)
    else:
        print("unknown scenario", SCEN); sys.exit(2)
    if os.environ.get("DRIVE_CAPTURE") == "1":   # a correctness look for an upload arm: the canvas as a PNG + a luma / code check
        cp = os.path.join(OUT, "cap.png")
        r = req("/api/render_frame", {"output_path": cp, "time": 0.0}, timeout=30)
        try:
            import numpy as np
            from PIL import Image
            a = np.asarray(Image.open(cp).convert("RGB")).astype(float)
            h, w = a.shape[:2]; rows = 128 if w > 2000 else 64; v = 0
            y0, y1 = h - rows + rows // 4, h - rows // 4
            for c in range(10):
                x0, x1 = int(c * w / 10 + w / 40), int(c * w / 10 + 3 * w / 40)
                if a[y0:y1, x0:x1].mean() >= 128:
                    v |= 1 << c
            print(f"CAPTURE {r} size {w}x{h} mean rgb {a.reshape(-1, 3).mean(0).round(1).tolist()} code {v}", flush=True)
        except Exception as e:  # noqa: BLE001
            print(f"CAPTURE {r} decode failed: {e}", flush=True)
    json.dump(windows, open(os.path.join(OUT, "windows.json"), "w"), indent=1)


if __name__ == "__main__":
    main()
