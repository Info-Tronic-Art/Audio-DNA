#!/bin/bash
D=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/44b528dd-1232-4d5c-a683-0145bc3a700e/scratchpad/tsanflags
for idx in 0 1 2; do
  sed -e "s/s->a = 2;/s->pad[$idx] = 2;/" -e "s/int r2() { return s->a; }/int r2() { return s->pad[$idx]; }/" $D/addr.cpp > $D/addr_p$idx.cpp
  clang++ -std=c++20 -fsanitize=thread -g -O1 $D/addr_p$idx.cpp -o $D/addr_p$idx 2>/dev/null
  n=""
  for i in 1 2 3; do c=$(TSAN_OPTIONS="exitcode=0" $D/addr_p$idx 2>&1 | grep -cE "WARNING: ThreadSanitizer"); n="$n $c"; done
  echo "pair2 on pad[$idx] (byte offset $((4+4*idx))): reports per run:$n"
done
