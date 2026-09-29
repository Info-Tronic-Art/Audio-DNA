#!/usr/bin/env python3
"""q2table.py TAGDIR... -- per arm (w2: a 4 x 4K STILLS window + video windows per launch): stills fps, video fps, gap,
GPU ms (GL_TIME_ELAPSED of the frame, p50/p90) stills vs video, video-sync ms/frame (lookup + advance + pick + upload),
per-upload CPU ms, render (renderOpenGL) wall p50/p90, flushBuffer p50, GL thread / decode threads / FFmpeg frame threads
CPU ms/s, sws ms per frame. Writes TAGDIR/q2.json."""
import glob, json, os, statistics, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import analyze

med = lambda v: round(statistics.median(v), 2) if v else None


def thr(w, prefix):
    return sum(r[0] for r in w.get("threads_top", []) if r[1].startswith(prefix))


for tagdir in sys.argv[1:]:
    rows = []
    for run in sorted(glob.glob(os.path.join(tagdir, "r*")), key=lambda p: int(os.path.basename(p)[1:])):
        if not os.path.exists(os.path.join(run, "windows.json")):
            continue
        s = analyze.summarize(run)
        st = [w for w in s["windows"] if w["tag"] == "w2_stills"]
        for w in s["windows"]:
            if not w["tag"].startswith("w2_video"):
                continue
            rows.append({"run": os.path.basename(run), "tag": w["tag"], "fps": w["fps_frames"], "poll": w["fps_poll_median"],
                         "stills_fps": st[0]["fps_frames"] if st else None,
                         "gpu": w.get("gpu_ms"), "stills_gpu": st[0].get("gpu_ms") if st else None,
                         "vsync": w.get("videosync_ms_per_frame"), "per_upload": w.get("per_upload_ms"),
                         "upload_ms_frame_mean": w.get("upload_ms_per_frame_mean"), "uploads_per_frame": w.get("uploads_per_frame_mean"),
                         "render": w.get("render_ms"), "render_cpu": w.get("render_cpu_ms_p50_p90"), "flush": w.get("flush_ms"),
                         "stills_render": st[0].get("render_ms") if st else None, "stills_flush": st[0].get("flush_ms") if st else None,
                         "gl_ms_s": thr(w, "OpenGL"), "dec_ms_s": thr(w, "VideoDecode"), "ff_ms_s": thr(w, "av:"),
                         "sws": w.get("sws_ms"), "memcpy": w.get("memcpy_ms_per_frame_mean"), "load": w["load"],
                         "ecore": w.get("gl_on_ecore_frac"), "lost_frac": w.get("miss_frac"),
                         "wake_p99": (w.get("wake_latency_ms") or [None] * 3)[2]})
    json.dump(rows, open(os.path.join(tagdir, "q2.json"), "w"), indent=1)
    if not rows:
        print(os.path.basename(tagdir), "no rows"); continue
    g = lambda k, i=None: [r[k][i] if i is not None else r[k] for r in rows if r[k] is not None and (i is None or r[k][i] is not None)]
    stills = sorted({(r["run"], r["stills_fps"]) for r in rows if r["stills_fps"]})
    print(f"{os.path.basename(tagdir):<11} launches {len(set(r['run'] for r in rows))} vwin {len(rows)} video fps med {med(g('fps'))} "
          f"[{min(g('fps'))}, {max(g('fps'))}] stills fps med {med([x for _, x in stills])} | GPU p50 video {med(g('gpu', 0))} "
          f"stills {med(g('stills_gpu', 0))} p90 video {med(g('gpu', 1))} | vsync ms/f p50 {med(g('vsync', 0))} p90 {med(g('vsync', 1))} "
          f"per-upload p50 {med(g('per_upload', 0))} upload ms/frame mean {med(g('upload_ms_frame_mean'))} memcpy {med(g('memcpy'))} | "
          f"render p50/p90 {med(g('render', 0))}/{med(g('render', 1))} (stills {med(g('stills_render', 0))}/{med(g('stills_render', 1))}) "
          f"render cpu p90 {med(g('render_cpu', 1)) if g('render_cpu', 1) else None} flush p50 {med(g('flush', 0))} (stills {med(g('stills_flush', 0))}) | "
          f"GL {med(g('gl_ms_s'))} dec {med(g('dec_ms_s'))} ffmpeg {med(g('ff_ms_s'))} ms/s sws {med(g('sws', 0))} | "
          f"E {med(g('ecore'))} wake p99 {med(g('wake_p99'))} load {min(g('load'))}-{max(g('load'))}")
