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
u8 (s-rta-0930 gop2): ruled per cap group of B; A's lines at that cap are the baseline, else all A lines (labelled); a cap
only A ran prints INFO; no B u8 line at all = FAIL "u8: no B launch".
u13_container_reverse (s-rta-1002b mkvidx, plan-mkvidx.md item 4 + ruling-mkvidx.md AM14; bars: probe-vupload.json "_u13"):
per scene the medians per arm; [CTRL-A] (i)-(iii) first, as INFO verdicts on the pre-lane arm that fix how the rules read;
then one rule line each: [FREEZE] (a), [MKV] (b), [HAP] (c), [CPU] (d) B vs A, [PARITY] (d) vs (e) in B, [GUARD] (d) B vs
A, [PP-PARITY] (f) vs (g) in B -- each FAILs with < 5 launches per arm of the scenes it reads; INFO: the big-share turn.
probe-vupload-ab.py --selftest: runs this file on synthetic TSVs (in a mkdtemp dir) and checks the exact u8 rule lines
(three TSVs) and the u13 rule lines (three: all-pass, a frozen B (a), too few launches); prints SELFTEST PASS / FAIL
(exit 0 / 1).
"""
import collections, json, math, os, statistics, subprocess, sys, tempfile


def selftest():
    u8 = "u8_reverse_column_1080x4"
    a0 = ("cap_mb=0 uploads_per_s=17.0 per_player_min=None per_player=None late=1790 hold_no_texture=0 pending=0 bytes_mb=None "
          "frames=None active=None over_budget=None cap_bytes_mb=None decoded_per_upload=75.9")
    b256 = ("cap_mb=256 uploads_per_s=118.0 per_player_min=29.5 per_player=29.5,29.6,29.7,29.8 late=10 hold_no_texture=0 "
            "pending=0 bytes_mb=249.4 frames=84 active=4 over_budget=0 cap_bytes_mb=64.0 decoded_per_upload=10.9")
    b0 = ("cap_mb=0 uploads_per_s=118.0 per_player_min=29.5 per_player=29.5,29.6,29.7,29.8 late=10 hold_no_texture=0 "
          "pending=0 bytes_mb=200.0 frames=268 active=4 over_budget=0 cap_bytes_mb=512.0 decoded_per_upload=1.1")
    cases = {   # name -> (the TSV's (arm, kv) lines per round, the expected exit code)
        "capped-only": ([("A", a0), ("B", b256)], 0),
        "mixed": ([("A", a0), ("B", b0), ("B", b256)], 0),
        "A-only": ([("A", a0)], 1),
    }
    fails = []
    with tempfile.TemporaryDirectory() as d:
        for name, (lines, want_rc) in cases.items():
            path = os.path.join(d, name + ".tsv")
            with open(path, "w") as f:
                for r in range(1, 6):
                    for arm, kv in lines:
                        f.write(f"{r} {arm} DATA {u8} {kv}\n")
            p = subprocess.run([sys.executable, os.path.abspath(__file__), path], capture_output=True, text=True)
            groups = collections.defaultdict(list)   # "cap_mb X" / "none" -> its PASS / FAIL lines
            cur, infos = "none", []
            for ln in p.stdout.splitlines():
                s = ln.strip()
                if s.startswith("-- u8 group "):
                    cur = s[len("-- u8 group "):]
                elif s.startswith("INFO"):
                    infos.append(s)
                elif s.startswith("PASS") or s.startswith("FAIL"):
                    groups[cur].append(s)
            rules = [x for g in groups.values() for x in g]
            checks = [("exit code %d" % want_rc, p.returncode == want_rc)]
            if name == "capped-only":
                g = groups["cap_mb 256.0"]
                checks += [("0 PASS / FAIL lines naming cap 0.0", sum("cap 0.0" in x for x in rules) == 0),
                           ("exactly 1 INFO 'cap 0.0: pre-lane arm only -- no rule'",
                            sum("cap 0.0: pre-lane arm only -- no rule" in x for x in infos) == 1),
                           ("exactly 5 'u8 cap 256.0' rule lines", sum("u8 cap 256.0" in x for x in rules) == 5 and len(g) == 5),
                           ("every rule line PASS", len(rules) == 5 and all(x.startswith("PASS") for x in rules))]
            elif name == "mixed":
                g0, g256 = groups["cap_mb 0.0"], groups["cap_mb 256.0"]
                checks += [("exactly 6 cap-0.0 rule lines (pooled / GC6 / late / bytes / over_budget / hold)", len(g0) == 6
                            and all(sum(k in x for x in g0) == 1 for k in ("pooled", "[GC6]", "median late", "video_gopcache_bytes",
                                                                           "over_budget", "hold_no_texture"))),
                           ("exactly 5 cap-256.0 rule lines", len(g256) == 5),
                           ("every rule line PASS", len(rules) == 11 and all(x.startswith("PASS") for x in rules))]
            else:
                checks += [("exactly 1 FAIL 'u8: no B launch'", sum(x == "FAIL  u8: no B launch" for x in rules) == 1),
                           ("no other rule line", len(rules) == 1)]
            for what, ok in checks:
                print(f"   {'ok  ' if ok else 'FAIL'}  selftest ({name}): {what}")
                if not ok:
                    fails.append(f"{name}: {what}")
            if any(not ok for _, ok in checks):
                print("      output was:\n" + "\n".join("      | " + x for x in p.stdout.splitlines()))
        # u13 (mkvidx AM14 (7)): (scene -> A kv, B kv); the A arm reproduces main's freeze / HAP / Matroska cost
        u13 = "u13_container_reverse"
        rev_ok = "uploads_per_s=29.9 late=0 bracket_ok=1 mono_ok=1 nonmono=0 decoded_per_upload=2.0 kf_lines=1"
        frozen = "uploads_per_s=3.0 late=400 bracket_ok=0 mono_ok=0 nonmono=0 decoded_per_upload=30.0 kf_lines=0"
        sA = {"mkv_cuesfront_reverse": frozen,
              "mkv_reverse": "uploads_per_s=29.9 late=0 bracket_ok=1 mono_ok=1 nonmono=0 decoded_per_upload=2.5 kf_lines=0",
              "hap600_reverse": "uploads_per_s=15.0 late=150 bracket_ok=1 mono_ok=0 nonmono=0 decoded_per_upload=2.4 kf_lines=0",
              "mkv_long_column_1080x4": "uploads_per_s=118.0 per_player_min=29.0 late=10 decoded_per_upload=8.0 kf_lines=0",
              "mp4_long_column_1080x4": "uploads_per_s=118.0 per_player_min=29.0 late=8 decoded_per_upload=3.0 kf_lines=0",
              "mkv_long_pingpong_turn": "uploads_per_s=30.0 late=0 max_gap_ms=40.0 turn_seen=1 kf_lines=0",
              "mp4_long_pingpong_turn": "uploads_per_s=30.0 late=20 max_gap_ms=300.0 turn_seen=1 kf_lines=0"}
        sB = {"mkv_cuesfront_reverse": rev_ok, "mkv_reverse": rev_ok,
              "hap600_reverse": "uploads_per_s=29.9 late=0 bracket_ok=1 mono_ok=1 nonmono=0 decoded_per_upload=1.05 kf_lines=0",
              "mkv_long_column_1080x4": "uploads_per_s=119.0 per_player_min=29.5 late=5 decoded_per_upload=2.0 kf_lines=4",
              "mp4_long_column_1080x4": "uploads_per_s=119.0 per_player_min=29.5 late=5 decoded_per_upload=2.0 kf_lines=0",
              "mkv_long_pingpong_turn": "uploads_per_s=30.0 late=20 max_gap_ms=300.0 turn_seen=1 kf_lines=1",
              "mp4_long_pingpong_turn": "uploads_per_s=30.0 late=10 max_gap_ms=250.0 turn_seen=1 kf_lines=0"}
        tags = ["[FREEZE]", "[MKV]", "[HAP]", "[CPU]", "[PARITY]", "[GUARD]", "[PP-PARITY]"]
        cases13 = {   # name -> (rounds, B's (a) line, the expected exit code)
            "u13-all-pass": (5, sB["mkv_cuesfront_reverse"], 0),
            "u13-frozen-B-a": (5, frozen, 1),
            "u13-too-few": (4, sB["mkv_cuesfront_reverse"], 1),
        }
        for name, (rounds, b_a, want_rc) in cases13.items():
            path = os.path.join(d, name + ".tsv")
            with open(path, "w") as f:
                for r in range(1, rounds + 1):
                    for sc in sA:
                        f.write(f"{r} A DATA {u13} scene={sc} {sA[sc]}\n")
                        f.write(f"{r} B DATA {u13} scene={sc} {b_a if sc == 'mkv_cuesfront_reverse' else sB[sc]}\n")
            p = subprocess.run([sys.executable, os.path.abspath(__file__), path], capture_output=True, text=True)
            rules = [ln.strip() for ln in p.stdout.splitlines() if ln.strip().startswith(("PASS", "FAIL"))]
            tagged = {t: [x for x in rules if x.split()[1] == t] for t in tags}
            ctrl = [ln for ln in p.stdout.splitlines() if ln.strip().startswith("INFO  [CTRL-A]")]
            checks = [("exit code %d" % want_rc, p.returncode == want_rc),
                      ("exactly 7 rule lines, one per u13 tag", len(rules) == 7 and all(len(v) == 1 for v in tagged.values())),
                      ("exactly 3 [CTRL-A] INFO lines", len(ctrl) == 3)]
            if name == "u13-all-pass":
                checks += [("every u13 rule PASS", all(x.startswith("PASS") for x in rules)),
                           ("[CTRL-A] (i) / (ii) / (iii) read the A arm as reproducing main",
                            sum("STOP" in x or "non-discriminating" in x or "does not reproduce" in x for x in ctrl) == 0)]
            elif name == "u13-frozen-B-a":
                fl = [x for x in rules if x.startswith("FAIL")]
                checks += [("exactly one FAIL and it is [FREEZE]", len(fl) == 1 and fl[0].split()[1] == "[FREEZE]")]
            else:
                checks += [("every u13 rule FAIL, none PASS", len(rules) == 7 and all(x.startswith("FAIL") for x in rules))]
            for what, ok in checks:
                print(f"   {'ok  ' if ok else 'FAIL'}  selftest ({name}): {what}")
                if not ok:
                    fails.append(f"{name}: {what}")
            if any(not ok for _, ok in checks):
                print("      output was:\n" + "\n".join("      | " + x for x in p.stdout.splitlines()))
    print("SELFTEST " + ("PASS" if not fails else "FAIL (" + "; ".join(fails) + ")"))
    sys.exit(0 if not fails else 1)


if len(sys.argv) > 1 and sys.argv[1] == "--selftest":
    selftest()

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
        # s-rta-0930 gop2: the bars are ruled per cap group of the LANE arm (B); the pre-lane arm's lines at the same cap are
        # its baseline, else all of them (labelled). A cap only the pre-lane arm ran gets an INFO line, never a rule (a
        # capped-only run no longer invents "cap 0.0" rules on empty B data); no B u8 line at all is one FAIL.
        capsB = sorted({kv.get("cap_mb") for r, a, kv in recs if a == "B"})
        if not capsB:
            rule(False, "u8: no B launch")
        for capmb in sorted({kv.get("cap_mb") for r, a, kv in recs if a == "A"} - set(capsB)):
            print(f"   INFO  u8 cap {capmb}: pre-lane arm only -- no rule")
        for capmb in capsB:
            B_ = [kv for r, a, kv in recs if a == "B" and kv.get("cap_mb") == capmb]
            A_ = [kv for r, a, kv in recs if a == "A" and kv.get("cap_mb") == capmb]
            label = f"cap_mb {capmb}"
            if not A_:
                A_ = [kv for r, a, kv in recs if a == "A"]
                capsA = sorted({kv.get("cap_mb") for kv in A_})
                label = f"cap_mb {capmb} (A baseline, cap {', '.join(str(c) for c in capsA)})" if A_ else label
            print(f"   -- u8 group cap_mb {capmb}")
            info_line(label, A_, B_)
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
    elif row == "u13_container_reverse":
        # s-rta-1002b mkvidx (plan-mkvidx.md item 4 + ruling-mkvidx.md AM14): the docstring; bars: probe-vupload.json "_u13".
        per = collections.defaultdict(lambda: collections.defaultdict(list))
        for r, arm, kv in recs:
            per[kv.get("scene")][arm].append(kv)
        SA, SB, SC, SD, SE, SF, SG = ("mkv_cuesfront_reverse", "mkv_reverse", "hap600_reverse", "mkv_long_column_1080x4",
                                      "mp4_long_column_1080x4", "mkv_long_pingpong_turn", "mp4_long_pingpong_turn")
        mm = lambda sc, arm, k: med([kv.get(k) for kv in per[sc][arm]])   # noqa: E731
        nn = lambda *scs: min(len(per[sc][arm]) for sc in scs for arm in ("A", "B"))   # noqa: E731
        for sc in (SA, SB, SC, SD, SE, SF, SG):
            info_line(sc, per[sc]["A"], per[sc]["B"])
        umin = float(VU["u7ReverseUploadsMin"]); lmax = float(VU["u7ReverseLateMax"]); hmax = float(VU["u13HapDecPerUploadMax"])
        # [CTRL-A] first: INFO verdicts on the pre-lane arm (A) -- they fix the interpretation, never a lane defect
        ua = mm(SA, "A", "uploads_per_s"); fz = float(VU["u13CtrlAFreezeMax"])
        print(f"   INFO  [CTRL-A] (i) A (a) median uploads/s {ua} < {fz:g}: " + (
            "the pre-lane app freezes on the front-Cues file -- [FREEZE] discriminates" if ua is not None and ua < fz else
            "STOP -- the live pre-lane app does not reproduce the front-Cues freeze: check a1080_g250_cuesfront.mkv's index "
            "before judging B") + f" (launches {len(per[SA]['A'])})")
        uc, dc = mm(SC, "A", "uploads_per_s"), mm(SC, "A", "decoded_per_upload")
        print(f"   INFO  [CTRL-A] (ii) A (c) median uploads/s {uc} < {umin:g} or decoded_per_upload {dc} > {hmax:g}: " + (
            "A fails [HAP]'s bars -- [HAP] discriminates" if (uc is not None and uc < umin) or (dc is not None and dc > hmax) else
            "[HAP] is non-discriminating in this run (recorded) -- check hap1080_tb600.mov's time base (1/600) before "
            "judging B") + f" (launches {len(per[SC]['A'])})")
        dd, de = mm(SD, "A", "decoded_per_upload"), mm(SE, "A", "decoded_per_upload"); cm = float(VU["u13CtrlACpuMin"])
        print(f"   INFO  [CTRL-A] (iii) A (d) median decoded_per_upload {dd} >= {cm:g} x A (e) {de}: " + (
            "the pre-lane app pays the Matroska cost -- [CPU] discriminates" if None not in (dd, de) and dd >= cm * de else
            "the live app does not reproduce the emulated Matroska cost -- a [CPU] FAIL is the expected outcome")
            + f" (launches {len(per[SD]['A'])} / {len(per[SE]['A'])})")
        for tg, sc, lab in (("[FREEZE]", SA, "(a)"), ("[MKV]", SB, "(b)"), ("[HAP]", SC, "(c)")):
            u, lt, bo, mo = (mm(sc, "B", k) for k in ("uploads_per_s", "late", "bracket_ok", "mono_ok"))
            nm = [kv.get("nonmono") for kv in per[sc]["B"]]; dp = mm(sc, "B", "decoded_per_upload")
            good = (nn(sc) >= 5 and u is not None and u >= umin and lt is not None and lt <= lmax and bo == 1 and mo == 1
                    and len(nm) > 0 and all(x == 0 for x in nm))
            extra = ""
            if tg == "[HAP]":
                good = good and dp is not None and dp <= hmax; extra = f", decoded_per_upload {dp} <= {hmax:g}"
            rule(good, f"{tg} {lab} {sc}: B medians uploads/s {u} >= {umin:g}, late {lt} <= {lmax:g}, bracket_ok {bo} == 1, "
                       f"mono_ok {mo} == 1{extra}; nonmono every B launch {nm} == 0 (A medians uploads/s "
                       f"{mm(sc, 'A', 'uploads_per_s')}, late {mm(sc, 'A', 'late')}); launches {nn(sc)}")
        bd, ad = mm(SD, "B", "decoded_per_upload"), mm(SD, "A", "decoded_per_upload"); cr = float(VU["u13CpuRatioMax"])
        be = mm(SE, "B", "decoded_per_upload"); pm = float(VU["u13ParityMax"])
        cpu_ok = nn(SD) >= 5 and None not in (bd, ad) and bd <= cr * ad
        par_ok = nn(SD, SE) >= 5 and None not in (bd, be) and bd <= pm * be
        rule(cpu_ok, f"[CPU] (d) B median decoded_per_upload {bd} <= {cr:g} x A's {ad} "
                     f"({None if ad is None else round(cr * ad, 3)}); launches {nn(SD)}")
        if not cpu_ok and par_ok and None not in (bd, ad) and ad:
            print(f"   INFO  [CPU] recorded verdict: live Matroska cost ratio R = {round(bd / ad, 3)} (B / A, (d)); with [PARITY] "
                  f"PASS this is not a merge blocker (AM14 (4)): the docs carry R")
        rule(par_ok, f"[PARITY] (d) vs (e): B median decoded_per_upload mkv {bd} <= {pm:g} x mp4 {be} "
                     f"({None if be is None else round(pm * be, 3)}); launches {nn(SD, SE)}")
        bl, al = mm(SD, "B", "late"), mm(SD, "A", "late"); bp, ap = mm(SD, "B", "per_player_min"), mm(SD, "A", "per_player_min")
        ls, ps = float(VU["u13LateSlack"]), float(VU["u13PerPlayerSlack"])
        rule(nn(SD) >= 5 and None not in (bl, al, bp, ap) and bl <= al + ls and bp >= ap - ps,
             f"[GUARD] (d) B median late {bl} <= A {al} + {ls:g} and B median per_player_min {bp} >= A {ap} - {ps:g}; "
             f"launches {nn(SD)}")
        fl, gl = mm(SF, "B", "late"), mm(SG, "B", "late"); pp = float(VU["u13PpLateSlack"])
        rule(nn(SF, SG) >= 5 and None not in (fl, gl) and fl <= gl + pp,
             f"[PP-PARITY] (f) vs (g): B median late mkv {fl} <= mp4 {gl} + {pp:g}; launches {nn(SF, SG)}")
        afl = mm(SF, "A", "late")
        print(f"   INFO  big-share turn: A mkv late {afl}, B mkv late {fl}, B mp4 late {gl}; max gaps A mkv "
              f"{mm(SF, 'A', 'max_gap_ms')} B mkv {mm(SF, 'B', 'max_gap_ms')} B mp4 {mm(SG, 'B', 'max_gap_ms')} ms; turn_seen B "
              f"{[kv.get('turn_seen') for kv in per[SF]['B']]} / {[kv.get('turn_seen') for kv in per[SG]['B']]}; decision (iv): "
              f"B mkv late - A mkv late = {None if None in (fl, afl) else fl - afl} (> 60 per window -> Q1 to Boris)")
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
