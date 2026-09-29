#!/usr/bin/env python3
"""analyze.py RUNDIR [--json] -- per-window summary of a diag-vfps launch (diag.tsv + windows.json).

Per window: fps (frames / window), swap-to-swap histogram in vblank units, renderOpenGL cost split, flushBuffer wait,
render-thread wait, JUCE swap-hack sleeps, display-link cadence, GL thread CPU placement, uploads, GPU time, decode
threads, per-thread CPU ms/s + QoS. QoS numbers: 0x21 USER_INTERACTIVE, 0x19 USER_INITIATED, 0x15 DEFAULT, 0x11 UTILITY,
0x09 BACKGROUND, 0 UNSPECIFIED."""
import json, os, statistics, sys
from collections import Counter, defaultdict

QOS = {0x21: "UI", 0x19: "UIN", 0x15: "DEF", 0x11: "UTIL", 0x09: "BG", 0: "UNSP"}


def pct(v, p):
    if not v:
        return None
    v = sorted(v)
    return v[min(len(v) - 1, int(p * (len(v) - 1) + 0.5))]


def load(run):
    f = 1.0
    recs = defaultdict(list); th = []; disp = []; cal = []; hdr = None
    for line in open(os.path.join(run, "diag.tsv"), errors="replace"):
        p = line.rstrip("\n").split("\t")
        if p[0] == "H":
            hdr = p; f = float(p[1]) / float(p[2])
        elif p[0] == "R":
            k = int(p[1]); recs[k].append((int(p[2]), int(p[3]), float(p[4]), float(p[5]), float(p[6]), float(p[7]), float(p[8])))
        elif p[0] == "C":
            th.append((float(p[1]), int(p[2]), int(p[4]), int(p[5]), int(p[6]), int(p[7]), int(p[8]), int(p[10]), p[11]))
        elif p[0] == "D":
            disp.append(p[1:])
        elif p[0] == "K":
            cal.append(p[1:])
    return f, recs, th, disp, cal, hdr


