#!/usr/bin/env python3
"""probe-vupload.py -- REST/pixel half of .harmony/probe-vupload.sh (s-rta-0929 vupload: the video upload path).

The .sh launches the app in TEST MODE (8080 TestServer + 7070 ApiServer), owns refuse / lock / quit; this file encodes
the fixtures (--make-fixtures, BEFORE the app starts, through .harmony/probe-video.py's fixture code: same fixtures, same
$VIDEO_FIXTURES reuse) and then runs the rows. It imports .harmony/probe-video.py as a module for its helpers (load,
trig, cap, code, playhead, in_bracket, settle_late, Poller, ...): the 7070 REST calls and the pixel decoding are the SAME
code as probe-video's. The TEST-ONLY fields and routes come from 8080. Plan: .harmony/.reports/s-rta-0929/plan-vupload.md
4.7 + HARMONY ADOPTION (VU5: every row asserts video_hold_no_texture delta 0 and video_late_frames delta 0 over its
steady on-screen windows; VU6 / VU7 / VU11).

usage: probe-vupload.py <root> <fresh-outdir> --make-fixtures [row,row,...]
       probe-vupload.py <root> <fresh-outdir> [row,row,...]
rows (run order): u2_gl_thread_qos u4a_context_cycle u4b_idle_ring_trim u6_crossfade_video u7_reverse_pingpong
      u8_reverse_column_1080x4 u9_reverse_cache_drop u10_reverse_4k u11_reverse_column_return u12_reverse_pixel_identity
      u13_container_reverse

u2_gl_thread_qos: INFO "QoS not applied (VU15)": prints 8080 /api/state gl_thread_qos (the render thread samples
  qos_class_self() every frame; 21 = DEFAULT). The lane's P2 raise to USER_INTERACTIVE (33) was reverted by adoption
  addendum 2 VU15 rule (a): w7's message-thread trigger round trips > 20 ms, 10 x 3 interleaved launches: main 2, P2 on
  11, P2 off 2 (of 50 each).
u4a_context_cycle: 1080p canvas, 4 x a1080 layers -- layer 0 (bottom) at "speed": 0.0 (frame 0 held: no newer frame
  ever reaches its clock), layers 1-3 playing -- ONE trigger_column, 2 s, s0; for each detached_ms in cycleDetachedMs
  [POST 8080 /api/debug/gl_context_cycle {"detached_ms": N} (0 = re-attach at once; 500 = the preview hidden long enough
  for the decode threads to park); poll gl_context_gen until it advanced (<= cycleWaitS: the new context recompiles every
  shader)]; 1 s; s1; settle_late; s2; cap; a 2 s steady window; s3. PASS (a) gl_context_gen delta == cycles; (b)
  video_hold_no_texture delta s0 -> s3 == 0 (RED before plan item u3: the shown slot was released at its upload, so after
  a context loss a player with no newer frame at or before its clock -- the speed-0 layer every frame, forever; a
  playing one after a long detach until its catch-up lands -- returned texture 0 = the FX-only trap. An immediate
  re-attach alone does not show it: the ~60 ms shader recompile lets every playing ring fill, commit-1 app 0 over 3 such
  cycles); (c) video_pending_frames delta s0 -> s3 == 0 and
  videos_pending 0 at s3; (d) the top layer's code within its playhead bracket; (e) VU5: video_late_frames delta over the
  steady window s2 -> s3 == 0 (the late frames across a cycle -- the stalled frames while every shader recompiles -- are
  printed as INFO); (f) INFO: gl_thread_qos after the cycles (was VU11's == 33 before the VU15 revert).
u4b_idle_ring_trim: 4K canvas, deck 0 = 4 x a4k, deck 1 = warm.png; trigger_column; 3 s; s0 (8080 phys_footprint_mb,
  `ps -o rss=` of the app, video_slots_purged); switch_deck 1; trimWaitS; s1; switch_deck 0; (the return) s2; cap; a 5 s
  window; s3 (the return: settle_late, returnSettleS more, settle_late again -- four 4K catch-ups end at different
  times, and the first 200 ms lull of settle_late alone is not steady state). PASS (a) video_slots_purged delta s0 -> s1 == 8 (4 players x the 2 slots that are not the shown one); (b)
  phys_footprint OR rss dropped by >= trimDropMinMb (4 x 2 x 33.2 MB = 265 expected; both printed); (c) after the return
  video_pending_frames +0, video_hold_no_texture +0 (s0 -> s3) and the top layer's code within its bracket (the un-purged
  slots carry a correct picture); (d) video_slots_purged does not grow while the deck is on screen (s2 -> s3 == 0);
  (e) VU5: video_late_frames delta over the on-screen windows (the 3 s before the switch is not polled; s2 -> s3) == 0 --
  the return itself is w4's bounded hold (1 <= late <= lateMaxReturn in probe-video w4), printed as INFO here.
  s-rta-0929b gopcache-fix (review SHOULD: plan 4.3's "bytes == 0" became false with GC8, and nothing bounded forward
  retention): (f) at s0 (3 s of plain forward Loop play) video_gopcache_frames <= 4 x fwdRetainMaxFrames (GC8 keeps
  kBehindFrames = 16 behind the clock + the writer's look-ahead; absent = FAIL: the pre-lane app has no cache), the bytes
  and the footprint delta printed; (g) after the idle trim (s1) video_gopcache_frames / _bytes == 0 (R-10).
u6_crossfade_video (VU5): 1080p canvas; 4 layers: layers 0-2 = a1080 in columns 0 and 1 (new players in column 1, cut);
  layer 3 (the TOP) = a1080 in column 0 and b1080 (negated) in column 1 with transitionSpeed 2.0 (the default Dissolve).
  trigger_column 0; 2 s; s0; trigger_column 1 (a crossfade A -> B on the top layer while three more players start on
  the same render frame: the bunch); capMid at +1.0 s; s1 at +2.5 s; settle; capAfter. PASS (a) video_hold_no_texture
  delta == 0; (b) video_late_frames delta == 0 (both chains); (c) capMid is a blend: luma > 20, dbox to A's and to B's
  expected frame both > boxTol, and at least one code-band cell has a mean in [48, 208] (a mixed cell, not one side);
  (d) capAfter's code has bit 9 (B) and (code & 511) within B's bracket; (e) video_pending_frames delta == 0.
u7_reverse_pingpong (VU7): four scenes on the 1080p canvas, each a fresh load of ONE clip: {g30_1080 (GOP 30), a1080_g250
  (GOP 250)} x {reverse (loopMode Loop, reverse on), ping-pong (loopMode PingPong)}; trig; 1 s; s0; a 5 s window; s1.
  Prints uploads/s and late frames per scene as DATA lines; the verdict is taken ACROSS apps by
  .harmony/probe-vupload-ab.sh --u7 (interleaved launches, >= 5 per arm): the lane's median uploads/s per scene >= 0.9 x
  the pre-lane app's, and its median late frames per scene <= the pre-lane app's. In a single run: INFO, except (VU16 =
  VU5) PASS video_hold_no_texture delta s0 -> s1 == 0 per scene (an app that predates the counter FAILs as absent).
--- s-rta-0929b gopcache (.harmony/.reports/s-rta-0929b/plan-gopcache.md 4.2 + HARMONY ADOPTION GC2-GC13) ---
Every absolute bar of these rows is judged by .harmony/probe-vupload-ab.py on the MEDIAN over >= 5 interleaved launches of
the lane app (GC13); one run prints the per-launch values (INFO lines + one DATA line per scene / row). Bars:
.harmony/probe-vupload.json "_gopcache" ... "_u11".
u7 gains five scenes and per-scene fields (decoded, dropped, seeks, skipped, decoded / seeks per upload, the longest upload
  gap from 15 ms polls of video_uploads, video_max_upload_gap_ms, video_reverse_nonmonotonic / video_direction_changes
  deltas, the cache's hits / misses / runs / MB): g30_1080_pingpong_turn / a1080_g250_pingpong_turn (in-point 0.7 by the
  prime retrigger: ~2 s forward, the top turn, ~3 s reverse; turn_seen = the playhead (sampled every 0.2 s) rose to an
  inner maximum and fell after it),
  a1080_g250_reverse_speed2 (GC2), g30_1080_flip_reverse / a1080_g250_flip_reverse (GC8: a Loop clip at in-point 0.6 playing
  FORWARD is flipped to reverse 1 s into the window by a hand-built take replayed over REST -- /api/perf/load + /api/perf/play,
  one clip-scope 'playing' point with the action 'reverse'; flip_gap_ms = the longest upload gap in [flip - 0.1, flip +
  1.5] s; reversed = every playhead sample after the flip fell; the replay is stopped after the scene). Every non-forward
  scene: the mid-window capture's code within its playhead bracket with a tolerance of 1 frame (u7BracketCt, GC11) and,
  after the window, u7MonoCaps captures u7MonoCapGapS apart whose codes must fall strictly, each in its bracket (GC4).
  U7_SCENES=a,b (env) runs only those scenes.
u8_reverse_column_1080x4: 4 x a1080 reverse (Loop) by ONE trigger_column; 1 s; s0; a 5 s window; s1: pooled uploads/s, the
  slowest player's (video_player_uploads slot deltas, GC6), late, hold, pending, max video_gopcache_bytes over 50 ms polls,
  over_budget, cap, decodes per upload, fps, phys_footprint delta. VIDEO_ENV=ADNA_GOPCACHE_BUDGET_MB=256 -> cap_mb 256 (GC7).
u9_reverse_cache_drop: deck 0 = 1 x a1080 reverse, deck 1 = warm.png; trig; 3 s; s0; switch_deck 1; trimWaitS; s1 (the idle
  trim: bytes / frames / active 0, phys_footprint drop vs the counted bytes -- GC10); switch_deck 0; settle; cap (bracket, ct
  1); a 3 s window (late).
u10_reverse_4k (INFO): 1 x a4k reverse on the 4K canvas: uploads/s, late, bytes, frames, decodes per upload, fps.
u11_reverse_column_return (GC9): deck 0 = 4 x a1080 reverse, deck 1 = warm.png; trigger_column; 3 s; switch_deck 1;
  trimWaitS (the caches clear); switch_deck 0 with 10 ms polls for 2 s: the column's longest upload gap after the return
  and its first upload (pooled: both apps), per player from the slots and video_max_upload_gap_ms (lane app, INFO).
u12_reverse_pixel_identity (GC11): layer 167 = warm.png, layer 168 (Transparent) = a1080 reverse; 3 captures 0.4 s apart of
  the reversing clip (the lane app serves them from the GOP cache); for each, a forward speed-0 clip seeked to the same frame
  (in-point = code / 300, the prime retrigger) captured: PASS max |diff| over every channel == 0 for all three. Run once per
  upload path (VIDEO_ENV=ADNA_VIDEO_FORCE_FALLBACK=client / malloc; the default is the blit).
--- s-rta-1002b mkvidx (.harmony/.reports/s-rta-1002b/plan-mkvidx.md item 4 + ruling-mkvidx.md AM14) ---
u13_container_reverse: a Matroska file reverses like the same stream in MP4, and a QuickTime 1/600 HAP clip reverses at
  one decode per frame. Seven scenes, each a fresh load on the 1080p canvas (layers 169-172): (a) mkv_cuesfront_reverse
  (a1080_g250_cuesfront.mkv: Cues at the front -- main calls it intra-only and freezes), (b) mkv_reverse (a1080_g250.mkv:
  Cues at the end), (c) hap600_reverse (hap1080_tb600.mov) -- each u7's "reverse" scene (the shared helper rev_scene: 1 s,
  a 5 s window, the mid capture's bracket, the GC4 back-to-back captures); (d) mkv_long_column_1080x4 / (e)
  mp4_long_column_1080x4 -- four a1080_g250_60s.mkv / .mp4 players reversing by ONE trigger_column at the default budget,
  1 s, a 10 s window (u8's window helper column_window): counters only (the code band and expected() assume 300 frames);
  (f) mkv_long_pingpong_turn / (g) mp4_long_pingpong_turn -- one 60 s clip, PingPong, in-point 0.95 (the prime
  retrigger), 1 s, a 12 s window (the top turn inside it): counters + turn_seen, no captures. Every scene prints one DATA
  line (u7 / u8's keys + kf_lines = the "[VideoPlayer] Keyframe index:" lines its players wrote to err.log: item 2's
  once-per-player witness, INFO -- 0 on a pre-lane app). The rules ([CTRL-A] [FREEZE] [MKV] [HAP] [CPU] [PARITY] [GUARD]
  [PP-PARITY]) are .harmony/probe-vupload-ab.py's, on medians over >= 5 interleaved launches per arm; one run is INFO.
"""
import importlib.util, json, os, subprocess, sys, time

