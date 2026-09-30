#!/usr/bin/env python3
"""probe-vupload-ab.py <ab.tsv> -- the summary of .harmony/probe-vupload-ab.sh (s-rta-0929 vupload).

Input lines: "<round> <arm> DATA <row> k=v k=v ...". A = the pre-lane app, B = the lane app (by convention).
- w2c_steady_4kx4_blit (plan-vupload.md 4.9): per launch the median of its loads' median fps -> mA (A launches), mB (B).
  fps4kBlitMin = floor(min(mB)) - 3 ONLY if that is >= max(mA) + 8; else null (INFO) -- never a knife-edge.
- u7_reverse_pingpong (adoption VU7): per scene, the median over launches of uploads_per_s and late per arm; PASS iff
  B's median uploads/s >= u7UploadsRatioMin x A's and B's median late <= A's.
- u7 (s-rta-0929b gopcache, plan-gopcache.md 4.2 + adoption GC2-GC13): additionally, on the MEDIAN over the lane app's
  (B) launches (GC13; >= 5 per arm): reverse / turn / flip / speed-2 scenes uploads/s >= u7ReverseUploadsMin x speed, late <=
  u7ReverseLateMax, the mid capture within its bracket (bracket_ok), the back-to-back captures decreasing (mono_ok, GC4),
  video_reverse_nonmonotonic 0 (GC4); turn scenes: exactly one direction change and the turn seen; the forward-window
  ping-pong scenes (GC12): B >= u7FwdRatioMin x A and late 0 in every B launch; g30 flip: max_gap_ms <= u7FlipGapMaxMs,
  g250 flip: B's median max_gap_ms < A's (GC8).
- u8 / u9 / u11: the bars of probe-vupload.json "_u8" / "_u9" / "_u11" on B's medians (u11: B <= A).
- every other row: per arm, per numeric key, the median over all its DATA lines (and n).
Each rule prints its own PASS / FAIL line tagged with its source ([VU7] [GC12] [ABS] ...); the exit code is 1 on any FAIL.
"""
import collections, json, math, os, statistics, subprocess, sys

rows = collections.defaultdict(list)   # row -> [(round, arm, {k: v})]
for line in open(sys.argv[1]):
    parts = line.split()
    if len(parts) < 4 or parts[2] != "DATA":
        continue
    kv = {}
    for p in parts[4:]:
        if "=" in p:
            k, v = p.split("=", 1)
            try:
                kv[k] = float(v)
            except ValueError:
                kv[k] = v
    rows[parts[3]].append((int(parts[0]), parts[1], kv))

