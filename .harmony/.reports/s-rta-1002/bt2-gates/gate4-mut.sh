#!/bin/bash
# s-rta-1002 bt2 GATE-4 -- the mutation table on the lane's REAL code (ruling-bt2-seats.md FINAL GATE LIST; AM5 one-tree
# method). A clean worktree of the lane commit is made under SCRATCH (the deliverable is never mutated); a normal cmake
# build of test_device_policy + test_audio_engine_devices runs there. Each mutant = ONE scripted edit of that copy
# (asserted to match exactly once), an incremental rebuild of the two targets, both binaries run with `-r junit`
# (per-case verdicts; Catch2 wraps long names in --list-tests), then the edit is reverted and `git diff` of the copy is
# proved empty before the next mutant. Verdict per mutant: KILLED iff >= 1 case of its pre-registered kill set fails.
# The worktree and its build dir are removed at the end.
#
# usage: gate4-mut.sh <lane-worktree> <lane-commit> <scratch-dir> <deps-dir>
set -euo pipefail
LANE=$1; REV=$2; S=$3; D=$4
WT=$S/wt-mut; BD=$S/build-mut
echo "=== GATE-4 $(date '+%F %T')  lane commit $(git -C "$LANE" rev-parse --short "$REV")"
git -C "$LANE" worktree add --detach "$WT" "$REV" >/dev/null 2>&1
trap 'git -C "$LANE" worktree remove --force "$WT" 2>/dev/null || true; rm -rf "$BD"; echo "cleanup: worktree + build dir removed"' EXIT
cmake -S "$WT" -B "$BD" -DCMAKE_BUILD_TYPE=Release -DAUDIODNA_BUILD_TEST_SERVER=ON -DFETCHCONTENT_FULLY_DISCONNECTED=ON \
  -DFETCHCONTENT_SOURCE_DIR_JUCE=$D/juce-src -DFETCHCONTENT_SOURCE_DIR_HTTPLIB=$D/httplib-src \
  -DFETCHCONTENT_SOURCE_DIR_CATCH2=$D/catch2-src -DFETCHCONTENT_SOURCE_DIR_SYPHON=$D/syphon-src >"$S/g4-configure.log" 2>&1

build_run() {  # build_run <tag>: rebuild both targets, print the IDs of failing cases ("NONE") and the case count
  cmake --build "$BD" --target test_device_policy test_audio_engine_devices -j3 >"$S/g4-build-$1.log" 2>&1 \
    || { echo "BUILD-FAILED"; return; }
  for b in test_device_policy test_audio_engine_devices; do "$BD/tests/$b" -r junit 2>/dev/null || true; done | python3 -c '
import sys, re, xml.etree.ElementTree as ET
data = sys.stdin.read()
n = 0; bad = []
for doc in re.findall(r"<testsuites.*?</testsuites>", data, re.S):
    for tc in ET.fromstring(doc).iter("testcase"):
        n += 1
        if tc.find("failure") is not None or tc.find("error") is not None:
            bad.append(tc.get("name").split(" ")[0])
print("%d %s" % (n, " ".join(sorted(set(bad))) or "NONE"))'
}

mutate() {  # mutate <file> <python-replace-expression>: apply one edit to the copy, exactly one match
  python3 - "$WT/$1" "$2" <<'PY'
import sys
p, spec = sys.argv[1], sys.argv[2]
old, new = spec.split('\n=>\n')
s = open(p).read()
assert s.count(old) == 1, 'mutant anchor matches %d times in %s' % (s.count(old), p)
open(p, 'w').write(s.replace(old, new))
PY
}

base=$(build_run baseline)
echo "baseline (unmutated copy): cases ${base%% *}, failing: ${base#* }"
[ "${base#* }" = NONE ] || { echo "GATE-4: the unmutated copy is not green"; exit 1; }

