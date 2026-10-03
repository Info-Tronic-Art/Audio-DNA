#!/usr/bin/env python3
"""make-bf9b-check.py -- Boris's test show for lane bf9b (s-rta-1002b; ruling-bf9b amendment 24, "WHAT ONLY BORIS CAN
CHECK"): decks are boxes of clips; the layers are ONE shared playing stack.

usage: make-bf9b-check.py <out-dir>
Writes into <out-dir> (created; the show names its media by ABSOLUTE path, so generate it where it will be opened --
never copy the folder elsewhere afterwards):
  bf9b-check.json   the show, in the format the bf9b app saves (top-level "layers" + "decks" of rows of clips)
  img/D<d>C<c>L<l>.png   one picture per cell: big "D<d> C<c>" + "layer <l>" (1-based, like the tabs and Boris's page),
                         one colour per deck
  vid/timecode-A.mp4, vid/timecode-B.mp4   60 s, 30 fps, a big running time code "A 00:12.3" over a green ramp
  five-rows.json    a DECK file (Deck > Load Deck...) with five rows of 4 pictures -- two more rows than the show has
                    layers, so loading it adds two layers (page step 8.12; ruling-bf9b-merge section 6)
  nine-rows.json    a DECK file with nine rows, one picture in row 1 / column 0 (0-based) and nothing else -- the
                    live ASan row's deck (ruling-bf9b-merge AM-6, steps L1-L5)
Needs python with Pillow, and ffmpeg on PATH. Opens no app, touches nothing else.

The show: 20 decks ("Deck 1".."Deck 20"), each 4 columns and exactly 3 rows (numColumns and every row's length set
explicitly). 3 shared layers, all Transparent, Normal blend, each in its own part of the picture so all three show at
once: Layer 1 = the left half, Layer 2 = the right half, Layer 3 = a small box in the middle of an edge (the videos).
Every cell holds a picture except: Deck 1 / Layer 3 / column 4 = timecode-A, Deck 2 / Layer 3 / column 4 = timecode-B,
and Deck 6 / Layer 2 / column 2 is EMPTY (page step 8.4: a column fire empties that layer).
"""
import colorsys, json, os, subprocess, sys

from PIL import Image, ImageDraw, ImageFont

DECKS, COLS, ROWS = 20, 4, 3
W, H = 640, 360

LAYER_TEMPLATE = {
    "name": "Layer 1", "id": 0, "type": 1, "opacity": 1.0, "visible": True, "bypassed": False, "solo": False,
    "muted": False, "autopilotEnabled": False, "ignoreColumnTrigger": False, "folded": False, "blendMode": 0,
    "keyingMode": 0, "keyThreshold": 0.1, "keySoftness": 0.1, "chromaKeyR": 0.0, "chromaKeyG": 1.0, "chromaKeyB": 0.0,
    "chromaKeyTolerance": 0.2, "dryWetMix": 1.0, "rotationX": 0.0, "rotationY": 0.0, "rotationZ": 0.0,
    "rotationSpeed": 0.0, "scale3D": 1.0, "transitionSpeed": -1.0, "defaultAutopilotAction": 2,
    "defaultAutopilotDuration": 3, "defaultAutopilotCustomBeats": 4, "autopilotLoops": 1, "autopilotEndOfVideo": False,
    "layerWidth": 1920, "layerHeight": 1080, "autoSize": 0, "transitionMode": 24, "transitionBlendMode": 0,
    "positionX": 0.0, "positionY": 0.0, "layerScale": 1.0, "layerRotation": 0.0, "layerAnchorX": 0.0,
    "layerAnchorY": 0.0, "feedbackEnabled": False, "feedbackAmount": 0.5, "feedbackScaleX": 0.98,
    "feedbackScaleY": 0.98, "feedbackRotation": 0.0, "feedbackOffsetX": 0.0, "feedbackOffsetY": 0.0,
    "feedbackLumaKey": 0.0, "feedbackPreset": "", "layerEffects": []}
# Layer transform (layer_transform: uv = uv / scale - translate, anchor 0): Layer 1 = the left half, Layer 2 = the
# right half (scale 0.5, x 0 / 1, the middle band y 0.25..0.75), Layer 3 = a quarter-size box centred on an edge.
LAYER_PLACE = [dict(layerScale=0.5, positionX=0.0, positionY=0.5),
               dict(layerScale=0.5, positionX=1.0, positionY=0.5),
               dict(layerScale=0.25, positionX=1.5, positionY=0.0)]

