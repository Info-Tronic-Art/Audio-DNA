#!/bin/bash
# probe-effects-parity.sh -- s-rta-0926 live witness for the clip-effects -> layer-effects FBO aliasing bug
# (ms-white-diagnosis-2.md finding B): ONE per-clip effect + ONE layer effect, opacity 1.0, no transform,
# rendered all-zero RGBA before the fix. Pixel-decoded (never file hashes). Screen-safe: open -g, no
# screen capture, no Output window, graceful quit. BUILD dir: PARITY_BUILD_DIR (default build).
set -u
ROOT="$(cd "$(dirname "$0")/.." && pwd)"; A='http://127.0.0.1:7070'
APP="$ROOT/${PARITY_BUILD_DIR:-build}/AudioDNA_artefacts/Release/Audio-DNA.app"; PY="$ROOT/.venv/bin/python"
OUT="${1:-/tmp/audiodna-effects-parity}"; mkdir -p "$OUT"; PASS=0; FAIL=0
ok(){ echo "PASS  $1"; PASS=$((PASS+1)); }; no(){ echo "FAIL  $1"; FAIL=$((FAIL+1)); }
pgrep -f 'MacOS/Audio-DN[A]' >/dev/null && { echo "REFUSE: Audio-DNA already running"; exit 64; }
: > "$OUT/out.log"; : > "$OUT/err.log"; open -g --stdout "$OUT/out.log" --stderr "$OUT/err.log" "$APP"
for _ in $(seq 1 40); do [ -n "$(curl -s --max-time 1 "$A/api/health")" ] && break; sleep 1; done
sleep 2
curl -s --max-time 6 -X POST "$A/api/load_composition" -H 'Content-Type: application/json' \
  -d "{\"path\":\"$ROOT/.harmony/probe-effects-parity.json\"}" | grep -q '"ok":[[:space:]]*true' && ok "fixture loaded" || no "fixture rejected"
sleep 1; curl -s --max-time 6 -X POST "$A/api/trigger_clip" -H 'Content-Type: application/json' -d '{"layer":0,"column":0}' >/dev/null; sleep 2
for n in a b c; do curl -s --max-time 20 -X POST "$A/api/render_frame" -H 'Content-Type: application/json' -d "{\"output_path\":\"$OUT/parity_$n.png\"}" >/dev/null; sleep 0.5; done
for n in a b c; do
  R="$("$PY" -c "
from PIL import Image; import numpy as np
a=np.asarray(Image.open('$OUT/parity_$n.png').convert('RGBA')).astype(float)
print(round((a[...,3]>0).mean(),3), round(a[...,:3].std(),1))" 2>/dev/null)"
  AF="${R% *}"; SD="${R#* }"
  awk -v a="$AF" -v s="$SD" 'BEGIN{exit !(a+0>=0.5 && s+0>=3)}' && ok "frame $n non-blank (alpha>0 frac $AF, RGB std $SD)" || no "frame $n BLANK (alpha>0 frac $AF, RGB std $SD)"
done
osascript -e 'tell application "Audio-DNA" to quit' >/dev/null 2>&1
for _ in $(seq 1 30); do pgrep -f 'MacOS/Audio-DN[A]' >/dev/null || break; sleep 1; done
pgrep -f 'MacOS/Audio-DN[A]' >/dev/null && { pkill -f 'MacOS/Audio-DN[A]'; sleep 2; }
pgrep -f 'MacOS/Audio-DN[A]' >/dev/null && no "app still running" || ok "app terminated"
echo; echo "$PASS PASS / $FAIL FAIL"; [ "$FAIL" -eq 0 ]
