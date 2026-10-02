#!/bin/bash
R2=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/44b528dd-1232-4d5c-a683-0145bc3a700e/scratchpad/ruling2
for f in /Users/boriskarpman/projects/RealTimeAudio/src/audio/AudioEngine.cpp "$R2/AE_c3.cpp" "$R2/AE_guarded.cpp"; do
  printf "%-60s " "$(basename "$f") ($( [ "$f" = /Users/boriskarpman/projects/RealTimeAudio/src/audio/AudioEngine.cpp ] && echo 655d232 || echo scratch))"
  LINT_AE="$f" "$R2/out_lint/run_x_ae2_lint" 2>/dev/null | tail -2 | head -1
done