import numpy as np
from PIL import Image

ROOT, OUT = sys.argv[1], sys.argv[2]
MAKE = len(sys.argv) > 3 and sys.argv[3] == "--make-fixtures"
_rows_arg = (sys.argv[4] if len(sys.argv) > 4 else "") if MAKE else (sys.argv[3] if len(sys.argv) > 3 else "")
ONLY = set(_rows_arg.split(",")) if _rows_arg else None

# probe-video.py as a module (its argv: <root> <out> [--make-fixtures] <its rows>): helpers + fixture code + counters.
_pv_rows = "__none__"
sys.argv = [sys.argv[0], ROOT, OUT] + (["--make-fixtures"] if MAKE else []) + [_pv_rows]
_spec = importlib.util.spec_from_file_location("probe_video", os.path.join(ROOT, ".harmony", "probe-video.py"))
pv = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(pv)

T = "http://localhost:8080"   # the TestServer listens on "localhost" (may be ::1)
VU = json.load(open(os.path.join(ROOT, ".harmony", "probe-vupload.json")))
LV = VU["layers"]
ROW_FIXTURES = {
    "u2_gl_thread_qos": [], "u4a_context_cycle": ["a1080_g250.mp4"], "u4b_idle_ring_trim": ["a4k_g250.mp4"],
    "u6_crossfade_video": ["a1080_g250.mp4", "b1080_g250.mp4"], "u7_reverse_pingpong": ["g30_1080.mp4", "a1080_g250.mp4"],
    "u8_reverse_column_1080x4": ["a1080_g250.mp4"], "u9_reverse_cache_drop": ["a1080_g250.mp4"],
    "u10_reverse_4k": ["a4k_g250.mp4"], "u11_reverse_column_return": ["a1080_g250.mp4"],
    "u12_reverse_pixel_identity": ["a1080_g250.mp4"],
    "u13_container_reverse": ["a1080_g250.mkv", "a1080_g250_cuesfront.mkv", "a1080_g250_60s.mp4", "a1080_g250_60s.mkv",
                              "hap1080_tb600.mov"],
}
ONLY_SCENES = {x for x in os.environ.get("U7_SCENES", "").split(",") if x}   # restrict u7 to these scenes (re-runs)
ok, no, info, check = pv.ok, pv.no, pv.info, pv.check


def tstate():
    try:
        return pv.S.get(T + "/api/state", timeout=6).json()
    except Exception as e:  # noqa: BLE001
        no(f"8080 /api/state: {e}"); return None


def app_rss_mb():
    pids = [l.split()[0] for l in subprocess.run(["ps", "-eo", "pid=,ucomm="], capture_output=True, text=True).stdout.splitlines()
            if len(l.split()) >= 2 and l.split()[1] == "Audio-DNA"]
    if len(pids) != 1:
        return None
    r = subprocess.run(["ps", "-o", "rss=", "-p", pids[0]], capture_output=True, text=True).stdout.strip()
    return int(r) / 1024.0 if r else None


