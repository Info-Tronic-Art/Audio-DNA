#!/bin/bash
# Build each mutant (one tree: its own headers + objects) with the seat cases + the regression sets; print failing cases.
R2=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/44b528dd-1232-4d5c-a683-0145bc3a700e/scratchpad/ruling2
for m in noProgress noFilter noLiveness lostNever noSettle stoppedBypassSeq; do
  bash "$R2/rig_seats.sh" "m_$m" "$R2/mut_$m" "$R2/x_seats_main.cpp" "$R2/x_seats_new.cpp" "$R2/x_bt2_new_keep.cpp" "$R2/tdp_main.cpp" "$R2/x_bt2_red.cpp" "$R2/x_r13.cpp" >/dev/null 2>&1 || { echo "$m BUILD FAILED"; continue; }
  failed=""
  for b in x_seats_main x_seats_new x_bt2_new_keep tdp_main x_bt2_red x_r13; do
    f=$("$R2/out_m_$m/run_$b" --list-tests 2>/dev/null | grep -E '^  [A-Z]' | sed 's/^  //' | while read -r name; do
          "$R2/out_m_$m/run_$b" "$name" >/dev/null 2>&1 || echo "${name%% *}"
        done | tr '\n' ' ')
    failed="$failed$f"
  done
  echo "MUTANT $m -> failing: ${failed:-NONE (survived)}"
done
