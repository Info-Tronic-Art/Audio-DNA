#!/usr/bin/env python3
"""probe-boxes-perf.py -- gate B6 of lane bf9b (ruling-bf9b B6; definitions: ruling-bf9b-merge AM-15). Driven by
.harmony/probe-boxes-perf.sh (which launches and quits the apps); this file loads the fixtures, samples and judges.

usage: probe-boxes-perf.py selftest
       probe-boxes-perf.py session <root> <out> <media> <results.json> <label> <jobs>   (an app is up on 7070)
       probe-boxes-perf.py verdict <results.json> [dry]

DEFINITIONS (AM-15): a run's value = the mean of frame_time_ms sampled every 0.5 s after a 5 s warm-up; SD = the
standard deviation of one arm's RUN MEANS; pooled SD = sqrt((SD_a^2 + SD_b^2) / 2); THR = max(1.0 ms, 3 x pooled SD).
Read on the same runs, same THR rule: gpu_time_ms, and callback cost = the mean of peak_callback_ms read every 0.5 s
(reading resets it, so each sample is that window's peak).
  B6(i)    INFO  MAIN0 vs FH, 1 deck 3 layers (3 videos): FH - MAIN0 > THR -> "B6(i) STOP-FOR-A-LOOK", else "B6(i) INFO".
  B6(ii)   BAR   FH 20 decks (K2v's fixture: 60 copies of ramp12.mp4, 3 playing layers) vs FH 1 deck with the same 3
                 clips. mean(20) - mean(1) > THR -> "B6(ii) FAIL". Else, when either arm's (max - min) of its run
                 means >= THR -> "B6(ii) INFO-NOISY" (re-run quiet; never a PASS). Else "B6(ii) PASS".
                 A PASS carries "(metric sensitivity not proven)" unless FM6_PROVEN below is set.
  B6(ii-b) INFO  the same comparison on gpu_time_ms and on callback cost: a difference > that metric's THR ->
                 "B6(ii-b) STOP-FOR-A-LOOK (<metric>)".
  B6(iii)  INFO  K8's walk (20 decks of pictures, switch_deck 1..19, 0) sampled every 100 ms: mean frame_time_ms and
                 the number of samples with peak_frame_time_ms > 25 ms, per arm.
QUIET: before every run `ps -Ao pcpu=,etime=,comm=` is printed (top lines, with every number). A compiler / linker /
build tool running, or any process at >= 50 % CPU other than Audio-DNA and WindowServer (which composites the app's
own window: 48-51 % while the app renders, measured 2026-10-03), prints "NOTQUIET <ps line>" and exits 4: the .sh
then prints "PROBE-BOXES-PERF BLOCKED (machine not quiet: <ps line>)". BLOCKED is never a pass.
"""
import importlib.util, json, math, os, statistics, subprocess, sys, time

# Set by the lane report's FIX-4 section when fact FM-6 was measured (a 2 ms busy loop per deck in the composite pass
# made this driver print "B6(ii) FAIL"); empty = a PASS carries "(metric sensitivity not proven)".
FM6_PROVEN = "2026-10-03 16:27-16:32, lane report bf9b-fix.md stage FIX-4 fact FM-6: +37.040 ms, B6(ii) FAIL"

WARM, STEP = 5.0, 0.5
METRICS = ("frame_time_ms", "gpu_time_ms", "peak_callback_ms")
BUILD_TOOLS = ("clang", "clang++", "cc1plus", "ld", "cmake", "make", "ninja", "swift-frontend")

# --selftest: RUN MEANS of frame_time_ms recorded by this driver on the lane's Release app, 2026-10-03 (lane report
# bf9b-fix.md, stage FIX-4 "dry pass": 5 runs per arm of 12 s; a dry pass, never a verdict).
RECORDED = {"one": [2.541, 2.376, 2.269, 2.368, 2.382], "twenty": [2.432, 2.367, 2.421, 2.46, 2.333]}


def sd(xs):
    return statistics.stdev(xs) if len(xs) >= 2 else 0.0


def thr_of(a, b):
    pooled = math.sqrt((sd(a) ** 2 + sd(b) ** 2) / 2.0)
    return max(1.0, 3.0 * pooled), pooled


def judge_ii(one, twenty):
    """-> (token, text). token in PASS / INFO-NOISY / FAIL."""
    thr, pooled = thr_of(one, twenty)
    diff = statistics.mean(twenty) - statistics.mean(one)
    spread = max(max(one) - min(one), max(twenty) - min(twenty))
    txt = (f"mean(20 decks) {statistics.mean(twenty):.3f} ms - mean(1 deck) {statistics.mean(one):.3f} ms = "
           f"{diff:+.3f} ms; SD {sd(twenty):.3f} / {sd(one):.3f}, pooled {pooled:.3f}, THR {thr:.3f} ms; widest arm "
           f"spread (max - min) {spread:.3f} ms; run means 20: {[round(x, 3) for x in twenty]} 1: "
           f"{[round(x, 3) for x in one]}")
    if diff > thr:
        return "FAIL", txt
    if spread >= thr:
        return "INFO-NOISY", txt + " -- re-run on a quiet machine; not a pass"
    return "PASS", txt


