M=/Users/boriskarpman/projects/RealTimeAudio
SP=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/73d4f54c-e9d5-409c-8b5d-694bfd57c171/scratchpad
O=$SP/gate/bf10; P=$HOME/.local/opt/projectm-4.1.1-fbo1
echo "== START $(date '+%H:%M:%S')"
rm -rf $SP/apps/pre-bf10.app; cp -R $M/build/AudioDNA_artefacts/Release/Audio-DNA.app $SP/apps/pre-bf10.app
git -C $M commit -q -m "merge(s-rta-1002b): lane/bf10 - MilkDrop fills the composition canvas: libprojectM 4.1.1 (03aa8a7) patched with projectm_opengl_render_frame_fbo, installed to ~/.local/opt/projectm-4.1.1-fbo1 by cmake/projectm/build-projectm.sh; the build refuses a projectM without it; ProjectMSource renders into its canvas-sized FBO (window framebuffer untouched), forces alpha 1, unbinds samplers 0-15, restores GL state on every return; test_projectm_canvas_gl T1-T6b; probe-milkdrop m1-m10 + attach-mode refusal of any app the run did not start; probe-vupload-ab.sh samples re-run launches; Pitfall 66. Doc conflicts resolved (64 / 65 / 66 kept)

Co-Authored-By: Claude Opus 5.5 (1M context) <noreply@anthropic.com>" || { echo COMMIT FAILED; exit 1; }
echo "merged $(git -C $M log --oneline -1 | cut -c1-10)"
while [ -n "$(ps -eo ucomm= | awk '$1=="Audio-DNA"')" ]; do echo "$(date +%T) an Audio-DNA is running -- not rebuilding under it"; sleep 60; done
echo "== G0.1 reconfigure + build $(date '+%H:%M:%S')"; cmake -S $M -B $M/build > $O/cfg.log 2>&1; echo "configure rc $?"; grep -E 'libprojectM:' $O/cfg.log | head -2
cmake --build $M/build -j4 > $O/build.log 2>&1; echo "build rc $?"
echo "== G0.2 $(nm -gU $P/lib/libprojectM-4.dylib | grep -c _projectm_opengl_render_frame_fbo) export"
A=$M/build/AudioDNA_artefacts/Release/Audio-DNA.app/Contents/MacOS/Audio-DNA
echo "== G0.3 rpath: $(otool -l $A | grep -A2 LC_RPATH | grep path | sed 's/^ *//' | tr '\n' ';')"
echo "== G0.4 isystem prefix in ProjectMSource compile cmd: $(python3 -c "
import json;d=json.load(open('$M/build/compile_commands.json'))
print(sum(1 for e in d if e['file'].endswith('ProjectMSource.cpp') and '$P/include' in e['command']))" 2>/dev/null)"
echo "== G0.5 negative control"; D=$M/build/_deps; cmake -S $M -B $SP/build-negctl -DAUDIODNA_PROJECTM_PREFIX=/nonexistent -DCMAKE_BUILD_TYPE=Release -DFETCHCONTENT_FULLY_DISCONNECTED=ON -DFETCHCONTENT_SOURCE_DIR_JUCE=$D/juce-src -DFETCHCONTENT_SOURCE_DIR_HTTPLIB=$D/httplib-src -DFETCHCONTENT_SOURCE_DIR_CATCH2=$D/catch2-src -DFETCHCONTENT_SOURCE_DIR_MELATONIN_INSPECTOR=$D/melatonin_inspector-src -DFETCHCONTENT_SOURCE_DIR_SYPHON=$D/syphon-src > $O/negctl.log 2>&1; echo "negctl rc $? (expect != 0)"; grep -m1 -E 'lacks projectm_opengl_render_frame_fbo|CMake Error' $O/negctl.log | cut -c1-160; rm -rf $SP/build-negctl
echo "== G1.1"; $M/build/tests/test_projectm_canvas_gl > $O/t.log 2>&1; echo "rc $? $(grep -E 'All tests passed|test cases' $O/t.log | tail -1)"
until mkdir /tmp/audiodna-ctest.lock 2>/dev/null; do sleep 15; done
echo "== G1.2 ctest $(date '+%H:%M:%S')"; ctest --test-dir $M/build -j1 > $O/ctest.log 2>&1; rm -rf /tmp/audiodna-ctest.lock
grep -E 'tests passed' $O/ctest.log; ctest --test-dir $M/build -N | tail -1; grep -E '\*\*\*Failed' $O/ctest.log | head
echo "== G2 live $(date '+%H:%M:%S')"
LANE=harmony-bf10 . $SP/lib/lock.sh
acquire_lock || { echo LOCK FAILED; exit 1; }
if [ -n "$(adna)" ]; then echo "foreign Audio-DNA running -- skip live"; else LANE=harmony-bf10 LOCK_LIB=$SP/lib/lock.sh bash $M/.harmony/probe-milkdrop.sh $O/live > $O/g2.log 2>&1; echo "probe rc $?"; fi
release_lock
grep -E 'PASS|FAIL|GREEN|RED' $O/g2.log | tail -6
sleep 16; outwins; $M/.venv/bin/python -c "
import Quartz
wl=Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionAll, Quartz.kCGNullWindowID)
print('UNC', len([w for w in wl if 'UserNotificationCenter' in str(w.get('kCGWindowOwnerName',''))]))"
echo "Audio-DNA after: $(ps -eo ucomm= | awk '$1=="Audio-DNA"' | wc -l | tr -d ' ')"
echo "== DONE $(date '+%H:%M:%S')"