CLIP_TEMPLATE = {
    "name": "", "id": 0, "mediaType": 1, "mediaFile": "", "cameraDeviceIndex": -1, "sourceType": "", "hasAlpha": False,
    "beatDivision": 4.0, "videoBeats": 4.0, "sourceParams": [], "transportMode": 0, "loopMode": 0, "speed": 1.0,
    "reverse": False, "startOffset": 0.0, "inPoint": 0.0, "outPoint": 1.0, "beatSnap": False, "beatSnapMode": 0,
    "autopilotAction": 0, "autopilotDuration": 0, "autopilotCustomBeats": 4, "autopilotSpecificCol": -1,
    "clipOpacity": 1.0, "clipWidth": 1920, "clipHeight": 1080, "blendOverride": 0, "fitMode": 0, "alphaType": 0,
    "channelR": True, "channelG": True, "channelB": True, "channelA": True, "positionX": 0.0, "positionY": 0.0,
    "scale": 1.0, "rotation": 0.0, "anchorX": 0.0, "anchorY": 0.0, "effects": [], "cuepoints": [],
    "presetPlaylist": [], "playlistCycleMode": 3, "playlistTrigger": 0, "playlistTriggerBeats": 8,
    "playlistBlendSeconds": 1.5, "playlistEnabled": False}

COMP_TEMPLATE = {
    "name": "bf9b-check", "activeDeckIndex": 0, "masterOpacity": 1.0, "bpmMultiplier": 1, "quantizeMode": 0,
    "outputWidth": 1920, "outputHeight": 1080, "outputDisplay": -1, "masterSpeed": 1.0, "masterSignal": 1.0,
    "crossfaderPhase": 0.5, "crossfaderBlendMode": 0, "crossfaderBehaviour": 0, "crossfaderCurve": 0,
    "compPositionX": 0.0, "compPositionY": 0.0, "compScale": 1.0, "compRotation": 0.0, "compAnchorX": 0.0,
    "compAnchorY": 0.0, "autopilotDirection": 1, "autopilotDurationMode": 0, "autopilotClipLoops": 1,
    "autopilotLoop": False, "autopilotMasterLayer": -1, "ptaOpaqueCycleBeats": 16, "ptaOpaquePlayUntilEnd": False,
    "ptaTransparentCycleBeats": 8, "ptaTransparentMaxLayers": 2, "ptaTransparentRandomize": True,
    "ptaEffectCycleBeats": 4, "ptaEffectMaxLayers": 2, "ptaEffectRandomize": True, "ptaPerTypeEnabled": False,
    "ptaGlobalRandomize": False, "ptaLoopAutopilot": True, "autoPresetOnGenre": False, "smartAutopilotEnabled": False,
    "structuralSceneEnabled": False, "genreDeckAssignment": [-1] * 8, "genrePresetNames": [""] * 8,
    "globalEffects": [], "connect": {"gripHoldMs": 250.0, "handBackGlideMs": 120.0}, "routines": [], "routineBank": []}

VIDEOS = {(1, 3, 4): "A", (2, 3, 4): "B"}     # (deck, layer, column), 1-based
EMPTY = {(6, 2, 2)}


def font(size):
    for f in ("/System/Library/Fonts/Supplemental/Arial Bold.ttf", "/Library/Fonts/Arial Bold.ttf"):
        if os.path.exists(f):
            return ImageFont.truetype(f, size)
    return ImageFont.load_default()


def centred(dr, y, text, f, fill):
    bb = dr.textbbox((0, 0), text, font=f)
    dr.text(((W - (bb[2] - bb[0])) / 2 - bb[0], y - bb[1]), text, fill=fill, font=f)


def picture(out, d, c, l):
    p = os.path.join(out, "img", f"D{d}C{c}L{l}.png")
    r, g, b = colorsys.hsv_to_rgb(((d - 1) * 0.137) % 1.0, 0.75, 0.42 + 0.12 * (c - 1) / 3.0)
    im = Image.new("RGB", (W, H), (int(r * 255), int(g * 255), int(b * 255)))
    dr = ImageDraw.Draw(im)
    centred(dr, 70, f"D{d} C{c}", font(150), (255, 255, 255))
    centred(dr, 265, f"layer {l}", font(56), (235, 235, 235))
    im.save(p)
    return p