DG=src/audio/DeviceGuard.cpp; DP=src/audio/DevicePolicy.cpp; AE=src/audio/AudioEngine.cpp
NL=$'\n'
declare -a NAME FILE EDIT KILL
add() { NAME+=("$1"); FILE+=("$2"); EDIT+=("$3"); KILL+=("$4"); }
add "no AdoptInput"            $DP "        return Reapply::AdoptInput;${NL}=>${NL}        return Reapply::None;   // MUTANT" "R6 R11 R12 R13 P14"
add "no InputLost"             $DP "        return Reapply::InputLost;${NL}=>${NL}        return Reapply::None;   // MUTANT" "R8 R12 P14"
add "switch a working mic"     $DP "    return Reapply::None;${NL}}${NL}${NL}juce::String toString(Reapply r)${NL}=>${NL}    return lists.inputs.size() > 1 ? Reapply::AdoptInput : Reapply::None;   // MUTANT${NL}}${NL}${NL}juce::String toString(Reapply r)" "R9 R2"
add "no restore"               $DG "    if (haveDevice && manager_.getCurrentAudioDevice() == nullptr)${NL}=>${NL}    if (false)   // MUTANT" "R11 R14"
add "launch seq not recorded"  $DG "        lastAttemptSeq_ = guarded->lastScan().seq;   // AFTER the open: a list change the open itself caused is not new${NL}=>${NL}        (void) guarded;   // MUTANT: the scan is never recorded" "R10 R11 R4b"
add "no settle"                $DG "    startTimer(settleMs_);   // restarts: re-evaluated after the LAST change message${NL}=>${NL}    startTimer(1);   // MUTANT" "R1 R13 R17"
add "no progress guard"        $DG "    if (action == dp::Reapply::AdoptInput && noInputOn_ == scan.lists.inputs)${NL}=>${NL}    if (false)   // MUTANT" "R15 R11"
add "restore without the still-listed filter" $DG "        if (!lists.inputs.contains(keep.inputDeviceName))${NL}            keep.inputDeviceName = {};${NL}        if (!lists.outputs.contains(keep.outputDeviceName))${NL}=>${NL}        if (false)   // MUTANT${NL}            keep.inputDeviceName = {};${NL}        if (false)   // MUTANT" "R14"
add "no stopped-device row (devicePlaying forced true)" $DG "dp::reconcile(haveDevice, haveDevice && device->isPlaying(), previous.inputDeviceName${NL}=>${NL}dp::reconcile(haveDevice, haveDevice, previous.inputDeviceName" "R16"
add "DeviceStopped exempt from the scan gate" $DG "    if (scan.seq == lastAttemptSeq_)${NL}=>${NL}    if (scan.seq == lastAttemptSeq_ && action != dp::Reapply::DeviceStopped)   // MUTANT" "R16"
add "lostInput never set"      $DG "        lostInput_ = ((action${NL}=>${NL}        if (false) lostInput_ = ((action" "R18"
add "setSourceMode's re-open restored" $AE "        combinedCallback_.useInputForAnalysis.store(true, std::memory_order_relaxed);${NL}${NL}=>${NL}        combinedCallback_.useInputForAnalysis.store(true, std::memory_order_relaxed);${NL}${NL}        // Re-open device with input enabled${NL}        auto setup = deviceManager_.getAudioDeviceSetup();${NL}        setup.inputChannels.setRange(0, 2, true);  // Enable stereo input${NL}        deviceManager_.setAudioDeviceSetup(setup, true);${NL}${NL}" "AE1 AE2"
add "a re-open guarded by getCurrentAudioDevice()" $AE "        combinedCallback_.useInputForAnalysis.store(true, std::memory_order_relaxed);${NL}=>${NL}        combinedCallback_.useInputForAnalysis.store(true, std::memory_order_relaxed);${NL}        if (deviceManager_.getCurrentAudioDevice() != nullptr) { auto s = deviceManager_.getAudioDeviceSetup(); s.inputChannels.setRange(0, 2, true); deviceManager_.setAudioDeviceSetup(s, true); }" "AE2"

# mutant 12 restores BOTH old blocks: the File branch's re-open is a second edit of the same mutant
FILE12_EXTRA="        combinedCallback_.useInputForAnalysis.store(false, std::memory_order_relaxed);${NL}${NL}=>${NL}        combinedCallback_.useInputForAnalysis.store(false, std::memory_order_relaxed);${NL}${NL}        // Can disable input channels to reduce latency${NL}        auto setup = deviceManager_.getAudioDeviceSetup();${NL}        setup.inputChannels.clear();${NL}        deviceManager_.setAudioDeviceSetup(setup, true);${NL}${NL}"

printf '\n| # | mutant | file | lines changed | kill set | failing cases | verdict |\n|---|---|---|---|---|---|---|\n' > "$S/g4-table.md"
survivors=0
for i in "${!NAME[@]}"; do
  mutate "${FILE[$i]}" "${EDIT[$i]}"
  [ "${NAME[$i]}" = "setSourceMode's re-open restored" ] && mutate "${FILE[$i]}" "$FILE12_EXTRA"
  changed=$(git -C "$WT" diff --numstat | awk '{a+=$1; d+=$2} END {print "+" a " -" d}')
  r=$(build_run "m$i")
  fails=${r#* }
  killed=no
  for k in ${KILL[$i]}; do [[ " $fails " == *" $k "* ]] && killed=yes; done
  [ "$killed" = yes ] && verdict=KILLED || { verdict=SURVIVED; survivors=$((survivors + 1)); }
  printf '| %d | %s | %s | %s | {%s} | %s (of %s cases) | %s |\n' $((i + 1)) "${NAME[$i]}" "${FILE[$i]}" "$changed" "${KILL[$i]}" "$fails" "${r%% *}" "$verdict" | tee -a "$S/g4-table.md"
  git -C "$WT" checkout -- src   # the scratch copy only (never the lane tree)
  git -C "$WT" diff --quiet || { echo "revert failed after mutant $((i + 1))"; exit 2; }
done
echo "reverted: git -C wt-mut diff = '$(git -C "$WT" diff --stat)' (empty)"
post=$(build_run post)
echo "post (all mutants reverted): cases ${post%% *}, failing: ${post#* }"
cat "$S/g4-table.md"
[ "$survivors" -eq 0 ] && echo "GATE-4: 13 / 13 KILLED" || { echo "GATE-4: $survivors SURVIVOR(S)"; exit 1; }
