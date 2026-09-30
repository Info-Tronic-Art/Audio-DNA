# Session s-rta-0929 — secondary (MINIMAL, foreign repo), 2026-09-29 08:35 → ~22:15

role: secondary · profile: MINIMAL · repo: ~/projects/RealTimeAudio · main loop: Opus 5.5 (1M) · ultracode (workflows)
boot HEAD: adf9b8a · close HEAD: see git log (all pushed) · running log: .harmony/s-rta-0929-work.md
reports/evidence: .harmony/.reports/s-rta-0929/ (3 plans + adoptions/addenda, 8 attack papers, diag-vfps + tools + audit,
reviews, critics, lane reports, Boris page, wf + gate scripts)

## Rows
| t (approximate — the stamped rows are in .harmony/s-rta-0929-work.md) | kind | item | result |
|---|---|---|---|
| 08:38 | check | boot ctest | 920/920 re-derived |
| 08:45 | launch | plan asyncload, plan g4cpu (Fable), diag-vfps | 3 workflows |
| 09:20 | adopt | asyncload AL1-AL12 (Append / Duplicate FIFO; hung-open leak; audio witness) | build lane |
| 10:00 | adopt | g4cpu G1-G10 (no TopBar layer; bar after levers) | build lane |
| 13:55 | diag | diag-vfps: per-LOAD clock-phase bunching; 4K upload = GL-thread CPU | audit SOUND_WITH_GAPS |
| 14:10 | merge | lane/asyncload -> 7c95c4e | RED 30/49; GREEN 83/0 + 3/0; ctest 942; Pitfall 58 |
| 14:30 | adopt | vupload VU1-VU13 (9 MUST adopted) | build lane |
| 16:45 | merge | lane/g4cpu -> c432e3f (STOP ruled G11-G15; rebase lane) | RED absent; GREEN all rows; ctest 947; Pitfall 59 |
| 19:40 | ruling | vupload VU15 pre-registered w7 test | rule (a): QoS reverted |
| 20:14 | merge | lane/vupload -> 5b7f461 | RED 6/11 + w1c 111.1; GREEN 21/0, 102/0, w10 3 paths; ctest 967; Pitfall 60 |
| 21:40 | gate | FINAL battery on main | all GREEN except idle-paint (2 startup crashes, v0) -> re-run GREEN |

## Harmony errors (no gate would surface them)
1. Packet wording made my constraint read as a Boris ruling (g4cpu plan). 2. "Per launch" carried as ESTABLISHED (wrong).
3. Stale "~1.1 s" freeze figure inherited into a plan's RED prediction. 4. Static critic prompt in round 2. 5. `git commit -am`
swept a foreign dirty file (amended before push). 6. One temp file under /tmp.

## Counts (run, not inherited)
ctest 967/967 on main after the last merge · Catch2 targets 107 · unpushed 0 after the close commit
