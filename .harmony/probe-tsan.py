#!/usr/bin/env python3
"""probe-tsan.py <a|b|c|d|e> <rundir> -- the REST half of ONE ThreadSanitizer app-sweep launch (lane tsan, s-rta-1002).

Driven by .harmony/probe-tsan.sh (which launches the TSan app, holds the rig gate and quits it); adapted from the
s-rta-0929b / s-rta-0930 harness (.harmony/.reports/s-rta-0930/tsan-harness/scen.py). Production port 7070, a fresh
connection per request (Connection: close). Never opens an Output window, never sends synthetic input, never loads a
composition while a scenario is stepping. Fixture media: $TSAN_MEDIA (made by probe-tsan.sh's mkfix).

Scenarios (plan-tsan.md T8 as amended by ruling-tsan.md amendment 12):
  a  idle 15 s (health polls only).
  b  load 16 x 1080p + 4 x 4K + 4 images (2 decks), trigger_column 0, hold 20 s, GET /api/state.
  c  load the mixed 2-deck composition, 17-18 REST calls 0.8 s apart (trigger_clip / trigger_column / switch_deck /
     audio/source / load_source / set_layer_opacity), hold to 15 s, GET /api/state.
  d  fades + storms (amendment 12): 2 decks x 3 layers x 4 columns (v1080 videos + images), every layer
     transitionSpeed 0.5, deck 0 layer 1 autopilotEnabled (PlayNext, Beat1 = the shortest duration), one clip
     beatSnapMode Beat (deck 0 layer 0 column 2), one Bar (deck 1 layer 2 column 1), one empty cell per deck (layer 2
     column 3); everything set at LOAD time. POST /api/set_bpm {"bpm": 240} (Manual BPM: beats without audio). Then 48
     steps 0.25 s apart (~12 s): every step set_layer_opacity (rotating layers) + one trigger (trigger_column /
     trigger_clip of a new cell / the same cell again = the active cell / the empty cell = clear / two more layers),
     set_master_signal every other step, switch_deck every 8th step (~2 s). Reads while stepping: /api/health only.
     ONE GET /api/state at the end (state.json: render_pending_fired / render_autopilot_advances /
     render_tuple_adopts on the lane). FORBIDDEN in d: GET /api/composition, perf/*, any load while stepping (R5 / R7).
  e  INFO (the "tsan-r5" baseline): the d fixture, set_bpm 240, perf/record -> 3 triggers -> perf/stop -> perf/load ->
     perf/play (as probe-routines.sh drives them) -> hold 4 s -> perf/stop_play, GET /api/state; the take folder this
     run created under ~/Documents/Audio-DNA/Takes is removed afterwards.
"""
import json, os, shutil, sys, time
import requests

A = "http://127.0.0.1:7070"
M = os.environ.get("TSAN_MEDIA") or os.environ.get("SWEEP_MEDIA") or ""
SC, OUT = sys.argv[1], sys.argv[2]
LOG = open(os.path.join(OUT, "scen.log"), "a")


def log(m):
    LOG.write("%s %s\n" % (time.strftime("%H:%M:%S"), m))
    LOG.flush()


def get(p, t=8):
    try:
        r = requests.get(A + p, timeout=t, headers={"Connection": "close"})
        return r.status_code, r.text
    except Exception as e:
        return None, str(e)


def post(p, body, t=8):
    try:
        r = requests.post(A + p, json=body, timeout=t, headers={"Connection": "close"})
        return r.status_code, r.text
    except Exception as e:
        return None, str(e)


def iclip(cid, path, **kw):
    c = {"name": f"c{cid}", "id": cid, "mediaType": 1, "mediaFile": path, "effects": []}
    c.update(kw)
    return c


def vclip(cid, path, **kw):
    c = {"name": f"v{cid}", "id": cid, "mediaType": 2, "mediaFile": path, "transportMode": 0, "loopMode": 0,
         "speed": 1.0, "effects": []}
    c.update(kw)
    return c