def selftest():
    one, twenty = RECORDED["one"], RECORDED["twenty"]
    if len(one) < 5 or len(twenty) < 5:
        print("SELFTEST FAIL (no recorded numbers in this script)"); return 1
    t0, x0 = judge_ii(one, twenty)
    t1, x1 = judge_ii(one, [x + 2.0 for x in twenty])
    print(f"selftest recorded:          B6(ii) {t0} -- {x0}")
    print(f"selftest 20-deck arm +2 ms: B6(ii) {t1} -- {x1}")
    good = t0 == "PASS" and t1 == "FAIL"
    print(f"SELFTEST {'PASS' if good else 'FAIL'} (recorded numbers: B6(ii) {t0}; shifted by 2 ms: B6(ii) {t1}; "
          f"expected PASS and FAIL)")
    return 0 if good else 1


def ps_top(n=4):
    out = subprocess.run(["ps", "-Ao", "pcpu=,etime=,comm="], capture_output=True, text=True).stdout.splitlines()
    rows = []
    for ln in out:
        p = ln.split(None, 2)
        if len(p) == 3:
            try:
                rows.append((float(p[0]), p[1], p[2]))
            except ValueError:
                pass
    rows.sort(reverse=True)
    return rows[:n], rows


def quiet_or_exit(tag):
    top, rows = ps_top()
    print(f"      ps before {tag}: " + " | ".join(f"{c:.1f} {e} {os.path.basename(n)}" for c, e, n in top), flush=True)
    for c, e, n in rows:
        base = os.path.basename(n)
        if base in BUILD_TOOLS or (c >= 50.0 and base not in ("Audio-DNA", "WindowServer")):
            print(f"NOTQUIET {c:.1f} {e} {n}", flush=True)
            sys.exit(4)


def session(root, out, media, results, label, jobs):
    sys.argv = [os.path.join(root, ".harmony", "probe-boxes.py"), root, out, media, ""]
    spec = importlib.util.spec_from_file_location("probe_boxes", sys.argv[0])
    pb = importlib.util.module_from_spec(spec); spec.loader.exec_module(pb)
    secs = float(os.environ.get("PERF_SECS", "60"))
    res = json.load(open(results)) if os.path.exists(results) else []

    def clips3():
        return {li: pb.ramp(f"k2v_d{dk}r{li}") for li, dk in ((0, 0), (1, 7), (2, 13))}

    def wait_players(n):
        t0 = time.time(); st = {}
        while time.time() - t0 < 20.0:
            st = pb.state() or {}
            if st.get("video_players") == n:
                return True
            time.sleep(0.5)
        print(f"FAIL  {label}: VALID -- video_players {st.get('video_players')} != {n}", flush=True)
        pb.FAIL += 1
        return False

    def load_one():
        v = clips3()
        if None in v.values():
            return False
        cells = {(0, li, 0): pb.vid(v[li]) for li in range(3)}
        if not pb.show("b6_one", 1, 3, 1, cells, {1: {"type": 1}, 2: {"type": 1}}):
            return False
        if not wait_players(3):
            return False
        for li in range(3):
            pb.trig(li, 0); time.sleep(0.3)
        return True

    def load_twenty():
        cells = {}
        for dk in range(20):
            for li in range(3):
                v = pb.ramp(f"k2v_d{dk}r{li}")
                if v is None:
                    return False
                cells[(dk, li, 0)] = pb.vid(v)
        if not pb.show("b6_twenty", 20, 3, 1, cells, {1: {"type": 1}, 2: {"type": 1}}):
            return False
        if not wait_players(60):
            return False
        pb.switch(0); pb.trig(0, 0); time.sleep(0.3)
        pb.switch(7); pb.trig(1, 0); time.sleep(0.3)
        pb.switch(13); pb.trig(2, 0); time.sleep(0.3)
        pb.switch(19)
        return True

    def sample(arm, part, run):
        quiet_or_exit(f"{part} {label} {arm} run {run}")
        time.sleep(WARM)
        pb.state()                                   # resets the peaks
        t0 = time.time(); k = 0; rows = {m: [] for m in METRICS}
        while time.time() - t0 < secs:
            k += 1
            pb.sleep_until(t0 + STEP * k)
            s = pb.state() or {}
            for m in METRICS:
                if isinstance(s.get(m), (int, float)):
                    rows[m].append(float(s[m]))
        if not rows["frame_time_ms"]:
            print(f"FAIL  {label} {arm}: no frame_time_ms sample", flush=True); pb.FAIL += 1; return
        means = {m: (statistics.mean(v) if v else None) for m, v in rows.items()}
        res.append({"part": part, "app": label, "arm": arm, "run": run, "n": len(rows["frame_time_ms"]), **means})
        json.dump(res, open(results, "w"), indent=1)
        shown = {m: (None if means[m] is None else round(means[m], 3)) for m in METRICS}
        print(f"RUN   {part} {label} {arm} run {run}: n {len(rows['frame_time_ms'])} mean frame_time_ms "
              f"{shown['frame_time_ms']} gpu_time_ms {shown['gpu_time_ms']} callback {shown['peak_callback_ms']}",
              flush=True)

    def walk():
        cells = {(dk, 0, 0): pb.img(dk, 0) for dk in range(20)}
        if not pb.show("b6_walk", 20, 1, 1, cells):
            return
        pb.trig(0, 0); time.sleep(1.0)
        quiet_or_exit(f"iii {label} walk")
        pb.state(); ft, over = [], 0
        t0 = time.time(); k = 0
        for dk in list(range(1, 20)) + [0]:
            pb.switch(dk, wait=False)
            for _ in range(5):
                k += 1; pb.sleep_until(t0 + 0.1 * k)
                s = pb.state() or {}
                if isinstance(s.get("frame_time_ms"), (int, float)):
                    ft.append(float(s["frame_time_ms"]))
                if float(s.get("peak_frame_time_ms", 0.0)) > 25.0:
                    over += 1
        if ft:
            res.append({"part": "iii", "app": label, "arm": "walk", "run": 1, "n": len(ft),
                        "frame_time_ms": statistics.mean(ft), "over25": over})
            json.dump(res, open(results, "w"), indent=1)
            print(f"RUN   iii {label} walk: n {len(ft)} mean frame_time_ms {statistics.mean(ft):.3f}; samples with "
                  f"peak_frame_time_ms > 25 ms: {over}", flush=True)

    for job in [j for j in jobs.split(",") if j]:
        kind, _, run = job.partition(":")
        if kind in ("one", "i"):
            if load_one():
                sample("one", "i" if kind == "i" else "ii", int(run or 1))
        elif kind == "twenty":
            if load_twenty():
                sample("twenty", "ii", int(run or 1))
        elif kind == "walk":
            walk()
    return 1 if pb.FAIL else 0


