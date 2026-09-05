# S166-L5a — An "8 beats" LFO never completes a cycle. Fix the phase, and make it gateable.

REPORT_FILE: /Users/boriskarpman/projects/RealTimeAudio/.harmony/.reports/s166/builder-L5a-bar-fold.md

## YOU OWN THE BUILD SLOT (once I say go — I will message you)
Do not run `cmake` or `ctest` until I message you that the slot is free; another gate is
using the TSan build tree right now. Read, verify and EDIT freely in the meantime.

## THE DEFECT — confirmed from source by an independent reviewer, not inherited folklore
`OscillatorSignal::getValue` (`src/signal/OscillatorSignal.h`, the `getValue` override —
RE-GREP BY ANCHOR TEXT) computes:
```
float totalBeatPhase = snapshot.beatPhase + static_cast<float>(snapshot.beatInBar);
float cyclePhase = std::fmod(totalBeatPhase / beatDuration_, 1.0f);
```
`beatInBar` is 0–3, so `totalBeatPhase` is bounded to **[0, 4) — one 4/4 bar**. The UI offers
`beatDuration_` values of 0.25, 0.5, 1, 2, 4 and **8**. For `beatDuration_ = 8`, `cyclePhase`
can only ever reach `4/8 = 0.5` before `beatInBar` rolls over and the total resets near zero.
**So an "8 beat" oscillator never completes a cycle at all — it retraces the first half of its
waveform, once per bar, forever.** It is not "twice as fast", which is how an earlier design note
described it; the reviewer corrected that and you should trust the corrected description.
`EnvelopeSignal` (`src/signal/EnvelopeSignal.h`) builds `totalBeatPhase` the same way and has the
same defect for any `beatDuration_ > 4`. Its default is 4.0, which is exactly the boundary case
that still works — so the bug is invisible until someone picks a longer period.

## THE FIX IS ALREADY PAID FOR — the data exists
`FeatureSnapshot` (`src/analysis/FeatureSnapshot.h`) ALREADY publishes `uint16_t barCount`
("bars since last phrase reset") alongside `beatInBar` and `barPhase`. **No change to the audio
analysis pipeline or to `FeatureBus` is needed or permitted** — that component is explicitly
hardened (zero TSan suppressions) and is not in your fence. Use what is already in the snapshot.

The phase should advance across bars, e.g. `beatPhase + beatInBar + 4 * barCount`, before the
divide. Derive the exact expression yourself and justify it.

## THE JUDGMENT CALL YOU MUST NAME, NOT SILENTLY DECIDE
`barCount` is "bars since last **phrase** reset" — it RESETS (default phrase length is 8 bars,
configurable 1–32) and it is a `uint16_t` that can wrap. So a naive `4 * barCount` makes the
oscillator **jump** at every phrase reset, unless the cycle length divides the phrase length
evenly. With the default 8-bar phrase and an 8-beat (2-bar) cycle it happens to land exactly on a
cycle boundary — but with a 5-bar phrase, or a 4-beat cycle against a 3-bar phrase, it will jump
mid-waveform, visibly, in front of an audience.
**Report the options and your recommendation; implement the one you can defend.** Consider at
least: accepting the jump (and saying so), deriving phase from `phrasePhase` instead, or keeping
a monotonic bar counter. Do NOT change what the analysis publishes. If you conclude the honest
fix needs data that does not exist yet, say so and implement the best bounded improvement — a
cycle that completes and jumps rarely is strictly better than one that never completes at all,
but I want the trade named in your report, not buried.

## SECOND DELIVERABLE — make the tempo layer gateable
`/api/inject_features` (`src/api/ApiServer.cpp`, the `handleInjectFeatures` body — find it by
anchor text) accepts `beatPhase`, `barPhase`, `phrasePhase`, `bpm` and others, but **NOT
`beatInBar` and NOT `barCount`** — the two fields this bug is about. Add them, in exactly the
style of the existing lines, clamped to their real ranges (`beatInBar` 0–3; `barCount` to
`uint16_t`). Follow the existing clamping precedent in that function (see the `structuralState`
and `detectedGenre` lines, which cite a thread-safety design doc). Without this, neither I nor
any future session can prove this fix from outside the app — that is the actual reason it is in
this packet.

## WHAT DONE MEANS
1. An oscillator with `beatDuration_ = 8` completes a FULL cycle over 8 beats.
2. Values at 0.25, 0.5, 1, 2 and 4 beats are UNCHANGED — this must not alter the shapes that work
   today. Prove it with unit tests over several bars, not one.
3. `EnvelopeSignal`'s equivalent defect is fixed the same way, or you explain why it must differ.
4. `beatInBar` and `barCount` are injectable and clamped.
5. Unit tests cover: a full 8-beat cycle reaching both extremes; the short durations unchanged;
   and whatever behaviour you chose at a phrase reset — a test that PINS your decision, so the
   next person who changes it has to face it.

## FENCE — you may edit ONLY these
- `src/signal/OscillatorSignal.h`
- `src/signal/EnvelopeSignal.h`
- `src/api/ApiServer.cpp` (the inject handler ONLY — no other endpoint, no other behaviour)
- a test file under `tests/` and its `tests/CMakeLists.txt` entry
Anything else — and especially anything under `src/analysis/` — STOP and report.

## BUILD AND TEST RULES — this repo's documented false-green traps
- **Capture the build exit code and READ it before quoting any test number.** Do NOT pipe the
  build through `tail`/`tee` and read `$?` — that reads the pipe's exit code, not the compiler's.
  zsh has no `PIPESTATUS`. Redirect to a file and read `$?` immediately. A builder hit exactly
  this trap twice earlier today and caught it; do not be the one who does not.
- ctest baseline is **233/233** at HEAD `f53a8f1`, produced on a clean serialized build. Re-run
  it; never inherit it. Build dir `build`, Release.
- Prove your new tests load-bearing: neutralize the fix, rebuild, confirm the 8-beat test FAILS,
  restore byte-identically (md5), confirm it PASSES. Report both numbers.
- Do NOT launch the app. I gate it — and for this lane I have a real oracle: `/api/signals`
  reports every signal's live cached value, so with your injection fields I can sweep phase from
  outside and watch the waveform complete.

## REPORTING
STATUS, your phase expression and why, the phrase-reset trade-off and your choice, the injection
fields added, build exit code, before/after ctest, load-bearing proof, PACKET QUALITY, and
anything outside the fence.