def layer(lid, clips, speed=0.0, **kw):
    lay = {"name": f"L{lid}", "id": lid, "opacity": 1.0, "visible": True, "blendMode": 0, "type": 0,
           "transitionSpeed": speed, "layerEffects": [], "clips": clips}
    lay.update(kw)
    return lay


def comp(tag, decks, gts=0.0):
    c = {"name": tag, "activeDeckIndex": 0, "masterOpacity": 1.0, "globalTransitionSpeed": gts, "outputWidth": 1920,
         "outputHeight": 1080,
         "decks": [{"name": n, "id": i, "numColumns": 4, "layers": ls} for i, (n, ls) in enumerate(decks)]}
    p = os.path.join(OUT, tag + ".json")
    json.dump(c, open(p, "w"), indent=1)
    return p


def big():   # many-video: deck A 4 layers x 4 cols of 1080p (16 videos), deck B 4 x 4K + 4 images
    cid = 100
    la = []
    for l in range(4):
        cl = []
        for c in range(4):
            cid += 1
            cl.append(vclip(cid, f"{M}/v1080_{l*4+c:02d}.mp4"))
        la.append(layer(9100 + l, cl))
    lb = [layer(9110, [vclip(300 + k, f"{M}/v4k_{k}.mp4") for k in range(4)]),
          layer(9111, [iclip(310 + k, f"{M}/img_{k}.png") for k in range(4)])]
    return comp("sweepbig", [("A", la), ("B", lb)])


def mix():   # for the trigger / deck-switch scenario: 2 decks
    la = [layer(9200, [vclip(400 + k, f"{M}/v1080_{k:02d}.mp4") for k in range(4)]),
          layer(9201, [iclip(410 + k, f"{M}/img_{k}.png") for k in range(4)])]
    lb = [layer(9210, [iclip(420 + k, f"{M}/img_{(k+1)%4}.png") for k in range(4)]),
          layer(9211, [vclip(430 + k, f"{M}/v1080_{k+4:02d}.mp4") for k in range(4)])]
    return comp("sweepmix", [("A", la), ("B", lb)])


# Scenario d's fixture (amendment 12). Layer keys per Layer::toVar, clip keys per Clip::toVar; dryWetMix / scale3D
# are written because Layer::fromVar reads them unguarded (a missing key would load 0).
FADE = 0.5
SNAP_BEAT, SNAP_BAR = 1, 2                 # Clip::BeatSnapMode
AP_PLAY_NEXT, AP_BEAT1 = 2, 1              # Clip::AutopilotAction::PlayNext, Clip::AutopilotDuration::Beat1


def fades():
    def dlayer(lid, clips, **kw):
        return layer(lid, clips, speed=FADE, dryWetMix=1.0, scale3D=1.0, **kw)
    d0 = [dlayer(9300, [vclip(500, f"{M}/v1080_00.mp4"), iclip(501, f"{M}/img_0.png"),
                        vclip(502, f"{M}/v1080_01.mp4", beatSnapMode=SNAP_BEAT, beatSnap=True),
                        iclip(503, f"{M}/img_1.png")]),
          dlayer(9301, [iclip(510 + k, f"{M}/img_{k}.png") for k in range(4)],
                 autopilotEnabled=True, defaultAutopilotAction=AP_PLAY_NEXT, defaultAutopilotDuration=AP_BEAT1),
          dlayer(9302, [vclip(520, f"{M}/v1080_02.mp4"), iclip(521, f"{M}/img_2.png"),
                        vclip(522, f"{M}/v1080_03.mp4"), None])]
    d1 = [dlayer(9310, [iclip(530 + k, f"{M}/img_{(k+1)%4}.png") for k in range(4)]),
          dlayer(9311, [vclip(540 + k, f"{M}/v1080_{k+4:02d}.mp4") for k in range(4)]),
          dlayer(9312, [iclip(550, f"{M}/img_3.png"),
                        vclip(551, f"{M}/v1080_08.mp4", beatSnapMode=SNAP_BAR, beatSnap=True),
                        iclip(552, f"{M}/img_0.png"), None])]
    return comp("tsanfades", [("A", d0), ("B", d1)], gts=0.5)