def late_hold(tag, s0, s1, what):
    """VU5 over a steady on-screen window."""
    lt = pv.delta(tag, s0, s1, "video_late_frames"); h = pv.delta(tag, s0, s1, "video_hold_no_texture")
    if lt is not None:
        check(lt == 0, f"{tag}: (VU5) video_late_frames delta over {what} {lt} == 0")
    if h is not None:
        check(h == 0, f"{tag}: (VU5) video_hold_no_texture delta over {what} {h} == 0")


def qos_info(tag, label):
    """INFO since VU15: the render thread's QoS raise (P2) was reverted -- printed, never judged."""
    q = pv.counter(tstate(), "gl_thread_qos")
    info(f"{tag}: {label} gl_thread_qos {q} -- QoS not applied (VU15) (21 = DEFAULT, 33 = USER_INTERACTIVE)")
    return q


# ---------------------------------------------------------------- rows
def u2(tag):
    q = qos_info(tag, "the render thread's")
    pv.data(tag, qos=q)


def four_layers(tag, lids, name, size, base):
    return pv.deck(0, [pv.layer(lids[i], [pv.vclip(base + i, name)]) for i in range(4)])


def u4a(tag):
    lids = LV[tag]; det = [int(x) for x in VU["cycleDetachedMs"]]; cycles = len(det)
    # layer 0 (bottom) holds frame 0 at speed 0 -- no newer frame will ever be at or before its clock, so after a context
    # loss only the HELD slot can give it a picture; layers 1-3 play (the top one's code is checked).
    layers = [pv.layer(lids[0], [pv.vclip(10, "a1080_g250.mp4", speed=0.0)])]
    layers += [pv.layer(lids[i], [pv.vclip(10 + i, "a1080_g250.mp4")]) for i in range(1, 4)]
    if not pv.load(tag, [pv.deck(0, layers)], "1080", 4):
        return
    pv.trigger_column(0)
    for i in range(4):
        pv.wait_active(i, 0)
    time.sleep(2.0)
    s0 = pv.state(); t0s = tstate(); g0 = pv.field(tag, t0s, "gl_context_gen")
    if g0 is None:
        return
    gens = []
    for c in range(cycles):
        gb = tstate()["gl_context_gen"]; tc = time.time()
        try:
            r = pv.S.post(T + "/api/debug/gl_context_cycle", json={"detached_ms": det[c]}, timeout=6)
            body = r.json()
        except Exception as e:  # noqa: BLE001
            no(f"{tag}: gl_context_cycle {c + 1}: {e}"); return
        if not body.get("ok"):
            no(f"{tag}: gl_context_cycle {c + 1} answered {r.status_code} {body}"); return
        adv = None
        while time.time() - tc < float(VU["cycleWaitS"]):
            g = tstate()["gl_context_gen"]
            if g > gb:
                adv = time.time() - tc; break
            time.sleep(0.02)
        gens.append(None if adv is None else round(adv, 2))
        time.sleep(0.5)
        pv.data(tag, cycle=c + 1, detached_ms=det[c], hold_no_texture_so_far=pv.dz(s0, pv.state(), "video_hold_no_texture"))
    time.sleep(1.0)
    s1 = pv.state(); pv.settle_late(); s2 = pv.state()
    f, pb, pa = pv.cap_bracket(tag, 3)
    time.sleep(2.0); s3 = pv.state(); t3s = tstate()
    gd = t3s["gl_context_gen"] - g0
    hnt = pv.delta(tag, s0, s3, "video_hold_no_texture"); pend = pv.delta(tag, s0, s3, "video_pending_frames")
    print(f"      {tag}: cycles {cycles} (detached {det} ms), gen advanced after {gens} s, gen delta {gd}, hold_no_texture {hnt}, pending {pend}, "
          f"videos_pending {pv.counter(s3, 'videos_pending')}, late across the cycles (INFO) {pv.dz(s0, s1, 'video_late_frames')}, "
          f"late after settle {pv.dz(s2, s3, 'video_late_frames')}, uploads {pv.dz(s0, s3, 'video_uploads')}, {pv.la()}", flush=True)
    pv.data(tag, gen_delta=gd, hold_no_texture=hnt, pending=pend, late_cycles=pv.dz(s0, s1, "video_late_frames"),
            late_steady=pv.dz(s2, s3, "video_late_frames"))
    check(gd == cycles, f"{tag}: (a) gl_context_gen delta {gd} == {cycles} (every cycle re-created the context)")
    if hnt is not None:
        check(hnt == 0, f"{tag}: (b) video_hold_no_texture delta {hnt} == 0 across {cycles} context cycles "
                        f"(every shown player always had its picture)")
    if pend is not None:
        check(pend == 0 and pv.counter(s3, "videos_pending") == 0,
              f"{tag}: (c) video_pending_frames delta {pend} == 0 and videos_pending {pv.counter(s3, 'videos_pending')} == 0")
    if f is not None:
        c = pv.code(f, pv.BAND["1080"]); good, br = pv.in_bracket(c, pb, pa)
        check(good, f"{tag}: (d) the top layer's code {c} within its playhead bracket {br}")
    lt = pv.delta(tag, s2, s3, "video_late_frames")
    if lt is not None:
        check(lt == 0, f"{tag}: (e) VU5 video_late_frames delta over the 2 s steady window after the cycles {lt} == 0")
    qos_info(tag, "(f) after the cycles:")


def u4b(tag):
    lids = LV[tag]; warm = os.path.join(OUT, "warm.png")
    decks = [four_layers(tag, lids, "a4k_g250.mp4", "4k", 20),
             pv.deck(1, [pv.layer(LV["u4b_away"], [pv.iclip(30, warm)])])]
    if not pv.load(tag, decks, "4k", 4):
        return
    pv.trigger_column(0)
    for i in range(4):
        pv.wait_active(i, 0)
    time.sleep(3.0)
    s0 = pv.state(); t0 = tstate(); r0 = app_rss_mb()
    pv.switch(1); time.sleep(float(VU["trimWaitS"]))
    s1 = pv.state(); t1 = tstate(); r1 = app_rss_mb()
    pv.switch(0); pv.settle_late(); sr = pv.state()
    time.sleep(float(VU["returnSettleS"])); pv.settle_late(); s2 = pv.state()   # 4 x 4K catch-ups end at different times
    f, pb, pa = pv.cap_bracket(tag, 3)
    time.sleep(5.0); s3 = pv.state()
    pur = pv.delta(tag, s0, s1, "video_slots_purged")
    gf0, gb0 = pv.counter(s0, "video_gopcache_frames"), pv.counter(s0, "video_gopcache_bytes")
    gf1, gb1 = pv.counter(s1, "video_gopcache_frames"), pv.counter(s1, "video_gopcache_bytes")
    fp0, fp1 = pv.counter(t0, "phys_footprint_mb"), pv.counter(t1, "phys_footprint_mb")
    dfp = None if fp0 is None or fp1 is None else round(fp0 - fp1, 1)
    drss = None if r0 is None or r1 is None else round(r0 - r1, 1)
    print(f"      {tag}: slots purged {pur}, phys_footprint {fp0} -> {fp1} MB (drop {dfp}), rss {r0 and round(r0)} -> "
          f"{r1 and round(r1)} MB (drop {drss}), late on return (INFO, w4's bounded hold) {pv.dz(s1, sr, 'video_late_frames')} "
          f"+ {pv.dz(sr, s2, 'video_late_frames')} in the next {VU['returnSettleS']} s, "
          f"{pv.la()}", flush=True)
    mb = lambda b: None if b is None else round(b / 1048576.0, 1)
    print(f"      {tag}: forward retention at s0 {gf0} frames / {mb(gb0)} MB (4 players), after the trim {gf1} frames / "
          f"{mb(gb1)} MB; the footprint drop above includes the retained frames' {mb(gb0)} MB", flush=True)
    pv.data(tag, purged=pur, footprint_drop=dfp, rss_drop=drss, late_return=pv.dz(s1, sr, "video_late_frames"),
            late_return_tail=pv.dz(sr, s2, "video_late_frames"), late_steady=pv.dz(s2, s3, "video_late_frames"),
            retained_frames=gf0, retained_mb=mb(gb0), retained_after_trim=gf1)
    rmax = 4 * int(VU["fwdRetainMaxFrames"])
    check(gf0 is not None and gf0 <= rmax, f"{tag}: (f) forward retention video_gopcache_frames {gf0} <= {rmax} "
                                           f"(4 x fwdRetainMaxFrames; {mb(gb0)} MB)")
    check(gf1 == 0 and gb1 == 0, f"{tag}: (g) after the idle trim video_gopcache_frames {gf1} / _bytes {gb1} == 0")
    if pur is not None:
        check(pur == 8, f"{tag}: (a) video_slots_purged delta {pur} == 8 (4 players x 2 free slots; the shown one is kept)")
    lim = float(VU["trimDropMinMb"])
    check((dfp is not None and dfp >= lim) or (drss is not None and drss >= lim),
          f"{tag}: (b) phys_footprint drop {dfp} MB or rss drop {drss} MB >= {lim:g}")
    pend = pv.delta(tag, s0, s3, "video_pending_frames"); hnt = pv.delta(tag, s0, s3, "video_hold_no_texture")
    if pend is not None and hnt is not None:
        check(pend == 0 and hnt == 0, f"{tag}: (c) after the return video_pending_frames +{pend} and video_hold_no_texture "
                                      f"+{hnt} == 0")
    if f is not None:
        c = pv.code(f, pv.BAND["4k"]); good, br = pv.in_bracket(c, pb, pa)
        check(good, f"{tag}: (c) the top layer's code {c} within its playhead bracket {br} (the un-purged slots carry a "
                    f"correct picture)")
    grow = pv.delta(tag, s2, s3, "video_slots_purged")
    if grow is not None:
        check(grow == 0, f"{tag}: (d) video_slots_purged delta {grow} == 0 while the deck is on screen (5 s)")
    late_hold(tag, s2, s3, "the 5 s on screen after the return")


