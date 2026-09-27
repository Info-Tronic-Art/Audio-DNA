WT=/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w8
SP=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/92434c6b-4aab-4138-83af-d5f37c430398/scratchpad/renderperf
B=$WT/build-lane
M=$SP/mut; rm -rf $M; mkdir -p $M
sha_before=$(shasum -a 256 $WT/src/render/FrameRing.h $WT/tests/test_frame_ring.cpp)
build() { # $1 name, $2 include-dir, $3 test source
  /usr/bin/c++ -I$2 -I$WT/src -I$B/_deps/catch2-src/src/catch2/.. -I$B/_deps/catch2-build/generated-includes -O3 -DNDEBUG -std=c++20 -arch arm64 -c $3 -o $M/$1.o && \
  /usr/bin/c++ -O3 -arch arm64 $M/$1.o -o $M/bin-$1 $B/_deps/catch2-build/src/libCatch2Main.a $B/_deps/catch2-build/src/libCatch2.a
}
run() { echo "=== $1 ($2)"; $M/bin-$1 2>&1 | grep -E "FAILED|passed|failed|^  REQUIRE|^  CHECK|with message|with expansion" | head -12; echo "exit=${PIPESTATUS[0]}"; }
# M0: unmutated copy -- must pass
mkdir -p $M/m0/render; cp $WT/src/render/FrameRing.h $M/m0/render/; build m0 $M/m0 $WT/tests/test_frame_ring.cpp; run m0 "unmutated copy: expect PASS"
# M1: + framesAgo
mkdir -p $M/m1/render; sed 's/return (writeIndex - 1 - framesAgo + capacity \* 2) % capacity;/return (writeIndex - 1 + framesAgo + capacity * 2) % capacity;/' $WT/src/render/FrameRing.h > $M/m1/render/FrameRing.h
diff $WT/src/render/FrameRing.h $M/m1/render/FrameRing.h
build m1 $M/m1 $WT/tests/test_frame_ring.cpp; run m1 "formula - framesAgo -> + framesAgo: expect FAIL"
# M3: clamp to frameCount removed (reads cells not yet written in this lifetime)
mkdir -p $M/m3/render; sed 's/if (framesAgo > maxDelay) framesAgo = maxDelay;/(void)maxDelay;/' $WT/src/render/FrameRing.h > $M/m3/render/FrameRing.h
diff $WT/src/render/FrameRing.h $M/m3/render/FrameRing.h
build m3 $M/m3 $WT/tests/test_frame_ring.cpp; run m3 "clamp to frameCount removed: expect FAIL"
# M2: test copy -- the reset does NOT clear written[] (weaker check): expect PASS
sed 's/            std::fill(written.begin(), written.end(), false);/            \/\/ mutation M2: written[] NOT cleared/' $WT/tests/test_frame_ring.cpp > $M/test_frame_ring_m2.cpp
diff $WT/tests/test_frame_ring.cpp $M/test_frame_ring_m2.cpp
build m2 $M/m0 $M/test_frame_ring_m2.cpp; run m2 "reset keeps written[] (test copy): expect PASS"
sha_after=$(shasum -a 256 $WT/src/render/FrameRing.h $WT/tests/test_frame_ring.cpp)
[ "$sha_before" = "$sha_after" ] && echo "deliverables unchanged:" && echo "$sha_after"
