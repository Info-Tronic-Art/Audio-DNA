# Session s-rta-0928b — secondary (MINIMAL, foreign repo), 2026-09-28 17:14 → 2026-09-29 ~07:45

role: secondary · profile: MINIMAL · repo: ~/projects/RealTimeAudio · main loop: Opus 5.5 (1M) · ultracode (workflows)
boot HEAD: b9c9ab2 · close HEAD: see git log (all pushed) · running log: .harmony/s-rta-0928b-work.md
reports/evidence: .harmony/.reports/s-rta-0928b/ (5 plans + adoptions/addenda, 10 attack papers, 2 diagnoses + tools, reviews,
critics, audits, Boris page)

## Rows
| t | kind | item | result |
|---|---|---|---|
| 17:20 | launch | diag-idle, diag-media, plan seqvram (Fable), lane sampleat | 4 workflows |
| 18:03 | merge | lane/sampleat -> 5b78d43 | Harmony teeth: pre-fix rule FAILs 2 new tests, always-nominal FAILs 1; ctest 848 |
| 20:18 | diag | diag-media (S1b video re-seek loop 14/3.8 fps x 3.4 s; S2 drop black 5-14 frames; ...) | audit r2 SOUND_WITH_GAPS |
| 22:47 | merge | lane/seqvram -> a066f00 (+ Harmony fix round F1-F9) | RED absence; GREEN 66/0 x2; ctest 869; Pitfall 54 |
| 23:25 | diag | diag-idle (whole-window CoreGraphics union repaint by 4 animating widgets) | audit SOUND_WITH_GAPS |
| 01:10 | merge | lane/mediaopen -> 6495063 (async loads deferred) | RED 5/18; GREEN 38/0 x2; ctest 880; Pitfall 55 |
| 02:14 | merge | lane/video -> 9cf9537 (2 fix rounds + rebase lane) | RED w3/w7; GREEN 57/0 + 56/1 -> W1b ruling -> w2 GREEN; ctest 897; Pitfall 56 |
| 06:40 | merge | lane/idlepaint -> 046221e (2 fix rounds, addenda 1-3) | RED i1 17.0 ms / 319 ms/s; ctest 920; Pitfall 57 |
| 06:45 | gate | FINAL battery on main | see HANDOFF session section |

## Harmony errors (no gate would surface them)
1. A `cd` in a read command (repeat of s-rta-0928 error 2) + one sed temp file outside the scratchpad.
2. Read src/recording/TempoMap.cpp to design a teeth mutation (Iron Law #1).
3. Typed "21:1x" into 4 log rows instead of stamping from date (corrected).
4. W1 ruling set a relative perf bar from a stills number measured under load -> knife-edge row (W1b demoted it to INFO).
5. J1 ruling's "no periodic or background repaint" wording read as forbidding a one-shot repaint -> a lost round.

## Counts (run, not inherited)
ctest 920/920 on main after the last merge · Catch2 targets 101 · unpushed 0 after the close commit
