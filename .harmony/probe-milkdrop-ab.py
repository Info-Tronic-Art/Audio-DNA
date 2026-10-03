#!/usr/bin/env python3
"""probe-milkdrop-ab.py -- the G3 decision rule of ruling-bf10 amendment 5 (s-rta-1002b lane bf10) over the ab.tsv
files that .harmony/probe-vupload-ab.sh writes for `.harmony/probe-milkdrop.sh perf_md_1080,perf_md_4k,perf_md_1080_2l`.

usage: probe-milkdrop-ab.py <out>/ab.tsv [<out2>/ab.tsv]   (a second file = 5 more rounds; its rounds are appended)
ab.tsv line: "<round> <arm A|B> DATA perf_md_<x> fps=<mean> fps_min=<min> frame_ms=<mean> gpu_ms=<mean> gpu_peak_ms=<max>"

Per round r (interleaved A then B): d_r = B_r - A_r for gpu_ms and for fps (launch means). Bars on medians over rounds:
  perf_md_1080 (GATE): (a) median fps_B >= 58 when median fps_A >= 58; otherwise median d_fps >= -2;
                       (b) median d_gpu <= 1.0 ms.
  perf_md_4k   (GATE): (a) median d_gpu <= 2.0 ms; (b) median d_fps >= -3.
  perf_md_1080_2l (INFO): medians of both arms and of d.
Decidability (a delta bar): spread(d) = max(d) - min(d) <= 2 x the bar's size (d_gpu: 2.0 ms at 1080, 4.0 ms at 4K;
d_fps: 4 at 1080 (the -2 variant), 6 at 4K). Decidable: PASS iff the median meets the bar, else FAIL. Not decidable:
UNPROVEN (never PASS) -- after 5 rounds run 5 more into a second OUTDIR and pass both files. The absolute 1080 bar (a)
is always decidable. 4K absolute fps is reported; both arms < 58 fps triggers Boris question Q1 (exempts nothing).
"""
import statistics as stx
import sys

GATES = {
    "perf_md_1080": {"gpu": 1.0, "fps_delta": -2.0, "abs_fps": 58.0},
    "perf_md_4k": {"gpu": 2.0, "fps_delta": -3.0},
}


def parse(paths):
    rows = {}   # tag -> round -> arm -> dict
    off = 0
    for p in paths:
        mx = 0
        for line in open(p):
            parts = line.split()
            if len(parts) < 4 or parts[2] != "DATA" or not parts[3].startswith("perf_md_"):
                continue
            r = int(parts[0]) + off; arm = parts[1]; tag = parts[3]; mx = max(mx, int(parts[0]))
            vals = {}
            for kv in parts[4:]:
                if "=" in kv:
                    k, v = kv.split("=", 1)
                    try:
                        vals[k] = float(v)
                    except ValueError:
                        pass
            rows.setdefault(tag, {}).setdefault(r, {})[arm] = vals
        off += mx
    return rows


def delta_bar(name, ds, bar, upper):
    """upper=True: median d <= bar; else median d >= bar. Decidable iff spread <= 2 x |bar|."""
    med = stx.median(ds); spread = max(ds) - min(ds)
    met = med <= bar if upper else med >= bar
    if spread <= 2.0 * abs(bar):
        v = "PASS" if met else "FAIL"
    else:
        v = "UNPROVEN"
    return v, (f"{name}: median d {med:+.3f} (bar {'<=' if upper else '>='} {bar:+.1f}), spread(d) {spread:.3f} "
               f"(decidable iff <= {2.0 * abs(bar):.1f}) -> {v}{'' if v != 'UNPROVEN' or met else ' (median OVER the bar)'}")


def main():
    if len(sys.argv) < 2:
        print(__doc__); sys.exit(2)
    rows = parse(sys.argv[1:])
    overall = []
    for tag in ("perf_md_1080", "perf_md_4k", "perf_md_1080_2l"):
        rs = rows.get(tag, {})
        paired = sorted(r for r, arms in rs.items() if "A" in arms and "B" in arms)
        if not paired:
            print(f"{tag}: no paired rounds"); overall.append((tag, "NO DATA")); continue
        fa = [rs[r]["A"].get("fps", float("nan")) for r in paired]; fb = [rs[r]["B"].get("fps", float("nan")) for r in paired]
        ga = [rs[r]["A"].get("gpu_ms", float("nan")) for r in paired]; gb = [rs[r]["B"].get("gpu_ms", float("nan")) for r in paired]
        dfps = [b - a for a, b in zip(fa, fb)]; dgpu = [b - a for a, b in zip(ga, gb)]
        print(f"{tag}: rounds {len(paired)}")
        print(f"  fps    median A {stx.median(fa):.2f}  median B {stx.median(fb):.2f}  median d {stx.median(dfps):+.2f}  "
              f"spread(d) {max(dfps) - min(dfps):.2f}   per round A {[round(x, 1) for x in fa]} B {[round(x, 1) for x in fb]}")
        print(f"  gpu_ms median A {stx.median(ga):.3f}  median B {stx.median(gb):.3f}  median d {stx.median(dgpu):+.3f}  "
              f"spread(d) {max(dgpu) - min(dgpu):.3f}   per round A {[round(x, 2) for x in ga]} B {[round(x, 2) for x in gb]}")
        g = GATES.get(tag)
        if g is None:
            print(f"  verdict: INFO (no gate)"); overall.append((tag, "INFO")); continue
        verdicts = []
        if "abs_fps" in g:
            if stx.median(fa) >= g["abs_fps"]:
                v = "PASS" if stx.median(fb) >= g["abs_fps"] else "FAIL"
                print(f"  (a) median fps_B {stx.median(fb):.2f} >= {g['abs_fps']:.0f} (median fps_A {stx.median(fa):.2f} "
                      f">= {g['abs_fps']:.0f}) -> {v}")
                verdicts.append(v)
            else:
                v, txt = delta_bar("(a) d_fps (fps_A < 58 variant)", dfps, g["fps_delta"], False)
                print("  " + txt); verdicts.append(v)
            v, txt = delta_bar("(b) d_gpu", dgpu, g["gpu"], True); print("  " + txt); verdicts.append(v)
        else:
            v, txt = delta_bar("(a) d_gpu", dgpu, g["gpu"], True); print("  " + txt); verdicts.append(v)
            v, txt = delta_bar("(b) d_fps", dfps, g["fps_delta"], False); print("  " + txt); verdicts.append(v)
            if stx.median(fa) < 58 and stx.median(fb) < 58:
                print("  4K: BOTH arms below 58 fps -> Boris question Q1 (exempts nothing)")
        verdict = "FAIL" if "FAIL" in verdicts else ("UNPROVEN" if "UNPROVEN" in verdicts else "PASS")
        if verdict == "UNPROVEN" and len(paired) < 10:
            print("  UNPROVEN after < 10 rounds: run 5 more rounds into a second OUTDIR and pass both ab.tsv files")
        print(f"  verdict: {verdict}")
        overall.append((tag, verdict))
    print("G3 " + " ".join(f"{t}={v}" for t, v in overall))


if __name__ == "__main__":
    main()
