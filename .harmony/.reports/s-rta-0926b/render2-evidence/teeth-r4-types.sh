#!/bin/bash
# RED-first + teeth for the Layer::canBePersistent unit test (tests/test_compositor.cpp, [persistent]).
#  (1) RED by absence: the REAL test compiled against the pre-step-4 Layer.h (git show <base>:src/model/Layer.h,
#      overlaid first on the include path) does not compile.
#  (2) Teeth: the REAL test compiled against MUTATED COPIES of Layer.h fails; the deliverable is never touched.
set -u
WT=/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/wf_41d6317f-a47-1
BASE=${BASE:-3d92481}
T=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/a07fe8e4-258f-47f0-88b1-97bfb5129bb6/scratchpad/render2/teeth4
D=$WT/build-lane/tests; FM=$D/CMakeFiles/test_compositor.dir/flags.make
DEFS=$(grep '^CXX_DEFINES' "$FM" | cut -d= -f2-); INCS=$(grep '^CXX_INCLUDES' "$FM" | cut -d= -f2-); FLAGS=$(grep '^CXX_FLAGS =' "$FM" | cut -d= -f2-)
rm -rf "$T"; mkdir -p "$T"
run() { # $1 overlay dir $2 label
  # shellcheck disable=SC2086
  /usr/bin/c++ $DEFS -I"$1" $INCS $FLAGS -c "$WT/tests/test_compositor.cpp" -o "$1/t.o" 2> "$1/compile.err" || {
    echo "=== $2: COMPILE FAILED (RED)"; grep -m2 "error:" "$1/compile.err" | sed "s|$WT/||"; return; }
  LINK=$(cat "$D/CMakeFiles/test_compositor.dir/link.txt")
  LINK=${LINK//CMakeFiles\/test_compositor.dir\/test_compositor.cpp.o/$1/t.o}
  LINK=${LINK//-o test_compositor /-o $1/test_mut }
  (cd "$D" && eval "$LINK") || { echo "=== $2: LINK FAILED"; return; }
  echo "=== $2:"; "$1/test_mut" "[persistent]" 2>&1 | grep -E "FAILED|REQUIRE|test cases|All tests passed" | sed "s|$WT/||" | head -8
}
echo "base: $BASE; deliverable sha256 before: $(shasum -a 256 $WT/src/model/Layer.h | cut -c1-16)"
mkdir -p "$T/base/model"; git -C "$WT" show "$BASE:src/model/Layer.h" > "$T/base/model/Layer.h"; run "$T/base" "base Layer.h ($BASE) + new test"
mut() { mkdir -p "$T/$1/model"; cp "$WT/src/model/Layer.h" "$T/$1/model/Layer.h"; /usr/bin/sed -i '' "$2" "$T/$1/model/Layer.h"
  cmp -s "$WT/src/model/Layer.h" "$T/$1/model/Layer.h" && { echo "=== $1: MUTATION DID NOT APPLY"; return; }
  diff "$WT/src/model/Layer.h" "$T/$1/model/Layer.h" | grep '^>' | head -1; run "$T/$1" "mutation $1"; }
mut allow_mask 's/return t == Type::Opaque || t == Type::Transparent || t == Type::FXOnly;/return t != Type::ThreeD;/'
mut drop_fxonly 's/return t == Type::Opaque || t == Type::Transparent || t == Type::FXOnly;/return t == Type::Opaque || t == Type::Transparent;/'
echo "deliverable sha256 after:  $(shasum -a 256 $WT/src/model/Layer.h | cut -c1-16)"
