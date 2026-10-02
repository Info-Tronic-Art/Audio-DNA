#!/bin/bash
D=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/44b528dd-1232-4d5c-a683-0145bc3a700e/scratchpad/tsanflags
clang++ -std=c++20 -fsanitize=thread -g -O1 $D/addr3.cpp -o $D/addr3 || exit 1
echo -n "no refill: "; for i in 1 2 3; do TSAN_OPTIONS=exitcode=0 $D/addr3 2>&1 | grep -c "WARNING: ThreadSanitizer" | tr '\n' ' '; done; echo
echo -n "refill (16 ordered writes by main): "; for i in 1 2 3; do TSAN_OPTIONS=exitcode=0 $D/addr3 x 2>&1 | grep -c "WARNING: ThreadSanitizer" | tr '\n' ' '; done; echo
