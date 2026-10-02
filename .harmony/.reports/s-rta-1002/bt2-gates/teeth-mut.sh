#!/bin/bash
# bt2 fix round (teeth): the reviewers' surviving mutants X1 / X2 / X5 / X6 on a scratch one-tree copy (gate4-mut.sh method).
# A detached worktree of the lane HEAD gets the fix round's tests/test_device_policy.cpp (NEW) or keeps HEAD's (OLD);
# each mutant = one scripted src edit (anchor asserted to match once), incremental rebuild of test_device_policy,
# `-r junit` per-case verdicts + the failing assertion expressions, then `git checkout -- src` and `git diff --quiet -- src`.
# usage: teeth-mut.sh <lane-worktree> <new-test-file> <scratch> <deps-dir>
set -euo pipefail
LANE=$1; NEWTEST=$2; S=$3; D=$4
WT=$S/wt-mut; BD=$S/build-mut
echo "=== teeth $(date '+%F %T')  lane HEAD $(git -C "$LANE" rev-parse --short HEAD)"
git -C "$LANE" worktree add --detach "$WT" HEAD >/dev/null 2>&1
trap 'git -C "$LANE" worktree remove --force "$WT" 2>/dev/null || true; rm -rf "$BD"; echo "cleanup: worktree + build dir removed"' EXIT
cmake -S "$WT" -B "$BD" -DCMAKE_BUILD_TYPE=Release -DAUDIODNA_BUILD_TEST_SERVER=ON -DFETCHCONTENT_FULLY_DISCONNECTED=ON \
  -DFETCHCONTENT_SOURCE_DIR_JUCE=$D/juce-src -DFETCHCONTENT_SOURCE_DIR_HTTPLIB=$D/httplib-src \
  -DFETCHCONTENT_SOURCE_DIR_CATCH2=$D/catch2-src -DFETCHCONTENT_SOURCE_DIR_SYPHON=$D/syphon-src >"$S/configure.log" 2>&1

build_run() {  # build_run <tag>: rebuild, print "<cases> <failing ids|NONE>" then the failing assertion lines
  cmake --build "$BD" --target test_device_policy -j4 >"$S/build-$1.log" 2>&1 || { echo "BUILD-FAILED"; return; }
  { "$BD/tests/test_device_policy" -r junit 2>/dev/null || true; } > "$S/junit-$1.xml"
  python3 - "$S/junit-$1.xml" <<'PY'
import sys, re, xml.etree.ElementTree as ET
data = open(sys.argv[1]).read()
n = 0; bad = []; lines = []
for doc in re.findall(r"<testsuites.*?</testsuites>", data, re.S):
    for tc in ET.fromstring(doc).iter("testcase"):
        n += 1
        f = tc.find("failure")
        if f is None: f = tc.find("error")
        if f is not None:
            cid = tc.get("name").split(" ")[0]
            bad.append(cid)
            for line in (f.text or "").splitlines():
                if line.startswith("FAILED") or "at /" in line or line.strip().startswith(("CHECK", "REQUIRE")):
                    pass
            msg = (f.get("message") or "").strip()
            loc = [l.strip() for l in (f.text or "").splitlines() if "test_device_policy.cpp:" in l]
            lines.append("    %s: %s  [%s]" % (cid, msg, loc[0].split("/")[-1] if loc else "?"))
print("%d %s" % (n, " ".join(sorted(set(bad))) or "NONE"))
for l in lines: print(l)
PY
}

mutate() {
  python3 - "$WT/$1" "$2" <<'PY'
import sys
p, spec = sys.argv[1], sys.argv[2]
old, new = spec.split('\n=>\n')
s = open(p).read()
assert s.count(old) == 1, 'mutant anchor matches %d times in %s' % (s.count(old), p)
open(p, 'w').write(s.replace(old, new))
PY
}

DG=src/audio/DeviceGuard.cpp; NL=$'\n'
declare -a NAME EDIT KILL
add() { NAME+=("$1"); EDIT+=("$2"); KILL+=("$3"); }
add "X1 lostInput guard removed" "    if (nowInput != previous.inputDeviceName)${NL}        lostInput_ =${NL}=>${NL}    if (true)   // MUTANT X1${NL}        lostInput_ =" "R16 R18"
add "X2 openDefaultDevices never closes" "    if (manager_.getCurrentAudioDevice() != nullptr)${NL}        manager_.closeAudioDevice();${NL}=>${NL}    if (false)   // MUTANT X2${NL}        manager_.closeAudioDevice();" "R16"
add "X5 restore with treatAsChosenDevice=true (= coreaudio NIT-1)" "manager_.setAudioDeviceSetup(keep, false);${NL}=>${NL}manager_.setAudioDeviceSetup(keep, true);   // MUTANT X5" "R11 R14"
add "X6 restore gate without haveDevice" "    if (haveDevice && manager_.getCurrentAudioDevice() == nullptr)${NL}=>${NL}    if (manager_.getCurrentAudioDevice() == nullptr)   // MUTANT X6" "(equivalent: none expected)"

for arm in OLD NEW; do
  if [ "$arm" = NEW ]; then cp "$NEWTEST" "$WT/tests/test_device_policy.cpp"; else git -C "$WT" checkout -- tests/test_device_policy.cpp; fi
  echo "--- arm $arm tests (tests/test_device_policy.cpp sha $(shasum -a 256 "$WT/tests/test_device_policy.cpp" | cut -c1-12))"
  r=$(build_run "$arm-base"); echo "baseline (src unmutated): cases ${r%%$'\n'*}"; echo "$r" | sed 1d
  for i in "${!NAME[@]}"; do
    mutate $DG "${EDIT[$i]}"
    changed=$(git -C "$WT" diff --numstat -- src | awk '{a+=$1; d+=$2} END {print "+" a " -" d}')
    r=$(build_run "$arm-m$i")
    first=${r%%$'\n'*}
    echo "$arm | ${NAME[$i]} | $DG $changed | kill set {${KILL[$i]}} | failing: ${first#* } (of ${first%% *} cases)"
    echo "$r" | sed 1d
    git -C "$WT" checkout -- src
    git -C "$WT" diff --quiet -- src || { echo "revert failed after ${NAME[$i]}"; exit 2; }
    echo "    reverted: git diff -- src = '$(git -C "$WT" diff --stat -- src)' (empty)"
  done
done
r=$(build_run post); echo "post (NEW tests, all mutants reverted): cases ${r%%$'\n'*}"
echo "final scratch diff vs HEAD: $(git -C "$WT" diff --stat | tail -1)"