def u6(tag):
    lids = LV[tag]; a, b = "a1080_g250.mp4", "b1080_g250.mp4"
    layers = [pv.layer(lids[i], [pv.vclip(40 + 2 * i, a), pv.vclip(41 + 2 * i, a)]) for i in range(3)]
    layers.append(pv.layer(lids[3], [pv.vclip(50, a), pv.vclip(51, b)], speed=2.0))
    if not pv.load(tag, [pv.deck(0, layers)], "1080", 8):
        return
    if not pv.wait_no_compiler(tag):
        return
    pv.trigger_column(0)
    for i in range(4):
        pv.wait_active(i, 0)
    time.sleep(2.0)
    s0 = pv.state(); pol = pv.Poller(); pol.start(); t0 = time.time()
    pv.trigger_column(1)
    time.sleep(max(0.0, 1.0 - (time.time() - t0)))
    pA0, pB0 = pv.playhead(3, 0), pv.playhead(3, 1)
    mid = pv.cap(f"{tag}_mid")
    pA1, pB1 = pv.playhead(3, 0), pv.playhead(3, 1)
    time.sleep(max(0.0, 2.5 - (time.time() - t0)))
    pol.stop(); s1 = pv.state(); pv.settle_late()
    f, pb, pa = pv.cap_bracket(tag, 3, 1)
    hnt = pv.delta(tag, s0, s1, "video_hold_no_texture"); lt = pv.delta(tag, s0, s1, "video_late_frames")
    pend = pv.delta(tag, s0, s1, "video_pending_frames")
    print(f"      {tag}: max callback {pol.max('peak_callback_ms')} ms, hold_no_texture {hnt}, late {lt}, pending {pend}, "
          f"uploads {pv.dz(s0, s1, 'video_uploads')}, deferred {pv.dz(s0, s1, 'video_uploads_deferred')}, {pv.la()}", flush=True)
    pv.data(tag, hold_no_texture=hnt, late=lt, pending=pend, deferred=pv.dz(s0, s1, "video_uploads_deferred"))
    if hnt is not None:
        check(hnt == 0, f"{tag}: (a) video_hold_no_texture delta {hnt} == 0 over the fade")
    if lt is not None:
        check(lt == 0, f"{tag}: (b) video_late_frames delta {lt} == 0 over the fade (both chains + the 3 new players)")
    if mid is not None and None not in (pA0, pA1, pB0, pB1):
        fa = pv.fixture_frame(a, pv.expected((pA0 + pA1) / 2)); fb = pv.fixture_frame(b, pv.expected((pB0 + pB1) / 2))
        if fa is not None and fb is not None:
            da, db, lu = pv.dbox(mid, fa), pv.dbox(mid, fb), pv.luma(mid)
            band = pv.BAND["1080"]; h, w = mid.shape[:2]
            y0, y1 = h - band + band // 4, h - band // 4
            cells = [float(mid[y0:y1, int(c * w / 10 + w / 40):int(c * w / 10 + 3 * w / 40), :3].mean()) for c in range(10)]
            mixed = [round(x) for x in cells if 48 <= x <= 208]
            check(lu > 20 and da > pv.TOL and db > pv.TOL and len(mixed) > 0,
                  f"{tag}: (c) capMid is a blend: luma {lu:.1f} > 20, dbox to A {da:.1f} and to B {db:.1f} both > {pv.TOL}, "
                  f"mixed code cells {mixed} (cells {[round(x) for x in cells]})")
    if f is not None:
        c = pv.code(f, pv.BAND["1080"]); good, br = pv.in_bracket(c, pb, pa, mask=511)
        check(bool(c & 512) and good, f"{tag}: (d) capAfter code {c} has bit 9 (B) and {c & 511} within B's bracket {br}")
    if pend is not None:
        check(pend == 0, f"{tag}: (e) video_pending_frames delta {pend} == 0")