def timecode_video(out, tag, seconds=60, fps=30):
    p = os.path.join(out, "vid", f"timecode-{tag}.mp4")
    big, small = font(130), font(44)
    proc = subprocess.Popen(["ffmpeg", "-y", "-loglevel", "error", "-f", "rawvideo", "-pix_fmt", "rgb24", "-s",
                             f"{W}x{H}", "-r", str(fps), "-i", "-", "-g", str(fps), "-c:v", "libx264", "-pix_fmt",
                             "yuv420p", p], stdin=subprocess.PIPE)
    for k in range(seconds * fps):
        t = k / fps
        g = int(40 + 180 * t / seconds)
        im = Image.new("RGB", (W, H), (20, g, 40))
        dr = ImageDraw.Draw(im)
        centred(dr, 60, f"{int(t) // 60:02d}:{int(t) % 60:02d}.{int(t * 10) % 10}", big, (255, 255, 255))
        centred(dr, 255, f"video {tag} (time code)", small, (240, 240, 240))
        proc.stdin.write(im.tobytes())
    proc.stdin.close()
    if proc.wait() != 0 or not os.path.exists(p):
        sys.exit(f"ffmpeg failed for {p}")
    return p


def deck_file(out, fname, name, d, rows, cells):
    """A deck file as Deck::toVar writes it: rows of "clips" only (a new-format row: Load Deck adds a default layer
    for each row beyond the show's). `cells` = the (row, column) pairs, 1-based, that hold a picture."""
    cid = 0
    row_list = []
    for l in range(1, rows + 1):
        clips = []
        for c in range(1, COLS + 1):
            if (l, c) not in cells:
                clips.append(None)
                continue
            cid += 1
            clips.append(dict(CLIP_TEMPLATE, name=f"D{d} C{c}", id=cid, mediaFile=picture(out, d, c, l)))
        row_list.append({"clips": clips})
    path = os.path.join(out, fname)
    with open(path, "w") as f:
        json.dump({"name": name, "id": 99 + d, "numColumns": COLS, "layers": row_list}, f, indent=1)
    print(f"wrote {path}: deck \"{name}\", {rows} rows x {COLS} columns, {cid} clip(s)")


def main():
    if len(sys.argv) != 2:
        sys.exit(__doc__)
    out = os.path.abspath(sys.argv[1])
    os.makedirs(os.path.join(out, "img"), exist_ok=True)
    os.makedirs(os.path.join(out, "vid"), exist_ok=True)
    vids = {key: timecode_video(out, tag) for key, tag in VIDEOS.items()}
    comp = dict(COMP_TEMPLATE)
    comp["layers"] = []
    for l in range(1, ROWS + 1):
        lj = dict(LAYER_TEMPLATE, name=f"Layer {l}", id=l - 1)
        lj.update(LAYER_PLACE[l - 1])
        comp["layers"].append(lj)
    cid = 0
    decks = []
    for d in range(1, DECKS + 1):
        rows = []
        for l in range(1, ROWS + 1):
            cells = []
            for c in range(1, COLS + 1):
                if (d, l, c) in EMPTY:
                    cells.append(None)
                    continue
                cid += 1
                if (d, l, c) in vids:
                    cj = dict(CLIP_TEMPLATE, name=f"timecode {VIDEOS[(d, l, c)]}", id=cid, mediaType=2,
                              mediaFile=vids[(d, l, c)])
                else:
                    cj = dict(CLIP_TEMPLATE, name=f"D{d} C{c}", id=cid, mediaFile=picture(out, d, c, l))
                cells.append(cj)
            assert len(cells) == COLS
            rows.append({"clips": cells})
        decks.append({"name": f"Deck {d}", "id": 99 + d, "numColumns": COLS, "layers": rows})
    comp["decks"] = decks
    path = os.path.join(out, "bf9b-check.json")
    with open(path, "w") as f:
        json.dump(comp, f, indent=1)
    print(f"wrote {path}: {DECKS} decks x {ROWS} rows x {COLS} columns, {cid} clips (2 videos), 1 empty cell")
    deck_file(out, "five-rows.json", "Five Rows", 21, 5, {(l, c) for l in range(1, 6) for c in range(1, COLS + 1)})
    deck_file(out, "nine-rows.json", "Nine Rows", 22, 9, {(2, 1)})


if __name__ == "__main__":
    main()
