C3=$SP/apps/C3/Audio-DNA.app
step capt-C3 bash $SP/run-capt.sh C3 $C3
step canvas-C3 env CANVAS_APP=$C3 bash $WT/.harmony/probe-canvas.sh $O
step parity-C3 env PARITY_APP=$C3 bash $WT/.harmony/probe-effects-parity.sh $O
step rstate-C3 env RSTATE_APP=$C3 bash $WT/.harmony/probe-render-state.sh $O
step outputs-C3 env OUTP_APP=$C3 bash $WT/.harmony/probe-outputs.sh $O
step crossfade-C3 env XFADE_APP=$C3 bash $WT/.harmony/probe-crossfade.sh $O
step fitmode-C3 env FIT_APP=$C3 bash $WT/.harmony/probe-fitmode.sh $O
step deckclock-C3 env DCLOCK_APP=$C3 bash $WT/.harmony/probe-deck-clock.sh $O
