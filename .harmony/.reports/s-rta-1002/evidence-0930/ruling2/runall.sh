#!/bin/bash
# usage: runall.sh <tag> <binary-basename>...   (whole binaries, compact totals)
R2=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/44b528dd-1232-4d5c-a683-0145bc3a700e/scratchpad/ruling2
tag=$1; shift
for b in "$@"; do
  printf "%-6s %-18s " "$tag" "$b"
  "$R2/out_$tag/run_$b" 2>/dev/null | tail -3 | grep -E "passed|failed" | tr '\n' ' '
  echo
done
