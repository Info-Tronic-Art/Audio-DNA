# s-rta-0926b — running work log (secondary, MINIMAL, ultracode/workflows)

Boot 2026-09-26 ~18:37. HEAD bc69fd0 (code = 6e8f120), unpushed 0. build/ binary 18:12:35 (= 6e8f120). Disk 378 GB free.
10 cores. No app running, no live lock, no worktrees. Dirty at boot (not ours, leave): .harmony/.harmony-version, AGENTS.md (untracked).

## START HERE items
1. DONE — first-call token number 54,790 (ctx-now.sh first call; <= 75,000 PASS, was 101,568). Up-channel:
   idea-ledger idea-2026-09-26-RealTimeAudio-1790462247885395697; inbox status-note updated.
2. Routine feel questions (9, incl. same-BPM realign product call) relayed to Boris in chat 18:4x. Awaiting answers.
3. Render follow-ups (xfade-report §5) — diagnose LIVE.
4. BPM thread safety (setManualBPM / Tap from message thread; Link per-tick realign).
5. Recorder provisional save empty tempo map; pre-ebbff22/e5ceb98 takes (no migration).

## Plan — wave 1 (one workflow, 3 worktree lanes, live app serialized by /tmp/audiodna-live.lock)
- R render (opus builder, worktree): apply parity-trace.diff in a scratch build; live-diagnose R1 outgoing-clip temporal
  key during crossfade, R2 persistent-layer keys across decks, R3 applyTransition ignores outgoing transform, R4
  compositePersistentLayers skips stages, R5 temporal FBO created mid-pass. Report BEFORE fix. Fix only the clear ones
  (with RED-first reproducers); R1 (per-clip temporal history) stops at options -> Fable ruling -> wave 2.
- B bpm (opus builder, worktree): TSan/unit reproducer of the message-thread writes; route setManualBPM/Tap through an
  analysis-thread request like requestResync; Link per-tick same-tempo must not realign; explicit same-BPM set_bpm
  semantics UNCHANGED (Boris question 9 pending).
- C recorder (sonnet builder, worktree): provisional save carries a tempo map; count old takes on disk; migration = options only.
Each lane: builder -> pinned reviewer -> <=1 fix round. Harmony: RED new probes on build/ (6e8f120) BEFORE merge,
merge, cmake -S . -B build + rebuild, ctest, probes GREEN, look at frames.
- 18:5x launched wave 1 w4aozsb4x (wf_41d6317f-a47): render (opus xhigh, worktree) / bpm (opus xhigh, worktree) /
  recorder (sonnet high, worktree); each -> pinned reviewer (sonnet) -> <=1 fix round; render forks -> 3 blind seats +
  Fable ruling (REPORT .harmony/.reports/s-rta-0926b/ruling-render-forks.md). Reports: .harmony/.reports/s-rta-0926b/.
- 20:1x wave 1 DONE (10 agents): render R1 VERIFIED (fork) + R2-R5 VERIFIED+fixed, reviewer PASS; bpm race TSan-proven
  + fixed, Link 30 Hz phase reset fixed, reviewer PASS_WITH_NITS; recorder early tempo save fixed, reviewer PASS_WITH_NITS.
  Fable ruling (ruling-render-forks.md): R1 = B' (per-layer outgoing slot: copy buffer, swap ring); R4-opaque (1) blend
  over + honour opacity; R4-router (1) never publish; R4-types FX Only + media-less yes, Mask/3D no + toggle disabled.
  HARMONY DECIDES: adopt all four. C-migration: none. LINK-RAMP: (b) a Link tempo change never realigns (wave 2).
- RECEIVER-VERIFY caught: recorder lane census WRONG (read key tempo/a, disk is tempoMap): real 158 takes, 139 empty,
  0 empty after ebbff22; the "7 anomalous takes" do not exist. Notebook + report corrected (30143db).
- Harmony RED on build/ 6e8f120: probe-render-state "PY 1 PASS / 18 FAIL" RED; probe-crossfade k,l "PY 4 PASS / 4 FAIL"
  RED; tempo witness (take.json 1.5 s after Record) tempoMap [] RED.
