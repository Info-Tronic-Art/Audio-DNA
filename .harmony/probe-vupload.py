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

u2_gl_thread_qos: 8080 /api/state gl_thread_qos == 33 (QOS_CLASS_USER_INTERACTIVE; the render thread samples
  qos_class_self() every frame). RED on the pre-lane app: absent; on an app before plan item u2 (P2): 21 (DEFAULT).
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
  printed as INFO); (f) VU11: gl_thread_qos == 33 after the cycles (the QoS is re-applied by every new context).
u4b_idle_ring_trim: 4K canvas, deck 0 = 4 x a4k, deck 1 = warm.png; trigger_column; 3 s; s0 (8080 phys_footprint_mb,
  `ps -o rss=` of the app, video_slots_purged); switch_deck 1; trimWaitS; s1; switch_deck 0; settle_late; s2; cap; a 5 s
  window; s3. PASS (a) video_slots_purged delta s0 -> s1 == 8 (4 players x the 2 slots that are not the shown one); (b)
  phys_footprint OR rss dropped by >= trimDropMinMb (4 x 2 x 33.2 MB = 265 expected; both printed); (c) after the return
  video_pending_frames +0, video_hold_no_texture +0 (s0 -> s3) and the top layer's code within its bracket (the un-purged
  slots carry a correct picture); (d) video_slots_purged does not grow while the deck is on screen (s2 -> s3 == 0);
  (e) VU5: video_late_frames delta over the on-screen windows (the 3 s before the switch is not polled; s2 -> s3) == 0 --
  the return itself is w4's bounded hold (1 <= late <= lateMaxReturn in probe-video w4), printed as INFO here.
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
  the pre-lane app's, and its median late frames per scene <= the pre-lane app's. In a single run: INFO.
"""
import importlib.util, json, os, subprocess, sys, time

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
}
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


def qos_check(tag, label):
    st = tstate(); q = pv.field(tag, st, "gl_thread_qos")
    if q is not None:
        check(q == 33, f"{tag}: {label} gl_thread_qos {q} == 33 (QOS_CLASS_USER_INTERACTIVE; 21 = DEFAULT)")
    return q


# ---------------------------------------------------------------- rows
def u2(tag):
    q = qos_check(tag, "the render thread's")
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
    qos_check(tag, "(f) VU11 after the cycles:")


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
    pv.switch(0); pv.settle_late(); s2 = pv.state()
    f, pb, pa = pv.cap_bracket(tag, 3)
    time.sleep(5.0); s3 = pv.state()
    pur = pv.delta(tag, s0, s1, "video_slots_purged")
    fp0, fp1 = pv.counter(t0, "phys_footprint_mb"), pv.counter(t1, "phys_footprint_mb")
    dfp = None if fp0 is None or fp1 is None else round(fp0 - fp1, 1)
    drss = None if r0 is None or r1 is None else round(r0 - r1, 1)
    print(f"      {tag}: slots purged {pur}, phys_footprint {fp0} -> {fp1} MB (drop {dfp}), rss {r0 and round(r0)} -> "
          f"{r1 and round(r1)} MB (drop {drss}), late on return (INFO, w4's bounded hold) {pv.dz(s1, s2, 'video_late_frames')}, "
          f"{pv.la()}", flush=True)
    pv.data(tag, purged=pur, footprint_drop=dfp, rss_drop=drss, late_return=pv.dz(s1, s2, "video_late_frames"))
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


def u7(tag):
    lid = LV[tag]; k = 0
    for name in ("g30_1080.mp4", "a1080_g250.mp4"):
        for mode, extra in (("reverse", {"reverse": True, "loopMode": 0}), ("pingpong", {"loopMode": 1})):
            k += 1; sc = f"{os.path.splitext(name)[0]}_{mode}"
            if not pv.load(f"{tag}_{sc}", [pv.deck(0, [pv.layer(lid, [pv.vclip(60 + k, name, **extra)])])], "1080", 1):
                continue
            if not pv.wait_no_compiler(tag):
                return
            pv.trig(0, 0); pv.wait_active(0, 0); time.sleep(1.0)
            s0 = pv.state(); pol = pv.Poller(pv.FPS_POLL).window(5.0); s1 = pv.state()
            upl = pv.dz(s0, s1, "video_uploads"); lt = pv.dz(s0, s1, "video_late_frames")
            print(f"      {tag}[{sc}]: INFO uploads/s {None if upl is None else round(upl / 5.0, 1)}, late {lt}, median fps "
                  f"{pol.median('fps')}, skipped {pv.dz(s0, s1, 'video_frames_skipped')}, seeks {pv.dz(s0, s1, 'video_seeks')}, "
                  f"{pv.la()}", flush=True)
            pv.data(tag, scene=sc, uploads_per_s=None if upl is None else round(upl / 5.0, 2), late=lt,
                    fps=pol.median("fps"), hold_no_texture=pv.dz(s0, s1, "video_hold_no_texture"))


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
            ("u7_reverse_pingpong", lambda: u7("u7_reverse_pingpong"))]
    for name, fn in rows:
        if ONLY is None or name in ONLY:
            print(f"--- {name}", flush=True)
            fn()
    print(f"\nPY {pv.PASS} PASS / {pv.FAIL} FAIL", flush=True)
    sys.exit(0 if pv.FAIL == 0 else 1)


if __name__ == "__main__":
    main()
