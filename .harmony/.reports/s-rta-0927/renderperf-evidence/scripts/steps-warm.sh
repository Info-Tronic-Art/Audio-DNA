BASE=/Users/boriskarpman/projects/RealTimeAudio/build/AudioDNA_artefacts/Release/Audio-DNA.app
C1=$SP/apps/C1/Audio-DNA.app
step warm-C1-1 env T2PY=t2warm.py T2DIR=t2warm bash $SP/run-t2x.sh C1 $C1 1 71 1920 1080
step warm-C1-2 env T2PY=t2warm.py T2DIR=t2warm bash $SP/run-t2x.sh C1 $C1 2 72 1920 1080
step warm-C1-3 env T2PY=t2warm.py T2DIR=t2warm bash $SP/run-t2x.sh C1 $C1 3 73 1920 1080
step warm-C1-4k env T2PY=t2warm.py T2DIR=t2warm bash $SP/run-t2x.sh C1 $C1 4 74 3840 2160
step warm-base-1 env T2PY=t2warm.py T2DIR=t2warm bash $SP/run-t2x.sh base $BASE 1 71 1920 1080
step warm-base-2 env T2PY=t2warm.py T2DIR=t2warm bash $SP/run-t2x.sh base $BASE 2 72 1920 1080
step rstate-r1counts-C1-alone env RSTATE_APP=$C1 bash $WT/.harmony/probe-render-state.sh $O r1_counts