VU = json.load(open(os.path.join(os.path.dirname(os.path.abspath(__file__)), "probe-vupload.json")))
rc = 0
BUDGET = min(int(VU["gopcacheBudgetBytes"]),
             (int(subprocess.run(["sysctl", "-n", "hw.memsize"], capture_output=True, text=True).stdout.strip() or 0) // 16)
             or int(VU["gopcacheBudgetBytes"]))


def med(xs):
    xs = [x for x in xs if isinstance(x, float)]
    return statistics.median(xs) if xs else None


def rule(good, msg):
    global rc
    rc |= 0 if good else 1
    print(f"   {'PASS' if good else 'FAIL'}  {msg}")


def info_line(label, A_, B_):
    ks = sorted({k for kv in A_ + B_ for k, v in kv.items() if isinstance(v, float)})
    for arm, xs in (("A", A_), ("B", B_)):
        if xs:
            print(f"      {label} {arm} medians (n {len(xs)}): " + ", ".join(f"{k} {med([kv.get(k) for kv in xs])}" for k in ks))


for row, recs in sorted(rows.items()):
    print(f"== {row}")
    if row == "w2c_steady_4kx4_blit":
        per = collections.defaultdict(list)
        for r, arm, kv in recs:
            per[(arm, r)].append(kv.get("fps"))
        mA = sorted(round(med(v), 2) for (a, r), v in per.items() if a == "A" and med(v) is not None)
        mB = sorted(round(med(v), 2) for (a, r), v in per.items() if a == "B" and med(v) is not None)
        print(f"   launch medians A (pre-lane) {mA}")
        print(f"   launch medians B (lane)     {mB}")
        if len(mA) >= 5 and len(mB) >= 5:
            bar = math.floor(min(mB)) - 3
            if bar >= max(mA) + 8:
                print(f"   fps4kBlitMin = floor(min(B)) - 3 = {bar} (>= max(A) {max(mA)} + 8: set it)")
            else:
                print(f"   fps4kBlitMin stays null: floor(min(B)) - 3 = {bar} < max(A) {max(mA)} + 8 (a knife-edge: INFO)")
        else:
            print(f"   fewer than 5 launches per arm ({len(mA)} / {len(mB)}): no bar")
    elif row == "u7_reverse_pingpong":
        per = collections.defaultdict(lambda: collections.defaultdict(list))
        for r, arm, kv in recs:
            per[kv.get("scene")][arm].append(kv)
        ratio = float(VU["u7UploadsRatioMin"]); fwd = float(VU["u7FwdRatioMin"])
        for sc, arms in sorted(per.items()):
            A_, B_ = arms.get("A", []), arms.get("B", [])
            ua = med([kv.get("uploads_per_s") for kv in A_]); ub = med([kv.get("uploads_per_s") for kv in B_])
            la = med([kv.get("late") for kv in A_]); lb = med([kv.get("late") for kv in B_])
            n = (len(A_), len(B_))
            good = None not in (ua, ub, la, lb) and ub >= ratio * ua and lb <= la and min(n) >= 5
            rule(good, f"[VU7]  {sc}: uploads/s A {ua} B {ub} (>= {ratio} x A: {None if ua is None else round(ratio * ua, 2)}), "
                       f"late A {la} B {lb} (B <= A), launches {n}")
            info_line(sc, A_, B_)
            if sc.endswith("_pingpong"):   # GC12: the forward-window scenes, tight
                lates = [kv.get("late") for kv in B_]
                rule(None not in (ua, ub) and ub >= fwd * ua and all(x == 0 for x in lates) and min(n) >= 5,
                     f"[GC12] {sc}: B median uploads/s {ub} >= {fwd} x A {ua} ({None if ua is None else round(fwd * ua, 2)}), late "
                     f"every B launch {lates} == 0")
                continue
            spd = 2.0 if sc.endswith("_speed2") else 1.0
            umin = float(VU["u7ReverseUploadsMin"]) * spd; lmax = float(VU["u7ReverseLateMax"])
            rule(ub is not None and ub >= umin and len(B_) >= 5, f"[ABS]  {sc}: (a) B median uploads/s {ub} >= {umin:g}")
            rule(lb is not None and lb <= lmax and len(B_) >= 5, f"[ABS]  {sc}: (b) B median late {lb} <= {lmax:g}")
            for k, want, lab in (("bracket_ok", 1, "(d) mid capture within its bracket (ct 1)"),
                                 ("mono_ok", 1, "(GC4) back-to-back captures strictly decreasing, each in its bracket"),
                                 ("nonmono", 0, "(GC4) video_reverse_nonmonotonic")):
                m = med([kv.get(k) for kv in B_])
                rule(m is not None and m == want and len(B_) >= 5, f"[ABS]  {sc}: {lab}: B median {m} == {want} "
                                                                  f"(per launch {[kv.get(k) for kv in B_]}; A {[kv.get(k) for kv in A_]})")
            if "_turn" in sc:
                for k, want, lab in (("dirchg", 1, "(GC4) exactly one direction change"), ("turn_seen", 1, "the turn inside the window")):
                    m = med([kv.get(k) for kv in B_])
                    rule(m == want and len(B_) >= 5, f"[ABS]  {sc}: {lab}: B median {m} == {want} (per launch {[kv.get(k) for kv in B_]})")
            if "_flip" in sc:
                ga = med([kv.get("max_gap_ms") for kv in A_]); gb = med([kv.get("max_gap_ms") for kv in B_])
                rv = med([kv.get("reversed") for kv in B_])
                rule(rv == 1, f"[GC8]  {sc}: the REST flip reversed the clip in B (median reversed {rv}; A {[kv.get('reversed') for kv in A_]})")
                if sc.startswith("g30"):
                    lim = float(VU["u7FlipGapMaxMs"])
                    rule(gb is not None and gb <= lim and len(B_) >= 5, f"[GC8]  {sc}: B median max upload gap {gb} ms <= {lim:g} (A {ga})")
                else:
                    rule(None not in (ga, gb) and gb < ga and min(n) >= 5, f"[GC8]  {sc}: B median max upload gap {gb} ms < A {ga} ms")
    elif row == "u8_reverse_column_1080x4":
        for capmb in sorted({kv.get("cap_mb") for r, a, kv in recs}):
            B_ = [kv for r, a, kv in recs if a == "B" and kv.get("cap_mb") == capmb]
            A_ = [kv for r, a, kv in recs if a == "A" and kv.get("cap_mb") == capmb]
            info_line(f"cap_mb {capmb}", A_, B_)
            floors = 4 * int(VU["floorFrames"]) * int(VU["frameBytes1080"])
            budget = (capmb * 1048576 if capmb else BUDGET) + floors
            m = lambda k: med([kv.get(k) for kv in B_])   # noqa: E731
            enough = len(B_) >= 5
            if capmb:
                rule(enough and (m("per_player_min") or 0) >= float(VU["u8CapPerPlayerMin"]),
                     f"[GC7]  u8 cap {capmb} MB: B median slowest player {m('per_player_min')} uploads/s >= {VU['u8CapPerPlayerMin']}")
            else:
                rule(enough and (m("uploads_per_s") or 0) >= float(VU["u8PooledMin"]),
                     f"[ABS]  u8: B median pooled uploads/s {m('uploads_per_s')} >= {VU['u8PooledMin']}")
                rule(enough and (m("per_player_min") or 0) >= float(VU["u8PerPlayerMin"]),
                     f"[GC6]  u8: B median slowest player {m('per_player_min')} uploads/s >= {VU['u8PerPlayerMin']}")
            rule(enough and m("late") is not None and m("late") <= float(VU["u8LateMax"]), f"[ABS]  u8 cap {capmb}: B median late {m('late')} <= {VU['u8LateMax']}")
            rule(enough and m("bytes_mb") is not None and m("bytes_mb") * 1048576 <= budget,
                 f"[ABS]  u8 cap {capmb}: B median max video_gopcache_bytes {m('bytes_mb')} MB <= budget + floors {round(budget / 1048576.0, 1)} MB")
            rule(enough and all((kv.get("over_budget") == 0) for kv in B_) if not capmb else enough,
                 f"[ABS]  u8 cap {capmb}: over_budget delta per B launch {[kv.get('over_budget') for kv in B_]} (== 0 without a cap)")
            rule(enough and all(kv.get("hold_no_texture") == 0 and kv.get("pending") == 0 for kv in B_),
                 f"[ABS]  u8 cap {capmb}: hold_no_texture / pending 0 in every B launch")
    elif row == "u9_reverse_cache_drop":
        A_ = [kv for r, a, kv in recs if a == "A"]; B_ = [kv for r, a, kv in recs if a == "B"]
        info_line("u9", A_, B_)
        m = lambda k: med([kv.get(k) for kv in B_])   # noqa: E731
        enough = len(B_) >= 5
        rule(enough and m("bytes0_mb") is not None and float(VU["u9BytesMinMb"]) <= m("bytes0_mb") <= BUDGET / 1048576.0
             and (m("frames0") or 0) >= int(VU["floorFrames"]), f"[ABS]  u9 (a): B median bytes at s0 {m('bytes0_mb')} MB in "
             f"[{VU['u9BytesMinMb']}, {round(BUDGET / 1048576.0)}], frames {m('frames0')} >= {VU['floorFrames']} (INFO: >= "
             f"{float(VU['u9FramesFracInfo']) * 300:g} -- {(m('frames0') or 0) >= float(VU['u9FramesFracInfo']) * 300})")
        rule(enough and m("bytes1") == 0 and m("frames1") == 0 and m("active1") == 0,
             f"[ABS]  u9 (b): after the trim B median bytes {m('bytes1')} / frames {m('frames1')} / active {m('active1')} == 0")
        rule(enough and m("footprint_drop_frac") is not None and m("footprint_drop_frac") >= float(VU["u9FootprintDropFracMin"]),
             f"[GC10] u9: B median phys_footprint drop {m('footprint_drop_frac')} of the counted bytes >= {VU['u9FootprintDropFracMin']}")
        rule(enough and m("bracket_ok") == 1 and all(kv.get("hold_no_texture") == 0 for kv in B_),
             f"[ABS]  u9 (c): back on screen the code within its bracket (B median {m('bracket_ok')}), hold 0 every launch")
        rule(enough and m("late") is not None and m("late") <= float(VU["u9LateMax"]), f"[ABS]  u9 (d): B median late {m('late')} <= {VU['u9LateMax']}")
    elif row == "u11_reverse_column_return":
        A_ = [kv for r, a, kv in recs if a == "A"]; B_ = [kv for r, a, kv in recs if a == "B"]
        info_line("u11", A_, B_)
        ga = med([kv.get("pooled_gap_ms") for kv in A_]); gb = med([kv.get("pooled_gap_ms") for kv in B_])
        rule(None not in (ga, gb) and gb <= ga and min(len(A_), len(B_)) >= 5,
             f"[GC9]  u11: B median pooled upload gap after the return {gb} ms <= A {ga} ms (first upload A "
             f"{med([kv.get('first_upload_ms') for kv in A_])} B {med([kv.get('first_upload_ms') for kv in B_])} ms)")
    else:
        for arm in ("A", "B"):
            ks = sorted({k for r, a, kv in recs if a == arm for k, v in kv.items() if isinstance(v, float)})
            if ks:
                print(f"   {arm}: " + ", ".join(f"{k} {med([kv.get(k) for r, a, kv in recs if a == arm])}" for k in ks)
                      + f" (n {sum(1 for r, a, kv in recs if a == arm)})")
sys.exit(rc)
