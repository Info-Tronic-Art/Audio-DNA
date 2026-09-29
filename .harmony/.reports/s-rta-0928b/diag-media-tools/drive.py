# drive.py OUTDIR SCENARIO -- diag-media REST driver (7070, one fresh connection per request).
# Phases are marked in the app log through the TEMPORARY /api/diag/media hook (op "mark", layer = phase index), so
# the analyzer attributes every [GF]/[GV]/[HB]/[DG] line to a phase in the app's own clock.
import json, os, sys, time, urllib.request

A = 'http://127.0.0.1:7070'
OUT, SCEN = sys.argv[1], sys.argv[2]
M = '/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/e9ff9dc6-f159-4d37-b0a9-871bf522d258/scratchpad/diag-media/media'
ERR = os.path.join(OUT, 'app-err.log')
log = []
phases = []


def ev(what, **kw):
    kw.update(t=time.time(), what=what); log.append(kw); print(json.dumps(kw), flush=True)


def req(path, body=None, timeout=10):
    data = None if body is None else json.dumps(body).encode()
    r = urllib.request.Request(A + path, data=data, method='GET' if body is None else 'POST',
                               headers={'Content-Type': 'application/json', 'Connection': 'close'})
    try:
        with urllib.request.urlopen(r, timeout=timeout) as f:
            return json.loads(f.read().decode())
    except Exception as e:  # noqa: BLE001
        return {'ok': False, 'error': str(e)}


def hook(op, **kw):
    b = {'op': op}; b.update(kw)
    return req('/api/diag/media', b)


def rss_mb():
    try:
        import subprocess
        out = subprocess.run(['ps', '-eo', 'rss=,ucomm='], capture_output=True, text=True).stdout
        return [round(int(l.split()[0]) / 1024.0, 1) for l in out.splitlines() if l.split()[-1:] == ['Audio-DNA']]
    except Exception:  # noqa: BLE001
        return None


def mark(name):
    i = len(phases); phases.append(name)
    r = hook('mark', layer=i)
    ev('mark', i=i, name=name, r=r, rss_mb=rss_mb())


def count(tag):
    try:
        return open(ERR, errors='replace').read().count(tag)
    except OSError:
        return 0


def wait_log(tag, before, limit=60.0):
    t0 = time.time()
    while time.time() - t0 < limit:
        if count(tag) > before:
            return time.time() - t0
        time.sleep(0.1)
    return None


# ------------------------------------------------------------------ composition files
def clip(cid, path, mt, **extra):
    # speed is loaded unguarded (Clip.cpp:215; a missing key = 0 = a frozen clip): always 1.0 here
    c = {'name': 'c%d' % cid, 'id': cid, 'mediaType': mt, 'mediaFile': path, 'effects': [], 'speed': 1.0,
         'transportMode': 0, 'loopMode': 0, 'inPoint': 0.0, 'outPoint': 1.0}
    c.update(extra); return c


def seq_clip(cid, paths, fps):
    return {'name': 's%d' % cid, 'id': cid, 'mediaType': 5, 'mediaFile': '', 'sequenceFiles': paths,
            'sequenceFps': float(fps), 'effects': [], 'speed': 1.0, 'transportMode': 0, 'loopMode': 0,
            'inPoint': 0.0, 'outPoint': 1.0}


def layer(lid, clips):
    return {'name': 'L%d' % lid, 'id': lid, 'opacity': 1.0, 'visible': True, 'blendMode': 0, 'type': 0,
            'transitionSpeed': 0.0, 'layerEffects': [], 'clips': clips}


def deck(did, layers, ncols):
    return {'name': 'D%d' % did, 'id': did, 'numColumns': ncols, 'layers': layers}


def write_comp(tag, decks):
    comp = {'name': 'diagmedia-' + tag, 'activeDeckIndex': 0, 'masterOpacity': 1.0, 'globalTransitionSpeed': 0.0,
            'outputWidth': 1920, 'outputHeight': 1080, 'decks': decks}
    p = os.path.join(OUT, 'comp_%s.json' % tag)
    json.dump(comp, open(p, 'w'), indent=1)
    return p


def load(tag, decks):
    p = write_comp(tag, decks)
    mark('load_' + tag)
    before = count(' 0 mc.loadComposition ')
    t0 = time.time()
    r = req('/api/load_composition', {'path': p}, timeout=30)
    w = wait_log(' 0 mc.loadComposition ', before, 90)
    ev('load', tag=tag, r=r, waited=w, total=time.time() - t0)
    time.sleep(0.5)


def trig(li, col):
    r = req('/api/trigger_clip', {'layer': li, 'column': col})
    ev('trig', layer=li, col=col, r=r)


def grid(prefix, kind, n_layers=4, n_cols=4, start_id=100):
    lays, cid = [], start_id
    for li in range(n_layers):
        cl = []
        for c in range(n_cols):
            k = li * n_cols + c + 1
            if kind == 'v1080':
                cl.append(clip(cid, '%s/vid/c1080_%02d.mp4' % (M, k), 2))
            elif kind == 'v4k':
                cl.append(clip(cid, '%s/vid/c4k_%02d.mp4' % (M, k), 2))
            elif kind == 'img4k':
                cl.append(clip(cid, '%s/img4k/img_%02d.jpg' % (M, k), 1))
            cid += 1
        lays.append(layer(10 + li, cl))
    return lays


def empty():
    return [deck(1, [layer(1, [None])], 1)]


SEQ_PNG = ['%s/seq_png/frame_%04d.png' % (M, i) for i in range(1, 301)]
SEQ_JPG = ['%s/seq_jpg/frame_%04d.jpg' % (M, i) for i in range(1, 301)]


