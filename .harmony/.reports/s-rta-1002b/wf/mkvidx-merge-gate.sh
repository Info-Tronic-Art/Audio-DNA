M=/Users/boriskarpman/projects/RealTimeAudio
SP=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/73d4f54c-e9d5-409c-8b5d-694bfd57c171/scratchpad
O=$SP/gate/mkvidx
echo "== START $(date '+%H:%M:%S')"
git -C $M add -f .harmony/s-rta-1002b-work.md .harmony/.reports/s-rta-1002b/review-mkvidx-*.md 2>/dev/null; git -C $M commit -q -m "docs(s-rta-1002b): work log + mkvidx reviews r1 / r2

Co-Authored-By: Claude Opus 5.5 (1M context) <noreply@anthropic.com>"
PRE=$(git -C $M rev-parse --short HEAD); rm -rf $SP/apps/pre-mkvidx.app; cp -R $M/build/AudioDNA_artefacts/Release/Audio-DNA.app $SP/apps/pre-mkvidx.app; echo "pre-merge app copy = $PRE (src fa9604d)"
git -C $M merge --no-ff lane/mkvidx -q -m "merge(s-rta-1002b): lane/mkvidx - Matroska / WebM reverse and ping-pong match the same stream in MP4: the keyframe model follows the demuxer's live index (re-read at the top of decodeStep, rebuilt only when it changed, never for intra-only streams); intra-only needs every keyframe one frame apart (a front-Cues long-GOP MKV no longer freezes in reverse); a run's seek aims at the frame's middle (HAP at 1/600 reverses fully); probe u13_container_reverse + ab rules with per-rule selftest teeth; Pitfall 64

Co-Authored-By: Claude Opus 5.5 (1M context) <noreply@anthropic.com>" || { echo MERGE FAILED; git -C $M status --short | head; exit 1; }
echo "merged $(git -C $M log --oneline -1 | cut -c1-12); code diff vs lane head: [$(git -C $M diff lane/mkvidx HEAD --stat -- src tests .harmony/*.py .harmony/*.json docs CLAUDE.md | tail -1)]"
while [ -n "$(ps -eo ucomm= | awk '$1=="Audio-DNA"')" ]; do echo "$(date +%T) an Audio-DNA is running (Boris?) -- not rebuilding under it"; sleep 60; done
echo "== G1 build $(date '+%H:%M:%S')"; cmake --build $M/build -j4 > $O/build.log 2>&1; echo "build rc $?"
until mkdir /tmp/audiodna-ctest.lock 2>/dev/null; do sleep 15; done
echo "== G1 ctest $(date '+%H:%M:%S')"; ctest --test-dir $M/build -j1 > $O/ctest.log 2>&1; rm -rf /tmp/audiodna-ctest.lock
grep -E 'tests passed' $O/ctest.log; ctest --test-dir $M/build -N | tail -1; grep -E '\*\*\*Failed' $O/ctest.log | head
echo "== G3 $(date '+%H:%M:%S')"; $M/build/tests/test_gop_cache_store "mkvidx*" -s 2>&1 | grep -E '^mkvidx|^  ?mkvidx|keyRels|rebuilds|test cases|All tests' | head -40 > $O/g3-mkvidx.txt; tail -3 $O/g3-mkvidx.txt
$M/build/tests/test_gop_cache_store "gop2*" -s 2>&1 > $O/g3-gop2.txt; grep -E 'test cases|All tests' $O/g3-gop2.txt
echo "== G4 $(date '+%H:%M:%S')"; $M/.venv/bin/python $M/.harmony/probe-vupload-ab.py --selftest 2>&1 | tail -2
echo "== G5"; for f in $(git -C $M diff --name-only $PRE HEAD -- tests/fixtures); do p=$(shasum -a 256 $M/$f | cut -c1-12); n=$(grep -c "$p" $M/.harmony/.reports/s-rta-1002b/ruling-mkvidx.md); echo "$f $p in-ruling:$n"; done
echo "== G8"; V=$M/src/media/VideoPlayer.cpp; echo "i $(grep -c '+ 0.5 \* frameDur_ - runSeekBackSec_' $V) ii-a $(grep -c 'seekToTimestamp(ptsOfRel(nextRel));' $V) ii-b $(grep -c 'seekToTimestamp(ptsOfRel(target));' $V) iii [$(grep -rn -e keyRels_ -e gopFramesEst_ -e keyIndexEntries_ -e keyIndexFirstTs_ -e keyIndexLastTs_ $M/src | grep -v -e 'src/media/VideoPlayer.cpp:' -e 'src/media/VideoPlayer.h:' | wc -l | tr -d ' ')]"; awk '/VideoPlayer::decodeStep\(/{f=1} f&&/[;{]/{n++} f&&n>=2{print "iv first stmt: " $0; exit}' $V | head -2
echo "== G9 CLAUDE.md $(wc -c < $M/CLAUDE.md) B; rendering stale $(grep -c 'MKV / WebM hold a 1-entry index after open' $M/docs/claude/rendering.md); pitfall stale $(grep -c '(MKV / WebM keep a 1-entry index after open' $M/docs/claude/pitfalls.md); P64 $(grep -c '^64\.' $M/docs/claude/pitfalls.md) idx64 $(grep -c '^64\. ' $M/CLAUDE.md)"
echo "probe-vupload.json blob at HEAD: $(git -C $M rev-parse HEAD:.harmony/probe-vupload.json)"
echo "== G2 TSan $(date '+%H:%M:%S')"; B=$SP/build-tsan-main; D=$M/build/_deps
[ -d $B ] || cmake -S $M -B $B -DCMAKE_BUILD_TYPE=RelWithDebInfo -DADNA_SANITIZE=thread -DAUDIODNA_BUILD_TEST_SERVER=ON -DFETCHCONTENT_FULLY_DISCONNECTED=ON -DFETCHCONTENT_SOURCE_DIR_JUCE=$D/juce-src -DFETCHCONTENT_SOURCE_DIR_HTTPLIB=$D/httplib-src -DFETCHCONTENT_SOURCE_DIR_CATCH2=$D/catch2-src -DFETCHCONTENT_SOURCE_DIR_MELATONIN_INSPECTOR=$D/melatonin_inspector-src -DFETCHCONTENT_SOURCE_DIR_SYPHON=$D/syphon-src > $O/tsan-cfg.log 2>&1
cmake --build $B -j4 --target test_gop_cache_store test_video_decode_trace test_gop_cache > $O/tsan-build.log 2>&1; echo "tsan build rc $?"
for t in test_gop_cache_store test_video_decode_trace test_gop_cache; do TSAN_OPTIONS=halt_on_error=0:abort_on_error=0 $B/tests/$t > $O/tsan-$t.log 2>&1; echo "$t rc $? warnings $(grep -c 'WARNING: ThreadSanitizer' $O/tsan-$t.log) $(grep -E 'test cases' $O/tsan-$t.log | tail -1)"; done
echo "== DONE $(date '+%H:%M:%S')"
