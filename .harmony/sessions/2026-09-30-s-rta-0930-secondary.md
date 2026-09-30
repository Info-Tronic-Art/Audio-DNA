# Session s-rta-0930 — secondary (MINIMAL, foreign repo), 2026-09-30 11:53 → ~16:15

role: secondary · profile: MINIMAL · repo: ~/projects/RealTimeAudio · main loop: Opus 5.5 (1M) · ultracode (workflows)
boot HEAD: 655d232 · close HEAD: see git log (all pushed) · running log: .harmony/s-rta-0930-work.md
reports/evidence: .harmony/.reports/s-rta-0930/ (3 plans + 3 rulings (+ bt2 seats addendum), 8 attack papers, TSan RED
baseline on main, parameterized TSan harness, lane report gop2.md + 2 reviews, gate-gop2/ evidence, Boris page, wf/)
Planning model: Fable limit hit at 11:56 (all 3 dispatches) -> every plan / ruling ran on Opus 5.5 max; Boris confirmed
mid-session (verbatim): "just so you know, we are out of fable usage so you will need to do all fable work with opus 5.5".
Close trigger: Boris (verbatim, mid-turn): "finish current tasks then eos".

## Rows
| t (stamped rows in .harmony/s-rta-0930-work.md) | kind | item | result |
|---|---|---|---|
| 11:54 | boot | handoff end section + screen-safety law; state clean | 3 lanes planned (tsan, gop2, bt2) |
| 11:56 | deviation | Fable limit on all 3 plan dispatches | re-pinned opus max (Law #11 deviation logged) |
| 12:04 | RED | TSan build of main 655d232, sweep a/b/c x3 | 25 unique races, all 5 families; 9/9 clean quits, 0 dialogs |
| 13:51 | error | plan stage: seat papers not written to disk, tsan planner wrote nothing | papers recovered from journal; tsan re-planned skeleton-first |
| 13:52 | adopt | plan-gop2 + ruling-gop2 (26 attacks ruled) | build lane launched |
| 14:39 | ruling | bt2 seats addendum | ready_to_build (carried) |
| 14:57 | lane | gop2 DONE 48dd81b; reviews PASS_WITH_NITS x2, 0 MUST | no fix round |
| 14:59 | merge | lane/gop2 -> 7123b9d | G1 ctest 1055/1055; G2 TSan 0; G3 selftest PASS |
| 15:03 | gate | G4 GC7 @256 MB 5x2 vs PRE | PASS: late 100 -> 0, slowest 30.2/s, over-budget 0, 0 taints |
| 15:22 | gate | G5 uncapped no-regression 5x2 | PASS except strict "B<A" flip-gap row tied 79.0/79.0 (one B outlier 148.8 ms) |
| 15:25 | gate | G6 u10 4K @512 5x2 | PASS 2.4 -> 13.8/s, late 553 -> 320 |
| 15:43 | gate | G5b identity x3, w10 forward identity x3 paths, G7, media-open, seq-vram | all GREEN; 0 UNC, 0 Output windows, 0 .ips |
| 15:30 | ruling | tsan ruling (27 acc / 11 rej, 18 amendments) | ready_to_build (carried) |
| 15:45 | gate | pre-registered u7 flip re-run 5x2 | see work log (result row) |
| ~16:10 | eos | Boris "finish current tasks then eos" | eos-secondary |

## Harmony errors (no gate would surface them)
1. plans.js handed the ruling stage attack-paper PATHS, not contents; the seats returned StructuredOutput and wrote no file
   -> bt2 ruled without its seats (gop2's ruling recovered them itself). 2. The first tsan planner had no write-early
   instruction and ended with nothing on disk (~40 min). 3. Two commits during the live perf A/B fired the graphify
   rebuild hook (CPU) mid-gate. 4. One `cd` into the reports dir early (no effect).

## Counts (run, not inherited)
ctest 1055/1055 on main 7123b9d · Catch2 targets 111 (registrations; 110 built) · unpushed 0 after the close commit
