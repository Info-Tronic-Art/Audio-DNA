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
- FLAKY (to chase): ctest 'ThumbnailCache treats a changed mtime as a miss' failed 1 of 3 full -j8 runs after replay-restore merge; 5/5 alone. Unrelated area. Hypotheses: shared temp path across parallel tests, or mtime granularity. Discriminator: grep test file for temp paths; run full suite -j8 x10 counting.
- replay-restore MERGED LOCAL 85afb70 (NOT pushed): ctest 483/483 (flaky thumbnail test aside); probe-step3 pre-fix RED 71/8, post 77/2: layer0 not restored within 1.5 s though preambleFired=8. Diagnosis lane wx31iq27m (lane/0925-rr-fix). Push HELD until probe green.
- rr-fix MERGED (preamble skipped opacity at default). ctest 484/484; probe-step3 79/0 (pre-fix RED 71/8, mid 77/2). Replay snap-back LIVE-VERIFIED. Pushing.
- wave4 wmfuuw384: replayend (Boris call 3 hold + backwards-seek), resync (call 9), flakythumb. Master Signal w3g3t3due still running. After both: step-7 docs + Deck Loa / No-clip-selected polish.
- DISK FULL incident 2026-09-26 (worktrees ~27 GB). Boris freed space; Harmony removed 9 merged worktrees (diffs of 2 leftovers saved to reports). 105 GB free. Rule in gotchas.md. Master Signal: step0 committed (lane/0925-mastersignal 3ca29df, review PASS); step1 partial uncommitted in worktree wf_6324cbaf-186-5 -> resume.
- Master Signal step0+step1 DONE on lane/0925-mastersignal-s1 (review PASS x2, ctest 511 in lane). NOT merged yet: probe-mastersignal.sh being written (wzr9hvjxw) -> RED on current build -> merge -> rebuild -> GREEN + regression probes (step3, onset, downbeat) + Tier-1 effects visual (render path changed in step0) + fader critic.
- Master Signal MERGED LOCAL (not pushed). ctest 512/512. probe-mastersignal pre 8/8 RED -> post 15/1: B1 frames PURE WHITE (B2 same image renders fine) -> diagnosis lane wafhk7fqh. Push HELD. Also owed after: step3/onset/downbeat regression + Tier-1 effects visual + fader critic.
- DOC COUNT DRIFT: CLAUDE.md says 24 REST endpoints; code has 28 unique non-perf /api paths (grep). Step-7 docs lane must derive counts by command.
- build2 after merges failed 'No rule to make target catch2 depend'; no agent command touched ./build (scanned 3 workflows' journals). Inferred: CMake regeneration race inside the parallel build after CMakeLists changed by merge. Re-configure + build fixed it (531/531). Discriminator if it recurs: run cmake -S . -B build before every post-merge build.
- MERGED local: flakythumb, replayend, resync (6b1c9c2). ctest 531/531. probe-resync 16/0 (pre 6/3). probe-step3 88/5 (end rows) -> wyldkxyb3 diagnosing (probe timing hypothesis). Master Signal white-frame lane wafhk7fqh. Push HELD.
- ms-white r2 (review-ms-white-r2.md, no new commit -- HEAD still 52cd76c on lane/0925-ms-white): rig-scoped round, no app launch. Built fresh in scratch dir (deleted after) proven compiled from the lane branch (not the stale shared build/ r2 caught) -- nm confirms applyClipOpacity's 9-param/forceCopy signature is actually present; ctest 516/516 incl. all 4 opacity-alias regression cases. Report: ms-white-r2-build.md. Live probe-mastersignal.sh B1 GREEN re-confirmation is still outstanding -- out of scope for this round's rig rules, unresolved by any report/work-log entry as of this write. Push HELD.
- 05dbc4a ms-white merged: ctest 535/535, probe-mastersignal 16/0 BUT VACUOUS: b1 frames still all-zero RGBA (numpy). Probe md5 check flawed. Re-diagnosis lane launched. PUSH HELD. probe-step3 93/0 (end-of-replay verified).
- Boris: work till 50% ctx then EOS (at 40% now). Plan: ms-white2 gate -> regression probes (onset, downbeat, Tier-1 effects visual) -> push -> EOS. Deferred to next session: step-7 docs (derive counts by command), Deck Loa clip, No-clip-selected overlap, fader critic panel.
- ms-white diag2: first fix effective in 3 controlled reruns; my 11:49 all-zero frames did NOT reproduce (UNEXPLAINED -> gate new probe 3x for intermittency). Second aliasing instance found (clip->layer effects hand-off), being fixed.
- regression on 05dbc4a: probe-onset-render 13/0, probe-downbeat-level 14/0. Tier-1 effects pytest deferred (Eyes render_frame skips effect chain per pitfall 28; deck-mode effect render covered by probe-mastersignal B1/B2).