- merged 7713aa4 render, efa40ab bpm, c9823ef recorder; build/ reconfigured + rebuilt 20:23; ctest 588/588 (serial).
- MY ERROR: `cd .harmony && ...` moved the main loop cwd (same as s-rta-0926 error #5). Restored. Habit: no `cd` in the
  main loop at all — absolute paths, git -C, script files.
- Harmony GREEN on merged build/ (5f84899 code = c9823ef): render-state "PY 19 PASS / 0 FAIL"; crossfade "PY 35 PASS / 0 FAIL";
  effects-parity "PY 46 PASS / 0 FAIL"; manual-bpm 18/0; downbeat 14/0; routines 74/0 (pause 1.8); mastersignal 22/0;
  tempo witness GREEN (tempoMap start anchor bpm 120 at 1.5 s). Frame looked at: k_outgoing_transform refA/mid02/05/08/refB
  strip — real blend of the scaled outgoing clip into the incoming grid.
- resync 15/1 once (V2 opacity 0.3987 at 64 ms) -> A/B/C x2 (merged / bpm-only W2 / render-only W1): 6/6 16/0 (V2 0.003-0.009
  at 62-108 ms). NOT a regression. INFERRED cause: load avg ~13 right after build+ctest delayed state publish past 64 ms.
- step3 T2 p95 FAIL on ALL three builds (merged 19.57, bpm-only 17.29, render-only 16.17; all other 93 rows PASS). Render-only
  touches no analysis/capture code -> ENVIRONMENTAL (Stremio running ~25% CPU, same as s-rta-0926 bisect). Cheapest
  discriminating test: re-run probe-step3 on build/ with Stremio closed (Boris's app — ask).
- 20:5x wave 2 launched (wf_b6cc814e-3f6): render2 (W1, opus: ruling steps 1-4 + empty-active-deck persistence + docs),
  bpm2 (W2, opus: LINK-RAMP (b) + default-build Link toggle disabled/never feeds 120 BPM + nits/docs), probehygiene
  (isolated, sonnet: probe-effects-parity fresh dir + render_frame check). Paused ~10 min for a quiet step3 run.
- Boris closed Stremio 20:5x -> probe-step3 on merged build/ x2: 94 PASS / 0 FAIL both (p95 10.11 / 10.75 ms). T2 miss was
  ENVIRONMENTAL (Stremio) — CLOSED. Wave 2 resumed (wu2yiazvi).
- 21:0x BORIS ANSWERED Q10: time effects CONTINUE across a clip change ("continuation of the effect ... like a fade ... smooth")
  = ruling R1 B' default (carry over) — already what render2 builds. Recorded in BORIS_DECISIONS.md "Playback Behaviour".
  Q11/Q12: he asked what deck 1 / deck 2 mean -> re-explained with the deck tabs. Q1-9: he could not see them (my first
  message's mid-turn text) -> re-sent in plain words.
- FINDING (UX, from Boris's confusion + source lookup): the toolbar buttons "Deck Save" / "Deck Load" do NOT save/load a
  grid deck — loadDeck() (MainComponent.cpp:3751) loads a legacy PresetManager::DeckState (audio/image/slideshow/UI
  settings) and never touches composition decks or tabs. Real grid decks: Deck menu > New Deck (MainComponent.cpp:6100),
  or Compositions browser > Decks row click (appendDeckFromFile, :3107-3177). DeckView tabs have no "+" / context menu.
  Name collision confuses the product's own owner -> candidate loose end (rename the toolbar pair, e.g. "Setup Save/Load",
  or retire the v1 row). NOT fixed this session (Boris-facing naming = his call).
- 21:2x BORIS routine answers: 1 GLIDE (not hard cut) -> build item; 3/4/5/6/7 yes (current behaviour stands);
  2 "where is the stop control?" -> explain TopBar Stop + ask again; 8 "what is (bar N)?" -> explain; 9 verbatim: "we only
  need longer than 4 bar counts for routines and that should be displayed with the routine and nothing else. Top bar count
  should go 1-2-3-4-1 etc" -> ambiguous (display request vs same-BPM restart) -> lookup agent on counters, then confirm.
  Recorded in BORIS_DECISIONS.md "Playback Behaviour".
- FACTS (lookup, cited): pad text idle "N: name" / waiting "(next bar)" / playing "(bar N)" = bar within the routine's own
  cycle, restarts each loop (RoutineBankModel.h:47-91). Stop: click the playing pad = stop that routine; TopBar Stop (square)
  = routineEngine_.stopAll() + stop/rewind every clip on the active deck (MainComponent.cpp:701-712). TopBar "Bar N" =
  barCount+1 (bars since last phrase reset, grows), "Phr X.XX" = phrasePhase decimal (TopBar.cpp:524/533). set_bpm resets
  ONLY the sub-beat phase (always, even same value); bar/beatInBar counters untouched (BPMTracker.cpp:561-599).
  MY ERROR: my Q9 to Boris said the same-BPM resend "restarts the beat count at beat 1" — false (only sub-beat phase).
  I asked from the handoff's paraphrase without checking. Habit: verify the mechanism before phrasing a Boris question.
- plan3 launched (wf_83de54ba-147): Fable draft -> 3 blind seats -> Fable final: (1) routine-start GLIDE, (2) TopBar bar
  count 1-2-3-4-1, (3) same-BPM realign (Harmony decides on merits). Build after wave 2 merges (TopBar/BPMTracker overlap).
- 21:4x BORIS: 11 yes + "how does the override work currently?"; 12 FINISH THE FADE + deck switch must not touch clips
  playing in layers; 2 NO STOP MODEL — routines are ended by replace/remove like clips; asks for a routine DISPLAY design
  (per layer? layer strip? multi-layer routine shown in every layer with the same name?); 8 "(bar N)" — where/necessary?;
  NEW: Preview/Output panel must keep the COMPOSITION's aspect ratio (1920x1080 / 2K / 4K setting). All verbatim in
  BORIS_DECISIONS.md. Next: fact lookup (override, inactive-deck clips/fades, preview aspect, comp resolution setting) +
  routine-display design study (Fable + critics + mockup artifact).
- 21:5x routine-ux design workflow (wf_8f851904-bb9; Boris: "have fable think through how we will display and use the
  routines" + "that is after we have created them" -> scope = display/use of CREATED routines, creation unchanged):
  recon (sonnet) -> 3 Fable designs (routine-as-clip / layer-centric / performer-workflow) -> 4 critic seats -> Fable
  synthesis design-final.md -> HTML mockup (headless-Chrome shots) -> 4 visual critics -> 1 revision -> Harmony opens link.
  MY ERROR: the first launch had `${ACK = ''}` inside a template literal (assignment to an undeclared name -> would throw
  at the synthesis step in module strict mode). Caught on re-read, stopped, patched, relaunched. HABIT: syntax-check every
  workflow script (node new Function on the body) BEFORE launching, not after.
- FACTS (scout, cited): persistent-layer OVERRIDE while on another deck = none except Master opacity (dims all); the
  inactive deck's strips are not shown (DeckView.cpp:78); OSC/REST/MIDI layer controls address the ACTIVE deck only; Solo is
  per-deck (does not hide other decks' persistent layers). Inactive decks: video FREEZES (resumes same position), crossfades
  freeze, autopilot paused. PREVIEW SIZE follows the panel component (756x878 portrait) — Composition Resolution control
  (CompositionInspector, default 1920x1080, saved) is NOT used by the GL pipeline; per-deck resolutionSelector_ override
  exists; render_frame captures the panel viewport. TopBar Stop "[]" = stopAll routines + stop active-deck clips.
- plan4 launched (wf_33efa8a0-2f7): Fable plan -> 3 seats -> Fable final: (1) composition resolution drives the picture's
  shape in Preview + Output (render strategy, 4K cost, probe re-baseline), (2) inactive decks keep time (fades finish;
  video/autopilot decision).
- 22:xx wave 2 DONE (8 agents): render2 (R1 B' outgoing slot, R4-opaque, R4-types + toggle, empty-active-deck persistence,
  docs/Pitfall 35) reviewer PASS_WITH_NITS, critic PASS/PASS/PASS; bpm2 (LINK-RAMP b, default-build Link never enabled +
  TopBar toggle disabled) reviewer PASS, critic PASS x3; probehygiene (effects-parity fresh dir + mtime) PASS_WITH_NITS.
  RECEIVER-VERIFY: render2 DONE_WITH_CONCERNS -> its first RED run was contaminated by a stub harness that failed to bind
  7070 and drove render2's real app (discarded, re-run); probehygiene claims its stub runs held the lock — the stray run is
  not provable now (tmp dir cleaned). Rule filed in .harmony/gotchas.md. Harmony RED on build/ (wave-1 main):
  probe-render-state new rows "PY 3 PASS / 8 FAIL" (3 = guards) RED. Merged b022a28 render2, cef89f5 bpm2, 50b4bce
  probehygiene; reconfigured + rebuilt; ctest 599/599; CLAUDE.md 24,366 B. Looked at: Persistent toggle crop (Mask dimmed),
  TopBar Link crop (dimmed, same bounds) — PASS.
  Follow-ups: LookAndFeel drawToggleButton ignores isEnabled (app-wide); Mask loaded persistent=true shows checked-disabled
  with no way to clear; probes' pgrep matches the linker line; crossfade/render-state cap() lack mtime hardening; one-time
  18-24 ms hitch when the spare ring is first created (first crossfade on a Split/Stutter layer).
- Harmony GREEN on build/ 50b4bce: render-state "PY 31 PASS / 0 FAIL"; crossfade "PY 35 PASS / 0 FAIL"; effects-parity
  (hardened) "PY 46 PASS / 0 FAIL"; manual-bpm 18/0; resync 16/0; downbeat 14/0; routines 74/0; mastersignal 22/0; step3
  94/0; tempo witness GREEN. Frame looked at: r1_temporal refA/mid02/05/08/11/refB (wipe: each side its own clip, no ghost).
- 22:3x wave 2b launched (wf_6f166b85-e43): uitoggle (W1: LookAndFeel disabled toggles dim app-wide + Persistent enable rule
  "persistable OR currently set"; full 3-critic panel) + probehygiene2 (W2: lock-owner check, exact-binary pgrep, cap()
  delete-first+mtime in crossfade/render-state; all probes re-run GREEN). Pushed 0 unpushed after wave-2 gate.
- 23:0x plan3 FINAL (plan3-final.md) ADOPTED by Harmony: A = (c) a tempo VALUE (typed / REST / OSC) never realigns; Tap +
  Resync are the beat gestures. B = TopBar one line "Bar 1..4" (barCount % 4 + 1), "Phr X.XX" removed. C = restore GLIDE:
  one beat ending on the boundary (quarter-beat floor), loop return + re-fire same rule, in-flight glides released at
  any non-restoring end (the draft's grip-leak caught by the critics), cancel via Player::holds. Boris defaults: glide 1 beat;
  loop return eases. Wave 3a launched (wf_156f9cfa-a36): X tempo-glide (opus, isolated) A then C; Y topbar-count (sonnet,
  isolated) + micro critic. Probe header edits are probehygiene2's; lanes add rows only.
- 23:2x plan4 FINAL (plan4-final.md) ADOPTED: commit A = canvas = Composition outputWidth x outputHeight, ONE render into
  an offscreen canvas FBO, panel presents letter/pillar-boxed via box-filter downsample; per-deck resolutionSelector_
  retired (test-only override kept); runtime resolution change keeps histories; render_frame captures the canvas; perf
  gate c_perf_1080 (fps >= 58, frame_time <= 12 ms) + 4K report; fallback policy named. B1 = crossfades advance on
  inactive decks (ships). B2 = video/imageseq clocks advance without decoding + per-deck autopilot (Boris Q1; default ON).
  FINDINGS: F3 the second-display OutputWindow shows ONLY the legacy single image, never the composition (Boris Q3 — big);
  F1 AddDeckCmd leaves new deck id 0 -> LayerStateKey collision with deck 0 (R2 reopened for Deck > New Deck) -> small lane;
  F2 (INFERRED) P25 cross-deck transition captures the NEW deck as outgoing -> deck switches are cuts -> verify live, fix
  after commit A (same renderOpenGL block); F4 capture path synchronous; Q4 portrait clips stretched today.
  PLAN: launch the plan4 lane in W1 (warm render build) after wave 2b merges (max 3 build lanes; W1 busy with uitoggle).
- 23:3x BORIS: Q1 decks KEEP PLAYING while off screen (plan4 B2 ON, confirmed). Q4 default STRETCH + a per-clip choice
  Stretch / Bars / Crop (new small feature; Fable spec -> build after plan4 commit A). Asked for the full open-question list.
- 23:4x BORIS: (1) output to ANY number of connected displays incl. the main screen -> TOP job (Fable plan5);
  (2) TopBar Stop = routines only (drop the clip rewind) -> small commit after lane X; (3) deck save/load: asked if next to
  the deck tabs is more intuitive than the Comp tab -> recommend tabs (+ / right-click) with the browser as the library;
  (4) NO extra UI for other decks' persistent layers; (5) glide length -> recommend 1 beat; (6) loop ease yes + per-routine
  Ease/Jump control -> engine flag after lane X + UI in the routine-display mockup revision.
- 23:5x BORIS: Save/Load Composition in the top menu Comp menu AND the bottom-right Compositions tab (whose Save-Composition
  + entry-click load are UNWIRED today); Save Deck in the deck tab row. -> plan6 (Fable design + mockup + critics):
  deck tab row actions, comp save/load wiring, legacy Row1 toolbar retirement, F1 AddDeckCmd id fix.
- 23:5x BORIS "go with recs": deck-tab row (+ New/Load; right-click Save/Save As/Rename/Duplicate/Remove), browser = library,
  legacy Deck Save/Load retired; glide 1 beat; per-routine Ease/Jump. plan6 launched (wf_72a8f8b4-e1e): recon -> Fable spec
  -> 2 seats -> Fable final (no approval mockup; built UI gets the critic panel). Running now: wave 2b (w4c5t8bnt), wave 3a
  (wukglsh9r), routine-ux (wui4m9e4t), plan5 outputs (wf4762g01), plan6 (wutnzhihz). Queued: plan4 build lane in W1 after
  wave 2b merges; Stop=routines-only + per-routine Ease/Jump engine flag after wave 3a; mockup revision with answers 2/4/6.
- 23:5x routine-ux DONE (19 agents; design-final.md + mockup.html + 13 shots; visual panel r2 4/4 PASS). Design: 22 px
  ROUTINES row of 8 pads above the column numbers; press = fire/restart (never stop); name bands on every layer strip it
  plays with an x (remove from all its layers); layer X also removes routines from that layer; "5/8" + sweep on the pad
  only; knobs it drives cyan + "ROUTINE"; V fader cyan (and the V-fader-doesn't-move bug fixed in slice 1); off-deck pad
  dims. Harmony looked at 03-playing-2layers.png: faithful to the app. Revision launched (wf_09d9b827-421) with Boris's later
  answers (glide lands on the bar per plan3, per-routine Ease/Jump, Stop=routines only, top-bar Q decided) + logic/UX
  critics; then open the link for Boris.
- 00:1x mockup revision: logic critic PASS; UX critic FAIL (MUST: "slice A/C" jargon in the s4 caption; SHOULD: open
  questions on the page). Harmony fixed both in place (plain-words caption; "Still your call" block with the 8 open
  questions + defaults), re-rendered s4 + top headless, looked at the top render. Opened mockup.html for Boris.
- MY PACKET DEFECT: wave 2b fix-round prompt embedded the original packet's STEP 0 "git checkout -B <lane> main" — the
  uitoggle fix builder correctly refused (it would have discarded the lane's 5 unmerged commits). HABIT: fix-round
  prompts must strip branch-creation steps and name the exact commit to continue from.