# ------------------------------------------------------------------ scenarios
# Every phase mark is posted BEFORE its action (both run on the message thread in order), so an action's cost falls
# inside its own phase window.
def scen_video():
    load('empty0', empty()); mark('idle_empty'); time.sleep(6)
    load('v1080', [deck(1, grid('v', 'v1080'), 4)]); mark('loaded_v1080_idle'); time.sleep(3)
    mark('v1080x1'); trig(0, 0); time.sleep(6)
    mark('v1080x4')
    for li in (1, 2, 3):
        trig(li, 0)
    time.sleep(6)
    load('v4k', [deck(1, grid('v', 'v4k'), 4)]); mark('loaded_v4k_idle'); time.sleep(3)
    mark('v4kx1'); trig(0, 0); time.sleep(6)
    mark('v4kx2'); trig(1, 0); time.sleep(6)
    mark('v4kx4'); trig(2, 0); trig(3, 0); time.sleep(6)
    load('prores', [deck(1, [layer(10, [clip(300, M + '/vid/prores1080.mov', 2)])], 1)]); mark('loaded_prores_idle'); time.sleep(2)
    mark('proresx1'); trig(0, 0); time.sleep(6)
    load('empty1', empty()); mark('idle_empty_end'); time.sleep(4)


def scen_seek():
    # a 10 s clip, GOP 250 (keyframes 0, 8.33 s) vs GOP 30 (every 1 s); a second deck to switch away to
    d1 = deck(2, [layer(20, [clip(401, M + '/img4k/img_01.jpg', 1)])], 1)
    for tag, f in (('g250', 'v1080_g250'), ('g30', 'v1080_g30'), ('g250_4k', 'v4k_g250')):
        load('seek_' + tag, [deck(1, [layer(10, [clip(400, '%s/vid/%s.mp4' % (M, f), 2, inPoint=0.5)])], 1), d1])
        mark('idle_' + tag); time.sleep(3)
        mark('play_' + tag); trig(0, 0); time.sleep(3)
        # retrigger = seekTo(inPoint 0.5) = 5.0 s (mid-GOP for g250)
        mark('retrig_' + tag); trig(0, 0); time.sleep(6)
        # deck away and back: the clock runs on (advanceClock); the return catches up by a seek + decode
        mark('away_' + tag); req('/api/switch_deck', {'deck': 1}); time.sleep(3.5)
        mark('back_' + tag); req('/api/switch_deck', {'deck': 0}); time.sleep(1.5)
        # a message-thread trigger while the GL thread is inside the catch-up decode: getVideoPlayer waits on
        # videoPlayerMutex_ (rd.getVideoPlayer.lockwait); it is a retrigger, so it seeks to inPoint 0.5 again
        mark('trig_in_catchup_' + tag); trig(0, 0); time.sleep(5)
    load('empty1', empty()); mark('idle_empty_end'); time.sleep(3)


def scen_images():
    load('empty0', empty()); mark('idle_empty'); time.sleep(6)
    # 4 layers x 8 cols: cols 0-3 4K JPEG images, cols 4-7 1080p videos (32 cells that stat on every paint)
    lays = grid('m', 'img4k')
    vl = grid('m', 'v1080', start_id=200)
    for li in range(4):
        lays[li]['clips'] = lays[li]['clips'] + vl[li]['clips']
    load('mixed', [deck(1, lays, 8)]); mark('loaded_mixed_idle'); time.sleep(6)
    mark('img4k_x4')
    for li in range(4):
        trig(li, 0)
    time.sleep(6)
    mark('refresh_x20'); hook('refresh', col=20); time.sleep(2)
    mark('repaint_x10')
    for i in range(10):
        hook('repaint'); time.sleep(0.2)
    time.sleep(1)
    mark('img_triggers')
    for i in range(8):
        trig(i % 4, (i // 4) + 1); time.sleep(0.5)
    time.sleep(2)
    mark('video_x4_mixed')
    for li in range(4):
        trig(li, 4)
    time.sleep(6)
    load('empty1', empty()); mark('idle_empty_end'); time.sleep(3)


def scen_seq():
    load('empty0', empty()); mark('idle_empty'); time.sleep(6)
    load('seq', [deck(1, [layer(10, [seq_clip(500, SEQ_PNG, 30), None]), layer(11, [seq_clip(501, SEQ_JPG, 30), None])], 2)])
    mark('loaded_seq_idle'); time.sleep(3)
    mark('seqpng_play'); trig(0, 0); time.sleep(12)
    mark('seqpng_seqjpg_play'); trig(1, 0); time.sleep(12)
    # drops (the Finder-drop handlers, via the hook): a 300-file sequence and single videos onto empty cells
    mark('drop_seq300'); hook('drop', layer=0, col=1, files=SEQ_JPG); time.sleep(4)
    mark('drop_v4k'); hook('drop', layer=1, col=1, files=[M + '/vid/v4k_g250.mp4']); time.sleep(3)
    mark('drop_v1080'); hook('drop', layer=1, col=1, files=[M + '/vid/v1080_g250.mp4']); time.sleep(3)
    mark('drop_img4k'); hook('drop', layer=0, col=1, files=[M + '/img4k/img_05.jpg']); time.sleep(3)
    load('empty1', empty()); mark('idle_empty_end'); time.sleep(3)


{'video': scen_video, 'seek': scen_seek, 'images': scen_images, 'seq': scen_seq}[SCEN]()
json.dump({'phases': phases, 'log': log}, open(os.path.join(OUT, 'drive-log.json'), 'w'), indent=1)
ev('done')
