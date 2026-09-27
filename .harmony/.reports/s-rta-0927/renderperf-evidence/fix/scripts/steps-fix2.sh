FX=$SP/fix/apps/fix/Audio-DNA.app
step fitmode-FIX env FIT_APP=$FX bash $WT/.harmony/probe-fitmode.sh $O
step crossfade-FIX env XFADE_APP=$FX bash $WT/.harmony/probe-crossfade.sh $O
step deckclock-FIX env DCLOCK_APP=$FX bash $WT/.harmony/probe-deck-clock.sh $O
step outputs-FIX env OUTP_APP=$FX bash $WT/.harmony/probe-outputs.sh $O
