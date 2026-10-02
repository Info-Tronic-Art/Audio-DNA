#!/bin/bash
# usage: runcases.sh <tag> <binary-basename> <case-pattern>...
R2=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/44b528dd-1232-4d5c-a683-0145bc3a700e/scratchpad/ruling2
tag=$1; bin=$2; shift 2
for t in "$@"; do
  printf "%-6s %-8s " "$tag" "${t%\*}"
  "$R2/out_$tag/run_$bin" "$t" 2>/dev/null | tail -2 | head -1
done
