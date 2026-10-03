#!/bin/bash
# P1 calibration ON THE FIX (lane app), plan I4 m2: m2 x5 at both sizes per candidate, one app launch (helper start/quit).
D=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/73d4f54c-e9d5-409c-8b5d-694bfd57c171/scratchpad/bf10-S3
WT=/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/bf10
APPX=$WT/build-lane/AudioDNA_artefacts/Release/Audio-DNA.app
PY=/Users/boriskarpman/projects/RealTimeAudio/.venv/bin/python
LIB=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/73d4f54c-e9d5-409c-8b5d-694bfd57c171/scratchpad/lib/lock.sh
LANE=bf10-S3; . $LIB
echo "== p1live start $(date +%T)"; echo "UNC before: $($PY $D/unc.py)"
acquire_lock || exit 1
ps -Ao pcpu=,etime=,comm= | sort -rn | head -4
mkdir -p $D/runs/p1live
if start_app "$APPX" $D/runs/p1live test; then
  for c in "BEST OF ADAMFX 2 Flexi + Geiss - Bipolar vs reaction diffusion + Another Flexi + Fishbrain  + Phat Zylot 5.milk" "suksma - sun pod gambit couch - flx infinity within a finite boundary --- Isosceles edit6.milk" "Hexcollie, Krash, bdrv, EoS n aderassi - Fractal rebirth - from hell nz+2.milk" "304.milk" "yin - 363 - Organic circuits (warming up).milk"; do
    echo "#### candidate: $c"
    MILKDROP_ATTACH=1 MILKDROP_P1="$c" bash $WT/.harmony/probe-milkdrop.sh $D/runs/p1live m2_live_no_stale,m2_live_no_stale,m2_live_no_stale,m2_live_no_stale,m2_live_no_stale | grep -E "^(PASS|FAIL)  m2|^PY|ERROR" | cut -c1-150
  done
  quit_app
fi
echo "outwins: $(outwins)"
release_lock; sleep 16; echo "UNC 16 s after quit: $($PY $D/unc.py)"; echo "== p1live end $(date +%T)"
