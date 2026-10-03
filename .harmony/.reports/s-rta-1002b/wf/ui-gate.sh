M=/Users/boriskarpman/projects/RealTimeAudio
SP=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/73d4f54c-e9d5-409c-8b5d-694bfd57c171/scratchpad
O=$SP/gate/ui
echo "== START $(date '+%H:%M:%S')"
rm -rf $SP/apps/pre-ui.app; cp -R $M/build/AudioDNA_artefacts/Release/Audio-DNA.app $SP/apps/pre-ui.app
git -C $M commit -q -m "merge(s-rta-1002b): lane/ui - clip file info (codec, size, frame rate) in the Clip inspector and the cell tooltip + Show in Finder (cell menu, inspector button); double-click the showing deck tab renames it in place (Enter / Tab / click-away keep, Esc cancels, empty keeps the old name; one Undo step); TEST-ONLY debug routes + probe-ui-files-rename.sh; Pitfall 65. Doc conflicts resolved (pitfalls 64 + 65 both kept)

Co-Authored-By: Claude Opus 5.5 (1M context) <noreply@anthropic.com>" || { echo COMMIT FAILED; exit 1; }
echo "merged $(git -C $M log --oneline -1 | cut -c1-10)"
while [ -n "$(ps -eo ucomm= | awk '$1=="Audio-DNA"')" ]; do echo "$(date +%T) an Audio-DNA is running -- not rebuilding under it"; sleep 60; done
for f in src/media/VideoInfo.h src/media/VideoPlayer.h src/media/VideoPlayer.cpp src/ui/ClipMediaText.h src/ui/ClipCell.h src/ui/ClipCell.cpp src/ui/DeckTabRow.h src/ui/DeckView.h src/ui/DeckView.cpp src/ui/ClipInspector.h src/ui/ClipInspector.cpp src/MainComponent.h src/MainComponent.cpp src/api/ApiServer.h src/api/ApiServer.cpp; do touch $M/$f; done
echo "== G0/G1 build $(date '+%H:%M:%S')"; cmake --build $M/build -j4 > $O/build.log 2>&1; echo "build rc $?"
echo "G0 warnings naming touched files:"; grep 'warning:' $O/build.log | grep -oE 'src/(media/VideoInfo.h|media/VideoPlayer\.(h|cpp)|ui/ClipMediaText.h|ui/ClipCell\.(h|cpp)|ui/DeckTabRow.h|ui/DeckView\.(h|cpp)|ui/ClipInspector\.(h|cpp)|MainComponent\.(h|cpp)|api/ApiServer\.(h|cpp)):' | sort | uniq -c
echo "== G1 executables"; for t in test_video_info test_clip_media_text test_clip_cell_media test_clip_inspector_media test_deck_tab_rename test_deck_tab_row test_undo_commands test_clip_inspector_paint_key; do r=$($M/build/tests/$t 2>&1 | grep -E 'All tests passed|test cases:' | tail -1); echo "$t: $r"; done
until mkdir /tmp/audiodna-ctest.lock 2>/dev/null; do sleep 15; done
echo "== G1 ctest $(date '+%H:%M:%S')"; ctest --test-dir $M/build -j1 > $O/ctest.log 2>&1; rm -rf /tmp/audiodna-ctest.lock
grep -E 'tests passed' $O/ctest.log; ctest --test-dir $M/build -N | tail -1; grep -E '\*\*\*Failed' $O/ctest.log | head; echo "probe_deck_tab_dispatch in ctest: $(ctest --test-dir $M/build -N | grep -c probe_deck_tab_dispatch)"
echo "== G2"; for t in test_hot_thread_io_lint test_render_thread_lint test_log_line_lint test_shared_field_types; do $M/build/tests/$t > /dev/null 2>&1; echo "$t rc $?"; done
echo "== G2b $(grep -rn 'AUDIODNA_DEBUG_SHOW' $M/src $M/tests | wc -l | tr -d ' ') hits"
echo "== G3 + G4 live $(date '+%H:%M:%S')"
LANE=harmony-ui . $SP/lib/lock.sh
acquire_lock || { echo "LOCK FAILED"; exit 1; }
[ -n "$(adna)" ] && { echo "an Audio-DNA is running -- not ours: skip live"; release_lock; exit 0; }
bash $M/.harmony/probe-ui-files-rename.sh $O/live --shots > $O/g3.log 2>&1; echo "probe rc $?"
release_lock
tail -5 $O/g3.log; grep -cE 'PASS' $O/g3.log; grep -E 'FAIL' $O/g3.log | head -8
ls $O/live/*.png 2>/dev/null | wc -l
sleep 16; outwins; $M/.venv/bin/python -c "
import Quartz
wl=Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionAll, Quartz.kCGNullWindowID)
print('UNC', len([w for w in wl if 'UserNotificationCenter' in str(w.get('kCGWindowOwnerName',''))]))"
echo "Audio-DNA after: $(ps -eo ucomm= | awk '$1=="Audio-DNA"' | wc -l | tr -d ' ')"
echo "== DONE $(date '+%H:%M:%S')"
