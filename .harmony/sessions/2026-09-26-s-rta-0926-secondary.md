# Session log — s-rta-0926 (2026-09-26 ~12:55 → ~19:00, SECONDARY, RealTimeAudio)

Profile: MINIMAL foreign-repo secondary. Harmony_Main SYSTEM files touched: NONE. Harmony_Main writes: NONE (up-channel
reply went to this repo's .harmony/idea-ledger.md via idea-capture.sh). Boris directive: "use workflows" (ultracode on).
Down-channel routed item (primary s234): split CLAUDE.md <= 25 KB — DONE. Running log: .harmony/s-rta-0926-work.md.
Reports: .harmony/.reports/s-rta-0926/. Every merge: Harmony rebuilt build/ and ran the live gates herself; every new
probe shown RED on the pre-change build first.

| type | ref | msg |
|---|---|---|
| finding | parity | START HERE #1 "clip+layer effect blank" did NOT reproduce on 4fca2c5 (0/104 frames); s-rta-0925 misread "2 PASS / 3 FAIL" as "2/3 FAIL". Harmony re-ran: 5/0, frame looked at |
| shipped | 8e1906d | probe-step3 polls inputSource after withAudio Play (source switches ~225 ms after Play; was a one-read race) -> 93/0 |
| shipped | 0bf0ad3 | recorder step-7 docs; REST/OSC/test counts derived by command |
| shipped | e1ed9cc | Deck Load sized from its measured label; "No clip selected" drawn below the Dashboard |
| shipped | 0ecd7a7 | ONE scratch-pool rule (pickEffectTarget never writes what a pass reads or a caller holds): crossfade between effected clips now blends (was: held outgoing, hard cut) + 6 more instances of the class; probe-crossfade; hardened probe-effects-parity |
| shipped | 1908413 | TopBar "Master Signal:" whole-word label, readouts on Master Signal + Master, magenta accent, tooltips |
| shipped | bb0a7c0 | CLAUDE.md 112,696 -> ~24 KB + 13 on-demand docs/claude/*.md (no loss: 941 verbatim + 11 declared edits; fresh reader 7/7) |
| shipped | f43b72a 58b14d7 | L-R Routines slice 1: Routine model + bank in the composition, sliceRoutine, compileRoutine, RoutineEngine (next-bar start, restore by default, loop/once, gesture-begin stacking), REST /api/routine/*, OSC /audiodna/routine/{slot}, TriggerRoutine binding, auto-play capture |
| shipped | ebbff22 | every saved take carries the recorder clock's tempo map (was [] in EVERY live take -> no routine could be cut) |
| shipped | e5ceb98 | RecorderClock counts beats from Record (every take's stamps were late by the beat phase at Record, up to ~1 beat); routine Beat edge = tracker's beat |
| shipped | 2bf1d56 | Record tab Routines strip: 8 pads + Save Routine row (from bar, to bar, name) |
| shipped | 7253597 | Manual BPM: detected beats no longer move the phase (bars stretched to 11-13 s when the mic heard music); probe-manual-bpm |
| shipped | 6e8f120 | cleanup: EffectChain mid-chain dry/wet feedback loop; mirror tests drive the real ScratchPool; bar editors clamp >= 1; Deck Save measured |
| gate | ctest | 539 at boot -> 546 -> 556 -> 565 -> 573 -> 580/580 at close (6e8f120) |
| gate | live | probe-crossfade RED 9 FAIL -> 27/0; probe-effects-parity 39/0 -> 46/0; probe-mastersignal 22/0; probe-routines RED 23/46 (pre-1b), 69/5 (stamp bug) -> 74/0 x3 at startBeatInBar 3.45-3.80; probe-step3 94/0; probe-manual-bpm RED 12/6 -> 18/0; probe-resync 16/0; probe-downbeat-level 14/0 (final set on 6e8f120) |
| gate | visual | window-only shots looked at: Deck Load, No clip selected, TopBar faders; crossfade mid frames; routines ref/pert/rest; strip snapshots; critic panels on polish, fader (pinned r2), strip (pinned) |
| finding | t2 | probe-step3 T2 FAILs mid-afternoon were ENVIRONMENTAL (A/B/A e1ed9cc vs 1908413: both fail early, both pass late; Stremio held the built-in speaker) — probe now WARNs on a third-party audio assertion |
| slip | critics | two fader critics judged SOURCE on main / another worktree (my packet named no branch) -> gotcha: pin worktree+branch+commit in every critic/reviewer packet |
| slip | report-lost | a worktree lane's gitignored report vanished with its auto-cleaned worktree; rebuilt from the workflow result -> gotcha: commit reports or return them in full |
| slip | lock | live-lock released without an owner check; one teardown raced another lane's acquire -> owner-checked release |
| slip | sed | my own sed on a copied shot script wiped the line holding ROOT/PY (one lost shot, re-shot) |
| slip | cd | two compound commands cd'd my main loop into a worktree / build dir (harness moved cwd); absolute paths since |
| learning | mirror-gap | three bugs today lived in the gap between a tested pure seam and the untested wiring that feeds it (tempoMap never copied; clock seeded only at phase 0; render aliasing mirror tests). Live probes that assert the FED field on a real recording caught all three |
