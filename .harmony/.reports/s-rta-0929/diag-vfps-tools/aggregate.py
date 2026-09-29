#!/usr/bin/env python3
"""aggregate.py RUNS_DIR/TAG [TAG2 ...] -- one line per window per launch (+ per-arm medians), from analyze.summarize.
Columns: launch, window, polled fps median, frame fps, swap-interval histogram (vblanks: count), miss fraction,
render ms p50/p90, flush ms p50, DL->frame ms p50/p90, GL on E-core fraction, GL QoS, uploads/frame, upload ms/frame
(p50 of frames with an upload), GPU ms p50, decode wait p50, sws p50, load."""
import glob, os, statistics, sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import analyze  # noqa: E402


def gl_qos(w):
    for r in w.get("threads_top", []):
        if r[1].startswith("OpenGL"):
            return f"{r[2]}/{r[0]}ms/s"
    return "?"


def dec_threads(w):
    v = [r for r in w.get("threads_top", []) if r[1].startswith("VideoDecode")]
    return f"{len(v)}x {v[0][2]} {round(sum(r[0] for r in v))}ms/s" if v else "-"


def main():
    for tagdir in sys.argv[1:]:
        runs = sorted(glob.glob(os.path.join(tagdir, "r*")), key=lambda p: int(os.path.basename(p)[1:]))
        print(f"=== {tagdir}")
        per = {}
        for run in runs:
            if not os.path.exists(os.path.join(run, "windows.json")):
                print(run, "no windows.json"); continue
            s = analyze.summarize(run)
            for w in s["windows"]:
                h = w.get("swap_iv_hist_vblanks")
                print(f"{os.path.basename(run):>4} {w['tag']:<11} poll {w['fps_poll_median']!s:>6} frames/s {w['fps_frames']:>6} "
                      f"hist {h} miss {w.get('miss_frac')} render {w.get('render_ms')} flush {w.get('flush_ms')} "
                      f"dl->fr {w.get('dl_to_frame_ms')} flushEndPh {w.get('flush_end_phase_ms')} E {w.get('gl_on_ecore_frac')} "
                      f"GL {gl_qos(w)} dec {dec_threads(w)} up/f {w.get('uploads_per_frame_mean')} "
                      f"upms {w.get('upload_ms_per_frame_with_upload')} gpu {w.get('gpu_ms')} sleeps {w.get('juce_sleeps')} "
                      f"load {w['load']}", flush=True)
                per.setdefault(w["tag"], []).append(w["fps_frames"])
        for t, v in per.items():
            print(f"  {t}: frames/s {sorted(v)} median {statistics.median(v):.1f} n={len(v)}")


if __name__ == "__main__":
    main()
