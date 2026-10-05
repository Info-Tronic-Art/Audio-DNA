<!-- Sibling projection, NOT part of CONTEXT.md — see .claude/skills/normalize/references/context-and-gotchas.md
     "binding-decisions.md Projection" and harmony-references/doc-unification-rules.md R1.1/R4.2.
     ADVISORY only: a stale stamp WARNs at boot, never blocks (R4.1/R4.3). Re-normalize to refresh. -->
synced-at-normalize: 2026-09-04

# Binding Decisions — RealTimeAudio

Currently-binding decisions relevant to this project, projected from Harmony_Main's primary
memory (R1.1 classes) plus this repo's own equally-binding local rulings. This is a PROJECTION —
no rule below was authored here; each is cited to its source of record, and anything ambiguous is
flagged as such rather than resolved.

## SYSTEM-level (from Harmony_Main memory — binds by R1.1)

- **Session role (primary/secondary) is a pure function of boot, fixed for the session's life; no
  in-session secondary→primary promotion exists.** A fresh boot is required to acquire a primary
  seat. Citation: `memory/DECISION_LOG.md`, heading "2026-06-13 — Remove secondary→primary
  in-session promotion [PERMANENT]".
- **Operating model: the primary works only on Harmony itself; secondaries/tertiaries (sessions
  like this one, in a specific repo) are for that repo's own work and code-building — "the fruit
  of Harmony."** Citation: `memory/DECISION_LOG.md`, heading "2026-08-22 (s128) — The primary's
  job, net-negative sessions, and retirement as a duty [PERMANENT]".
- **The party that builds is never the party that verifies: when a Builder finishes, Harmony runs
  the behavioral gate herself and dispatches an independent Reviewer to read the source.** Tester
  is optional (browser-heavy QA only). Citation: `memory/PREFERENCES.md`, heading "2026-06-17 —
  \"You build, I test\" — SUPERSEDED 2026-08-22 (s128)" (current rule stated there), corroborated
  by the bullet "Parallel Tester dispatch — SUPERSEDED 2026-08-21 (s126)" in the same file's
  Confirmed Preferences list. Reaffirmed project-locally — see PROJECT-level section below.
- **Autonomous operation is the permanent session mode: work the backlog with no human in the
  loop, and any friction that impedes unattended operation is fixed in the system on sight, never
  merely worked around.** Where a rule requires Boris, an autonomous path must be defined rather
  than waited on. Citation: `memory/DECISION_LOG.md`, heading "2026-08-18 — DIRECTIVE [PERMANENT]:
  AUTONOMOUS OPERATION IS THE PERMANENT SESSION MODE".
  - Same entry, load-bearing corollary on code changes: when code is added, code it replaces must
    be fully removed (not left alongside it), and a deeper pass must remove code that became
    orphaned as a *consequence* of the change even if it never appeared in the diff. STATUS on
    that corollary is logged as NOT-IMPLEMENTED (as a system-wide enforced gate) — treat as
    standing intent, not a proven mechanical gate.
- **A builds-never-verifies method (parallel generation / serial integration, Reviewer-on-source +
  Harmony-on-behavior, disk-verified fences, one-change-per-commit) was ruled worth making durable
  system-wide.** Citation: `memory/DECISION_LOG.md`, heading "2026-08-13 — [BORIS] DIRECTIVE:
  integrate the s93 working method into how Harmony works" (full method at
  `memory/specs/s93-parallel-verified-build-method.md`). AMBIGUOUS: this entry is a directive, not
  tagged `[PERMANENT]`, and its cited integration targets are system-doc placements inside
  Harmony_Main (kernel, execution-protocol, etc.) — whether it *itself* still binds a foreign
  project session by R1.1's "system/cross-project scope" disjunct, versus only its already-adopted
  descendant (the builds-never-verifies rule above, which IS carried in PREFERENCES.md), is a
  judgment call this projection does not resolve.
- **Model/compute tier convention for dispatched work (fleet-level): reviewer and researcher roles
  run on the lighter tier; the main/decision-making loop runs on the stronger tier.** Citation:
  `memory/DECISION_LOG.md`, heading "2026-05-29 — Opus 4.8 fleet switch + re-baseline (GO)
  [PERMANENT]". AMBIGUOUS/WEAK: this entry governs Harmony's own fleet pins, not RealTimeAudio's
  dispatch directly — included because a RealTimeAudio session that dispatches its own
  builder/reviewer agents may be expected to follow the same reviewer-stays-lighter-tier shape; not
  a confirmed requirement for this repo. A companion RULING E ("councils that decide get the
  strong tier") at `memory/DECISION_LOG.md` "2026-08-15 — [BORIS-DELEGATED] s100 council rulings"
  is explicitly logged STATUS: NOT-IMPLEMENTED — do not treat it as binding.
- **This repo's own project map is stale past its re-normalize kill-date and is a named, live
  risk: do not trust `.harmony/FEATURES.md` / surface maps here without re-verifying against
  disk.** Citation: `memory/KNOWN_RISKS.md`, heading "Re-normalize backlog — all 7 registered
  projects past the kill-date" (RealTimeAudio named explicitly: "HEAVY, 116 feature-affecting
  files, 121 commits"; re-seen unchanged through 2026-09-03). NOTE ON SOURCE: this is a
  KNOWN_RISKS entry, not a `memory/DECISION_LOG.md` entry — included because R1.1 (which this
  projection is scoped to) names "a KNOWN_RISKS entry that NAMES the project" as its own, separate
  binding class, and this one names RealTimeAudio directly.
- **Down-channel / project-boundary convention: the Harmony primary is the sole writer that
  appends new records into a project's `.harmony/inbox.md`; the project session only flips
  existing records' status lines (single-writer-per-direction).** Citation for the DECISION_LOG
  mention: `memory/DECISION_LOG.md`, heading "2026-09-02 - [HARMONY] s158 RULINGS (the drain and
  two Boris project tasks; Fable main loop)" — bullet "Boris mid-session tasks," which states
  Phase 3 of normalize "stays Boris-gated in each secondary" (RealTimeAudio named) and that
  down-channel records were routed to all four projects that session. The finer single-writer
  mechanics are stated in this repo's own `.harmony/inbox.md` header comment (citing
  `C-boundary-ruling.md` s148 §2b), which is a spec file, not a DECISION_LOG entry — flagged here
  since step 3's source was DECISION_LOG.md and this detail lives one hop away from it.
  AMBIGUOUS: whether "Phase 3 stays Boris-gated" constrains THIS write (an in-flight Phase 3
  normalize projection) is not resolved by this projection — it is stated as read, not
  interpreted.

**Checked for and NOT found in `memory/DECISION_LOG.md` or `memory/PREFERENCES.md` (two
independent grep patterns each, so treat as a real absence, not a missed search):** any
system-level rule about screen-safety / owner-attended gates (this class exists only as a
PROJECT-level rule below — see SCREEN-SAFETY LAW), and any system-level rule assigning a specific
compute tier to a *secondary session's own* dispatches (only the general fleet-pin convention
above exists, plus one NOT-IMPLEMENTED proposal noted above).

## PROJECT-level (this repo's own rulings — equally binding, local scope)

### 2026-09-05 (s-rta-0906) — BORIS RULINGS, SPOKEN DIRECTLY. THREE REVERSE STANDING INSTRUCTIONS.

Answers to eight questions put to Boris at boot of s-rta-0906. Verbatim intent preserved;
each is BINDING and outranks any earlier handoff text in this repo that contradicts it.

1. **PUSH. The "NOTHING PUSHED — do not push" standing instruction is REVERSED.**
   Boris: "push. this is in development and I don't want to lose your work." 158 commits
   pushed at s-rta-0906 boot (`1eff4f7..6858ec2`). Push routinely from now on; losing work
   is the risk being managed, not premature release.

2. **DO NOT HIDE, GREY OUT, OR DELETE DEAD UI. WIRE IT UP.** The "honesty batch" (L4) and
   the rack deletion (L-DEL) — both of which Harmony AND the Fable architect had recommended
   across two sessions — are REJECTED AS FRAMED. Boris considers those surfaces NECESSARY
   FEATURES that are not built yet, not lies to be hidden. He will confirm each once given
   detail. **Deleting/hiding a dead control is now the wrong default in this repo; building
   it is the right one.**

