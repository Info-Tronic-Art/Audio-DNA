step capt-base bash $SP/run-capt.sh base /Users/boriskarpman/projects/RealTimeAudio/build/AudioDNA_artefacts/Release/Audio-DNA.app
step capt-C0 bash $SP/run-capt.sh C0 $SP/apps/C0/Audio-DNA.app
step canvas-C0 env CANVAS_APP=$SP/apps/C0/Audio-DNA.app bash $WT/.harmony/probe-canvas.sh $O
