#!/bin/bash
S=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/73d4f54c-e9d5-409c-8b5d-694bfd57c171/scratchpad/bf10-S2
P="/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/bf10/resources/projectm_presets"
WT=/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/bf10
$S/dumpbin "$P/$(ls "$P" | grep -F '[319]' | head -1)" 1920 1080 $S/f319_1920.ppm 120
$S/dumpbin "$P/$(ls "$P" | grep -F 'Superstrings' | head -1)" 1080 1920 $S/fss_1080x1920.ppm 120
$S/dumpbin $WT/tests/fixtures/milkdrop/bf10_circle.milk 1080 1920 $S/fcircle_1080x1920.ppm 90
for f in f319_1920 fss_1080x1920 fcircle_1080x1920; do sips -s format png $S/$f.ppm --out $S/$f.png >/dev/null 2>&1; sips -Z 640 $S/$f.png --out $S/$f-small.png > /dev/null 2>&1; done
ls -la $S/*.png
