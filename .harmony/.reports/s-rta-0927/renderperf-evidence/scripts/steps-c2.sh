C2=$SP/apps/C2/Audio-DNA.app
step capt-C2 bash $SP/run-capt.sh C2 $C2
step canvas-C2 env CANVAS_APP=$C2 bash $WT/.harmony/probe-canvas.sh $O
step fitmode-C2 env FIT_APP=$C2 bash $WT/.harmony/probe-fitmode.sh $O
step rstate-C2 env RSTATE_APP=$C2 bash $WT/.harmony/probe-render-state.sh $O
