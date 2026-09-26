#!/bin/bash
# Teeth for tests/test_layer_state_key.cpp: compile the REAL test against MUTATED COPIES of
# render/LayerStateKey.h (the deliverable is never touched; sha256 printed before/after).
set -u
WT=/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/wf_41d6317f-a47-1
T=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/a07fe8e4-258f-47f0-88b1-97bfb5129bb6/scratchpad/teeth
D=$WT/build-lane/tests
FM=$D/CMakeFiles/test_layer_state_key.dir/flags.make
DEFS=$(grep '^CXX_DEFINES' "$FM" | cut -d= -f2-)
INCS=$(grep '^CXX_INCLUDES' "$FM" | cut -d= -f2-)
FLAGS=$(grep '^CXX_FLAGS =' "$FM" | cut -d= -f2-)
echo "deliverable sha256 before: $(shasum -a 256 "$WT/src/render/LayerStateKey.h" | cut -d' ' -f1)"
for M in drop_deck drop_chain_bit; do
  mkdir -p "$T/$M/render"
  cp "$WT/src/render/LayerStateKey.h" "$T/$M/render/LayerStateKey.h"
  if [ "$M" = drop_deck ]; then
    /usr/bin/sed -i '' 's/return (std::uint64_t{ deckId } << 32) | std::uint64_t{ layerId };/(void)deckId; return std::uint64_t{ layerId };/' "$T/$M/render/LayerStateKey.h"
  else
    /usr/bin/sed -i '' 's/return clipChain(deckId, layerId | kLayerChainBit);/return clipChain(deckId, layerId);/' "$T/$M/render/LayerStateKey.h"
  fi
  diff "$WT/src/render/LayerStateKey.h" "$T/$M/render/LayerStateKey.h" | head -4
  # shellcheck disable=SC2086
  /usr/bin/c++ $DEFS -I"$T/$M" $INCS $FLAGS -c "$WT/tests/test_layer_state_key.cpp" -o "$T/$M/t.o" || { echo "$M: COMPILE FAILED"; continue; }
  LINK=$(cat "$D/CMakeFiles/test_layer_state_key.dir/link.txt")
  LINK=${LINK//CMakeFiles\/test_layer_state_key.dir\/test_layer_state_key.cpp.o/$T/$M/t.o}
  LINK=${LINK//-o test_layer_state_key/-o $T/$M/test_mut}
  (cd "$D" && eval "$LINK") || { echo "$M: LINK FAILED"; continue; }
  echo "=== mutation $M:"
  "$T/$M/test_mut" 2>&1 | grep -E "FAILED|REQUIRE\(|test cases|with expansion" | head -12
done
echo "deliverable sha256 after:  $(shasum -a 256 "$WT/src/render/LayerStateKey.h" | cut -d' ' -f1)"
