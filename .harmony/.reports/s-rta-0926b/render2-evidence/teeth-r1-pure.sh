#!/bin/bash
# RED-first + teeth for the R1 pure pieces (s-rta-0926b render2, build step 1).
#  (1) RED by absence: the REAL new tests compiled against the BASE headers (git show <base>:...) do not compile.
#  (2) Teeth: the REAL tests compiled against MUTATED COPIES of the new headers fail; the deliverables are
#      never touched (sha256 printed before/after).
set -u
WT=/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/wf_41d6317f-a47-1
BASE=${BASE:-858acd1}
T=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/a07fe8e4-258f-47f0-88b1-97bfb5129bb6/scratchpad/render2/teeth
D=$WT/build-lane/tests
rm -rf "$T"; mkdir -p "$T"
flags() { local FM=$D/CMakeFiles/$1.dir/flags.make
  DEFS=$(grep '^CXX_DEFINES' "$FM" | cut -d= -f2-); INCS=$(grep '^CXX_INCLUDES' "$FM" | cut -d= -f2-)
  FLAGS=$(grep '^CXX_FLAGS =' "$FM" | cut -d= -f2-); }
build_run() { # $1 target  $2 overlay dir  $3 label
  flags "$1"
  # shellcheck disable=SC2086
  /usr/bin/c++ $DEFS -I"$2" $INCS $FLAGS -c "$WT/tests/$1.cpp" -o "$2/t.o" 2> "$2/compile.err" || {
    echo "=== $3: COMPILE FAILED (RED)"; grep -m3 "error:" "$2/compile.err" | sed "s|$WT/||"; return; }
  LINK=$(cat "$D/CMakeFiles/$1.dir/link.txt")
  LINK=${LINK//CMakeFiles\/$1.dir\/$1.cpp.o/$2/t.o}
  LINK=${LINK//-o $1 /-o $2/test_mut }
  (cd "$D" && eval "$LINK") || { echo "=== $3: LINK FAILED"; return; }
  echo "=== $3:"; "$2/test_mut" 2>&1 | grep -E "FAILED|REQUIRE|test cases|All tests passed" | sed "s|$WT/||" | head -14
}
echo "base: $BASE"
echo "deliverable sha256 before: $(shasum -a 256 $WT/src/render/LayerStateKey.h $WT/src/render/CrossfadeHistory.h | cut -c1-16 | tr '\n' ' ')"
# (1) RED by absence on the base headers
O="$T/base_lsk/render"; mkdir -p "$O"; git -C "$WT" show "$BASE:src/render/LayerStateKey.h" > "$O/LayerStateKey.h"
build_run test_layer_state_key "$T/base_lsk" "base LayerStateKey.h + new test_layer_state_key.cpp"
# the new test against the BASE source tree (git archive of $BASE's src/, which has no CrossfadeHistory.h)
mkdir -p "$T/base_src"; git -C "$WT" archive "$BASE" src | tar -x -C "$T/base_src"
echo "(base has src/render/CrossfadeHistory.h: $(git -C "$WT" cat-file -e "$BASE:src/render/CrossfadeHistory.h" 2>/dev/null && echo yes || echo no))"
flags test_crossfade_history
INCS_BASE=${INCS//-I$WT\/src/-I$T/base_src/src}
# shellcheck disable=SC2086
/usr/bin/c++ $DEFS $INCS_BASE $FLAGS -c "$WT/tests/test_crossfade_history.cpp" -o "$T/base_src/t.o" 2> "$T/base_src/compile.err" \
  && echo "=== base src/ + new test_crossfade_history.cpp: compiled?!" \
  || { echo "=== base src/ + new test_crossfade_history.cpp: COMPILE FAILED (RED)"; grep -m2 "error" "$T/base_src/compile.err" | sed "s|$WT/||"; }
# (2) teeth: mutated copies
mut() { # $1 name  $2 header  $3 sed expr  $4 target
  local O="$T/$1/render"; mkdir -p "$O"; cp "$WT/src/render/$2" "$O/$2"
  /usr/bin/sed -i '' "$3" "$O/$2"
  if cmp -s "$WT/src/render/$2" "$O/$2"; then echo "=== $1: MUTATION DID NOT APPLY"; return; fi
  diff "$WT/src/render/$2" "$O/$2" | grep '^>' | head -2
  build_run "$4" "$T/$1" "mutation $1"
}
mut lsk_drop_outgoing_bit LayerStateKey.h 's/return clipChain(deckId, layerId | kOutgoingChainBit);/return clipChain(deckId, layerId);/' test_layer_state_key
mut cfh_fire_every_frame CrossfadeHistory.h 's/const bool start = running \&\& (!wasRunning/const bool start = running \&\& (true/' test_crossfade_history
mut cfh_ignore_pair CrossfadeHistory.h 's/|| prev != lastPrev || active != lastActive/|| false/' test_crossfade_history
mut cfh_ignore_rewind CrossfadeHistory.h 's/|| progress < lastProgress);/);/' test_crossfade_history
mut cfh_no_prev_gate CrossfadeHistory.h 's/const bool running = progress < 1.0f \&\& prev >= 0;/const bool running = progress < 1.0f;/' test_crossfade_history
echo "deliverable sha256 after:  $(shasum -a 256 $WT/src/render/LayerStateKey.h $WT/src/render/CrossfadeHistory.h | cut -c1-16 | tr '\n' ' ')"
