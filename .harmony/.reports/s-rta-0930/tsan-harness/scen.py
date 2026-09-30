#!/usr/bin/env python3
"""scen.py <a|b|c> <rundir> : REST half of one sweep launch (production port 7070, Connection: close on every request)."""
import json, os, sys, time, threading
import requests
A = "http://127.0.0.1:7070"
import os; M = os.environ.get('SWEEP_MEDIA', "/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/44b528dd-1232-4d5c-a683-0145bc3a700e/scratchpad/sweep/media")
SC, OUT = sys.argv[1], sys.argv[2]
S = requests.Session(); S.headers["Connection"] = "close"
LOG = open(os.path.join(OUT, "scen.log"), "a")
def log(m):
    LOG.write("%s %s\n" % (time.strftime("%H:%M:%S"), m)); LOG.flush()
def get(p, t=8):
    try:
        r = S.get(A + p, timeout=t); return r.status_code, r.text
    except Exception as e:
        return None, str(e)
def post(p, body, t=8):
    try:
        r = S.post(A + p, json=body, timeout=t); return r.status_code, r.text
    except Exception as e:
        return None, str(e)
def iclip(cid, path): return {"name": f"c{cid}", "id": cid, "mediaType": 1, "mediaFile": path, "effects": []}
def vclip(cid, path): return {"name": f"v{cid}", "id": cid, "mediaType": 2, "mediaFile": path, "transportMode": 0, "loopMode": 0, "speed": 1.0, "effects": []}
def layer(lid, clips): return {"name": f"L{lid}", "id": lid, "opacity": 1.0, "visible": True, "blendMode": 0, "type": 0, "transitionSpeed": 0.0, "layerEffects": [], "clips": clips}
def comp(tag, decks):
    c = {"name": tag, "activeDeckIndex": 0, "masterOpacity": 1.0, "globalTransitionSpeed": 0.0, "outputWidth": 1920, "outputHeight": 1080,
         "decks": [{"name": n, "id": i, "numColumns": 4, "layers": ls} for i, (n, ls) in enumerate(decks)]}
    p = os.path.join(OUT, tag + ".json"); json.dump(c, open(p, "w"), indent=1); return p
def big():   # many-video: deck A 4 layers x 4 cols of 1080p (16 videos), deck B 4 x 4K + 4 images
    cid = 100
    la = []
    for l in range(4):
        cl = []
        for c in range(4):
            cid += 1; cl.append(vclip(cid, f"{M}/v1080_{l*4+c:02d}.mp4"))
        la.append(layer(9100 + l, cl))
    lb = [layer(9110, [vclip(300 + k, f"{M}/v4k_{k}.mp4") for k in range(4)]),
          layer(9111, [iclip(310 + k, f"{M}/img_{k}.png") for k in range(4)])]
    return comp("sweepbig", [("A", la), ("B", lb)])
def mix():   # for trigger / deck-switch scenario: 2 decks
    la = [layer(9200, [vclip(400 + k, f"{M}/v1080_{k:02d}.mp4") for k in range(4)]),
          layer(9201, [iclip(410 + k, f"{M}/img_{k}.png") for k in range(4)])]
    lb = [layer(9210, [iclip(420 + k, f"{M}/img_{(k+1)%4}.png") for k in range(4)]),
          layer(9211, [vclip(430 + k, f"{M}/v1080_{k+4:02d}.mp4") for k in range(4)])]
    return comp("sweepmix", [("A", la), ("B", lb)])
def hold(secs, poll=3.0):
    t0 = time.time()
    while time.time() - t0 < secs:
        time.sleep(min(poll, max(0.1, secs - (time.time() - t0))))
        s, b = get("/api/health"); log("health %s %s" % (s, b[:120]))
if SC == "a":
    log("scenario a: idle"); hold(15, 5)
elif SC == "b":
    p = big(); t = time.time()
    s, b = post("/api/load_composition", {"path": p}, t=150); log("load_composition %s %s (%.1fs)" % (s, b[:200], time.time() - t))
    s, b = post("/api/trigger_column", {"column": 0}); log("trigger_column 0 %s %s" % (s, b[:80]))
    hold(20, 4)
    s, b = get("/api/state"); log("state %s %s" % (s, " ".join(b.split())))
elif SC == "c":
    p = mix(); t = time.time()
    s, b = post("/api/load_composition", {"path": p}, t=150); log("load_composition %s %s (%.1fs)" % (s, b[:200], time.time() - t))
    s, b = get("/api/sources"); log("sources %s %s" % (s, b[:200]))
    src = None
    try:
        j = json.loads(b); L = j if isinstance(j, list) else (j.get("sources") or [])
        for x in L:
            n = x if isinstance(x, str) else (x.get("name") or x.get("type") or x.get("id"))
            if n: src = n; break
    except Exception as e: log("sources parse %s" % e)
    steps = [("trigger_clip", {"layer": 0, "column": 0}), ("trigger_clip", {"layer": 1, "column": 1}), ("trigger_column", {"column": 2}),
             ("switch_deck", {"deck": 1}), ("trigger_column", {"column": 1}), ("trigger_clip", {"layer": 1, "column": 3}),
             ("audio/source", {"mode": "file"}), ("audio/source", {"mode": "input"}),
             ("switch_deck", {"deck": 0}), ("trigger_column", {"column": 3}), ("trigger_clip", {"layer": 0, "column": 1}),
             ("set_layer_opacity", {"layer": 0, "opacity": 0.6}),
             ("audio/source", {"mode": "file"}), ("switch_deck", {"deck": 1}), ("audio/source", {"mode": "input"}),
             ("trigger_column", {"column": 0}), ("switch_deck", {"deck": 0})]
    if src: steps.insert(6, ("load_source", {"source_type": src}))
    t0 = time.time()
    for i, (ep, body) in enumerate(steps):
        s, b = post("/api/" + ep, body); log("%s %s -> %s %s" % (ep, json.dumps(body), s, b[:80]))
        time.sleep(0.8)
    rest = 15 - (time.time() - t0)
    if rest > 0: hold(rest, 3)
    s, b = get("/api/state"); log("state %s %s" % (s, " ".join(b.split())))