# ---------------------------------------------------------------- s-rta-0929b gopcache (plan-gopcache.md 4.2 + adoption)
BUDGET = min(int(VU["gopcacheBudgetBytes"]), int(subprocess.run(["sysctl", "-n", "hw.memsize"], capture_output=True,
                                                                 text=True).stdout.strip() or 0) // 16 or int(VU["gopcacheBudgetBytes"]))
ENV_CAP_MB = None   # GC7: a launch with VIDEO_ENV=ADNA_GOPCACHE_BUDGET_MB=<n> (TEST-SERVER builds) runs u8 under that budget
for _kv in (os.environ.get("VIDEO_ENV") or "").split():
    if _kv.startswith("ADNA_GOPCACHE_BUDGET_MB="):
        ENV_CAP_MB = int(_kv.split("=", 1)[1])


class FastPoller:
    """/api/state (7070) every `interval` s in a thread: (t, video_uploads, video_player_uploads, fps, bytes). The upload
    gaps are measured from the polls: a gap = the time between two polls at which the count changed (+- one interval)."""

    def __init__(self, interval=0.015):
        self.rows = []; self._run = False; self._th = None; self.interval = interval

    def _loop(self):
        s = pv.requests.Session(); s.headers["Connection"] = "close"
        while self._run:
            try:
                d = s.get(pv.A + "/api/state", timeout=6).json()
                self.rows.append((time.time(), d.get("video_uploads"), d.get("video_player_uploads"), d.get("fps"),
                                  d.get("video_gopcache_bytes")))
            except Exception:  # noqa: BLE001
                pass
            time.sleep(self.interval)

    def start(self):
        self.rows = []; self._run = True
        self._th = pv.threading.Thread(target=self._loop, daemon=True); self._th.start(); return self

    def stop(self):
        self._run = False; self._th.join(); return self

    def changes(self, series, t0=None, t1=None):
        out = []; last = None
        for t, v in series:
            if v is None:
                continue
            if last is not None and v != last and (t0 is None or t >= t0) and (t1 is None or t <= t1):
                out.append(t)
            last = v
        return out

    def max_gap_ms(self, t0, t1, slot=None):
        """The longest interval in [t0, t1] with no upload (pooled, or of one player slot): t0 counts as a start."""
        ser = [(t, u if slot is None else (p[slot] if p else None)) for t, u, p, _f, _b in self.rows]
        ch = [t0] + self.changes(ser, t0, t1) + [t1]
        return round(1000.0 * max(b - a for a, b in zip(ch, ch[1:])), 1) if len(ch) > 1 else None

    def first_ms(self, t0, slot=None):
        ser = [(t, u if slot is None else (p[slot] if p else None)) for t, u, p, _f, _b in self.rows]
        ch = self.changes(ser, t0)
        return round(1000.0 * (ch[0] - t0), 1) if ch else None

    def count_at(self, t):
        best = None
        for tt, u, _p, _f, _b in self.rows:
            if tt <= t and u is not None:
                best = u
        return best

    def median(self, i):
        v = [r[i] for r in self.rows if r[i] is not None]
        return pv.statistics.median(v) if v else None

    def max(self, i):
        v = [r[i] for r in self.rows if r[i] is not None]
        return max(v) if v else None


class PlayheadSampler:
    """The layer's active clip playhead (/api/composition) every `interval` s in a thread: [(t, playhead)]."""

    def __init__(self, li, interval=0.2):
        self.li = li; self.rows = []; self._run = False; self._th = None; self.interval = interval

    def _loop(self):
        while self._run:
            p = pv.playhead(self.li)
            if p is not None:
                self.rows.append((time.time(), p))
            time.sleep(self.interval)

    def start(self):
        self.rows = []; self._run = True
        self._th = pv.threading.Thread(target=self._loop, daemon=True); self._th.start(); return self

    def stop(self):
        self._run = False; self._th.join(); return self

    def falling_after(self, t):
        """Every step after t falls (a reversing clock, no wrap inside): 1 / 0; None with < 3 samples."""
        v = [p for tt, p in self.rows if tt >= t]
        return None if len(v) < 3 else int(all(b < a for a, b in zip(v, v[1:])))

    def turn(self):
        """A top turn inside the window: the maximum is an inner sample, rising before it, falling after it."""
        v = [p for _t, p in self.rows]
        if len(v) < 5:
            return None
        i = max(range(len(v)), key=lambda k: v[k])
        return int(0 < i < len(v) - 1 and all(b > a for a, b in zip(v[:i], v[1:i + 1]))
                   and all(b < a for a, b in zip(v[i:], v[i + 1:])))


def rev_bracket(c, pb, pa, ct):
    """Direction-agnostic playhead bracket: c within [min(e(pb), e(pa)) - ct, max(...) + ct] (no wrap inside)."""
    if pb is None or pa is None or c is None:
        return False, "no playhead"
    lo, hi = sorted((pv.expected(pb), pv.expected(pa)))
    return lo - ct <= c <= hi + ct, f"[{lo}-{ct}, {hi}+{ct}]"


def active_slots(s0, s1):
    """video_player_uploads deltas of the slots that moved (the players of this scene), or None (an app without it)."""
    a, b = pv.counter(s0, "video_player_uploads"), pv.counter(s1, "video_player_uploads")
    if a is None or b is None:
        return None
    return {i: b[i] - a[i] for i in range(min(len(a), len(b))) if b[i] - a[i] > 0}


def mono_caps(tag, li, n):
    """GC4: n captures back to back of a reversing layer -- each code within its playhead bracket (ct 1) and the codes
    strictly decreasing. Returns (ok, codes, bracket_fails)."""
    codes = []; bad = []; nxt = time.time()
    for i in range(n):
        time.sleep(max(0.0, nxt - time.time())); nxt = time.time() + float(VU["u7MonoCapGapS"])   # >= 3 frames apart
        f, pb, pa = pv.cap_bracket(f"{tag}_mono{i}", li)
        if f is None:
            return False, codes, ["capture failed"]
        c = pv.code(f, pv.BAND["1080"]); codes.append(c)
        good, br = rev_bracket(c, pb, pa, int(VU["u7BracketCt"]))
        if not good:
            bad.append(f"{c} {br}")
    ok = len(codes) == n and all(b < a for a, b in zip(codes, codes[1:])) and not bad
    return ok, codes, bad


def flip_take(tag, lid, cid, at):
    """GC8: the forward -> reverse flip of a PLAYING Loop clip over REST -- a hand-built v3 take (no checkpoint decks, no
    audio) whose one clip-scope 'playing' point is the action 'reverse' at `at` s (MainComponent::applyClipPlaying,
    Origin::Replay); /api/perf/load + /api/perf/play fire it on the live clip."""
    d = os.path.join(OUT, f"flip_{tag}.adna-take"); os.makedirs(d, exist_ok=True)
    cp = {"activeDeckIndex": 0, "quantizeMode": 0, "bpm": 120.0, "audioAction": "", "decks": []}
    take = {"format": "audiodna-take", "version": 3, "minReader": 3, "features": ["lanes", "tempoMap", "checkpoint0"],
            "meta": {"recordedAt": "2026-09-30T00:00:00Z", "app": "0.1.0", "duration": at + 1.0, "durationBeats": 2.0 * (at + 1.0)},
            "tempoMap": [{"t": 0.0, "beat": 0.0, "sample": 0, "bpm": 120.0, "why": "start"}],
            "checkpoint0": cp, "checkpointEnd": cp, "markers": [],
            "lanes": [{"key": {"scope": "clip", "deck": {"i": 0, "rel": True, "name": ""},
                               "layer": {"i": 0, "id": lid, "name": f"L{lid}"}, "col": {"i": 0, "clip": f"v{cid}"},
                               "control": "playing"},
                       "kind": "discrete",
                       "points": [{"seq": 1, "t": at, "sample": int(at * 48000), "beat": 2.0 * at, "bpm": 120.0,
                                   "origin": "human", "action": "reverse", "v": 1}]}]}
    json.dump(take, open(os.path.join(d, "take.json"), "w"), indent=1)
    return d


def perf_status():
    try:
        return pv.S.get(pv.A + "/api/perf/status", timeout=6).json()
    except Exception as e:  # noqa: BLE001
        return {"error": str(e)}


U7_SCENES = [   # (scene, fixture, clip extras, kind, speed, in-point)
    ("g30_1080_reverse", "g30_1080.mp4", {"reverse": True, "loopMode": 0}, "reverse", 1.0, 0.0),
    ("g30_1080_pingpong", "g30_1080.mp4", {"loopMode": 1}, "forward", 1.0, 0.0),
    ("a1080_g250_reverse", "a1080_g250.mp4", {"reverse": True, "loopMode": 0}, "reverse", 1.0, 0.0),
    ("a1080_g250_pingpong", "a1080_g250.mp4", {"loopMode": 1}, "forward", 1.0, 0.0),
    ("g30_1080_pingpong_turn", "g30_1080.mp4", {"loopMode": 1}, "turn", 1.0, 0.7),
    ("a1080_g250_pingpong_turn", "a1080_g250.mp4", {"loopMode": 1}, "turn", 1.0, 0.7),
    ("a1080_g250_reverse_speed2", "a1080_g250.mp4", {"reverse": True, "loopMode": 0}, "reverse", 2.0, 0.0),
    ("g30_1080_flip_reverse", "g30_1080.mp4", {"loopMode": 0}, "flip", 1.0, 0.6),
    ("a1080_g250_flip_reverse", "a1080_g250.mp4", {"loopMode": 0}, "flip", 1.0, 0.6),
]


def witness_lines():
    """mkvidx: the "[VideoPlayer] Keyframe index:" lines in this launch's err.log so far (the decode thread's once-per-player
    witness that a keyframe model grew past its open-time index; INFO -- a pre-lane app never writes it)."""
    try:
        with open(os.path.join(OUT, "err.log"), errors="replace") as f:
            return sum("Keyframe index: " in ln for ln in f)
    except OSError:
        return None


def rev_scene(tag, lid, cid, sc, name, extra, kind, speed, ip, window=5.0, captures=True, witness=False):
    """One u7-style scene on ONE layer: a fresh load of `name`, trig (+ the prime retrigger for an in-point), 1 s, a
    `window` s window (the mid capture's bracket at 2.5 s and, for a non-forward scene, the GC4 back-to-back captures after
    it -- both only with `captures`), one INFO line, the VU16 hold check, one DATA line. Shared by u7 and u13 (mkvidx).
    Returns "stop" when the compiler wait gave up (the caller ends its row), else None."""
    ct = int(VU["u7BracketCt"]); ncap = int(VU["u7MonoCaps"])
    clip = pv.vclip(cid, name, ip=ip, **extra); clip["speed"] = speed
    w0 = witness_lines() if witness else None
    if not pv.load(f"{tag}_{sc}", [pv.deck(0, [pv.layer(lid, [clip])])], "1080", 1):
        return None
    if not pv.wait_no_compiler(tag):
        return "stop"
    take = None
    if kind == "flip":
        take = flip_take(sc, lid, cid, 1.0)
        pv.S.post(pv.A + "/api/perf/load", json={"folder": take}, timeout=6)
    pv.trig(0, 0); pv.wait_active(0, 0)
    if ip > 0.0:
        pv.trig(0, 0)   # prime: a first trigger plays from 0; the retrigger seeks to the in-point
    time.sleep(1.0)
    s0 = pv.state(); pol = FastPoller().start(); phs = PlayheadSampler(0).start(); t0 = time.time(); tflip = None
    if kind == "flip":
        pv.S.post(pv.A + "/api/perf/play", json={"withAudio": False}, timeout=6); tflip = time.time() + 1.0
    f = None
    if captures:
        time.sleep(max(0.0, 2.5 - (time.time() - t0)))
        f, pb, pa = pv.cap_bracket(f"{tag}_{sc}_mid", 0)
    time.sleep(max(0.0, window - (time.time() - t0)))
    t1 = time.time(); pol.stop(); phs.stop(); s1 = pv.state()
    upl = pv.dz(s0, s1, "video_uploads"); lt = pv.dz(s0, s1, "video_late_frames")
    dec = pv.dz(s0, s1, "video_frames_decoded"); sk = pv.dz(s0, s1, "video_seeks")
    kv = dict(scene=sc, uploads_per_s=None if upl is None else round(upl / window, 2), late=lt,
              fps=pol.median(3), hold_no_texture=pv.dz(s0, s1, "video_hold_no_texture"), decoded=dec,
              dropped=pv.dz(s0, s1, "video_frames_dropped"), seeks=sk, skipped=pv.dz(s0, s1, "video_frames_skipped"),
              decoded_per_upload=None if not upl or dec is None else round(dec / upl, 2),
              seeks_per_upload=None if not upl or sk is None else round(sk / upl, 3),
              max_gap_ms=pol.max_gap_ms(t0, t1), gap_counter_ms=pv.counter(s1, "video_max_upload_gap_ms"),
              nonmono=pv.dz(s0, s1, "video_reverse_nonmonotonic"), dirchg=pv.dz(s0, s1, "video_direction_changes"),
              hits=pv.dz(s0, s1, "video_gopcache_hits"), misses=pv.dz(s0, s1, "video_gopcache_misses"),
              runs=pv.dz(s0, s1, "video_gopcache_runs"), cache_mb=None if pv.counter(s1, "video_gopcache_bytes") is None
              else round(s1["video_gopcache_bytes"] / 1048576.0, 1))
    if f is not None:
        c = pv.code(f, pv.BAND["1080"])
        if kind == "forward":
            good, br = pv.in_bracket(c, pb, pa)
        else:
            good, br = rev_bracket(c, pb, pa, ct)
        kv.update(code=c, bracket_ok=int(good))
        info(f"{tag}[{sc}]: (d) the mid-window capture's code {c} within its playhead bracket {br}: {good}")
    if kind == "turn":
        kv["turn_seen"] = phs.turn()
    elif kind == "reverse":
        kv["reversed"] = phs.falling_after(t0 + 0.2)
    if kind == "flip":
        st = perf_status()
        n_after = pol.count_at(t1); n_flip = pol.count_at(tflip)
        kv.update(post_flip_uploads_per_s=None if n_after is None or n_flip is None else round((n_after - n_flip) / (t1 - tflip), 2),
                  flip_gap_ms=pol.max_gap_ms(tflip - 0.1, min(t1, tflip + 1.5)),
                  reversed=phs.falling_after(tflip + 0.3))
        info(f"{tag}[{sc}]: perf status after the flip take: {json.dumps(st)[:300]}")
        pv.S.post(pv.A + "/api/perf/stop_play", timeout=6)   # the replay stays 'playing' past its last point
    if kind != "forward" and captures:
        ok, codes, bad = mono_caps(f"{tag}_{sc}", 0, ncap)
        kv.update(mono_ok=int(ok))
        info(f"{tag}[{sc}]: (GC4) {ncap} back-to-back captures, codes {codes} strictly decreasing and each within its "
             f"bracket (ct {ct}): {ok}{' -- outside: ' + '; '.join(bad[:4]) if bad else ''}")
    if witness:
        w1 = witness_lines()
        kv["kf_lines"] = None if w0 is None or w1 is None else w1 - w0
    print(f"      {tag}[{sc}]: INFO " + ", ".join(f"{a} {b}" for a, b in kv.items() if a != "scene") + f", {pv.la()}",
          flush=True)
    h = pv.delta(f"{tag}[{sc}]", s0, s1, "video_hold_no_texture")
    if h is not None:   # VU16 (VU5): asserted per scene, not only printed
        check(h == 0, f"{tag}[{sc}]: (VU5) video_hold_no_texture delta over the {window:g} s window {h} == 0")
    pv.data(tag, **kv)
    return None


def u7(tag):
    """Every scene prints one DATA line; the absolute bars are judged by probe-vupload-ab.py on the MEDIAN of >= 5
    interleaved launches (GC13) -- here they are INFO, except VU16 (hold_no_texture per scene, asserted)."""
    lid = LV[tag]; k = 0
    for sc, name, extra, kind, speed, ip in U7_SCENES:
        k += 1; cid = 60 + k
        if ONLY_SCENES and sc not in ONLY_SCENES:
            continue
        if rev_scene(tag, lid, cid, sc, name, extra, kind, speed, ip) == "stop":
            return


def column_rev(tag, lids, name, size, base, extra=None):
    return pv.deck(0, [pv.layer(lids[i], [pv.vclip(base + i, name, reverse=True, loopMode=0, **(extra or {}))])
                       for i in range(len(lids))])


def column_window(window=5.0):
    """A `window` s window over a triggered column (u8; u13's column scenes): pooled and per-player uploads/s, late, hold,
    pending, the cache bytes (max over 50 ms polls), decodes per upload, fps, phys_footprint delta. Returns (kv, s0, s1,
    pol, t0, t1)."""
    s0 = pv.state(); t0s = tstate(); pol = FastPoller(0.05).start(); t0 = time.time(); time.sleep(window); pol.stop()
    t1 = time.time(); s1 = pv.state(); t1s = tstate()
    upl = pv.dz(s0, s1, "video_uploads"); sl = active_slots(s0, s1)
    per = None if sl is None else sorted(round(v / window, 2) for v in sl.values())
    dec = pv.dz(s0, s1, "video_frames_decoded")
    fp0, fp1 = pv.counter(t0s, "phys_footprint_mb"), pv.counter(t1s, "phys_footprint_mb")
    kv = dict(cap_mb=ENV_CAP_MB or 0, uploads_per_s=None if upl is None else round(upl / window, 2),
              per_player_min=None if not per else (per[0] if len(per) >= 4 else 0.0), per_player=None if per is None else ",".join(map(str, per)),
              late=pv.dz(s0, s1, "video_late_frames"), hold_no_texture=pv.dz(s0, s1, "video_hold_no_texture"),
              pending=pv.dz(s0, s1, "video_pending_frames"),
              bytes_mb=None if pol.max(4) is None else round(pol.max(4) / 1048576.0, 1),
              frames=pv.counter(s1, "video_gopcache_frames"), active=pv.counter(s1, "video_gopcache_active"),
              over_budget=pv.dz(s0, s1, "video_gopcache_over_budget"),
              cap_bytes_mb=None if pv.counter(s1, "video_gopcache_cap_bytes") is None else round(s1["video_gopcache_cap_bytes"] / 1048576.0, 1),
              decoded_per_upload=None if not upl or dec is None else round(dec / upl, 2),
              fps=pol.median(3), footprint_delta_mb=None if fp0 is None or fp1 is None else round(fp1 - fp0, 1),
              evictions=pv.dz(s0, s1, "video_gopcache_evictions"))
    return kv, s0, s1, pol, t0, t1


def u8(tag):
    """Four 1080p GOP-250 reversers by ONE trigger_column: pooled and per-player uploads/s, late, the cache bytes against the
    budget (GC6 / GC7). DATA only here; the bars are the ab summary's (median of >= 5 launches, GC13)."""
    lids = LV[tag]
    if not pv.load(tag, [column_rev(tag, lids, "a1080_g250.mp4", "1080", 70)], "1080", 4):
        return
    if not pv.wait_no_compiler(tag):
        return
    pv.trigger_column(0)
    for i in range(4):
        pv.wait_active(i, 0)
    time.sleep(1.0)
    kv = column_window(5.0)[0]
    print(f"      {tag}: INFO " + ", ".join(f"{a} {b}" for a, b in kv.items()) + f", budget {BUDGET} B, {pv.la()}", flush=True)
    pv.data(tag, **kv)


def u9(tag):
    """The idle trim clears a reversing player's cache (R-10) and the memory really returns (GC10)."""
    lid = LV[tag]; warm = os.path.join(OUT, "warm.png")
    decks = [pv.deck(0, [pv.layer(lid, [pv.vclip(80, "a1080_g250.mp4", reverse=True, loopMode=0)])]),
             pv.deck(1, [pv.layer(LV["u9_away"], [pv.iclip(81, warm)])])]
    if not pv.load(tag, decks, "1080", 1):
        return
    if not pv.wait_no_compiler(tag):
        return
    pv.trig(0, 0); pv.wait_active(0, 0); time.sleep(3.0)
    s0 = pv.state(); t0s = tstate()
    pv.switch(1); time.sleep(float(VU["trimWaitS"]))
    s1 = pv.state(); t1s = tstate()
    pv.switch(0); pv.settle_late()
    f, pb, pa = pv.cap_bracket(tag, 0)
    s2 = pv.state(); time.sleep(3.0); s3 = pv.state()
    b0, b1 = pv.counter(s0, "video_gopcache_bytes"), pv.counter(s1, "video_gopcache_bytes")
    fp0, fp1 = pv.counter(t0s, "phys_footprint_mb"), pv.counter(t1s, "phys_footprint_mb")
    kv = dict(bytes0_mb=None if b0 is None else round(b0 / 1048576.0, 1), frames0=pv.counter(s0, "video_gopcache_frames"),
              bytes1=b1, frames1=pv.counter(s1, "video_gopcache_frames"), active1=pv.counter(s1, "video_gopcache_active"),
              footprint_drop_mb=None if fp0 is None or fp1 is None else round(fp0 - fp1, 1),
              late=pv.dz(s2, s3, "video_late_frames"), hold_no_texture=pv.dz(s0, s3, "video_hold_no_texture"))
    if kv["footprint_drop_mb"] is not None and b0:
        kv["footprint_drop_frac"] = round(kv["footprint_drop_mb"] / (b0 / 1048576.0), 3)
    if f is not None:
        c = pv.code(f, pv.BAND["1080"]); good, br = rev_bracket(c, pb, pa, int(VU["u7BracketCt"]))
        kv.update(code=c, bracket_ok=int(good))
    print(f"      {tag}: INFO " + ", ".join(f"{a} {b}" for a, b in kv.items()) + f", {pv.la()}", flush=True)
    pv.data(tag, **kv)


def u10(tag):
    """INFO: one 4K GOP-250 reverser."""
    lid = LV[tag]
    if not pv.load(tag, [pv.deck(0, [pv.layer(lid, [pv.vclip(90, "a4k_g250.mp4", reverse=True, loopMode=0)])])], "4k", 1):
        return
    if not pv.wait_no_compiler(tag):
        return
    pv.trig(0, 0); pv.wait_active(0, 0); time.sleep(1.0)
    s0 = pv.state(); pol = FastPoller(0.05).start(); time.sleep(5.0); pol.stop(); s1 = pv.state()
    upl = pv.dz(s0, s1, "video_uploads"); dec = pv.dz(s0, s1, "video_frames_decoded")
    kv = dict(uploads_per_s=None if upl is None else round(upl / 5.0, 2), late=pv.dz(s0, s1, "video_late_frames"),
              bytes_mb=None if pv.counter(s1, "video_gopcache_bytes") is None else round(s1["video_gopcache_bytes"] / 1048576.0, 1),
              frames=pv.counter(s1, "video_gopcache_frames"), decoded_per_upload=None if not upl or dec is None else round(dec / upl, 2),
              fps=pol.median(3), hold_no_texture=pv.dz(s0, s1, "video_hold_no_texture"))
    print(f"      {tag}: INFO " + ", ".join(f"{a} {b}" for a, b in kv.items()) + f", {pv.la()}", flush=True)
    pv.data(tag, **kv)


def u11(tag):
    """GC9: the switch back to a deck with a column of four 1080p GOP-250 reversers (their caches were trimmed off screen):
    the column's longest upload gap after the return (pooled polls, both apps), the time to its first upload; per player
    (video_player_uploads slots, lane app) and video_max_upload_gap_ms (INFO)."""
    lids = LV[tag]; warm = os.path.join(OUT, "warm.png")
    decks = [column_rev(tag, lids, "a1080_g250.mp4", "1080", 100), pv.deck(1, [pv.layer(LV["u11_away"], [pv.iclip(105, warm)])])]
    if not pv.load(tag, decks, "1080", 4):
        return
    if not pv.wait_no_compiler(tag):
        return
    pv.trigger_column(0)
    for i in range(4):
        pv.wait_active(i, 0)
    time.sleep(3.0)
    pv.switch(1); time.sleep(float(VU["trimWaitS"]))
    s0 = pv.state(); pol = FastPoller(0.01).start(); time.sleep(0.1); tr = time.time(); pv.switch(0)
    time.sleep(2.0); pol.stop(); s1 = pv.state()
    sl = active_slots(s0, s1)
    kv = dict(pooled_gap_ms=pol.max_gap_ms(tr, tr + 2.0), first_upload_ms=pol.first_ms(tr),
              gap_counter_ms=pv.counter(s1, "video_max_upload_gap_ms"), late=pv.dz(s0, s1, "video_late_frames"),
              hold_no_texture=pv.dz(s0, s1, "video_hold_no_texture"))
    if sl:
        firsts = [pol.first_ms(tr, slot=i) for i in sorted(sl)]; gaps = [pol.max_gap_ms(tr, tr + 2.0, slot=i) for i in sorted(sl)]
        kv.update(player_first_max_ms=max(x for x in firsts if x is not None) if any(x is not None for x in firsts) else None,
                  player_gap_max_ms=max(x for x in gaps if x is not None) if any(x is not None for x in gaps) else None)
    print(f"      {tag}: INFO " + ", ".join(f"{a} {b}" for a, b in kv.items()) + f", {pv.la()}", flush=True)
    pv.data(tag, **kv)


def u12(tag):
    """GC11: reverse identity on the REAL upload path of this launch (blit / client / malloc by VIDEO_ENV): a frame shown by a
    reversing 1080p GOP-250 clip (served from the GOP cache on the lane app) captured, then the SAME frame index shown by a
    forward speed-0 clip seeked to it: max |diff| over every channel == 0."""
    lids = LV[tag]; warm = os.path.join(OUT, "warm.png"); band = pv.BAND["1080"]
    lay_lo = pv.layer(lids[0], [pv.iclip(110, warm)])
    lay_hi = pv.layer(lids[1], [pv.vclip(111, "a1080_g250.mp4", reverse=True, loopMode=0)], type=1)
    if not pv.load(f"{tag}_rev", [pv.deck(0, [lay_lo, lay_hi])], "1080", 1):
        return
    pv.trig(0, 0); pv.wait_active(0, 0); pv.trig(1, 0); pv.wait_active(1, 0); time.sleep(2.0)
    s0 = pv.state(); revs = []
    for i in range(3):
        f = pv.cap(f"{tag}_rev{i}")
        if f is not None:
            revs.append((pv.code(f, band), os.path.join(OUT, f"{tag}_rev{i}.png")))
        time.sleep(0.4)
    s1 = pv.state()
    info(f"{tag}: reverse captures {[c for c, _ in revs]}, cache hits over them {pv.dz(s0, s1, 'video_gopcache_hits')}")
    worst = None
    for j, (c, png) in enumerate(revs):
        lay_hi = pv.layer(lids[1], [pv.vclip(120 + j, "a1080_g250.mp4", ip=c / 300.0, speed=0.0)], type=1)
        if not pv.load(f"{tag}_fwd{j}", [pv.deck(0, [lay_lo, lay_hi])], "1080", 1):
            continue
        pv.trig(0, 0); pv.wait_active(0, 0); pv.trig(1, 0); pv.wait_active(1, 0); pv.trig(1, 0); time.sleep(0.5)
        pv.wait_videos_pending_zero(); pv.settle_late(); time.sleep(0.3)
        g = pv.cap(f"{tag}_fwd{j}")
        if g is None:
            continue
        cf = pv.code(g, band)
        a = np.asarray(Image.open(png).convert("RGBA")).astype(np.int16)
        b = np.asarray(Image.open(os.path.join(OUT, f"{tag}_fwd{j}.png")).convert("RGBA")).astype(np.int16)
        md = int(np.abs(a - b).max()) if a.shape == b.shape else 999
        worst = md if worst is None else max(worst, md)
        info(f"{tag}: frame {c}: reverse capture vs forward speed-0 capture (code {cf}): max |diff| {md}")
        pv.data(tag, frame=c, fwd_code=cf, max_diff=md)
    if worst is not None and len(revs) == 3:
        check(worst == 0 and all(c > 0 for c, _ in revs), f"{tag}: (GC11) reverse frames {[c for c, _ in revs]} equal their forward "
                                                          f"captures exactly: worst max |diff| {worst} == 0")
    else:
        no(f"{tag}: (GC11) fewer than 3 reverse / forward capture pairs")


U13_SCENES = [   # (scene, fixture, kind) -- mkvidx plan item 4 (a)-(g)
    ("mkv_cuesfront_reverse", "a1080_g250_cuesfront.mkv", "reverse"),
    ("mkv_reverse", "a1080_g250.mkv", "reverse"),
    ("hap600_reverse", "hap1080_tb600.mov", "reverse"),
    ("mkv_long_column_1080x4", "a1080_g250_60s.mkv", "column"),
    ("mp4_long_column_1080x4", "a1080_g250_60s.mp4", "column"),
    ("mkv_long_pingpong_turn", "a1080_g250_60s.mkv", "turn"),
    ("mp4_long_pingpong_turn", "a1080_g250_60s.mp4", "turn"),
]


def u13(tag):
    """s-rta-1002b mkvidx: Matroska vs MP4 reverse / ping-pong and a 1/600 HAP reverse (the docstring). DATA per scene;
    the rules are probe-vupload-ab.py's (median of >= 5 interleaved launches per arm)."""
    lids = LV[tag]; k = 0
    for sc, name, kind in U13_SCENES:
        k += 1; cid = 200 + 10 * k   # clip ids 210-283 (never a layer id)
        if kind == "reverse":
            r = rev_scene(tag, lids[0], cid, sc, name, {"reverse": True, "loopMode": 0}, "reverse", 1.0, 0.0, witness=True)
        elif kind == "turn":
            r = rev_scene(tag, lids[0], cid, sc, name, {"loopMode": 1}, "turn", 1.0, 0.95, window=12.0, captures=False,
                          witness=True)
        else:
            w0 = witness_lines()
            if not pv.load(f"{tag}_{sc}", [column_rev(tag, lids, name, "1080", cid)], "1080", 4):
                continue
            if not pv.wait_no_compiler(tag):
                return
            pv.trigger_column(0)
            for i in range(4):
                pv.wait_active(i, 0)
            time.sleep(1.0)
            kv, s0, s1, pol, t0, t1 = column_window(10.0)
            w1 = witness_lines()
            kv = dict(scene=sc, **kv, max_gap_ms=pol.max_gap_ms(t0, t1), seeks=pv.dz(s0, s1, "video_seeks"),
                      nonmono=pv.dz(s0, s1, "video_reverse_nonmonotonic"), dirchg=pv.dz(s0, s1, "video_direction_changes"),
                      kf_lines=None if w0 is None or w1 is None else w1 - w0)
            print(f"      {tag}[{sc}]: INFO " + ", ".join(f"{a} {b}" for a, b in kv.items() if a != "scene")
                  + f", budget {BUDGET} B, {pv.la()}", flush=True)
            h = pv.delta(f"{tag}[{sc}]", s0, s1, "video_hold_no_texture")
            if h is not None:   # VU5, as every reverse scene
                check(h == 0, f"{tag}[{sc}]: (VU5) video_hold_no_texture delta over the 10 s window {h} == 0")
            pv.data(tag, **kv)
            r = None
        if r == "stop":
            return


def make_fixtures():
    need = set()
    for r, fs in ROW_FIXTURES.items():
        if ONLY is None or r in ONLY:
            need.update(fs)
    pv.ROW_FIXTURES.clear(); pv.ROW_FIXTURES["__vupload__"] = sorted(need)
    pv.ONLY = {"__vupload__"}
    pv.make_fixtures()   # exits 0 / 1


def main():
    if MAKE:
        make_fixtures(); return
    rows = [("u2_gl_thread_qos", lambda: u2("u2_gl_thread_qos")),
            ("u4a_context_cycle", lambda: u4a("u4a_context_cycle")),
            ("u4b_idle_ring_trim", lambda: u4b("u4b_idle_ring_trim")),
            ("u6_crossfade_video", lambda: u6("u6_crossfade_video")),
            ("u7_reverse_pingpong", lambda: u7("u7_reverse_pingpong")),
            ("u8_reverse_column_1080x4", lambda: u8("u8_reverse_column_1080x4")),
            ("u9_reverse_cache_drop", lambda: u9("u9_reverse_cache_drop")),
            ("u10_reverse_4k", lambda: u10("u10_reverse_4k")),
            ("u11_reverse_column_return", lambda: u11("u11_reverse_column_return")),
            ("u12_reverse_pixel_identity", lambda: u12("u12_reverse_pixel_identity")),
            ("u13_container_reverse", lambda: u13("u13_container_reverse"))]
    for name, fn in rows:
        if ONLY is None or name in ONLY:
            print(f"--- {name}", flush=True)
            fn()
    print(f"\nPY {pv.PASS} PASS / {pv.FAIL} FAIL", flush=True)
    sys.exit(0 if pv.FAIL == 0 else 1)


if __name__ == "__main__":
    main()