def summarize(run):
    f, recs, th, disp, cal, hdr = load(run)
    ns = lambda ticks: ticks * f
    ms = lambda ticks: ticks * f / 1e6
    wins = json.load(open(os.path.join(run, "windows.json")))
    ecpus = set()
    for c in cal:
        if c[0] == "background":
            ecpus = {int(x) for x in c[2].split("=")[1].split(",") if x}
    out = {"run": run, "hdr": hdr[4:] if hdr else None, "ecpus_bg_calibration": sorted(ecpus), "cal": cal,
           "display": disp[:1] + disp[-1:], "windows": []}
    for w in wins:
        a, b = w["t0"], w["t1"]
        inw = lambda t: a <= ns(t) < b
        fr = [r for r in recs[10] if inw(r[2])]
        sw = [r for r in recs[2] if inw(r[3])]
        dl = [r for r in recs[1] if inw(r[3])]
        dlm = [r for r in recs[4] if inw(r[3])]
        wt = [r for r in recs[3] if inw(r[3])]
        up = [r for r in recs[11] if inw(r[2])]
        ph = [r for r in recs[12] if inw(r[2])]
        gp = [r for r in recs[14] if inw(r[2])]
        de = [r for r in recs[21] if inw(r[2])]
        sws = [r for r in recs[22] if inw(r[2])]
        span = (b - a) / 1e9
        d = {"tag": w["tag"], "fps_poll_median": w["fps_median"], "load": round(w["load"][0], 2), "uploads": w.get("uploads"),
             "late": w.get("late"), "frames": len(fr), "fps_frames": round(len(fr) / span, 1)}
        # vblank period from the display link's inNow host times
        vs = [r[4] for r in dl]   # outputTime host time: an exact vsync grid (inNow is ~ the callback time on this Mac)
        per = [ms(y - x) for x, y in zip(vs, vs[1:])]
        P = statistics.median(per) if per else 8.3333
        d["dl_ticks"] = len(dl); d["dl_period_ms_median"] = round(P, 4)
        d["dl_period_hist"] = dict(Counter(round(x / P) for x in per))
        d["dl_matched"] = sum(1 for r in dlm if r[3] == 1.0); d["dl_unmatched"] = sum(1 for r in dlm if r[3] != 1.0)
        # swap-to-swap (flush end) intervals in vblank units
        se = [r[3] for r in sw]   # flushBuffer return
        iv = [ms(y - x) for x, y in zip(se, se[1:])]
        d["swap_iv_ms_median"] = round(statistics.median(iv), 3) if iv else None
        h = Counter(max(0, round(x / P)) for x in iv)
        d["swap_iv_hist_vblanks"] = {str(k): h[k] for k in sorted(h)}
        d["swap_iv_ms_quantiles"] = [round(pct(iv, q), 2) for q in (0.05, 0.25, 0.5, 0.75, 0.95)] if iv else None
        # frames missing a vblank: intervals > 1.5 P
        d["miss_frac"] = round(sum(1 for x in iv if x > 1.5 * P) / len(iv), 3) if iv else None
        # frame (renderOpenGL) cost and flush
        cost = [ms(r[3] - r[2]) for r in fr]
        fl = [ms(r[3] - r[2]) for r in sw]
        d["render_ms"] = [round(pct(cost, q), 2) for q in (0.5, 0.9, 0.99)] if cost else None
        d["flush_ms"] = [round(pct(fl, q), 2) for q in (0.5, 0.9, 0.99)] if fl else None
        d["juce_sleeps"] = sum(1 for r in sw if (r[5] % 1e6) >= 1e3)
        d["juce_minSwap"] = sorted({int(r[5] % 1e3) for r in sw})
        d["juce_underrun_max"] = max((int(r[5] // 1e6) for r in sw), default=None)
        d["juce_frameTime_hist"] = dict(sorted(Counter(int(r[4]) for r in sw).items()))
        wts = [ms(r[3] - r[2]) for r in wt if r[4] == 0.0]
        d["waits_nonblocking"] = sum(1 for r in wt if r[4] != 0.0)
        # wake-up latency of the GL render thread: a blocking wait (req 0) ends this long after the first display-link
        # callback (which notifies the condvar) that fell inside it -- runnable-but-not-running time
        dcb = sorted(r[2] for r in dl)
        import bisect
        lat_w = []
        for r in wt:
            if r[4] != 0.0:
                continue
            i = bisect.bisect_right(dcb, r[2])
            if i < len(dcb) and dcb[i] <= r[3]:
                lat_w.append(ms(r[3] - dcb[i]))
        d["wake_latency_ms"] = [round(pct(lat_w, q), 3) for q in (0.5, 0.9, 0.99)] if lat_w else None
        d["wake_latency_over1ms"] = sum(1 for x in lat_w if x > 1.0)
        # GL thread on-CPU time (builds with kind 17 / 7): renderOpenGL CPU vs wall; flush CPU vs wall
        c17 = {r[2]: r for r in recs[17]}
        oncpu = [((c17[r[2]][4] - c17[r[2]][3]) / 1e6, ms(r[3] - r[2])) for r in fr if r[2] in c17]
        if oncpu:
            d["render_cpu_ms_p50_p90"] = [round(pct([x for x, _ in oncpu], q), 3) for q in (0.5, 0.9)]
            d["render_offcpu_ms_p50_p90_p99"] = [round(pct([y - x for x, y in oncpu], q), 3) for q in (0.5, 0.9, 0.99)]
        c7 = [r for r in recs[7] if inw(r[2]) and r[4] == 2.0]
        if c7 and c17:
            ex = sorted((r[3], c17[r[2]][4]) for r in recs[10] if r[2] in c17 and inw(r[2]))   # (frame exit ticks, cpu ns)
            fc = []
            for r in c7:
                j = bisect.bisect_right([e[0] for e in ex], r[2]) - 1
                if j >= 0:
                    fc.append((r[3] - ex[j][1]) / 1e6)
            d["flush_cpu_ms_p50_p90"] = [round(pct(fc, q), 3) for q in (0.5, 0.9)] if fc else None
        d["wait_for_dl_ms"] = [round(pct(wts, q), 2) for q in (0.1, 0.5, 0.9)] if wts else None
        # DL tick -> frame start latency; frame start phase in the vblank (vs the DL's inNow host time)
        lat = []; phase = []; endph = []
        j = 0
        for r in fr:
            while j + 1 < len(dl) and dl[j + 1][3] <= r[2]:
                j += 1
            if dl and dl[j][3] <= r[2]:
                lat.append(ms(r[2] - dl[j][3]))
                phase.append((ms(r[2] - dl[j][3])) % P)
        g0 = dl[0][4] if dl else None   # grid anchor (outputTime)
        if g0 is not None:
            endph = [ms(r[3] - g0) % P for r in sw]
            d["frame_start_grid_phase_hist_1ms"] = dict(sorted(Counter(int(ms(r[2] - g0) % P) for r in fr).items()))
            d["flush_end_grid_phase_hist_1ms"] = dict(sorted(Counter(int(x) for x in endph).items()))
            d["dl_cb_grid_phase_ms"] = [round(pct([ms(r[2] - g0) % P for r in dl], q), 2) for q in (0.1, 0.5, 0.9)]
        d["dl_to_frame_ms"] = [round(pct(lat, q), 2) for q in (0.1, 0.5, 0.9, 0.99)] if lat else None
        d["dl_callback_after_vsync_ms"] = round(statistics.median([ms(r[2] - r[3]) for r in dl]), 3) if dl else None
        d["flush_end_phase_ms"] = [round(pct(endph, q), 2) for q in (0.1, 0.5, 0.9)] if endph else None
        d["frame_start_phase_hist_0.5ms"] = dict(sorted(Counter(int(x * 2) / 2 for x in phase).items())) if phase else None
        # GL thread placement
        cpus = Counter(r[4] for r in fr)
        d["gl_cpu_entry"] = dict(sorted(Counter(int(r[4]) for r in fr).items()))
        if ecpus:
            d["gl_on_ecore_frac"] = round(sum(v for c, v in cpus.items() if int(c) in ecpus) / max(1, len(fr)), 3)
        # split
        if ph:
            pre = [ms(r[3] - r[2]) for r in ph if r[3] > 0]
            comp = [ms(r[4] - r[3]) for r in ph if r[4] > 0 and r[3] > 0]
            chain = [ms(r[5] - r[4]) for r in ph if r[5] > 0 and r[4] > 0]
            pres = [ms(r[6] - r[5]) for r in ph if r[6] > 0 and r[5] > 0]
            d["split_ms_median_p90"] = {"pre": (round(pct(pre, .5), 3), round(pct(pre, .9), 3)) if pre else None,
                                        "composite": (round(pct(comp, .5), 3), round(pct(comp, .9), 3)) if comp else None,
                                        "chain_master": (round(pct(chain, .5), 3), round(pct(chain, .9), 3)) if chain else None,
                                        "present": (round(pct(pres, .5), 3), round(pct(pres, .9), 3)) if pres else None}
        if up:
            n = [r[3] for r in up]; ut = [ms(r[4]) for r in up]; vs_ = [ms(r[5]) for r in up]; mc = [ms(r[6]) for r in up]
            d["uploads_per_frame_mean"] = round(sum(n) / len(n), 3)
            upf = [ms(r[4]) for r in up if r[3] > 0]
            d["upload_ms_per_frame_with_upload"] = [round(pct(upf, q), 3) for q in (0.5, 0.9)] if upf else None
            d["upload_ms_per_frame_mean"] = round(sum(ut) / len(ut), 3)
            d["videosync_ms_per_frame"] = [round(pct(vs_, q), 3) for q in (0.5, 0.9)]
            d["memcpy_ms_per_frame_mean"] = round(sum(mc) / len(mc), 3)
            per_up = [ms(r[3]) for r in recs[13] if inw(r[2])]
            d["per_upload_ms"] = [round(pct(per_up, q), 3) for q in (0.5, 0.9, 0.99)] if per_up else None
        if gp:
            g = [r[3] for r in gp]
            d["gpu_ms"] = [round(pct(g, q), 2) for q in (0.5, 0.9)]
            d["gpu_samples_frac"] = round(len(gp) / max(1, len(fr)), 2)
        if de:
            dec = [ms(r[3]) for r in de]; post = [ms(r[4]) for r in de]
            d["decode_wait_ms"] = [round(pct(dec, q), 2) for q in (0.5, 0.9)]
            d["post_decode_ms"] = [round(pct(post, q), 2) for q in (0.5, 0.9)]
            d["decode_thread_cpus"] = dict(sorted(Counter(int(r[1]) for r in de).items()))
        if sws:
            d["sws_ms"] = [round(pct([ms(r[3]) for r in sws], q), 2) for q in (0.5, 0.9)]
            d["sws_per_s"] = round(len(sws) / span, 1)
        # threads: CPU ms/s over the window and QoS (samples nearest the window ends)
        by = defaultdict(list)
        for s in th:
            by[s[1]].append(s)
        rows = []
        for tid, ss in by.items():
            # the sample pair spanning the window (a thread closed right after the window has no sample after it:
            # then its last sample inside the window; a pair must cover >= 60 % of the window)
            s0 = [s for s in ss if ns(s[0]) <= a] or [s for s in ss if ns(s[0]) <= b][:1]
            s1 = [s for s in ss if ns(s[0]) >= b][:1] or [s for s in ss if ns(s[0]) <= b][-1:]
            if not s0 or not s1:
                continue
            x, y = s0[-1], s1[0]
            if ns(y[0] - x[0]) < 0.6 * (b - a):
                continue
            dt = ns(y[0] - x[0]) / 1e9
            cpu_ms_s = (y[2] - x[2]) / 1e6 / dt if dt > 0 else 0   # pth_user_time + pth_system_time are ns
            rows.append((round(cpu_ms_s, 1), y[8], QOS.get(y[6], hex(y[6])), y[3], y[4], tid))
        rows.sort(reverse=True)
        d["threads_top"] = rows[:14]
        out["windows"].append(d)
    return out


if __name__ == "__main__":
    r = summarize(sys.argv[1])
    if "--json" in sys.argv:
        print(json.dumps(r, indent=1))
    else:
        print(r["run"], r["hdr"], "E-cpus(bg cal):", r["ecpus_bg_calibration"], r["cal"])
        for dd in r["display"]:
            print("  D", " ".join(dd[1:]))
        for w in r["windows"]:
            print(json.dumps(w))
