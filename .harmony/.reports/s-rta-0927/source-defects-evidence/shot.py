"""Inspector shot under the TEMPORARY hook. argv: OUT tag [comp-json-to-check]. The hook (AUDIODNA_TMP_SHOT) acts 2.5 s
after startup; wait, then capture the Audio-DNA window only (Quartz window id), render the preview once (8080)."""
import sys, os, time, json, subprocess
from lc import *
OUT, tag = sys.argv[1], sys.argv[2]
time.sleep(7.0)
png = f"{OUT}/{tag}_window.png"
r = subprocess.run(["/Users/boriskarpman/projects/RealTimeAudio/.venv/bin/python", "/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/92434c6b-4aab-4138-83af-d5f37c430398/scratchpad/srcdef/winshot.py", png], capture_output=True, text=True)
log(r.stdout.strip(), r.stderr.strip()[-300:])
comp = get("/api/composition_params"); log("composition_params:", json.dumps(comp)[:400])
im = render(f"{OUT}/{tag}_preview_256.png", t=1.13, w=256, h=256); log("preview render:", fmt(metrics(im)))
if len(sys.argv) > 3 and os.path.exists(sys.argv[3]):
    d = json.load(open(sys.argv[3]))
    for deck in d.get("decks", []):
        for li, layer in enumerate(deck.get("layers", [])):
            for ci, clip in enumerate(layer.get("clips", [])):
                if isinstance(clip, dict) and clip.get("sourceType"):
                    log(f"saved file clip L{li} C{ci} {clip['sourceType']}: {len(clip.get('sourceParams', []))} params {[p['name'] for p in clip.get('sourceParams', [])]}")
