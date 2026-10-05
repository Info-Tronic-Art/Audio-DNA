#!/bin/bash
# OS-L21's RED arm on the 185147b app, on a FRESH image (the first try reused an image that still held filler.bin: the probe measured free space before truncating it and wrote a 0-byte filler)
O=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/31b4c846-ad01-4801-82d9-569a43efdd3b/scratchpad/gate-onesave-S1; L=/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/onesave; LANE=h-os1b
. /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/31b4c846-ad01-4801-82d9-569a43efdd3b/scratchpad/lib/lock.sh
acquire_lock || exit 1
hdiutil create -size 2m -fs HFS+ -volname onesave-m1b $O/m1b.dmg > $O/hdiutil-b.log 2>&1; mkdir -p $O/m1b
hdiutil attach -nobrowse -mountpoint $O/m1b $O/m1b.dmg >> $O/hdiutil-b.log 2>&1
rc=$?
echo "attach rc=$rc"
ONESAVE_FULLDISK=$O/m1b ONESAVE_APP=/Users/boriskarpman/projects/RealTimeAudio/build/AudioDNA_artefacts/Release/Audio-DNA.app ONESAVE_QPY=/Users/boriskarpman/projects/RealTimeAudio/.venv/bin/python bash $L/.harmony/probe-one-save.sh $O/runs os_l21 > $O/probe-l21-main-fresh.log 2>&1
rc=$?
echo "[l21 185147b, fresh image] rc=$rc"; grep -E '^(PASS|FAIL|INFO)  |^PROBE' $O/probe-l21-main-fresh.log | cut -c1-700
ls -la $O/m1b | awk '{print $5, $9}' | tr '\n' ';'; echo
hdiutil detach $O/m1b >> $O/hdiutil-b.log 2>&1
rc=$?
echo "detach rc=$rc; attached: $(hdiutil info | grep -c m1b.dmg)"
release_lock
sleep 16
echo "adna after: [$(adna | tr '\n' ' ')]; $(outwins)"
