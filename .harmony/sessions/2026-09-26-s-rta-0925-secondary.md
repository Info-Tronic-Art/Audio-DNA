# Session log — s-rta-0925 (2026-09-25 18:23 → 2026-09-26 ~13:30, SECONDARY, RealTimeAudio)

Profile: MINIMAL foreign-repo secondary. Harmony_Main SYSTEM files touched: NONE. Harmony_Main writes: NONE.
Boris directives: "Use workflows"; Boris calls 1-10 answered (binding-decisions.md 2026-09-25); "remove the video
slider keep master" + top-right fader linked + Master Signal slider + right-click bug; "keep gain visible"; Master
Signal Q2 "keep pulsing, this control is only for signals", Q3 "save it"; removed Xcode for disk space (rig fact);
"work till you have used 50% ctx then eos". Running log: .harmony/s-rta-0925-work.md. Reports: .harmony/.reports/s-rta-0925/.

| type | ref | msg |
|---|---|---|
| shipped | 611de65 00a0e53 | downbeatDetected is a beat-long LEVEL (premise of a one-hop pulse refuted by measurement); contract+tests+/api/bpm fields+probe-downbeat-level |
| shipped | 5e9d973 b6d5eba | Record panel polish: notice keyed to situation, instant status, measured tab widths, disabled tooltips |
| shipped | bbac78a | one master: TopBar fader is a view of masterOpacity (was a 2nd independent dimmer); Video Opacity twin removed; Composition right-click reset |
| shipped | 4dc8a77 | right-click reconcile (hasDefaultValue + tests) |
| shipped | 6d55fc9 | Per-Type Autopilot checkbox overlap (shared layout fn); tab renamed "Compositions" |
| shipped | 85afb70 + rr-fix | replay snap-back: checkpoint 0 as Program preamble fired at Play (Boris call 1); preamble skipped opacity-at-default bug fixed |
| shipped | lane/0925-mastersignal-s1 | Master Signal: step0 effect/source-param twins render-live; step1 post-analysis depth fader (REST/OSC/binding/persisted); Gain untouched |
| shipped | 52cd76c | clip transform + effect at opacity 1 rendered blank (FBO feedback loop) FIXED |
| shipped | replayend + probe | end of replay holds last look, input back to live (Boris call 3); backwards-seek pins |
| shipped | resync | manual Resync re-aligns tempo oscillators (Boris call 9); /api/resync + OSC /audiodna/resync |
| shipped | flakythumb | ThumbnailCache tests shared a scratch dir under -j8 (27% repro) -> per-process RAII dir |
| gate | ctest | 445 at boot -> 539/539 at close |
| gate | live | probe-step3 69/0 -> 79/0 (snap-back) -> 93/0 (end) -> 92/1 at close (inputSource timing row, see loose ends); probe-mastersignal 22/0 x3 (pixel oracle); probe-resync 16/0; probe-onset-render 13/0; probe-downbeat-level 14/0; every new probe shown RED on the pre-change build first |
| gate | visual | window-only shots + 3-seat critic panel; two MUSTs fixed and re-shot (Per-Type overlap, Comp/Decks abbreviation) |
| open | parity | 1 clip effect + 1 layer effect at opacity 1 renders BLANK; ms-white2 writeFBO fix did NOT fix it live (probe-effects-parity 2/3 FAIL before and after). START HERE |
| incident | touchid | builder ran lldb on a unit test -> Touch ID dialog on Boris's screen; rule: no debugger on ANY binary |
| incident | fullscreen | shooter took a full-screen "safety" capture; rule: agents use the Quartz window list, never full-screen |
| incident | disk | ~10 worktree lanes (+scratch builds) filled the disk to 0 bytes; hooks failed closed; Boris cleaned up; rule in gotchas.md |
| incident | xcode | Boris removed Xcode; build/ recovered to CLT SDK via cmake -U + CMakeFiles/4.2.3 reset |
| slip | overlap | two parallel lanes fixed the same right-click bug (packets did not fence scope) |
| slip | md5 | a render-gate row compared PNG file md5s and passed on two all-zero frames; pixel oracle now |
| slip | probes | the s-rta-0924b privacy habit had not landed in the probe scripts (open without -g, full-screen capture) until this session |
| learning | receiver-verify | three "PASS" reviews were wrong about live behaviour (preamble opacity, replay-end probe, blank frames); the live gate caught each — never merge on review alone |
