# s-rta-0925 work index (secondary, MINIMAL)
Boot: HANDOFF end section s-rta-0924b read. Boris: "Use workflows".
Workflow lanes (one run):
- A downbeat-pulse: downbeatDetected one-hop pulse class (TopBar.cpp:232) -> OnsetPulse pattern. plan->critic->build(worktree)->review. Harmony live gate after merge.
- B step4-polish: fix-plan §2 deferred (stale notice, Comp/Decks tab clipped BrowserPanel.cpp:74, disabled-reason captions). plan->build->review. Harmony: critic panel + window-only screenshots after.
- C roadmap: spec s167 §5 steps 5-7 + LATER lanes vs code -> what is actually next; plan + critic. No build.
Boris calls: deliver ONE at a time (call 1: replay snaps back to Record-time look?).
Reports: .harmony/.reports/s-rta-0925/
- Boris call 1 ANSWERED: replay snaps back first = YES (binding-decisions.md 2026-09-25). Verify built.
- Boris call 2 ANSWERED: 'Record Over' (already the live label).
- Boris call 3 ANSWERED: at end, hold last state + keep running (live audio if present, else params). Confirming interpretation.
- Boris call 4 ANSWERED: keep playing (already live).
- Boris call 5 ANSWERED: black on red (live).
- Boris call 6 ANSWERED: never delete (live).
- Boris call 7 ANSWERED: fixed location (live).
- Boris call 8 ANSWERED: optional name at Record (appears built; verify live).
- Boris call 9 ANSWERED: manual Resync re-aligns oscillators (build item).
- Boris call 10 ANSWERED: remove Video Opacity twin now. NEW: top-right master fader link, Master Signal slider (design), right-click reset bug. Workflow 2.
- rclick scope from Boris: ALL Composition-tab sliders fail. Gate must cover every Composition-tab control. Master Signal: post-analysis depth, Gain stays input (ruled).
- roadmap plan PASS (critic): steps 5,6 DONE; step 7 docs NOT done; next build = replay-restore (plan-roadmap.md §3). NOTE plan §3.11 assumes call 3 = auto-stop; Boris ruled HOLD — correct in wave 3 packet. Wave 3 candidates: replay-restore, step-7 docs, end-of-replay hold, manual-resync realign, Player backwards-seek skip bug (HANDOFF s168 add. 2), master-signal build.
- MERGED local (unpushed): downbeat 00a0e53, step4polish b6d5eba. Gates: build ok, ctest 455/455, probe-downbeat-level 14/0, probe-step3 69/0 (STEP3_BUILD_DIR=build). Visual gate workflow wa7gsgpia running. Probes fixed: open -g, no full-screen capture.
