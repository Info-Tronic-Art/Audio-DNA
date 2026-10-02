LANE=harmony-gate
. /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/267d1ba0-b658-4610-9f47-ef72f4b49872/scratchpad/lib/lock.sh
export TSAN_APP_main=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/267d1ba0-b658-4610-9f47-ef72f4b49872/scratchpad/build-tsan-main/AudioDNA_artefacts/RelWithDebInfo/Audio-DNA.app
acquire_lock || exit 70
bash /Users/boriskarpman/projects/RealTimeAudio/.harmony/probe-tsan.sh /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/267d1ba0-b658-4610-9f47-ef72f4b49872/scratchpad/gate/tsan/g3 "29:d@main 30:d@main 31:d@main" > /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/267d1ba0-b658-4610-9f47-ef72f4b49872/scratchpad/gate/tsan/g3-extra.log 2>&1; echo "rc=$?"
quit_app >/dev/null; release_lock
