#!/bin/bash
set -e
R2=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/44b528dd-1232-4d5c-a683-0145bc3a700e/scratchpad/ruling2
P=$R2/proto/audio/DeviceGuard.cpp
for m in noProgress noFilter noLiveness lostNever noSettle stoppedBypassSeq; do
  mkdir -p "$R2/mut_$m/audio"; cp "$R2"/proto/audio/* "$R2/mut_$m/audio/"
done
sed 's#if (action == dp::Reapply::AdoptInput && noInputOn_ == scan.lists.inputs)#if (false)#' "$P" > "$R2/mut_noProgress/audio/DeviceGuard.cpp"
sed -e 's#if (!lists.inputs.contains(keep.inputDeviceName))#if (false)#' -e 's#if (!lists.outputs.contains(keep.outputDeviceName))#if (false)#' "$P" > "$R2/mut_noFilter/audio/DeviceGuard.cpp"
sed 's#haveDevice && device->isPlaying(), previous#haveDevice, previous#' "$P" > "$R2/mut_noLiveness/audio/DeviceGuard.cpp"
sed 's#        lostInput_ = ((action#        if (false) lostInput_ = ((action#' "$P" > "$R2/mut_lostNever/audio/DeviceGuard.cpp"
sed 's#startTimer(settleMs_);   // restarts#startTimer(1);   // MUTANT restarts#' "$P" > "$R2/mut_noSettle/audio/DeviceGuard.cpp"
sed 's#if (scan.seq == lastAttemptSeq_)#if (scan.seq == lastAttemptSeq_ \&\& action != dp::Reapply::DeviceStopped)#' "$P" > "$R2/mut_stoppedBypassSeq/audio/DeviceGuard.cpp"
for m in noProgress noFilter noLiveness lostNever noSettle stoppedBypassSeq; do
  printf "%-18s changed lines: " $m; diff "$P" "$R2/mut_$m/audio/DeviceGuard.cpp" | grep -c '^>' || true
done
