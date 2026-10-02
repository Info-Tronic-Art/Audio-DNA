#!/bin/bash
# Run each already-built mutant binary once with the JUnit reporter; print the IDs of failing cases.
R2=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/44b528dd-1232-4d5c-a683-0145bc3a700e/scratchpad/ruling2
for tag in "$@"; do
  failed=""
  total=0
  for b in x_seats_main x_seats_new x_bt2_new_keep tdp_main x_bt2_red x_r13; do
    out=$("$R2/out_$tag/run_$b" -r junit 2>/dev/null | python3 -c '
import sys, xml.etree.ElementTree as ET
root = ET.fromstring(sys.stdin.read())
n = 0; bad = []
for tc in root.iter("testcase"):
    n += 1
    if tc.find("failure") is not None or tc.find("error") is not None:
        bad.append(tc.get("name").split(" ")[0])
print(n, " ".join(sorted(set(bad))))
')
    total=$((total + ${out%% *}))
    rest=${out#* }
    [ "$rest" != "$out" ] && failed="$failed $rest"
  done
  echo "$tag (cases $total) -> failing:${failed:- NONE}"
done
