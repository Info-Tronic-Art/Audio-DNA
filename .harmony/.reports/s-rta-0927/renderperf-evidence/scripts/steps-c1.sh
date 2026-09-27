C1=$SP/apps/C1/Audio-DNA.app
step t2-C1-1 bash $SP/run-t2.sh C1 $C1 1 71 1920 1080
step t2-C1-2 bash $SP/run-t2.sh C1 $C1 2 72 1920 1080
step t2-C1-3 bash $SP/run-t2.sh C1 $C1 3 73 1920 1080
step t2-C1-4k bash $SP/run-t2.sh C1 $C1 4 74 3840 2160
step rstate-C1-1 env RSTATE_APP=$C1 bash $WT/.harmony/probe-render-state.sh $O
step rstate-C1-2 env RSTATE_APP=$C1 bash $WT/.harmony/probe-render-state.sh $O
step rstate-C1-3 env RSTATE_APP=$C1 bash $WT/.harmony/probe-render-state.sh $O
step parity-C1 env PARITY_APP=$C1 bash $WT/.harmony/probe-effects-parity.sh $O
