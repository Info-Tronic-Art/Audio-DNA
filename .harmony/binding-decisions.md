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

27. **A ROUTINE STARTS ON THE NEXT BAR.** Verbatim: "bar". Not the next beat. Per-routine
    override stays available, and the global Quantize setting overrides when it is on —
    consistent with how a quantized clip trigger already behaves.

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
