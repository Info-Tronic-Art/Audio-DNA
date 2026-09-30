#!/bin/bash
# usage: run_batch.sh <asan|tsan> <batchname> "<n>:<scenario> <n>:<scenario> ..."
ARM=$1; BATCH=$2; SPEC=$3
S=${SWEEP_SP:-/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/44b528dd-1232-4d5c-a683-0145bc3a700e/scratchpad}
SW=${SWEEP_DIR:-$S/sweep}
LANE=${LANE:-sweep}
. ${LOCK_LIB:-$S/lib/lock.sh}
PY=/Users/boriskarpman/projects/RealTimeAudio/.venv/bin/python
if [ $ARM = asan ]; then APP=${ASAN_APP:?set ASAN_APP}; HLIM=90
else APP=${TSAN_APP:?set TSAN_APP}; HLIM=150; fi
BLOG=$SW/batch-$BATCH.log
exec > >(tee -a $BLOG) 2>&1
unc_all(){ $PY -c "
import Quartz
wl = Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionAll, Quartz.kCGNullWindowID)
print(len([w for w in wl if 'UserNotificationCenter' in str(w.get('kCGWindowOwnerName',''))]))"; }
audio_check(){  # 0 = all defaults built-in
  local d; d=$(system_profiler SPAudioDataType 2>/dev/null | grep -B2 -A6 Default)
  local bt; bt=$(system_profiler SPBluetoothDataType 2>/dev/null | grep -m1 State)
  local nd; nd=$(echo "$d" | grep -c "Default .*Device: Yes")
  local nb; nb=$(echo "$d" | grep -i "Transport:" | grep -vc "Built-in")
  echo "$(date +%T) audio: defaults=$nd non-builtin-transport=$nb bt[$bt] $(echo "$d" | grep -E 'Default|Transport' | tr -s ' ' | tr '\n' ';')"
  [ "$nd" -ge 2 ] && [ "$nb" -eq 0 ]
}
acquire_lock || exit 1
trap 'release_lock' EXIT
BSTART="$(date '+%Y-%m-%d %H:%M:%S')"; echo "=== batch $BATCH arm=$ARM start $BSTART"
echo "$BSTART" > $SW/batch-$BATCH.start
ls ~/Library/Logs/DiagnosticReports | grep -i "^Audio-DNA" | sort > $SW/ips-before-$BATCH.txt
echo "UNC windows (OptionAll) before: $(unc_all)"
audio_check || { echo "STOP: non-built-in default audio device (or no defaults) -- no launch"; exit 3; }
for item in $SPEC; do
  N=${item%%:*}; SC=${item##*:}
  RD=$SW/runs/$ARM-$N; rm -rf $RD; mkdir -p $RD
  echo "--- $ARM-$N scenario $SC  $(date +%T)"
  audio_check > $RD/audio-precheck.txt || { cat $RD/audio-precheck.txt; echo "STOP: audio device not built-in before $ARM-$N"; echo -e "$ARM\t$N\t$SC\tSKIPPED-BT\t" >> $SW/launches.tsv; break; }
  [ -n "$(adna)" ] && { echo "REFUSE: Audio-DNA running"; break; }
  BAD=0; for p in 7070 8080; do lsof -nP -iTCP:$p -sTCP:LISTEN >/dev/null 2>&1 && { echo "REFUSE: $p listener"; BAD=1; }; done; [ $BAD = 1 ] && break
  if [ $ARM = asan ]; then ENVOPT="ASAN_OPTIONS=halt_on_error=1:abort_on_error=0:detect_leaks=0:log_path=$RD/asan"
  else ENVOPT="TSAN_OPTIONS=halt_on_error=0:abort_on_error=0:exitcode=0:report_signal_unsafe=0:log_path=$RD/tsan"; fi
  T0=$(date +%s)
  open -g --env "$ENVOPT" --stdout $RD/app-out.log --stderr $RD/app-err.log "$APP"
  H=""; GONE=0
  for i in $(seq 1 $HLIM); do H=$(curl -s --max-time 3 -H 'Connection: close' http://127.0.0.1:7070/api/health); [ -n "$H" ] && break; [ -z "$(adna)" ] && [ $i -gt 3 ] && { GONE=1; break; }; sleep 1; done
  T1=$(date +%s); echo "health after $((T1-T0)) s gone=$GONE: ${H:0:120}"
  echo "$H" > $RD/health-start.json
  sleep 2
  L7070=$(lsof -nP -iTCP:7070 -sTCP:LISTEN 2>/dev/null | awk 'NR>1{print $1}' | head -1)
  echo "7070 listener: $L7070; pids: $(adna | tr '\n' ' '); outwins: $(outwins)"
  if [ -n "$H" ] && [ "$L7070" = "Audio-DNA" ]; then
    perl -e 'alarm 240; exec @ARGV' $PY $SW/scen.py $SC $RD </dev/null > $RD/scen.out 2>&1; echo "scen rc=$?"
  else echo "no scenario (app not up)"; fi
  ALIVE=$([ -n "$(adna)" ] && echo yes || echo no)
  HEND=$(curl -s --max-time 4 -H 'Connection: close' http://127.0.0.1:7070/api/health); echo "$HEND" > $RD/health-end.json
  OW=$(outwins); echo "alive at end: $ALIVE; outwins: $OW; health end: ${HEND:0:100}"
  quit_app | tee $RD/quit.txt
  echo "outwins after quit: $(outwins)"
  NREP=$(ls $RD | grep -cE '^(asan|tsan)\.'); echo "sanitizer files: $NREP"
  echo -e "$ARM\t$N\t$SC\thealthS=$((T1-T0))\tgone=$GONE\talive_end=$ALIVE\tsanfiles=$NREP\t$OW" >> $SW/launches.tsv
  sleep 3
done
LASTQUIT=$(date +%s); echo "last quit $(date +%T)"
sleep 16
echo "UNC windows (OptionAll) after: $(unc_all)"
ls ~/Library/Logs/DiagnosticReports | grep -i "^Audio-DNA" | sort > $SW/ips-after-$BATCH.txt
echo "new .ips: $(comm -13 $SW/ips-before-$BATCH.txt $SW/ips-after-$BATCH.txt | tr '\n' ' ')"
echo "=== batch $BATCH done $(date +%T)"
release_lock; trap - EXIT