3. **THE CORE PRODUCT LAW: AUDIO CONTROLS THE VIDEO.** Boris, verbatim: "Every single
   parameter, including the ones you mentioned will get the same exact method to connect
   them to an audio signal or an oscillator. All will be timed. This is the core of our
   application. Audio controls the video."
   Consequences that bind design from here:
   - The 21 `UniversalParamControl` fields with no compute-and-apply path are NOT an honesty
     problem to be papered over. They are the visible edge of a MISSING UNIVERSAL MECHANISM.
   - "Every single parameter" means the connect mechanism is UNIVERSAL and UNIFORM — one
     method, not per-panel special cases. Today it exists in exactly two places
     (`EffectStackView.cpp` and `ClipInspector.cpp`'s sourceParams loop).
   - Sources are audio signals AND oscillators. "All will be timed" — a connected parameter
     is time-driven, not only level-driven.

4. **BPM multiplier is REAL and load-bearing.** Boris: the /4 /2 x1 x2 x4 buttons are "used
   by the main BPM of the application." The L6 recon's finding stands (it cannot be applied
   at the analysis publish point without corrupting GenreDetector's absolute-BPM buckets),
   so the multiplier belongs downstream of analysis, at the point video timing consumes
   tempo — consistent with #3. Deleting the control is OFF the table.

5. **The current UI is DISPOSABLE. Features first.** Boris: "After we build all the features,
   I will give you the new UI. It will be a welcome change, but we need to make sure all the
   features can work first. The UI we have right now will be scrapped."
   Consequence: **do not spend budget on cosmetic UI work, restyling, or hiding.** Spend it on
   MECHANISM that survives a UI rewrite — model, engine, persistence, timing, connection
   plumbing. When choosing between two fixes, prefer the one that outlives the UI.

6. **Owner-attended work is available this session** — Boris is around, and the app may be
   launched and quit "anytime you want." The single-instance blockade that disabled every
   app-level gate in s-rta-0905 does not apply while this holds.


- **SCREEN-SAFETY LAW — mandatory, every session, no exceptions.** Audio-DNA's output window is a
  real fullscreen window on Boris's actual monitors, not a headless test artifact. Never end a
  session with it open; never `pkill`/SIGKILL the app while it is open (close the window first,
  quit gracefully); verify the SCREEN itself (`screencapture -x` + read the image), not just the
  process table, before declaring a session safe to close; state the app/window state explicitly
  in the handoff; minimise unattended fullscreen drive in automated gates; a Boris-reported screen
  artifact outranks whatever lane is running. Citation: `.harmony/HANDOFF.md`, heading
  "SCREEN-SAFETY LAW — MANDATORY, EVERY SESSION, NO EXCEPTIONS".
  - Owner-attended-gate corollary, from the latest (2026-08-04d) session: **output-window
    ("L-OUT") gates are OWNER-ATTENDED ONLY** — ask Boris to drive the output test himself; never
    let an automated gate open fullscreen unattended. Citation: `.harmony/HANDOFF.md`, section
    "SESSION 2026-08-04d (secondary, slim) — THE SURFACE MAP + THE PLAN. AUTHORITATIVE OVER ALL
    ABOVE.", subsection "NEXT SESSION — START HERE", item 5.
- **Boris's binding scope for this build arc: get it working, then upgrade — ESSENTIALS ONLY
  (every main surface does what it presents itself as doing; no upgrades, no polish; anything that
  improves an already-working surface is parked).** Citation:
  `.harmony/essentials-plan-2026-08-04d.md`, heading "RULINGS", preceding line "BORIS'S SCOPE,
  BINDING."
- **Output = Strategy A: native output must show real deck content** (rejected alternative:
  declaring Syphon the supported output path — that fixes the headline surface by redefining it
  away). Citation: `.harmony/essentials-plan-2026-08-04d.md`, heading "RULINGS", item 1 under
  "SETTLED"; reaffirmed at `.harmony/HANDOFF.md` section "SESSION 2026-08-04d…", heading
  "BORIS'S RULINGS THIS SESSION", bullet "Output = Strategy A."
- **Display targeting: KEEP the surface, simplify the UI only** — do not delete it (it is the
  app's only path to a projector). Citation: `.harmony/essentials-plan-2026-08-04d.md`, heading
  "RULINGS", item 2 under "SETTLED"; reaffirmed at `.harmony/HANDOFF.md`, heading "BORIS'S RULINGS
  THIS SESSION", bullet "Display targeting: KEEP, simplify the UI only."
- **Legacy v1 row-1 controls + the 10 preset slots: DELETE.** Citation:
  `.harmony/essentials-plan-2026-08-04d.md`, heading "RULINGS", item 3 under "SETTLED"; reaffirmed
  at `.harmony/HANDOFF.md`, heading "BORIS'S RULINGS THIS SESSION", bullet "Legacy v1 row-1
  controls + 10 preset slots: DELETE."
- **The Honesty batch (hide/remove Record tab, Timing placeholder, Comp-Inspector dead blocks,
  dead param-source trio) and the Rack (`EffectsRackPanel`) deletion are OPEN, not yet Boris-ruled**
  — both are recommended yes/DELETE by Harmony and the architect, but the plan and the handoff both
  mark them explicitly STILL OPEN. Citation: `.harmony/essentials-plan-2026-08-04d.md`, heading
  "RULINGS", section "OPEN"; `.harmony/HANDOFF.md`, heading "BORIS'S RULINGS THIS SESSION", bullet
  "STILL OPEN: honesty batch · the rack." AMBIGUOUS BY DESIGN — cited as open, not resolved here.
- **The party that builds never verifies** is independently reaffirmed at the project level (not
  only inherited from system PREFERENCES.md above). Citation: `.harmony/HANDOFF.md`, section
  ">>> BIRTH PROMPT FOR NEXT SESSION (paste this) — supersedes every earlier one in this file",
  "METHOD NOTES THAT KEEP PAYING", bullet "Delegate recon; run the behavioral gate yourself. The
  party that builds NEVER verifies."
- **Repo-local gotcha overriding a general habit: re-grep by anchor text, never trust a cited line
  number — line numbers in this repo drift constantly, evidenced repeatedly this arc.** Citation:
  `.harmony/HANDOFF.md`, section ">>> BIRTH PROMPT FOR NEXT SESSION…", paragraph beginning
  "INHERITED FACTS IN THIS REPO HAVE A BAD TRACK RECORD."
- **A negative from a grep is only as strong as its pattern** — a wrong pattern produced a false
  "no resolution lock exists anywhere" claim this arc (the symbol was `setLockedResolution`, the
  grep looked for `lockResolution`). Citation: `.harmony/HANDOFF.md`, section "SESSION
  2026-08-04d…", heading "CORRECTIONS — including one of Harmony's own", item 1.
- **`.harmony/` is gitignored with many files force-tracked; `git add` prints an "ignored" WARNING
  and still stages, which breaks `&&` chains — use `;` instead.** Citation: `.harmony/HANDOFF.md`,
  section ">>> BIRTH PROMPT FOR NEXT SESSION…", RIG paragraph (final bullet before the stale-clone
  warning).
- **`~/projects/RealTimeAudio copy` is a stale duplicate repo (HEAD `f128bdc`, Jul 11) — confirm
  you are in the real one; HEAD should descend from `7d3a203`.** Citation: `.harmony/HANDOFF.md`,
  same RIG paragraph, final sentence.

## Not found / explicitly out of scope for this projection

- No system-level DECISION_LOG or PREFERENCES entry assigns RealTimeAudio (or foreign-repo
  secondaries generally) a specific model/compute tier for their own dispatched work — see the
  weak/ambiguous fleet-pin citation above; do not infer a firm rule beyond what is cited.
- No system-level "project gotchas override system defaults" rule text was found in
  `memory/DECISION_LOG.md` under that framing (checked via "project.local", "override.*default",
  "local gotcha", "repo-local.*wins" and "project convention" — no hits). The closest system
  mechanism is R1.2 in `harmony-references/doc-unification-rules.md` ("a secondary READS
  [advisory classes] for context but is not constrained by them") plus the project's own
  gotchas file taking precedence in practice for this-repo-specific facts (e.g. the line-number
  and force-tracked-`.harmony/` items above) — stated as observed practice, not as a cited
  system rule.

---

## 2026-09-05 (s167) — BORIS RULINGS, SPOKEN DIRECTLY

Ten questions were put to Boris in plain English at session start; he answered all ten and
volunteered one new feature concept. Verbatim answers preserved, with the reading I acted on.

1. **OUTPUT WINDOW: LATER. ENGINE FIRST.** Verbatim: "fix it later. engine first."
   The projector window rendering nothing of the composition is a KNOWN, ACCEPTED gap for now.
   Do not re-raise it as a blocker; the connection engine outranks it.

2. **RECORDING: REPLAY *AND* OFFLINE RENDER — AND A NEW THIRD THING.** Verbatim: "A and B. It
   can be used to make a video recording without using much memory or resources, and they could
   also be modified later if user wants to edit something to make it better before finalizing.
   Or the users should be able to take pieces of it and save it as a routine to be run. This is
   something new I just came up with."
   Three distinct capabilities, in his own priority order:
   (a) live replay inside the app;
   (b) offline render of the log to video — explicitly motivated by COST: an event log is tiny
       compared to video frames, so recording a set cheaply and rendering later is the point;
   (c) **ROUTINES — NEW SCOPE.** Take a PIECE of a recording, save it as a named unit, re-run it.
       Also: the log should be EDITABLE before finalizing.
   Consequence for the recorder lane: the event log is not a debug artifact, it is a
   FIRST-CLASS MEDIA FORMAT. Design its schema so slicing, editing and re-running are possible
   later — stable event ids, absolute + musical timebase on every event, no positional coupling.

3. **AUDIO IN THE VIDEO FILE: YES, BUT I TRIAGE THE ORDER.** Verbatim: "do this but triage the
   correct build order." Harmony owns the sequencing decision; the feature is approved, not
   deferred indefinitely.

4. **TEMPO IN SILENCE: KEEP RUNNING FROM THE LAST / TAPPED BPM.** Verbatim: "B". Modulation must
   not go still between tracks. (The bar-counter fix is being built regardless; this decides the
   policy that sits on top of it.)

5. **A CUE DOES NOT SURVIVE A STOP.** Verbatim: "A". Stopping a clip cancels a pending quantized
   cue — and the GLOBAL Stop button must do the same, closing the fifth "outlives its context"
   path found this session. Stop means stop.

6. **OPACITY LIVES AT EXACTLY TWO LEVELS: PER-LAYER, AND ONE MASTER.** Verbatim: "Each layer
   should have an opacity, and there is a master opacity. Anything else would make this
   confusing."
   This is a PRODUCT SIMPLIFICATION RULING and it overrides the general "build the dead UI"
   default for this specific surface: the Composition's SECOND opacity knob is not built as its
   own thing — it merges into Master. Per-clip opacity was not named; queried separately.

7. **HAND-BACK GLIDES, IT DOES NOT SNAP.** Verbatim: "smooth transition back." When a human lets
   go of a control that a signal owns, the value eases back to the signal. Confirms the glide
   defaults; snap-to is not the behaviour he wants.

8. **BUILD THE DRAWABLE TIMELINE CURVE.** Verbatim: "build."

9. **PRESET MIGRATION: NOT YET DECIDED.** Verbatim: "not sure. clarify and give recs." Owed him
   a plain-English explanation plus a recommendation. Do not migrate anything until answered.

10. **MACRO BANKS AT ALL THREE LEVELS.** Verbatim: "all. Global, layer, clip." The single shared
    bank of 8 is not the end state; the model must not box out per-layer and per-clip banks.

**Standing rulings from s166 remain in force** (push routinely; do not hide or delete dead UI —
build it; "audio controls the video", all tempo-locked, current UI will be scrapped).

### 2026-09-05 (s167) — FOUR CLARIFYING ANSWERS, AND ONE OF THEM RESHAPES THE PRODUCT

11. **OPACITY MULTIPLIES ACROSS THREE LEVELS, AND CLIP OPACITY IS A CEILING.** Verbatim: "I
    think it's the same mechanism So keep it. They stack anyway we can fade on the master we
    could fade opacity on the layer and on the clip. If for some reason, I want to have a clip
    that is permanently 50% opacity, I could just set that in the clip and regardless of what I
    do inside of the master and the layer, it won't get past 50%. This could be useful for many
    things."
    RESOLVED: `final = masterOpacity * layerOpacity * clipOpacity`. His "permanently 50%" example
    IS multiplication — a clip at 0.5 can never exceed 50% no matter what master and layer do.
    His earlier "anything else would make this confusing" (ruling 6) referred to the COMPOSITION
    carrying TWO opacity knobs at the SAME level, NOT to the three levels. The redundant second
    composition field merges into master. Three levels, one knob each, multiplied.

12. **THE CAPTURED AUDIO IS PART OF A RECORDING, NOT A VIDEO EXTRA.** Verbatim: "Depending on
    what was used for the track, it was either the Audio from the microphone or from the sound
    card or from whatever. It gets recorded Along with the log and we will test it to make sure
    it stays in time."
    A recording is `{lanes + audio}` — the input the app actually heard, whatever its source,
    captured alongside the log. Consequence: a recording is SELF-CONTAINED; offline video render
    does not need the DJ's original track file. And ALIGNMENT IS A DELIVERABLE HE NAMED: a
    headless sync test proving lanes and audio stay in time is part of the feature, not an
    afterthought. Hazard for the builder: the existing full-rate PCM ring buffer off the audio
    callback is SINGLE-CONSUMER (owned by the analysis thread) — this needs a second tap, not a
    free reuse.

13. **>>> THE RECORDING IS A SET OF PER-CONTROL TIMELINES, ABLETON-STYLE. <<<** Verbatim: "You
    can change it while it's playing or you can stop and change it on the timeline. All of the
    different sliders and buttons used in a performance will be logged on its own timeline. These
    timelines can be changed similar to how Ableton live works."
    THIS IS THE LARGEST SCOPE STATEMENT SINCE THE CORE PRODUCT LAW. It is NOT an event log with
    an editor bolted on. Every slider and button used in a performance gets ITS OWN editable
    timeline; the performer can override live while it plays, or stop and edit the timeline
    afterwards.
    Two consequences that reach code being written RIGHT NOW:
    (a) **A TIMELINE IS A CONNECTION SOURCE.** A parameter can be owned by an audio signal, an
        oscillator — or a timeline. Which means the hand-drawn curve Boris approved (ruling 8)
        and a captured performance lane are THE SAME OBJECT, one drawn and one recorded. If that
        holds, `ConnSource` must carry it in Lane 2, which is being built today. Sent to the
        architect as the single highest-value question in the spec.
    (b) **OVERRIDE SEMANTICS ARE NOW A REAL DESIGN SURFACE** — override / re-enable / overwrite,
        composed with the existing grip-and-hand-back rule. Ableton is the stated reference.

14. **PRESET MIGRATION — STILL OPEN.** Verbatim: "Clarify this." He asked for the QUESTION to be
    clarified, not the policy. Re-put to him in concrete terms: the only lossy case is a knob
    that had TWO drivers stacked on it, and rather than making him remember, Harmony offered to
    scan his real Presets folder and report which files (if any) are affected before anything
    changes. Do not migrate until answered.

### 2026-09-05 (s167) — THE SEVEN RECORDING QUESTIONS, ANSWERED (and the editing model he specified)

15. **REPLAY IS AUDIO-LOCKED, AND SNAPS TO QUARTER BEATS.** Verbatim: "When I play the recording,
    the audio should be locked in with the changes in the knobs. Maybe to make it easy let it snap
    too quarter beats for replay."
    The audio is the master clock on replay, not a parallel track hoped to stay in sync. Quantise
    replay events to 1/4 beat as the default.

16. **>>> EVERY KNOB HAS ITS OWN OPENABLE LANE SHOWING *KEY* MOVEMENTS — NOT SAMPLES. <<<**
    Verbatim: "Each knob will have its own layer where I could click it open and it will show me
    the key knob movements that I can drag and change then save as a new file, connected to the
    same audio."
    Three requirements in one sentence: (a) per-knob lane, collapsed by default, expandable;
    (b) what it shows is **KEY movements** — he already rejected sample-per-frame editing before
    anyone proposed it; (c) **save as a NEW FILE connected to the SAME audio** — versioning where
    the audio is referenced, not copied. A take is cheap to fork.

17. **ROUTINE CLIP HITS FIRE ON THE RECORDED LAYER.** Verbatim: "recorded".
    BUILT s-rta-0926 (commit 58b14d7 + ebbff22), slice 1: a routine's `deckRelative` targets resolve
    on the recorded layer at the ACTIVE deck at fire time (`Routine.h`, D2/D9).

18. **OVERRIDE AND EDIT MODEL — his own words, and they match the spec's TOUCH + OVERWRITE.**
    Verbatim: "The knob will supersede whatever is happening, And will snap back to the recorded
    track as soon as it is let go. It could be in record over mode or it could just be in play
    mode and play mode. The timeline doesn't change and then record mode. The timeline is updated
    based on the knob movement. Also, if we don't wanna move the knob and we just stop the
    timeline, we can drag the individual points. We could also have a way to set the resolution so
    we can drag a few points."
    - The hand always wins, and hand-back is a SNAP-BACK to the recorded lane on release
      (consistent with the glide-back rule everywhere else — same behaviour, his words for it).
    - TWO MODES: **play** (your move is temporary, the lane is unchanged) and **record-over**
      (your move rewrites the lane). This is exactly the spec's TOUCH default + OVERWRITE-when-armed.
    - Editing without touching a knob: stop the timeline and drag points directly.
    - **He asked for a RESOLUTION control himself**, and named the reason: "if we have 30 frames
      per second, that is a lot of points to drag." He is asking for the design; see the
      point-editing answer filed alongside this.

19. **AUDIO CAPTURE IS A SWITCH.** Verbatim: "we can save audio or not. use a switch." This
    REFINES ruling 12: audio is still part of a take by design, but capturing it is user-controlled
    rather than unconditional. Default on; the switch is real, not a hidden setting.

20. **ROUTINES BANK vs GRID: HARMONY'S CALL.** Verbatim: "whatever is cleaner or more universal."
    DECIDED: **its own bank of pads**, per the architect's recommendation. A routine spans many
    layers and parameters at once, so putting it in a clip cell — which means "one clip on one
    layer" everywhere else in the app — would overload a cell's meaning. A separate bank keeps
    both concepts honest and is the more universal shape.
    BUILT s-rta-0926 (commit 58b14d7 + ebbff22), slice 1: `Composition::routineBank` (8 slots,
    `Composition::kRoutineBankSize`), fired via REST/OSC/binding pads and the Record-tab Routines strip
    (8 pads + Save Routine row, lane 3), not a clip cell.

21. **STILL OWED HIM: question 3 (routine once-vs-loop) was asked without context.** Verbatim:
    "what is this in reference to?" — a fair complaint about the question, not an answer. Re-put in
    plain terms with the concept explained first. DO NOT count it as answered.

### 2026-09-05 (s167) — TWO MORE, AND THE FIRST ONE PARTLY CONTRADICTS AN ARCHITECT REFUTATION

22. **A ROUTINE IS A KIND OF SIGNAL, WITH LOOP/ONCE AS ITS CONTROLS.** Verbatim: "it would be
    great if we could set it to loop or play once. Perhaps you could treat this similar to a
    signal. It's a routine and controls similar to how a signal works. This is just a very
    specific signal rather than just an oscillator."
    ANSWERS the once-vs-loop question: **both, as a setting.**
    But it says something larger: he wants ONE MENTAL MODEL. Audio bands, oscillators and routines
    are all "signals" — things that drive other things — differing in kind, not in category.
    **NOTE THE TENSION, DO NOT PAPER OVER IT.** An architect refuted exactly this unification at
    the OWNERSHIP level, with four reasons that still stand: a connection is the exclusive owner of
    one parameter, while a routine spans many parameters at once, stacks with other routines and
    with the hand, is silent between its gestures, and includes BUTTON events (clip hits, deck
    switches) that are not connectable at all.
    RESOLUTION ADOPTED — his model is right about presentation, the refutation is right about
    mechanism, and they do not actually collide: routines get signal-like CONTROLS (loop / once /
    direction / quantize) and live in the same family in the user's head. The one place the
    difference surfaces is that a routine is FIRED AS A UNIT rather than picked inside a single
    knob's source list — because it is not about one knob. A routine's individual continuous lane
    CAN be printed onto a knob, and at that moment it becomes an ordinary curve source. Flagged to
    Boris explicitly rather than silently resolved.
    BUILT s-rta-0926 (commit 58b14d7 + ebbff22), slice 1: loop/once + quantize are live
    per-routine settings; `direction` (reverse playback) stays DEFERRED to a later slice (`Player`
    has no reverse clock — a decreasing position is a seek, not a direction flag).

23. **>>> DRAW, DON'T DRAG. <<<** Verbatim: "If the knob recording doesn't change, there's nothing
    to record if I drag the feeder from 0 to 100 and park it at 100 the last thing you record is
    the final movement at 100. Maybe we could just adjust the resolution for easy movement. We
    wanna snap to to grid control, toggle, or button. I think the simpler solution is that we could
    just draw rather than drag and have the grid control or free hand. Also, if we could select and
    move it left and right, that will snap to grid."
    Four decisions, and the third dissolves the problem I was solving:
    (a) **Only CHANGES are recorded.** A knob dragged to 100 and parked records the movement and
        then nothing — silence in a lane means "unchanged", not "absent data". Sparse by nature,
        which is also why a take is kilobytes.
    (b) **Resolution stays** as a control for comfortable editing.
    (c) **EDITING IS DRAWING, NOT POINT-DRAGGING.** You draw the shape you want over the lane,
        either snapped to a grid or freehand, chosen by a toggle. This sidesteps the entire
        "30 fps means 300 points" problem rather than managing it: the user never handles points
        at all, and the tool decides the point density behind the scenes.
    (d) **Selection moves horizontally and snaps to the grid** — retiming a gesture without
        redrawing it.

### 2026-09-06 (s168) — RECORDING PRIORITY, SPOKEN DIRECTLY AT SESSION BOOT

24. **THE EVENT LOG IS THE PRODUCT; VIDEO IS SECONDARY.** Verbatim: "I definitely want to record
    the performances. The event logger is crucial. We can record videos as well, but the events
    are more important."
    CONFIRMS and RANKS the s167 NOW/LATER cut rather than changing it: spec §D14's NOW list
    (lanes, take format, audio tap, T1 sync test, player, RecordPanel, REST) is the priority;
    the offline video render (`L-D`) stays LATER. Do not trade budget from the log core to the
    video path. Recording performances is APPROVED without further gating.

### 2026-09-06 (s168) — THREE ANSWERS, SPOKEN DIRECTLY

25. **DROP vs OSCILLATOR PHASE: MAKE IT A SWITCH.** Verbatim: "switch". Asked whether a
    structural drop should restart oscillator shapes or let them flow through, he ruled the
    MECHANISM rather than the taste: both behaviours exist and the user chooses.
    CONFIRMS what was built this session — `resetPhaseOnStructural`, per-oscillator, DEFAULT
    FALSE (flow through the drop). The switch is the answer; do not remove either branch, and do
    not quietly pick one at some later refactor. When the UI is rebuilt this needs to be a real,
    visible control, not a runtime-only flag.

26. **A ROUTINE RESTORES THE STATE IT WAS RECORDED IN.** Verbatim: "restore". Firing a saved
    routine first puts the layers and knobs it uses back the way they were at record time, then
    plays. Per the recommendation he accepted, a per-routine "start from now" switch remains,
    for routines meant to layer on top of whatever is live. Restore is the DEFAULT.
    BUILT s-rta-0926 (commit 58b14d7 + ebbff22), slice 1: `restoreState` defaults true and fires
    the routine's preamble through `Player::firePreamble` before its lanes play; `restoreState =
    false` is the "Start from now" switch. Reading of "the state it was recorded in" = the state
    at the SLICE'S OWN START, not the take's full checkpoint 0 — flagged to Boris as one open
    question (plan section 10.1), not a design change.

27. **A ROUTINE STARTS ON THE NEXT BAR.** Verbatim: "bar". Not the next beat. Per-routine
    override stays available, and the global Quantize setting overrides when it is on —
    consistent with how a quantized clip trigger already behaves.
    BUILT s-rta-0926 (commit 58b14d7 + ebbff22), slice 1: default `quantize = Bar`; the global
    Quantize setting overrides per-routine when it is on; "2 Bar"/"4 Bar" parity is read from
    `barCount` (the same counter a quantized clip trigger uses), never `totalBarCount`.

28. **>>> THE AUDIO IS THE ANCHOR, AND MANY TAKES SHARE ONE AUDIO. <<<** Verbatim: "2-4 hours.
    We need to be able to use the recorded audio (lets say from a first show of a tour) to go
    through all the slider recordings and clean them up. Does that make sense? The first take will
    never be perfect, but we could reuse the same audio that's locked to the files so the user can
    redo it. This can be for a whole DJ set which could be a few hours, down to a song which will
    just be between 3 to 10 minutes." And: "we can have multiple slider recordings per audio as
    the logs are low memory usage."
    THIS IS A WORKFLOW STATEMENT, NOT A SIZE ANSWER, and it reshapes the on-disk model:
    - **Duration envelope: 3-10 minutes (one song) to 2-4 hours (a full set).** Design for four
      hours, not for a demo clip.
    - **The captured audio is a FIRST-CLASS, SHARED, REUSABLE ASSET.** One night's audio is
      recorded once and then performed against repeatedly — record the show, then rehearse and
      clean up the knob work over the real audio until it is right.
    - **N lane-sets reference 1 audio.** Cheap because a log is kilobytes and the audio is
      gigabytes. This is the reason the format is sparse in the first place.
    CONSEQUENCES THAT REACH CODE ALREADY WRITTEN — do not defer these:
    (a) **Audio must NOT be a private copy inside each `.adna-take` folder.** As built today the
        tap writes `audio.wav` beside `take.json`; a user who re-does a 4-hour set five times
        would burn ~14 GB duplicating identical audio. Audio needs its own store, referenced by a
        stable id + content hash, with the take holding a REFERENCE and a sample offset. Ruling 16
        already said "connected to the same audio, referenced not copied" — this makes it binding
        for the recorder core, not just the versioning UI.
    (b) **Long-take timebase accuracy is now load-bearing.** A reviewer MEASURED ~1.2 beats of
        reconstruction drift over 40 minutes at a 0.03 BPM bias, because `RecorderClock` writes a
        tempo anchor only on a >0.05 BPM change and the map then linearly extrapolates. Over a
        4-hour set that is several beats. The periodic anchor and the use of exact per-breakpoint
        stamps stop being nice-to-haves and become required before this ships.
    (c) **Disk budget: ~690 MB/hour, so ~2.8 GB for a 4-hour night** — acceptable exactly BECAUSE
        it is stored once and shared. It would not be acceptable per-take.
    (d) The "clean it up afterwards" loop is the DRAW-DON'T-DRAG editor (ruling 23) applied to a
        real recording. Editing is the point of recording, not a bonus feature.

## 2026-09-24 (s-rta-0924b) — BORIS RULING, SPOKEN DIRECTLY
- **NO BLUETOOTH AUDIO, EVER.** Boris verbatim: "we will never use bluetooth audio for any reason. it is slow
  and bad. never use it again." Consequences: never test, gate, tune or design for Bluetooth input/output;
  the rig's audio is the built-in (or wired) device.
  Boris follow-up verbatim: "we will only use hard wired sound input or the onboard mic" — supported inputs are
  EXACTLY: a hard-wired input or the MacBook's onboard mic. The JUCE 8.0.4 CoreAudio temp-buffer overflow that
  crashes startup on a Bluetooth HFP headset (ASan: .harmony/.reports/s-rta-0924b/asan-bt-startup-crash.log)
  is therefore NOT a product priority; any fix is at most a guard that keeps the app off Bluetooth devices.
  Open calls that only existed for Bluetooth (16 kHz "Air" meter n/a; low-rate output device) are MOOT.

## 2026-09-25 (s-rta-0925) — Replay snaps back first (open call 1)
- Boris: "yes" — replaying a whole take FIRST restores the look at the moment Record was pressed (clips, deck,
  settings), then plays the moves. Replay must look the same as the original performance every time.
  Implementation status: TO VERIFY against code (roadmap lane s-rta-0925); if not built, it is a build item.

## 2026-09-25 (s-rta-0925) — Button name "Record Over" (open call 2)
- Boris: "record over" — the overdub button is labelled "Record Over" (current default; no change needed).

## 2026-09-25 (s-rta-0925) — End of replay: hold, don't stop (open call 3)
- Boris verbatim: "It should keep playing at with the current parameters at the end. If there is no audio input,
  then it should play according to the parameters."
- Read as: replay does NOT stop or hand back at the take's end. The last recorded state (clips, deck, parameter
  values) stays in place and the visuals keep running from it. If live audio is coming in, they react to it; if
  there is no audio input, they run on the parameters alone (oscillators, manual BPM, set values). [interpretation
  restated to Boris for confirmation 2026-09-25]
- Implementation status: TO VERIFY (today: with-audio replay sits at audio end; wall-clock replay keeps counting —
  fix-plan D6). Build item for the playback-end lane.

## 2026-09-25 (s-rta-0925) — Stop Recording during Record Over keeps the replay (open call 4)
- Boris: "keep playing" — Stop Recording ends only the new recording; the replay underneath keeps playing.
  Matches current behaviour (RecordPanelModel.h:123 caption). No change needed.
- Call 3 reading was restated to Boris; he answered call 4 without objecting (not an explicit confirm).

## 2026-09-25 (s-rta-0925) — Stop Recording: black text on red (open call 5)
- Boris: "go with rec" — black text on bright red (AA contrast). Already live; no change.

## 2026-09-25 (s-rta-0925) — Stored audio is never deleted by the app (open call 6)
- Boris: "never delete" — the app never deletes stored audio; cleanup is Finder-only. Live today; no change.
  (Sole existing exception stays: AudioStore::abandonAsset for a failed arm's own never-finalized asset.)

## 2026-09-25 (s-rta-0925) — Audio store location stays fixed (open call 7)
- Boris: "go with rec" — fixed ~/Documents/Audio-DNA/Audio for now; a choosable location (tour SSD) is not
  requested. Live today; no change.

## 2026-09-25 (s-rta-0925) — Optional name at Record time (open call 8)
- Boris: "Optional name at Record time" — a name box before Record; empty = date + take number.
- Code signs it is already built: RecordPanelModel.h:65-68 "The take's name as the user typed it: the folder
  name"; nameEnabled gated off while recording (:176). TO VERIFY live: type a name, record, check the take
  folder name and the "Last take:" status line; and empty-name fallback.

## 2026-09-25 (s-rta-0925) — Manual Resync re-aligns oscillators (open call 9)
- Boris: "go with rec" — a MANUAL Resync (BPMTracker::resetPhrase) re-aligns tempo-oscillator shapes to the new
  downbeat (a small visible jump is accepted). Automatic resets keep flowing (unchanged). NOT built today
  (HANDOFF s168 addendum 1: Resync does not rewind the monotonic counter). Build item.
- BUILT s-rta-0925 lane resync 9802458: plan-resync.md. `FeatureSnapshot::resyncBarOrigin` +
  `barsSinceResync()`; `BPMTracker::requestResync()`/`applyResync()` (analysis-thread-only, replacing the
  message-thread `resetPhrase()`/`resetBeatPhase()` write pair -- also fixes a pre-existing phantom-bar edge
  and a downbeat-level-contract break neither prior implementation had noticed); `POST /api/resync` +
  OSC `/audiodna/resync`; `gateOnce()` re-pins a `loop=false` one-shot on a manual Resync. ctest 493/493
  (484 + 9 new). Live probe `.harmony/probe-resync.sh` written, not yet run by Harmony.

## 2026-09-25 (s-rta-0925) — Master opacity: one knob; top-right fader linked; new Master Signal slider (open call 10 + new)
- Boris (after seeing the app): "remove the video slider keep master" — delete the Composition inspector Video-section
  "Opacity" twin (CompositionInspector.cpp:156, bound to masterOpacity :414). Keep "Master" (:135).
- Top-right master fader is a SHORTCUT to the Composition "Master" knob — both must be linked (same value, each
  follows the other). Verify two-way sync; fix if not.
- NEW (Boris): a slider top right, next to master opacity, for "master audio AND all signals including oscillators,
  etc. This is taking the gain slider and adding the other signals." = one Master Signal level that scales every
  modulation signal (audio features + oscillators + envelopes + ...). Design lane; exact semantics restated to Boris.
- BUG (Boris): right-click on a slider does not reset to default. Fix or plan to fix.
- Boris (follow-up): "go with your recs" — input Gain stays the pre-analysis input level (may move to Preferences);
  the new Master Signal slider is purely post-analysis reaction depth (visual reactivity), never touching detection.
- Boris: right-click reset fails on ALL sliders in the Composition tab (scope evidence for the rclick lane).

## 2026-09-25 (s-rta-0925) — No Xcode on the rig
- Boris removed /Applications/Xcode_16.app on purpose (disk space). The project builds with the Command Line Tools
  only (/Library/Developer/CommandLineTools, SDK MacOSX26.2). Do not reinstall Xcode or depend on it; any build dir
  configured before 2026-09-25 20:32 needs the recovery in notebook.md (s-rta-0925 Xcode entry).

## 2026-09-25 (s-rta-0925) — Master Signal Q1: Gain stays visible
- Boris: "keep gain visible" — the input Gain slider stays in the top bar (left, next to Audio). Master Signal is a
  NEW fader at the top right next to Master (design: .harmony/.reports/s-rta-0925/diag-mastersignal.md).
- Master Signal Q2 — Boris: "keep pulsing, this control is only for signals" — at 0% only signal->parameter
  connections stop moving controls; effects/sources that read the beat clock or audio uniforms directly keep pulsing.
- Master Signal Q3 — Boris: "save it" — masterSignal persists with the composition (like Master); absent -> 1.0.
- Call 1 implementation: BUILT + live-verified 2026-09-25 (merges 85afb70 + rr-fix; probe-step3 79/0 incl. snap-back rows
  in both replay modes). Not restored by design: tempo, audio transport, video playheads, per-slot effect bypass,
  layer transform (plan-roadmap.md §3.1).

## 2026-10-02 (s-rta-1002b) — App-evaluation feedback BF1-BF9: Boris's answers (full message: .harmony/boris-feedback-backlog.md)
- BF1 record-to-clip — Boris: "whole ouput of select layer, stop start and snap to bar, next empty cell on top layer no
  need to select it, video files yes" -> Harmony's reading: the source is the whole output OR the selected layer (both
  offered; covers either reading of "of / or"); press to start, press to stop, both snapped to the bar; the recording
  lands in the next empty cell of the TOP layer automatically; saved as video files.
- BF2 sync dial — Boris: "default, -500 to +500 in 1 ms steps plus can enter in the amount then click plus or minus to
  fine tune, we will rarely have an early delay so that is not a worry just a control we have in case we want to have
  all early without loudness signals" -> one dial per venue profile (saved by name), -500..+500 ms, 1 ms steps, a typed
  value plus +/- fine-tune buttons; EARLY moves beat-locked signals only (no loudness), accepted by Boris.
- BF3 codec + Show in Finder; BF7 bars wherever beats; BF8 double-click deck rename — defaults taken (no objection).
  BF7 must reconcile with the 2026-09-26 bar-counting ruling (BORIS_DECISIONS.md "Routines feel rulings": "we only need
  longer than 4 bar counts for routines ... Top bar count should go 1-2-3-4-1 etc").
- BF4 envelope editor — Boris: "default is good, bigger envelope, what would it's own window look like? perhaps we don't
  create it's own window till later when we have finalized the ui or would you rather do it now to build it then change
  the ui placement?" -> draggable / add / delete points, bendable segments, bar lengths, a BIGGER editor; Harmony's
  recommendation given: build it as one self-contained editor component in a bigger in-place panel now, own window later.
- BF5 envelope from a sample — Boris: "all defaults good" -> silent sample, stretched to the set beat / bar length,
  envelopes pulled out per band (lows / mids / highs).
- BF6 timeline source — Boris: "when I set any clip parameter to timeline it should be locked to the clips playhead, same
  with layer, not necessary for composition controls" -> a clip parameter connected to Timeline follows the clip's
  playhead; a layer parameter follows its layer's playing clip's playhead; composition parameters need not support it.
- BF9 — Boris (verbatim in BORIS_DECISIONS.md "Deck change stops the old deck; Persistent removed"): switching decks
  stops the old deck's clips (nothing plays invisibly); the Persistent layer feature is REMOVED; ignore-column stays as
  the only "keep this layer" control. Supersedes 2026-09-26 (persistent over another deck, deck switch mid-fade,
  persistent in the layer strip) and CLAUDE.md rule 15. OPEN follow-up asked: what a deck shows when you come back to it.
- BF9 CLARIFIED (2026-10-02 14:44:36) — Boris: "when I switch between decks, do not change the clips playing in the layers or how they are
  playing. treat the decks as just a box of clips and I can switch between 20 decks looking for a clip and the playing
  will not be affected. does that make sense?" -> SUPERSEDES Harmony's earlier reading "deck change stops the old deck"
  and the follow-up question (resume vs empty) — moot. Model: layers = one shared playing stack; decks = boxes of clips;
  a deck switch changes only the grid; firing a clip puts it in its row's layer, replacing what played there. Consequence
  DEFAULTS put to Boris (no answer yet = default): layer settings (layer effects / opacity / blend / transition /
  autopilot) belong to the shared layer, one set for all decks (old shows: the first deck's layer settings win); every
  deck shows the show's layer rows; the deck-to-deck transition fade is removed (a deck switch has nothing to fade); the
  layer strip shows each layer's playing clip and the deck it came from; Persistent removed; ignore-column stays.
  Built (bf9b, s-rta-1002b; merged into main in s-rta-1003; ruling-bf9b + ruling-bf9b-merge): Composition::layers +
  Deck::rows + ClipRef (deck id, column) in the tuple; a deck switch changes only the grid; the column header lit only on
  the firing deck; the TopBar Fade removed; old shows -> the first deck's settings (one app-log line, nothing on screen).
  NOT on screen, by Boris's rulings of 2026-10-03 (sections below): no strip badge, no tab dot, no load notice, no Undo
  Remove button, no Remove Deck sentence. Defaults taken: Q1 a deleted deck's playing clip keeps playing until replaced;
  Q2 a loaded deck with more rows adds layers; Q3 withdrawn (genre deck auto-switch inert); Q4 Undo skips deck switches
  (ruling-bf9b amendment 10 flipped the plan's default); Q5 a column fire empties a layer whose cell is empty.

## 2026-10-03 (s-rta-1003) — Boris's answers to the Oct 2 page + new feedback BF11-BF30 (full message verbatim: .harmony/boris-feedback-backlog.md "Boris feedback of 2026-10-03"; recorded 2026-10-03 13:32:39)
- Column re-fire — Boris: "Firing a column that is already playing should restart its videos" -> a column fire restarts the
  videos of a column that is already playing.
- Layer strip — Boris: "The layer strip does not need to show the deck a clip is playing from." -> the strip's source-deck
  badge (ruling-bf9b amendment 16) is removed; SUPERSEDES the 2026-10-02 consequence default "the layer strip shows each
  layer's playing clip and the deck it came from" (the playing clip stays; the deck does not).
- Deck switch — Boris: "When I switch decks, the clips in the layers disappear. They should remain. The output also goes black
  when I switch decks in there are clips playing in a layer." -> restates "Decks are boxes of clips"; the app he tested does
  not contain the deck change yet.
- Record to clip — Boris: "When record to clip, we want to see the recording in the layer it is recording to while it is being
  recorded."; "Recording a clip should be prominently displayed, so it can be recorded as long as the user likes. There's no
  limit except hard drive Space."; "The recording should look exactly like the output. If there's a logo, it should look
  exactly as it's displayed."; "The recording will always play like a regular clip once it is recorded." -> no automatic end
  (replaces the 64-bar default); a prominent recording display; the recording is visible at its destination while it runs;
  the recording matches the output; a finished recording is an ordinary clip.
- Clip transport — Boris: "As far as a video clip is concerned, it is in two categories, either BPM synced throughout the
  whole thing, or just playing with a speed control. If it is BPM synced, regardless of the length, it is synced to the
  current playing BPM and it has bars that the user can set. We automatically set bars for a cliff, but the user can change
  the bars in it by amount." / "Regardless of the clip being BPM or speed controlled, if we are in Qantize mode, it is
  triggered on time by the Qantize method.  There should be no problem playing clips that are set up for BPM and clips that
  are set up for speed at the same time. Their speeds are just controlled differently."
- Sync — Boris: "For sync control we should have a way to remember it as part of a composition save."; "Sync lives in top
  bar"; "The user of the application will tap tempo and keep the application in time with the music. ... The visuals should
  be on the delay. The music is in time and the visuals should be delayed or a little ahead depending on how the system is
  wired." -> the sync setting is saved with the composition; its control is in the top bar; the tapped beat stays in time
  with the music and the visuals carry the offset.
- Envelopes — Boris: "I want to be able to use the whole thing or to shorten it manually but clicking to timing marks."; "I
  am fine using the envelope editor in the signal tab. I want all of the controls to work in that one rather than an extra
  bigger envelope window." -> NO bigger envelope editor (SUPERSEDES 2026-10-02 BF4 "bigger envelope"); every envelope control
  lives in the Signal tab's editor; a long sample is used whole or trimmed by hand on timing marks.
- Timeline — Boris: "For Milk drop there is no timeline, but there are effects." -> page Q24 (a).
- Quantize / units — Boris: "Quantize 1 bar, 1/2 bar, 1/4, 1/8, 1/6"; "Let’s get rid of beats and just have bars in most
  places unless beats are necessary. For setting a clips beats, these are done with beats and not bars. For most other
  things, we will use bars. For the circle at the top that counts off 1234 and then starts over, those are beats as well.";
  "I have beats and seconds in the milkdrop editor. I only need beats."
- Codecs — Boris: "We should pick the most optimal Kodex to use and not worry about the other ones. Web M is not necessary. I
  want codecs that decode easily and play well"
- Deck file names — Boris: "If there are ‘/‘ characters in a decks file name, do not worry about adding sub folders. They are
  just characters. If there are characters that are no good, then let me know and we will create a fix together"
- Keying / compositing — Boris: "Can you figure out how to get everything working in the keying menu? Does the keying slider
  actually do anything? Maybe we get rid of it. Some elements in the keying menu work and some don’t. Should we use the
  transparency slider to do the work for keying elements? Also review the compositing menu, right slider in layer strip.
  These elements don’t work: Creative, 3D,"
- Playhead drag — Boris: "When I grab the play head and move the timeline, I do not want it to jump back to where it was or
  where it should be playing before I grabbed it. I wanted to keep playing at the same speed, but play from wherever I drop
  the play head."
- Gain — Boris: "Gain slider in top bar could be twice as long."
- Everything else — Boris: "All the other defaults are good." -> every Oct 2 page question he did not name takes its default
  (list: boris-feedback-backlog.md BF30). Page item 35 (on-screen test strip) keeps its default: NOT run without his OK.

## 2026-10-03 (s-rta-1003) — Boris's answers to the 14 clarifying questions (recorded 2026-10-03 14:03:17; questions as asked: boris-feedback-backlog.md)
- Q1 deck tab dot — Boris: "drop" -> no dot on a deck's tab; with BF14 nothing on screen shows which deck a playing clip
  came from. SUPERSEDES ruling-bf9b amendment 16(a)-(c).
- Q2 fire again a video that was replaced — Boris: "restart" -> every fire starts a video from its start; the "resume where
  it left off" behaviour (bf9b C3, K10 (ii)) goes. Built in the transport lane.
- Q3 live picture in the destination cell while recording, not played into the output — Boris: "yes".
- Q4 a single-layer recording keeps its see-through parts — Boris: "yes".
- Q5 unit of a BPM-synced clip's length — Boris: "beats". Q6 number box plus /2 and x2 — Boris: "both".
- Q7 automatic length from the current BPM — Boris: "not current bpm but have a bpm and beats input so user can do it by
  numbers and a x2 and /2 control to double or half easily" -> a BPM-synced clip has a BPM input and a beats input.
- Q8 "1/6" = 1/16 — Boris: "yes". Q12 MilkDrop preset change labels — Boris: "bars".
- Q9 the beat wheel stays where he tapped, only the picture is shifted — Boris: "yes". Q10 opening a composition loads its
  saved venue and sync value — Boris: "yes".
- Q11 sample trim — Boris: "drag handles that snap".
- Q13 bad characters in a deck file name — Boris: "switch to - is fine for all bad chars".
- Q14 other codecs — Boris: "no more tuning" (they still open and play as today).
- Q7 follow-up (recorded 2026-10-03 14:10:11; starting values of a BPM-synced clip's BPM and beats boxes) — Boris: "7 follow up: default and
  bpms for clips to 120" -> a clip with no BPM in its file name is taken to be 120 BPM (Harmony's reading of "default": the
  proposed rule stands; the exact-120 reading is noted in boris-feedback-backlog.md).
- Q7 starting values, CLARIFIED (recorded 2026-10-03 14:10:25) — Boris: "default all bpms to 120" -> every clip's BPM starts at 120.
  SUPERSEDES the line above (Harmony's reading "the proposed rule stands" was wrong).

## 2026-10-03 (s-rta-1003) — Boris's answers A-E (recorded 2026-10-03 14:55:32; questions as asked: boris-feedback-backlog.md)
- Undo and the live layers — Boris: "Let's not allow control Z to change anything that is live in the layer strip. It changes
  anything else" -> Undo never changes what is playing in a layer; it undoes every other kind of change. SUPERSEDES the
  2026-10-02 default Q4 wording "Cmd+Z always undoes your last real change, like a clip you fired" (a fired clip is no
  longer undone) and ruling-bf9b-merge AM-7's exception (Undo of a Load Deck that added layers). Built in its own lane
  after the bf9b merge.
- Clip tab on a deck switch — Boris: "yes it stays in clip tab regardless of deck" -> the Clip tab keeps the clip being
  edited whatever deck is shown.
- Gain — Boris: "yes" (twice as long, same scale).
- Sync in a show — Boris: "every show remembers it's sync" -> every saved composition carries its venue and sync value, 0
  included; opening it sets the dial to that value. SUPERSEDES ruling-bf2-delta's default for a show saved at "Default" 0.
- "Undo Remove" button (recorded 2026-10-03 14:58:10) — Boris: "I don't wanna see an under removed button at all. We just use control Z. The
  only place that we will see undo remove, will be in the top edit menu." -> no Undo Remove button anywhere on the deck tab
  row; Cmd+Z and Edit > Undo are the way back. SUPERSEDES ruling-bf9b amendment 16(d)'s button and closes ruling-bf9b-merge Q-D.
- Remove Deck sentence (recorded 2026-10-03 14:59:28) — Boris: "yes remove the visible line. not needed" -> after a Remove Deck nothing is
  written in the top text line; no button, no sentence. SUPERSEDES ruling-bf9b-merge AM-12's "Remove Deck sentence in the
  file label" and ruling-bf9b amendment 16(d).
- No "what happened" texts (recorded 2026-10-03 15:30:49) — Boris: "We don't need any text indicating what has happened or what has happened.
  That is something that happens online and is not necessary in this application. It is extra overhead and bloat. Please
  remove it cleanly and completely." -> no on-screen notice announces an event. SUPERSEDES ruling-bf9b amendment 9(d) (the
  load notice) and 16(d), ruling-bf9b-merge AM-12's sentence, and ruling-bf2-delta's composition-open sync notice.
- No "what happened" texts, APP-WIDE (recorded 2026-10-03 15:42:01) — asked whether texts that were in the app before today go too, with
  failure reports kept until he had seen a list, Boris: "remove the list entirely and cleanly" -> every event-announcing
  on-screen text in the app is removed, pre-existing ones and failure reports included. SUPERSEDES every earlier ruling that
  created such a text (e.g. the yellow note on a microphone fallback).
- Edit menu (recorded 2026-10-03 16:06:40) — Boris: "we should have an edit menu." -> the app gets an Edit menu (Undo / Redo); the Undo item
  is where "Undo Remove Deck" is read. Built in the next UI pass.
- Failure messages — OPEN, NOT A RULING (recorded 2026-10-03 20:58:42, s-rta-1003b) — told "failure messages (e.g. a failed save) — keep or
  remove? Default: they go with the rest", Boris: "how would a save fail? not sure we need that" -> a question and a lean.
  The 2026-10-03 15:42:01 ruling ("remove the list entirely and cleanly") stays the binding text; Harmony answered and asked
  back whether the two save-failed alerts (facts-notices.md B06, B08) are the one exception. Default until he answers: they go.
- Failure messages — RULED (recorded 2026-10-03 20:59:50, s-rta-1003b) — asked "keep the two \"Save failed\" boxes as the one exception, or
  remove them too?" after Harmony recommended keeping exactly those two (Save Composition failed, Save Deck failed), Boris:
  "ok. the only fail message will be a failed save. remove all others" -> the save-failed alert stays (facts-notices.md B06,
  B08); every other failure text and every event text is removed. AMENDS the 2026-10-03 15:42:01 ruling ("remove the list
  entirely and cleanly") by this one exception. Open edge (Harmony's default, not his words): a failed TAKE or ROUTINE save
  message goes with "all others".

## 2026-10-03 (s-rta-1003b) — Boris's answers to the questions page (recorded 2026-10-03 23:46:03; questions as asked: boris-feedback-backlog.md, same stamp)
- All defaults, except the ones below — Boris: "answer to 12 questions. all defaults except for:"
- Playhead held still (Q2) — Boris: "2 b" -> while the mouse is down and still, the picture waits on that frame until he lets go.
- Playhead and the in / out points (Q10) — Boris: "If we set the inpoint and endpoint on the timeline of the clip, I should not be able
  to drag outside of the points. It should be as if that is the extent of the timeline unless I let go and drag one of the in or out
  points." -> the playhead cannot be dragged outside in..out; only moving an in or out point changes that extent.
- Beats, not bars, for a clip (Q4) — Boris: "For setting the clip beat marks, the clips are set with beats not bars. x2 or /2 are beat
  changes. bars will confuse this. A bar is for beats and is used in places where longer durations make sense and beats are used where
  exaggerations makes sense. We should be very clear where we are using beats and bars, but they are essentially the same thing like
  feet and inches" -> a clip's length and marks are in beats; x2 and /2 change beats; every place states its unit.
- A BPM-synced clip stays on the beat (Q9) — Boris: "9 b" -> the app keeps nudging it back onto the beat by itself.
- Cmd+Z and the layer strip (Q3) — Boris: "cmd z Does not change anything in the layer strip which is by default live based" -> Undo and
  Redo never change anything in the layer strip. WIDENS the 2026-10-03 rule "Let's not allow control Z to change anything that is
  live in the layer strip. It changes anything else".
- Failed saves (Q12) — Boris: "12 b" -> OPEN: the page's question 12 was rewritten after it was first opened and its B changed
  meaning; Harmony asked him which he read. Until he answers: every save he presses shows the box when it fails (ruling-notices H-1).
- Beat lines (new) — Boris: "when the Video is in beats rather than adjust by speed mode, the beats are shown with lines in the play
  head area whereas if it was speed control, it's just the basic play head and with beats control there are lines for each beat in
  the play head area and the play head moves past them on time" -> in beats mode: one line per beat in the playhead area, and the
  playhead crosses them on the beat; in speed mode: the plain playhead.

## 2026-10-04 (s-rta-1004) — Boris's answers to the 25 questions (recorded 2026-10-04 11:59:06; whole message verbatim + questions as asked + the 8 Resolume screenshots: boris-feedback-backlog.md, same stamp)
- All defaults, except the ones below — Boris: "All defaults good except for these:"
- A paused clip that is fired (Q1) — Boris: "when you pause a clip and then fire it, it stays, paused" -> it stays paused.
  REPLACES his default of 2026-10-03 (it played from its beginning). Where it stays: OPEN (question 26).
- Playhead held still (Q2) — Boris: "2 b" -> the picture waits on that frame until he lets go. Unchanged.
- Playhead and the in / out points (Q10) — Boris: "there are two conditions here. 1 the in point and out point of the timeline
  has not been set. In this condition, if you drag the play head, it cannot go beyond the edges of the time. 2 in and/or out
  points have been moved. In this condition, if you drag the play head, it cannot be dragged outside of the in and out points.
  You can click outside the timeline and In-N-Out points, but that won't do anything. The play head can only go to the edges as
  they are defined." -> the playhead never leaves the timeline's ends, nor the in and out points once they are moved; a click
  outside does nothing.
- The clip's transport control (Q4) — Boris: "I want you to study these images and mimic exactly how resolume is doing. It's
  transport control. Also, here is details from their manual on how the transport control works." -> the clip transport panel
  mimics Resolume's (screenshots .harmony/.reports/s-rta-1004/boris-resolume/, manual text in the backlog). RE-OPENS the
  transport plan's panel. Scope OPEN (questions 27-30).
- Bars, not beats (Q7) — Boris: "lets do bars here not beats. I know I said beats before but lets do bars" -> a clip's length
  is set in BARS. REVERSES "the clips are set with beats not bars" (2026-10-03 23:46:03).
- A BPM-synced clip stays on the beat (Q9) — Boris: "9 b" -> the app keeps nudging it back by itself. Unchanged.
- Cmd+Z and the layer strip (Q3) — Boris: "cmd-z does not affect anything in layer strip: play, play reverse, pause,
  transparency, bypass, solo, etc. if it is  the layer strip, cmd-z does not affect it. If a clip is triggered and plays, it is
  not affected." -> unchanged rule, now with his list.
- A removed layer (Q5) — Boris: "default is ok. If a layer strip is deleted, user can ctrl-z to get it back, but clip is not
  playing" -> Undo brings a removed layer back, its clip not playing.
- Sync from a controller (Q13) — Boris: "yes and for shows that are pre-programmed, this sync matters more. Key and pad is
  fine, knobs can skip the sync" -> a key or a pad is enough; no knob / fader binding for Sync.
- Which failures show a message (Q12) — Boris: "we should list all the things that can be saved, but if the user is changing
  things around settings, etc., and they do not save the composition, nothing is saved. If the user is recording a clip live,
  and that is not saved in a message should show. If the user is recording the show, that is not saved, and that should be
  shown. Nothing else." -> a live clip recording that was not saved and a show recording that was not saved show a message.
  REPLACES "12 b". OPEN: the "Save failed" box of Save show / Save Deck (question 34), what "recording the show" names
  (35), what an unsaved show means for things the app keeps by itself (36). OWED to him: the list of everything that can be saved.
- Record cannot start (Q19) — Boris: "display not enough HDD space to record." -> that state is displayed. Where: OPEN (37).
- Whole bars (Q20) — Boris: "we are going to use whole bars instead, so this doesn't happen. To create a 120 BPM, we figure out
  some kind of math and look at how resolution does it. It creates a nice in and out point and even amounts of bars in the
  timeline for 120 bpm. If the user pulls the outpoint in, then within that same amount of bars, it goes through less video,
  appearing to play slower, and if the user pulls the outpoint out, and the same amount of bars, it covers more video appearing
  to play faster." -> a BPM-synced clip spans a whole number of bars; the marked part (in to out) is played over that many
  bars, so moving the out point changes how fast the video looks. How the first number and out point are chosen and which
  numbers are allowed: OPEN (31, 32).
- Dropping the playhead between beats (Q24) — Boris: "yes but this will be bars now" -> default A (it plays on and eases back
  into time), in bars. Beat or bar: OPEN (33).
- The S fader in bars mode (Q25) — Boris: "doubles and halves and then smaller fractions to higher multiples, just like
  resolume does" -> it steps through halves and doubles as Resolume's Speed does; the exact list is read from Resolume.
- Every other question (6, 8, 11, 14, 15, 16, 17, 18, 21, 22, 23) takes its default.

## 2026-10-04 (s-rta-1004) — The sync dial is REPLACED by a Delay per output screen, as Resolume (recorded 2026-10-04 12:08:21 and 12:09:11; verbatim + screenshot: boris-feedback-backlog.md, same stamps)
- Delay per output screen — Boris: "I think we should copy what resolume does for delay. each output screen can be delayed and
  that is set on output display properties. Makes it simpler. These are the resolume screen output adjustment window" -> each
  output screen gets its own Delay, set in that output's display properties.
- Replace, not beside — Boris: "replace our sync with this" -> the sync dial (top-bar SYNC, the room list, earlier / later, the
  Sync key and pad targets) is not built further and does not merge. SUPERSEDES for the dial: "Sync lives in top bar";
  "-500 to +500 in 1 ms steps ..." (2026-10-02); "every show remembers it's sync" (2026-10-03 14:55:32) as far as it names the
  dial -- where a screen's Delay is remembered is OPEN (question 38).
- Lining up the beat — Boris: "We need to do something smart where we can move the beat forward or back to get it to match the
  image exactly but that's something that user can do. We can just put that in our manual" -> the user moves the beat; the
  manual says how. With which control: OPEN (question 41).
- The purpose — Boris: "so the video matches the audio at the soundboard or wherever the vj is stationed in middle of room
  preferably" -> the Delay is set so that picture and sound agree where the VJ stands.

## 2026-10-04 (s-rta-1004) — Boris's answers to clarifying questions 26-36 (recorded 2026-10-04 12:21:01; verbatim + questions as asked + the quit screenshot: boris-feedback-backlog.md, same stamp)
- A fired paused clip (26) — Boris: "stays in layer strip paused" -> it stays paused, in the layer strip. Which frame: read as
  the frame it was paused on (INFERRED; told to him as a reading).
- SMPTE and DJ-player modes (27) — Boris: "add this to our build plan later after core elements are built and tested, but leave
  a dropdown menu for those items and grey them out for now" -> later; listed greyed in the mode menu now.
- Random and BeatLoopr (28) — Boris: "build random and beatloopr" -> both are built.
- The fire menu (29) — Boris: "can you clarify?" -> OPEN; re-asked with an example as question 42.
- An uneven clip (31) — Boris: "always work with multiples of 4. If it is uneven, then move the outpoint in to keep those
  multiples. Any other way will not work with music. Music and especially DJ music, is always in multiples of four." -> the
  app moves the out point in; the clip is never stretched to fit.
- Which bar counts (32) — Boris: "c only multiples of 4" -> OPEN: C as asked was "Only 1, 2, 4, 8, 16, 32"; which set he
  means is asked as question 43.
- Dropping the playhead mid-bar (33) — Boris: "b can you program this reliably or should we change the plan?" -> B: the clip
  slides until its bars sit on the music's bars. His question is Harmony's to answer with a measurement, not a guess.
- The two recordings (35) — Boris: "we can only record the show which is recording all the parameters and the actual audio
  file, or recording a clip which records the actual video content into a video file and displays it in the nearest open
  cell" -> "the show" = the performance take with its audio; "a clip" = video into a file, shown in the nearest open cell.
- Quitting (36) — Boris: "when the user quits, we need to have a secondary window open to say this, which is what resolume
  does. Look at the image." -> a quit window as Resolume's: "Do you really want to quit? All unsaved progress will be lost."
  with Quit / Cancel / Save & Quit.
- Bars only on the timeline — Boris: "Timeline only shows bars. The only place we see beats is in the circle with 4 positions
  in top bar that shows the 4 beats repeating." -> the clip timeline draws bar lines only. REVERSES "there are lines for each
  beat in the play head area" (2026-10-03 23:46:03).
- Not named by him, defaults stand: 30 (no BPM box on the clip), 34 (the "Save failed" box stays for Save show / Save Deck),
  37 (the disk-space sign is a line in the Record tab).

## 2026-10-04 (s-rta-1004) — Boris's answers to questions 39-41: output settings, Syphon, the beat nudge (recorded 2026-10-04 12:23:42; verbatim + questions as asked: boris-feedback-backlog.md, same stamp)
- All of Resolume's Screen settings (39) — Boris: "yes add all of those output settings" -> every output gets Device, Delay,
  Opacity, Brightness, Contrast, Red, Green, Blue.
- Syphon (40) — Boris: "syphon is an output and treated with same output settings as a screen" -> Syphon gets the same output
  settings as a screen, the Delay included.
- The beat nudge (41) — Boris: "user can nudge main bpm forward or back and this needs to be displayed as "off beat by [+/- X]
  ms'" -> a control that moves the main beat earlier or later, with the text "off beat by [+/- X] ms". REPLACES this morning's
  "that's something that user can do. We can just put that in our manual" as far as "nothing new is built" was read from it.
- Not named by him, default stands: 38 (a screen's settings are remembered with the screen, whatever show is open).

## 2026-10-04 (s-rta-1004) — Boris's answers to questions 42-46: fires, 4 bars, recording boxes, Tap / Resync and the nudge (recorded 2026-10-04 12:31:04; verbatim + questions as asked: boris-feedback-backlog.md, same stamp)
- The nudge — Boris: "You are exactly right. A nudge just moves the placement of the downbeat in time not the tempo." -> the
  nudge moves the downbeat's place in time; the tempo is untouched.
- Every fire from the start (42) — Boris: "42 default" -> a fired clip starts at its beginning, every time; no per-clip
  "carry on" menu is built.
- Short clips (43) — Boris: "a short clip can have 4 bars but it will move super fast" -> a clip too short for 4 bars still
  gets 4 bars. OPEN: the set of bar counts (read as 4, 8, 12, 16 ...) and that such a clip plays SLOWER, not faster
  (question 47).
- What a recording holds (44) — Boris: "We should be able to record the video into a cell which will always be recorded to a
  folder. We can record the output to parameters which will record the audio file and record all the parameter movements
  which saves on hard drive space. We can also record the video file of the show, which will take a lot of hard drive space.
  We can also record the output, video file and the output parameters and the output audio. These are all check boxes on what
  will be recorded." -> check boxes choose what is recorded (video file, parameters, audio); a clip recorded into a cell is
  always also written to a folder. Which boxes: OPEN (question 48).
- Tap, Resync and the nudge number (45) — Boris: "If I tapped the tempo again, to set the tempo, the time does not change. If I
  press re-sync, then it does re-sync and that changes by how far off the beat we are." -> Tap leaves the number alone;
  Resync changes it. To what: OPEN (question 49).
- The beat circle (46) — Boris: "46 default" -> it moves with the nudge.
- What the nudge moves (recorded 2026-10-04 12:31:21; arrived mid-turn) — Boris: "If we are shifted forward or back, everything that is
  connected to BPM shifts forward or back. I mean everything. If the user twist the knob in real time, or triggers a clip, that
  is not affected unless it's set to be quantized" -> one shifted beat for every beat-driven thing; a knob turned or a clip
  fired by hand is not shifted; a quantised fire lands on the shifted beat.

## 2026-10-04 (s-rta-1004) — Boris's answers to questions 47-49 (recorded 2026-10-04 12:41:09; questions as asked: .harmony/.reports/s-rta-1004/boris-clarify-47-49.md)
- Bar counts and short clips (47) — Boris: "47 default" -> 4 bars is the least; bar counts are 4, 8, 12, 16 ...; a short clip
  starts slow and Speed makes it fast (the question's own words).
- The Record boxes (48) — Boris: "48 b" -> three boxes, each on its own: Parameters, Audio, Video.
- The nudge number after a Resync (49) — Boris: "49 default" -> it reads 0; Resync is a fresh start.
- Not named by him, default stands: 50 (the app remembers key and MIDI settings by itself, on this computer).
- Key and MIDI settings between launches (50; recorded 2026-10-04 12:41:19; arrived mid-turn) — Boris: "50 default" -> the app remembers
  them by itself, on this computer, whatever show is open (Export / Import stay).

## 2026-10-04 (s-rta-1004) — One Save: everything the user saves is saved with the show (recorded 2026-10-04 12:50:58; verbatim with his pasted list: boris-feedback-backlog.md, same stamp)
- One Save — Boris: "All of these things should be saved when a show is saved. There's no reason to save them separately:"
  (followed by the nine lines of his page's list A: the show, a deck, Collect Media, an effects look, FX Save, key and MIDI
  settings, the window layout, a snapshot, Save Routine) -> saving the show saves its decks, effects, key and MIDI settings,
  window layout and routines. OPEN: whether Save Deck and the effects saves stay as an export (80); key and MIDI settings
  with the show REVERSES "50 default" (81); Collect Media at every Save (82).
- Key and MIDI settings, changed (recorded 2026-10-04 12:52:00) — Boris: "50 change answer to B and also saved on the computer for other shows
  to keep these as these are computer and midi hardware settings" -> saved with the show AND kept on the computer for other
  shows. REPLACES "50 default". OPEN: which set is live when a show's differs from the computer's (question 83).

## 2026-10-04 (s-rta-1004) — Boris's answers to questions 80-82: decks through the list of shows, keys and MIDI, Collect Media (recorded 2026-10-04 12:56:12; verbatim + screenshot: boris-feedback-backlog.md, same stamp)
- Decks (80) — Boris: "A deck will go into all of the decks that are available and they are the decks that are available with
  a show like this is how resolume does it:" (+ a screenshot: a show named Example, "4 Decks", its four deck names) -> no
  separate deck save; the list of shows opens each show to its decks and a deck is taken from there.
- Key and MIDI settings (81) — Boris: "with show and app. these stay. if a user starts another show file from scratch, they can
  import the settings from another show file." -> kept on the computer AND saved in the show; settings can be imported from
  another show file.
- Collect Media (82) — Boris: "82 default" -> it stays its own command; Save does not copy media.
- Key and MIDI settings, again (83; recorded 2026-10-04 12:56:43; arrived mid-turn) — Boris: "83 they are in a show and can be imported to
  another show" -> they live in the show; another show gets them by import. OPEN: whether opening a show changes the live
  keys (question 85).
- Effects looks (84; recorded 2026-10-04 12:59:09; arrived mid-turn) — Boris: "every effect has many looks with specific parameter setups.
  these are saved with the app. always." -> the looks stay, kept by the app for every show; they are not part of "one Save".
- Do the keys change when a show is opened (85; same stamp) — Boris: "yes but you import the setup from the most recent show"
  -> yes, the show's keys take over; a new show takes its setup from the most recent show.
- The old look buttons (86; recorded 2026-10-04 13:22:56) — Boris: "86 default yes" -> the small Save and Load, FX Save and the ten slots are
  removed now; looks per effect (kept by the app, always) are their own build.

## 2026-10-04 (s-rta-1004) — Boris's answers to questions 71, 72, 74: no slide, pause is the clip's and is saved, Speed to 10 (recorded 2026-10-04 14:06:21; questions as asked: .harmony/.reports/s-rta-1004/boris-clarify-71-74.md)
- No slide (71) — Boris: "71 b" -> a clip out of time plays on and cuts ONCE, on the next "1", into time. REPLACES the
  slide he chose in question 33 ("b can you program this reliably or should we change the plan?").
- Pause (72) — Boris: "new clip plays, the old clip is permanently paused and if comp is saved, it is saved as paused. the
  clip is now paused until the user changes that setting." -> pause is a setting of the clip, kept until he changes it and
  saved with the show; a different clip fired on that layer plays.
- Timeline Speed (74) — Boris: "74 b" -> the range goes to 10.
- Not named by him, default stands: 51 (a screen's Delay runs 0 to 100 ms).

## 2026-10-04 (s-rta-1004) — Boris's answers to questions 61-63: the nudge's sign and text, the tempo row, the amount in the show (recorded 2026-10-04 14:21:32; questions as asked: .harmony/.reports/s-rta-1004/boris-clarify-61-63.md)
- Sign and text (61) — Boris: "61 b but lets call it "nudge X ms"" -> plus moves the beat EARLIER; the text reads "nudge X ms".
  REPLACES "off beat by [+/- X] ms" (12:23:42).
- The tempo row (62) — Boris: "do a similar to resolume: beatWheel play pause stop bpm# bpm- bpm+ nudgeBack nudgeForward /2 *2
  tap resync" -> that row, in that order; /2 and *2 work; play, pause, stop, BPM minus and BPM plus are new.
  OPEN: what play / pause / stop run (question 111).
- The amount and the show (63) — Boris: "good (this is just nudge amount)" -> a show's own nudge amount takes over when it is
  opened.
- Play, pause, stop in the tempo row (111; recorded 2026-10-04 14:29:06) — Boris: "just the bpm timer. If most of the show is set up to BPM,
  and the BPM goes stop, the BPM goes to zero nothing moves. If there are clips that are not BPM based, then they play just as
  they were and are unaffected" -> they run the BPM timer only; what is not BPM-based is unaffected.

## 2026-10-04 (s-rta-1004) — "Routines" become "actions"; Boris's answers to questions 101-104, 106 (recorded 2026-10-04 15:07:44; questions as asked: .harmony/.reports/s-rta-1004/boris-clarify-101-106.md)
- The name — Boris: "I want to change what we are calling routines to actions. Easier to remember" -> everywhere he sees or
  says "routine", the app says "action".
- Looks shipped with the app (101) — Boris: "I will build them later myself, but when the app is finished" -> none ship now.
- Signals in a look (102) — Boris: "102 B" -> a look remembers which signal drives each slider and plugs them in again when
  loaded. OVERRULES the looks ruling's "no signal connections".
- A changed look (103) — Boris: "For a default behavior I will want to create a look 3 when I change look 2 and save a
  differnt version of look 2. look 2 remains unchanged but I need to have a way to save look 2 if I tweak it a little." ->
  a new look by default; saving over the loaded look is also offered.
- Naming a look (104) — Boris: "open a name box but with default name look x that can easily be changed" -> a name box,
  already holding "Look X".
- His two old effect presets (106) — Boris: "delete them. this is a new build" -> deleted; nothing is converted.

## 2026-10-04 (s-rta-1004) — Boris's answers to questions 91, 93, 94 (recorded 2026-10-04 15:30:15; questions as asked: .harmony/.reports/s-rta-1004/boris-clarify-91-94.md)
- Clicks in the list of shows (91) — Boris: "91 default" -> one click on a show opens or closes its decks; a double-click on
  a show opens it (it asks first); a double-click on a deck adds it; one click on a deck does nothing.
- Return in the quit window (93) — Boris: "93 b" -> Return presses "Save & Quit", the lit button; Esc cancels.
- Keys at launch (94) — Boris: "94 default" -> the keys of the show saved last.
- A screen's Delay range (51; recorded 2026-10-04 15:35:01) — Boris: "51 default is good" -> 0 to 100 ms, as in Resolume.

## 2026-10-04 (s-rta-1004) — Boris's answers to questions 121-124; "Beat Repeat" (recorded 2026-10-04 15:59:53; questions as asked: .harmony/.reports/s-rta-1004/boris-clarify-121-124.md)
- Resync and out-of-time clips (121) — Boris: "121 a" -> they cut into time at once; his press is the "1".
- A paused clip in a saved show (122) — Boris: "122 a" -> it opens paused on the same frame.
- Where a clip fired between two "1"s cuts to (123) — Boris: "123 b" -> always back to its beginning, on the next "1".
- A beat loop switched off without Catch Up (124) — Boris: "124 a" -> the clip carries on and cuts into time on the next "1".
- The name — Boris: "also, lets change name to beat repeat. beat loopr is resolumes original name" -> the feature is called
  "Beat Repeat" in Audio-DNA.

## 2026-10-04 (s-rta-1004b) — Boris's answers to questions 125-129 and 131-134, his words on readings tempo-row R75, R76, R81, R83 (actions; Quantize leaves the top bar; a recording review screen), the three tracks, three Resolume screenshots (recorded 2026-10-04 21:03:09; whole message verbatim + the questions as asked + the pictures: boris-feedback-backlog.md, same stamp)
- The tempo step (125) — Boris: "125 whole numbers" -> "-" and "+" step in whole numbers.
- The row's stop and pause (126, 127) — Boris: "126 and 127 stop clears all clips from layer strips, pause stops them, tempo
  setting stays the same. Make thee new buttons as I asked" -> the row's stop takes every clip off the layer strips; its
  pause stops the clips; the tempo number stays. The row is his list of 14:21:32. OPEN: 135, 136 -- against his answer to
  111 ("just the bpm timer ... clips that are not BPM based ... are unaffected") it is not settled whether the beat stops
  and holds with the clips, and which clips a pause holds. No later line of his says REPLACES.
- Tap, Resync and a stopped beat (128) — Boris: "128 tapping tempo does not start anything but when click resync, that is
  the 1 and it begins on that button push" -> a Tap never starts the beat; Resync is the "1" and starts it on the press.
- The nudge across stop and play (129) — Boris: "129 b" -> stop and play never touch the nudge; only Resync zeroes it.
- Actions (on reading R75) — Boris: "R75 we are calling routines actions. We need to discuss this. The actions are bpm based
  as their timing is important and the bpm they were recorded with need to be saved so if the show is playing at a
  different speed, the actions will play in time. A take = action recording. After it is recorded it does not follow audio,
  the audio is gone when it has been turned into an action. The only time that an action will be connected to its audio is
  when the user is reviewing the show they recorded and is extracting a certain actions from it of a certain duration." ->
  TO DISCUSS, nothing is built on it yet. Readings R90-R94, questions 139-143.
- Manual (on reading R76) — Boris: "R76 please explain wha tit means to switch manual on" -> explained in chat; no rule.
- Quantize (on reading R81) — Boris: "R81 Quantize is for locking button pushes to beats when the user is doing it. If a
  clip is set to bpm then it's time will always be locked to the BPM hence automatically quantized. Our quantized setting
  is for helping the users button pushes stay in time for the recording of the actions. Even though the user will be a
  little early or late recording the show, it will clip to the exact bar or beat in the recording. We can just use this for
  cleaning up the recording afterwards. This is not for live usage. We should remove it from the top bar and use it in the
  recording review screen which we have yet to create. I have attached an image of what it should roughly look like.It will
  be rows and rows of parameters on a timeline and each parameter will have keyframes and values. Some values will go from
  0-1 and some will be radians and some will go from a negative number to a positive number." -> Quantize is not a live
  control: it leaves the top bar; it comes back as a clean-up tool in a recording review screen (new, not planned). The
  tempo-row reading R81 is VOID. REPLACES BF20's Quantize menu (1 bar, 1/2 bar, 1/4, 1/8, 1/16) as a live top-bar
  control; what of Quantize is on screen today is not yet established (a fact sheet is owed). Readings R86, R87. His
  picture did not arrive (RQ-1).
- Actions on screen (on reading R83) — Boris: "R83 I have no idea for how the routines, now called. Actions will be recorded
  and displayed. There will be a little area above each clip where there will be toggle buttons for each action that was
  recorded for that clip. If the actions are toggled on all those actions will play when the clip plays in time with the
  clip. The same will be true for a layer actions. They will be in the layer strip, and they were also be buttons that
  could be toggled on and off for each action. We will also find a place to have composition level actions, but they will
  simply be toggle buttons as well. When the clip starts playing, if its actions are turned on, they will play. Essentially
  they are just automations for any parameters within the clip or its effects." -> on / off buttons per action above each
  clip, in the layer strip, and for the composition; an action is an automation of parameters. TO DISCUSS with the entry
  above. Reading R89 (his first sentence), R94 (today's pads, bank and bands go).
- A look and a plugged signal (131) — Boris: "131 is the signal plugged into a slider in a clip in the show, or is it a look
  which is an effect preset in the effect library that can also be connected to a signal. If this is a clip in the show,
  then ctrl-z brings it back. There is no other way. If it is a look, there is no way to remove the signal unless the user
  drops it into the show (clip, layer or global) and then adds a signal and saves that look." -> read as A (R88): loading a
  look without a signal on a slider unplugs it; Cmd+Z brings it back. OPEN: 138 (looks in the effect list).
- Save over (132) — Boris: "132 default good" -> a small window asks first.
- The button after a change (133) — Boris: "133 no need for any of that. It can be called look 2 but no need to show that
  it was changed. Effects are usually changed by the user." -> the button keeps the look's name; no mark.
- The nine quick FX saves (134) — Boris: "134 delete them too" -> moved to the Trash by Harmony, 2026-10-04 21:01:09.
- The three tracks — Boris: "explain better what you need here: Three tracks you would really play, about 10 minutes each —
  tell me where the files are. The first transport measurement (how often the picture would cut) cannot run without them."
  -> explained in chat (the quoted sentence after "here:" is Harmony's own request, quoted back by him).
- Resolume facts — Boris: "here is the beat repeat screenshot: [Image #5]"; "A screenshot of a clip in BPM Sync with the
  loop menu on Random, showing Interval and Distance. this is essentially jumping to random 1's on the beat: [Image #6]
  [Image #7]"; "one click on time jumps 1 bpm, duration in timeline mode moves up 0.1" -> the pictures are
  .harmony/.reports/s-rta-1004b/boris-images/resolume-beat-repeat.png, resolume-random-1.png, resolume-random-2.png.
  OPEN: 137 (which row "time" is).