def verdict(results, dry):
    res = json.load(open(results)) if os.path.exists(results) else []

    def means(part, app, arm, m="frame_time_ms"):
        return [r[m] for r in res if r["part"] == part and r["app"] == app and r["arm"] == arm and r.get(m) is not None]

    tag = "DRY (no verdict) " if dry else ""
    one, twenty = means("ii", "FH", "one"), means("ii", "FH", "twenty")
    if one and twenty:
        tok, txt = judge_ii(one, twenty)
        if dry:
            print(f"B6(ii) {tag}-- {txt}")
        elif len(one) < 5 or len(twenty) < 5:
            print(f"B6(ii) NOT JUDGED (fewer than 5 runs per arm) -- {txt}")
        else:
            note = "" if (tok != "PASS" or FM6_PROVEN) else " (metric sensitivity not proven)"
            print(f"B6(ii) {tok}{note} -- {txt}")
        for m, name in (("gpu_time_ms", "gpu_time_ms"), ("peak_callback_ms", "callback cost")):
            a, b = means("ii", "FH", "one", m), means("ii", "FH", "twenty", m)
            if a and b:
                thr, pooled = thr_of(a, b); diff = statistics.mean(b) - statistics.mean(a)
                tok2 = "STOP-FOR-A-LOOK" if diff > thr else "INFO"
                print(f"B6(ii-b) {tag}{'' if dry else tok2 + ' '}({name}) -- mean(20) {statistics.mean(b):.3f} - "
                      f"mean(1) {statistics.mean(a):.3f} = {diff:+.3f} ms; THR {thr:.3f} ms")
    else:
        print("B6(ii) NOT RUN")
    a, b = means("i", "MAIN0", "one"), means("i", "FH", "one")
    if a and b:
        thr, pooled = thr_of(a, b); diff = statistics.mean(b) - statistics.mean(a)
        tok = "STOP-FOR-A-LOOK" if diff > thr else "INFO"
        print(f"B6(i) {tag}{'' if dry else tok + ' '}-- FH {statistics.mean(b):.3f} ms - MAIN0 {statistics.mean(a):.3f} "
              f"ms = {diff:+.3f} ms; THR {thr:.3f} ms; run means FH {[round(x, 3) for x in b]} MAIN0 "
              f"{[round(x, 3) for x in a]}")
    else:
        print("B6(i) NOT RUN")
    walks = [r for r in res if r["part"] == "iii"]
    for r in walks:
        print(f"B6(iii) INFO {r['app']}: walk mean frame_time_ms {r['frame_time_ms']:.3f} over {r['n']} samples; "
              f"samples with peak_frame_time_ms > 25 ms: {r['over25']}")
    if not walks:
        print("B6(iii) NOT RUN")
    return 0


if __name__ == "__main__":
    if len(sys.argv) >= 2 and sys.argv[1] == "selftest":
        sys.exit(selftest())
    if len(sys.argv) >= 8 and sys.argv[1] == "session":
        sys.exit(session(*sys.argv[2:8]))
    if len(sys.argv) >= 3 and sys.argv[1] == "verdict":
        sys.exit(verdict(sys.argv[2], len(sys.argv) > 3 and sys.argv[3] == "dry"))
    sys.exit(__doc__)
