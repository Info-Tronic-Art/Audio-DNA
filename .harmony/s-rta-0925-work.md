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
- wave2: opacity lane MERGED bbac78a (also carried a right-click fix). build ok, ctest 466/466. rclick lane overlaps -> reconcile workflow wev64z8r7 (lane/0925-rclick2). Visual gate for step4polish + Composition tab still owed (shooter aborted: Touch ID dialog). Unpushed.
- wave3 dispatched: visual-gate wxixldz40 (shots + OSC master link check + 3 critics), replay-restore build wbcxs7xg4 (fenced: no end-of-replay, no UI files). Master Signal build waits on Boris Q1-Q3 + prerequisite (effect/source param twins render-dead, diag-mastersignal §2).
- visual gate wxixldz40: Record states PASS (logic critic), master link PROVEN (OSC 0.3 -> masterLevel=masterOpacity=0.30, both UI 0.30), Video Opacity gone. MUSTs: Per-Type Autopilot checkbox overlaps Opaque row label (I viewed B_composition_inspector.png: confirmed); 'Comp/Decks' abbreviation vs UI Text Rules. Fix lane wncj85lc1 (lane/0925-visualfix). Push held.
- Master Signal Q1 ANSWERED: Gain stays visible. Q2 (beat-clock effects at 0%) and Q3 (persist) pending.
- Master Signal Q2 ANSWERED: keep pulsing; control scales signals only.
- Master Signal Q3 ANSWERED: save with composition. All 3 answered -> build (prereq: effect/source-param twin slice).
- Gates on merged main (6d55fc9, CLT-only rebuild): ctest 475/475, probe-step3 69/0, reshoot: Per-Type row fixed + 'Compositions' tab full (reshoot/comp-crop.png, composition.png). Found (pre-existing, next polish): 'Deck Loa' button clipped in the top row; 'No clip selected' text overlaps Clip-tab dashboard knobs.
