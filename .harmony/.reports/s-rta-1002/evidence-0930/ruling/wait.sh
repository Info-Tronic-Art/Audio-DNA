#!/bin/bash
D=/Users/boriskarpman/projects/RealTimeAudio/.harmony/.reports/s-rta-0930
A=$D/attack-bt2-coreaudio.md
B=$D/attack-bt2-gates.md
start=$(date +%s)
prevA=-1; prevB=-1; stable=0
while :; do
  now=$(date +%s)
  if [ $((now-start)) -ge 2700 ]; then echo "TIMEOUT after $((now-start))s"; ls -la $D; exit 2; fi
  if [ -f "$A" ] && [ -f "$B" ]; then
    sA=$(stat -f %z "$A"); sB=$(stat -f %z "$B")
    if [ "$sA" = "$prevA" ] && [ "$sB" = "$prevB" ]; then stable=$((stable+5)); else stable=0; fi
    prevA=$sA; prevB=$sB
    if [ $stable -ge 30 ]; then echo "BOTH PRESENT+STABLE after $((now-start))s: A=$sA B=$sB"; ls -la $D; exit 0; fi
  fi
  /bin/sleep 5
done
