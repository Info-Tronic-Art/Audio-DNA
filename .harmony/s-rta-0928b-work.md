# s-rta-0928b — running work log (secondary, MINIMAL, main loop Opus 5.5 1M, ultracode)
boot 2026-09-28 17:16:50, HEAD b9c9ab2. Birth prompt: HANDOFF.md top + "# >>> SESSION s-rta-0928" section.
Reports/evidence: .harmony/.reports/s-rta-0928b/. Scratchpad lib: e9ff9dc6 scratchpad/lib/lock.sh (self-tested: owner line "selftest <pid> <epoch>", release OK).

## Rows
| t | kind | item | result |
|---|---|---|---|
| 17:16 | boot | handoff + screen-safety law + inbox (all DONE) | CTX 9.4 %; no app, no lock, 1 worktree (main); disk 351 GB free; load 5-6 |
| 17:16 | error | a `cd` in a read command (repeat of s-rta-0928 error 2) | habit fix: shell vars (P=/abs/path) for every multi-file read; never `cd <dir> &&` to shorten paths |
| 17:21 | launch | 4 workflows: diag-idle wf_5e0566ce-03d (w23tkfyay), diag-media wf_1d2b3bb1-38e (wp8qup9nt), build sampleat wf_5f659395-fde (wfwljm1vm, sonnet builder: executes authored recipe plan-tempo §8, cost gate = no Fable), plan seqvram wf_21294cf2-826 (wesv3wtua, architect fable + 2 blind seats) | worktrees rta0928b-idle / -media / -sampleat; scripts in scratchpad/wf/ (syntax-checked) |
| 17:22 | check | main build/ current? | VERIFIED: no file in src/ or CMakeLists.txt newer than build/.../Audio-DNA (14:39) -> the pre-change app for RED = main HEAD |
| 17:43 | plan | seqvram (wf_21294cf2-826, tier fable) | plan-seqvram.md 551 lines; seats gl + vj: 6 MUST total |
| 17:50 | adopt | seqvram HARMONY ADOPTION H1-H17 appended | adopted GL MUST-1/2/3, SHOULD-1, NIT; VJ MUST-1/2/3, SHOULD-5, NIT; SHOULD-4 ruled doc+ctest. HARMONY FINDINGS the seats missed: H6 clip in/out points absent from Transport; H7 row v4's premise wrong (Belady keeps post-wrap frames -> a retrigger is a HIT; late>=1 would fail GREEN) -> v4_retrigger_hit + hold witness moved to v7. Machine: M1 Pro 32 GiB (Q1). |
| 18:02 | error | Harmony read src/recording/TempoMap.cpp (sed -n 75,112p) to design a teeth mutation — Iron Law #1 | habit fix: teeth/RED gates apply the mutation the lane REPORT names, by sed pattern + grep count, never by opening the source; if the report does not name it, a reviewer/builder writes the mutation script |
| 18:02 | lane | sampleat DONE ce50df9 (wf_5f659395-fde) | builder RED 2 FAIL -> GREEN; ctest 848/848; review correctness APPROVE (0 MUST/0 SHOULD/1 NIT) |
| 18:03 | gate | sampleat teeth by Harmony (lane worktree, pattern mutations, restored) | M0 3/3 pass; M1 pre-fix rule: #440 + #441 FAIL; M2 always-nominal: #441 FAIL; restore 3/3 pass, tree clean |
