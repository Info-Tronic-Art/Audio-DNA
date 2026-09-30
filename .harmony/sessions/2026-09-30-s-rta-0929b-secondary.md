# Session s-rta-0929b — secondary (MINIMAL, foreign repo), 2026-09-29 23:14 → 2026-09-30 ~11:15

role: secondary · profile: MINIMAL · repo: ~/projects/RealTimeAudio · main loop: Opus 5.5 (1M) · ultracode (workflows)
boot HEAD: d88d2ea · close HEAD: see git log (all pushed) · running log: .harmony/s-rta-0929b-work.md
reports/evidence: .harmony/.reports/s-rta-0929b/ (recon x2, research, prebuild, sweep + 29 verifier papers, 2 plans +
adoptions, 6 attack papers, reviews, lane reports on the merged branches, Boris page, wf + gate scripts)

## Rows
| t (approximate — stamped rows in .harmony/s-rta-0929b-work.md) | kind | item | result |
|---|---|---|---|
| 23:19 | evidence | 7 .ips parsed; heap signature x5 on 3 binaries | recon workflow launched |
| 23:45 | finding | 09-29 crashes = BT earbuds as default device (unified log 2/2 vs 661/661) = known s-rta-0924b JUCE overflow | START 1 re-framed |
| 23:48 | finding | JUCE fix is in 8.0.9, not 8.0.8 (hash-verified) | deferral corrected |
| 00:05 | adopt | plan-gopcache GC1-GC13 (16 MUSTs) | build lane |
| 00:14 | adopt | plan-btguard BG1-BG9 (16 MUSTs) | build lane |
| 00:40 | result | ASan 31 clean; TSan 29 unique -> 20 real (1 MEDIUM) | filed |
| 02:22 | incident | mutant re-sign reset the app's mic permission | blocked until Boris Allow |
| 08:04 | unblock | tccd authValue=2 re-verified | gates resume |
| 08:06 | merge | lane/btguard -> 6c73bb0 | RED 11/14 -> GREEN 25/0; ctest 1003 |
| 08:20 | merge | lane/gopcache -> 8015c76 (Pitfalls 61 / 62) | ctest 1049 |
| 09:54 | gate | FINAL battery | GREEN except 3 load-sensitive rows |
| 10:58 | gate | quiet A/B + re-runs | ab7 66/0; GC7 late bar missed (filed); re-runs GREEN |
| 11:05 | eos | Auto-EOS (off-ramp, all tasks terminal + tested) | eos-secondary |

## Harmony errors (no gate would surface them)
1. TSan smoke abort_on_error default -> dialog; misattributed first. 2. Re-signed mutant allowed by my fix-round packet ->
TCC reset. 3. LANE not exported to the A/B driver. 4. Battery run without checking for orphaned CPU burners. 5. Pathspec
commit on ignored files. 6. One cd.

## Counts (run, not inherited)
ctest 1049/1049 on main 8015c76 · Catch2 targets 111 · unpushed 0 after the close commit
