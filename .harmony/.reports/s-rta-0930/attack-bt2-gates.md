# ATTACK PAPER — lane bt2, seat gates (s-rta-0930)

Recovered by Harmony from the workflow journal (wf_d5f2da66-bc5): the seat returned its paper as structured output and did not write this file.

VERDICT: AMEND

## A1 [MUST] AdoptInput and InputLost fire regardless of source mode, so a mic appearing or an unused input vanishing while the performer plays a FILE closes and reopens the device mid-show.

EVIDENCE: plan I3/I4 reconcile() takes (haveDevice, openedInput, lists) only; AE:126-157 sourceMode_ is never consulted; file transport is prepared from the device start (V6) so a reopen re-prepares it and may change rate.

FIX: Gate AdoptInput/InputLost on sourceMode==MicInput (or defer until the next switch to mic); add a TDP/AE case with file mode.

## A2 [MUST] InputLost can switch away from an input that still works (list flicker longer than 250 ms, USB interface re-enumerating, rename), contradicting the never-switch-a-working-input goal, and its premise is unverified.

EVIDENCE: V10 chain / I1 are INFERRED; live A10/A11 run against a live device via runtime deny, so they pass whether or not the dead-combiner claim is true; R2 also admits an up-to-2s message-thread stall.

FIX: Ship I4 only after Boris's B2 hardware check, or require the input absent on two consecutive scans / a longer settle; state in G7 that A10 does not test the HAL dead-combiner.

## A3 [MUST] Silent product swap at a gig: a dropped wired mic silently falls back to the laptop mic and never returns; the notice stays hidden and the label only changes in mic mode.

EVIDENCE: hasInputDevice is name-based (AE:96-99) and state reads Ok; refreshAudioDeviceNotice only shows text for NoDevice/NoInput (MC:3075-3090); I5 writes the label only when source==MicInput; Q1 default (a) is stay-on-MacBook.

FIX: Make the fallback visible in the persistent notice (or default Q1 to a non-silent option until Boris answers) and make it show in file mode too.

## A4 [SHOULD] A wired mic plugged in mid-show while on the built-in mic does nothing, and nothing tells the performer; the notice text 'plug one in' is only true for the no-input state.

EVIDENCE: R9 test keeps the working mic; state is Ok so no notice; plan B2 describes only the no-mic case.

FIX: Document the behaviour in the notice/Boris checks; consider a one-line informational label.

## A5 [SHOULD] A re-apply during an active performance take, video recording or AudioTap capture at a possibly changed sample rate is neither tested nor gated.

EVIDENCE: plan touches only a perfRecord comment (MC:5697-5699); CC:160-175 tap push/gap logic is not exercised by any new test.

FIX: Hold re-apply while recording, or add a test that a rate-changing re-apply keeps the take consistent.

## A6 [SHOULD] C3 teeth are thin: AE1 only checks no change message / no XML with NO device open, and live opens counting depends on a synthetic start at addAudioCallback.

EVIDENCE: V1 (ADM:971-985, CC:179) baseline of 1 depends on init-before-addAudioCallback order; any other device call added to setSourceMode on a denied engine may pass AE1.

FIX: Add an independent device-start spy in TDP around setSourceMode on a mock type, and assert the ordering assumption in the probe.

## A7 [SHOULD] The hot-plug stand-in is a policy-level deny of a same-named device, not a new device or the HAL; GREEN live rows overclaim hot-plug coverage, and AE1 builds a real engine on the real HAL with a precheck race and ';'-joined env names.

EVIDENCE: I5, R4, R10, R11; I4 (macOS default published after the list change).

FIX: Label G7 as policy-path only; add a mock test where the new default arrives after the list change; make AE1 skip-by-default or add a deny-all hook.

## A8 [NIT] Timing-fragile probe rows and an uncovered launch-open-failure case.

EVIDENCE: A11a/C8a check at +2 s vs a 250 ms settle under load; A12 passes vacuously on main; removing hadDevice_ changes behaviour when the launch open fails (only lastAttemptSeq_ guards it, no row).

FIX: Use poll-until-stable bars and add an open-failed-at-launch TDP row.

