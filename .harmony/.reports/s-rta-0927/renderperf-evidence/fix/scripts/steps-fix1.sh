FX=$SP/fix/apps/fix/Audio-DNA.app
step capt-FIX env SKIPQUIET=1 bash $SP/run-capt.sh FIX $FX
step canvas-FIX env CANVAS_APP=$FX bash $WT/.harmony/probe-canvas.sh $O
step parity-FIX env PARITY_APP=$FX bash $WT/.harmony/probe-effects-parity.sh $O
step rstate-FIX env RSTATE_APP=$FX bash $WT/.harmony/probe-render-state.sh $O