def hold(secs, poll=3.0):
    t0 = time.time()
    while time.time() - t0 < secs:
        time.sleep(min(poll, max(0.1, secs - (time.time() - t0))))
        s, b = get("/api/health")
        log("health %s %s" % (s, b[:120]))


def final_state():
    s, b = get("/api/state")
    log("state %s %s" % (s, " ".join(b.split())))
    try:
        j = json.loads(b)
        json.dump(j, open(os.path.join(OUT, "state.json"), "w"), indent=1)
        log("counters render_pending_fired=%s render_autopilot_advances=%s render_tuple_adopts=%s"
            % (j.get("render_pending_fired"), j.get("render_autopilot_advances"), j.get("render_tuple_adopts")))
    except Exception as e:
        log("state parse %s" % e)


def load(p):
    t = time.time()
    s, b = post("/api/load_composition", {"path": p}, t=150)
    log("load_composition %s %s (%.1fs)" % (s, b[:200], time.time() - t))
    return s == 200


def storm_steps(n=48):
    """d's step list: [(offset_s, endpoint, body), ...] -- every step one opacity write and one trigger."""
    out = []
    for i in range(n):
        t = 0.25 * i
        out.append((t, "set_layer_opacity", {"layer": i % 3, "opacity": round(0.3 + 0.1 * (i % 7), 2)}))
        r, k = divmod(i, 6)
        if k == 0:
            out.append((t, "trigger_column", {"column": r % 4}))
        elif k == 1:
            out.append((t, "trigger_clip", {"layer": 0, "column": (r + 1) % 4}))
        elif k == 2:
            out.append((t, "trigger_clip", {"layer": 0, "column": (r + 1) % 4}))   # the same cell: the active cell
        elif k == 3:
            out.append((t, "trigger_clip", {"layer": 2, "column": 3}))             # the empty cell: a clear
        elif k == 4:
            out.append((t, "trigger_clip", {"layer": 1, "column": (r + 2) % 4}))
        else:
            out.append((t, "trigger_clip", {"layer": 2, "column": r % 3}))
        if i % 2 == 0:
            out.append((t, "set_master_signal", {"value": round(0.5 + 0.05 * (i % 10), 2)}))
        if i % 8 == 7:
            out.append((t, "switch_deck", {"deck": (i // 8 + 1) % 2}))
    return out


def run_steps(steps, label):
    t0 = time.time()
    late = 0.0
    for off, ep, body in steps:
        dt = t0 + off - time.time()
        if dt > 0:
            time.sleep(dt)
        late = max(late, time.time() - (t0 + off))
        s, b = post("/api/" + ep, body)
        log("%s %s %s -> %s %s" % (label, ep, json.dumps(body), s, b[:80]))
    return late


if SC == "a":
    log("scenario a: idle")
    hold(15, 5)
elif SC == "b":
    load(big())
    s, b = post("/api/trigger_column", {"column": 0})
    log("trigger_column 0 %s %s" % (s, b[:80]))
    hold(20, 4)
    final_state()
elif SC == "c":
    load(mix())
    s, b = get("/api/sources")
    log("sources %s %s" % (s, b[:200]))
    src = None
    try:
        j = json.loads(b)
        L = j if isinstance(j, list) else (j.get("sources") or [])
        for x in L:
            n = x if isinstance(x, str) else (x.get("name") or x.get("type") or x.get("id"))
            if n:
                src = n
                break
    except Exception as e:
        log("sources parse %s" % e)
    steps = [("trigger_clip", {"layer": 0, "column": 0}), ("trigger_clip", {"layer": 1, "column": 1}),
             ("trigger_column", {"column": 2}), ("switch_deck", {"deck": 1}), ("trigger_column", {"column": 1}),
             ("trigger_clip", {"layer": 1, "column": 3}), ("audio/source", {"mode": "file"}),
             ("audio/source", {"mode": "input"}), ("switch_deck", {"deck": 0}), ("trigger_column", {"column": 3}),
             ("trigger_clip", {"layer": 0, "column": 1}), ("set_layer_opacity", {"layer": 0, "opacity": 0.6}),
             ("audio/source", {"mode": "file"}), ("switch_deck", {"deck": 1}), ("audio/source", {"mode": "input"}),
             ("trigger_column", {"column": 0}), ("switch_deck", {"deck": 0})]
    if src:
        steps.insert(6, ("load_source", {"source_type": src}))
    t0 = time.time()
    for ep, body in steps:
        s, b = post("/api/" + ep, body)
        log("%s %s -> %s %s" % (ep, json.dumps(body), s, b[:80]))
        time.sleep(0.8)
    rest = 15 - (time.time() - t0)
    if rest > 0:
        hold(rest, 3)
    final_state()
elif SC == "d":
    if not load(fades()):
        sys.exit(3)
    s, b = post("/api/set_bpm", {"bpm": 240})
    log("set_bpm 240 %s %s" % (s, b[:80]))
    time.sleep(1.0)
    steps = storm_steps()
    # /api/health between steps only (amendment 12): one poll every 8th step, never /api/composition.
    polls = [(0.25 * i + 0.125, "health", None) for i in range(0, 48, 8)]
    t0 = time.time()
    late = 0.0
    for off, ep, body in sorted(steps + polls, key=lambda x: x[0]):
        dt = t0 + off - time.time()
        if dt > 0:
            time.sleep(dt)
        late = max(late, time.time() - (t0 + off))
        if ep == "health":
            s, b = get("/api/health")
            log("health %s %s" % (s, b[:120]))
        else:
            s, b = post("/api/" + ep, body)
            log("d %s %s -> %s %s" % (ep, json.dumps(body), s, b[:80]))
    log("d steps %d, worst lateness %.3f s" % (len(steps), late))
    time.sleep(1.0)                     # let the last queued triggers / fades run before the one state read
    final_state()
elif SC == "e":
    if not load(fades()):
        sys.exit(3)
    s, b = post("/api/set_bpm", {"bpm": 240})
    log("set_bpm 240 %s %s" % (s, b[:80]))
    time.sleep(1.0)
    name = "probe-tsan-e-%s-%d" % (time.strftime("%Y%m%d-%H%M%S"), os.getpid())
    folder = os.path.join(os.path.expanduser("~/Documents/Audio-DNA/Takes"), name + ".adna-take")
    s, b = post("/api/perf/record", {"name": name, "audio": False})
    log("perf/record %s %s" % (s, b[:120]))
    run_steps([(0.6, "trigger_clip", {"layer": 0, "column": 1}), (1.6, "trigger_column", {"column": 2}),
               (2.6, "trigger_clip", {"layer": 1, "column": 3}), (4.0, "perf/stop", {})], "e")
    time.sleep(1.5)
    s, b = post("/api/perf/load", {"folder": folder})
    log("perf/load %s %s" % (s, b[:120]))
    time.sleep(1.0)
    s, b = post("/api/perf/play", {"withAudio": False})
    log("perf/play %s %s" % (s, b[:120]))
    hold(4, 2)
    s, b = post("/api/perf/stop_play", {})
    log("perf/stop_play %s %s" % (s, b[:120]))
    final_state()
    if os.path.basename(folder).startswith("probe-tsan-e-") and os.path.isdir(folder):
        shutil.rmtree(folder)
        log("removed the take this run recorded: %s" % folder)
else:
    log("unknown scenario %s" % SC)
    sys.exit(2)
