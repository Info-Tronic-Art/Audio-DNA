#!/bin/bash
# Harmony's gate on lane one-save, stage S1: G-OS1 (unit + live rows with their RED arms, M-1, M-2, K7), G-OS-HIS (copies
# of his show), G-OS4-0 (his real folders: read-only checksums before and after). Test mode, a scratch settings file,
# NEVER an Output window; every app is started and quit by the run that launched it; the live lock is held for the batch.
R=/Users/boriskarpman/projects/RealTimeAudio
SP=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/31b4c846-ad01-4801-82d9-569a43efdd3b/scratchpad
L=$R/.claude/worktrees/onesave; O=$SP/gate-onesave-S1; PYQ=$R/.venv/bin/python
LANEAPP=$L/build-lane/AudioDNA_artefacts/Release/Audio-DNA.app
MAINAPP=$R/build/AudioDNA_artefacts/Release/Audio-DNA.app
SET="$HOME/Library/Audio-DNA/settings.json"
HIS="$HOME/Library/AudioDNA"
SHOWSHA=5aabe2e2562189d1f421cf4fe00239e62c94f3c49998c1cd31bfd98c32c6b78b
mkdir -p $O/runs
his_check() {  # G-OS4-0: read-only ls + shasum of his folders against the baseline of 2026-10-04 20:51:08
  local ok=0
  for pair in "Decks/feafeda.deck.json:1fbc590165e8ba3c6c29a527d757f05d137ae50db18900c6db0b1c73065acce6" "Decks/tes6.deck.json:c120d4b8c24228e4748f4d9f9ba1ef770477d16d1559c7d039ea8dca57747511" "Decks/testetst.deck.json:a8a9f61770cd7ac51e5986d1f1506963f411ca8e9f5cb51aeb91a79bdbc40833" "Decks/try.deck.json:3f43dd0e1a48e40d466b09c6df793a041347e2a27418c42e98fb00df4679d3f0" "compositions/test with harry.json:$SHOWSHA"; do
    f="${pair%%:*}"; want="${pair##*:}"; got=$(shasum -a 256 "$HIS/$f" 2>/dev/null | cut -d' ' -f1)
    [ "$got" = "$want" ] && ok=$((ok+1)) || echo "  DIFFERS: $f got ${got:-missing}"
  done
  echo "G-OS4-0 $1: $ok of 5 checksums equal the baseline; Decks holds $(ls "$HIS/Decks" | wc -l | tr -d ' ') file(s), compositions holds $(ls "$HIS/compositions" | wc -l | tr -d ' ') entr(ies): $(ls "$HIS/compositions" | tr '\n' ',')"
}
echo "$(date '+%F %T') GATE one-save S1 start; lane head $(git -C $L rev-parse --short HEAD); status lines $(git -C $L status --short | wc -l | tr -d ' ')"
echo "--- files changed 8b464a6..HEAD"; git -C $L diff --name-only 8b464a6..HEAD | tr '\n' ' '; echo
cmake --build $L/build-lane -j 6 > $O/build.log 2>&1
rc=$?
echo "lane build rc=$rc; compiled objects: $(grep -c 'Building CXX\|Building OBJCXX' $O/build.log)"
ctest --test-dir $L/build-lane -N > $O/ctestN.log 2>&1; tail -1 $O/ctestN.log
bash $SP/lib/ctest-mutex.sh $L/build-lane > $O/ctest-full.log 2>&1
rc=$?
echo "full SERIAL ctest rc=$rc"; grep -E 'tests passed|tests failed|Total Test time|ctest rc=|\*\*\*Failed|MUTEX' $O/ctest-full.log | head -10
ctest --test-dir $L/build-lane -R "^(safewrite|showfile|showbackup|appsettings):" > $O/ctest-unit.log 2>&1
rc=$?
echo "unit rows rc=$rc: $(grep -E 'tests passed|tests failed' $O/ctest-unit.log)"
$L/build-lane/tests/test_one_save_lint "[onesavelint]" > $O/lint.log 2>&1
rc=$?
echo "lint rows rc=$rc: $(tail -2 $O/lint.log | tr '\n' ' ' | cut -c1-120)"
$L/build-lane/tests/test_app_settings > $O/appsettings.log 2>&1
rc=$?
echo "test_app_settings rc=$rc: $(tail -2 $O/appsettings.log | tr '\n' ' ' | cut -c1-120)"
bash $L/.harmony/probe-one-save-selftest.sh > $O/selftest.log 2>&1
rc=$?
echo "probe self-test rc=$rc: $(grep 'SELFTEST' $O/selftest.log | tail -1)"
his_check "BEFORE the live rows"
echo "settings sha before: $(shasum -a 256 "$SET" | cut -d' ' -f1)"
echo "=== LIVE ROWS"
LANE=h-os1
. $SP/lib/lock.sh
acquire_lock
rc=$?
echo "acquire_lock rc=$rc"
[ $rc -ne 0 ] && { echo "LIVE ROWS NOT RUN: no lock"; exit 1; }
probe() {  # probe NAME APP ROWS  (extra env from the caller)
  ONESAVE_APP=$2 ONESAVE_QPY=$PYQ bash $L/.harmony/probe-one-save.sh $O/runs $3 > $O/probe-$1.log 2>&1
  local prc=$?
  echo "[$1] rc=$prc: $(grep -E '^(PASS|FAIL|INFO)  OS-|^PROBE-ONE-SAVE|^REFUSE|^FAIL  ' $O/probe-$1.log | cut -c1-420)"
}
probe lane $LANEAPP os_l13,os_l14,os_l14b
probe mut-os4 $L/build-mut-os4/AudioDNA_artefacts/Release/Audio-DNA.app os_l13,os_l14,os_l14b
probe mut-os5 $L/build-mut-os5/AudioDNA_artefacts/Release/Audio-DNA.app os_l14
probe mut-os23 $L/build-mut-os23/AudioDNA_artefacts/Release/Audio-DNA.app os_l14b
probe main185147b $MAINAPP os_l13
echo "--- OS-L21 = M-1 (a full 2 MB disk image)"
hdiutil create -size 2m -fs HFS+ -volname onesave-m1 $O/m1.dmg > $O/hdiutil.log 2>&1
rc=$?
echo "hdiutil create rc=$rc"
mkdir -p $O/m1
hdiutil attach -nobrowse -mountpoint $O/m1 $O/m1.dmg >> $O/hdiutil.log 2>&1
rc=$?
echo "hdiutil attach rc=$rc"
export ONESAVE_FULLDISK=$O/m1
probe l21-lane $LANEAPP os_l21
probe l21-main185147b $MAINAPP os_l21
probe l21-mut-os21 $L/build-mut-os21/AudioDNA_artefacts/Release/Audio-DNA.app os_l21
unset ONESAVE_FULLDISK
hdiutil detach $O/m1 >> $O/hdiutil.log 2>&1
rc=$?
echo "hdiutil detach rc=$rc; images still attached: $(hdiutil info | grep -c m1.dmg)"
echo "--- probe-boxes K7"
BOXES_APP=$LANEAPP bash $L/.harmony/probe-boxes.sh $O/runs k7_old_show > $O/k7-lane.log 2>&1
rc=$?
echo "[k7 lane] rc=$rc: $(grep -E 'k7_old_show|^PROBE|^REFUSE' $O/k7-lane.log | cut -c1-330 | tail -6)"
BOXES_APP=$MAINAPP bash $L/.harmony/probe-boxes.sh $O/runs k7_old_show > $O/k7-main.log 2>&1
rc=$?
echo "[k7 main185147b, RED arm] rc=$rc: $(grep -E 'k7_old_show save|^PROBE' $O/k7-main.log | cut -c1-330 | tail -3)"
echo "--- G-OS-HIS (copies of his show; taken from my backup copy, checked against his checksum)"
SRC="$R/.harmony/.reports/s-rta-1004/boris-show-backup/test with harry.json"
mkdir -p $O/his1 $O/his2 $O/his-out
cp "$SRC" "$O/his1/test with harry.json"; cp "$SRC" "$O/his2/test with harry.json"
echo "copy 1 sha $(shasum -a 256 "$O/his1/test with harry.json" | cut -c1-16) copy 2 sha $(shasum -a 256 "$O/his2/test with harry.json" | cut -c1-16) (his: ${SHOWSHA:0:16})"
launch() {  # launch APP TAG -> test mode, scratch settings file, pid recorded for quit_app
  [ -n "$(adna)" ] && { echo "REFUSE: an Audio-DNA is running (pid $(adna | tr '\n' ' ')): not ours"; return 1; }
  open -g --stdout $O/his-out/$2-out.log --stderr $O/his-out/$2-err.log --env "AUDIODNA_SETTINGS_FILE=$O/his-out/$2-settings.json" "$1" --args --test-mode
  for _ in $(seq 1 60); do [ -n "$(curl -s --max-time 1 -H 'Connection: close' http://127.0.0.1:7070/api/health)" ] && break; sleep 1; done
  sleep 2; adna | tr -d ' \n' > $OURPID; echo "$(date +%T) [$2] app up: pid $(cat $OURPID)"
}
launch $MAINAPP A && { python3 $O/gos_his.py A "$O/his1/test with harry.json" $O/his-out 2>&1 | cut -c1-600; quit_app; }
sleep 3
launch $LANEAPP B && { python3 $O/gos_his.py B "$O/his2/test with harry.json" $O/his-out $SHOWSHA 2>&1 | cut -c1-900; quit_app; }
sleep 3
launch $LANEAPP C && { python3 $O/gos_his.py C "$O/his2/test with harry.json" $O/his-out 2>&1 | cut -c1-900; quit_app; }
echo "copy 1 after (untouched by the 185147b app?): sha $(shasum -a 256 "$O/his1/test with harry.json" | cut -c1-16); his1 holds: $(ls $O/his1 | tr '\n' ',')"
release_lock
echo "=== AFTER"
sleep 16
$PYQ -c "
import Quartz
wl = Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionAll, Quartz.kCGNullWindowID)
a=[w for w in wl if 'Audio-DNA' in str(w.get('kCGWindowOwnerName', ''))]
u=[w for w in wl if 'UserNotificationCenter' in str(w.get('kCGWindowOwnerName', ''))]
print('16 s after the last quit: audio-dna windows %d, Output-named %d, UserNotificationCenter windows %d' % (len(a), len([w for w in a if 'Output' in str(w.get('kCGWindowName', ''))]), len(u)))"
echo "adna after: [$(adna | tr '\n' ' ')]; live lock present: $(ls -d /tmp/audiodna-live.lock 2>/dev/null | wc -l | tr -d ' ')"
echo "settings sha after:  $(shasum -a 256 "$SET" | cut -d' ' -f1)"
his_check "AFTER the live rows"
echo "lane worktree status lines after: $(git -C $L status --short | wc -l | tr -d ' ')"
echo "$(date '+%F %T') GATE one-save S1 end"
