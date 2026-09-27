BASE=/Users/boriskarpman/projects/RealTimeAudio/build/AudioDNA_artefacts/Release/Audio-DNA.app
step t2-base-1 bash $SP/run-t2.sh base $BASE 1 71 1920 1080
step t2-base-2 bash $SP/run-t2.sh base $BASE 2 72 1920 1080
step t2-base-3 bash $SP/run-t2.sh base $BASE 3 73 1920 1080
step t2-base-4k bash $SP/run-t2.sh base $BASE 4 74 3840 2160
step rstate-r1counts-base env RSTATE_APP=$BASE bash $WT/.harmony/probe-render-state.sh $O r1_counts
step capt-base-i env SKIPQUIET= bash $SP/run-capt.sh base-i $BASE
