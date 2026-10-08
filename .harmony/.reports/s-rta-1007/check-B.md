# CHECK B (skeleton, in progress)

## MY OWN READING OF HIS LINES (step 2, written before opening the paper)
- L34 (R141): adds a transparency slider next to each layer's cue button; it shows what the layer's transparency would look like in the preview, using the layer-settings transparency and the layer stack order. Touches R141 (adds a control; the "each cued layer's fader is ignored" part of R141/R149a is now in doubt) and R149(a). Open: is the slider preview-only, or the layer's own fader?
- L35 (R170): undock/move NOT now. Files-window double-click preview IS wanted now (reverses R170 "not in first build"). Name click (single) in a deck and files double-click play right away, take over the preview; a toggle button between cue mode and preview mode; returning to cue only by the toggle. Touches R170, R150 (name click), Q176 (return rule), R153 (the two buttons), R141 last sentence.
- L36 (R179): all good except a previewed clip shows WITH all its actions and is triggered on the 1 (in time with the music), unlike Resolume. Replaces R179(c); touches R151 ("starts from its beginning"; "plays while beat stopped"). Conflict inside message: L35 "play right away" vs L36 "on the 1" (L36 is the specific correction).
- L37 (176): answer A, but "read R170 for detail": clip takes whole preview alone; the cued layers come back only by the toggle button (not by firing, not by a cue press, not by clicking the name again).
- L38 (177): neither A nor B: a global effects AND actions toggle button called "master cue". Gaps: default state, scope (global transform / master fader?), applies in preview mode?
- L8: P3/P4/P5 layout items -> laid out by Harmony; DROPPED unless variants differ in function (P5 "dimmed output when nothing cued" differs in what the app shows).
- L9: explained once answers repeats (R129, L72 global stop, L89 etc. may cross over; R152 cue press recorded? L81 R137 only things that move).
- L1: no "today" in rules; ask focused questions on assumptions.
- L114 (R197): names list is to be kept; "Preview", "cue" words belong in the glossary.
- Not-named R149, R150, R151, R152, R153, R152: test against whole message.


## FINDINGS
- MUST R141 / R149 (a) / B-3: the paper replaces R149 (a) on a reading his words do not make. L34: "of course using the transparency setting in the layer settings". The RULE of R141 turns this into "the blend setting of its layer settings" and says the cue slider "starts at full" and ignores the layer's real transparency. That drops his "transparency setting". B-3 itself lists the other readings (b: starts at the layer's real value; c: also moves the output) and marks it LINE. It should be ASK: YES (not settled; ALT c would change what the audience sees; it is about what the app does). R149 (a) is REPLACED only if he confirms. Fix: RULE carries both readings; B-3 ASK: YES; quote him, do not paraphrase "transparency" as "blend".
- MUST D9 / B-11: cites L125 for MilkDrop. L125 is an empty line; the MilkDrop words are L126 ("I want you to create a dedicated document with how milk drop functions currently and that's it for this upcoming build"). Fix: HIS: L126; RULE and B-11 WHY: "(L126)".
- SHOULD B-2 / R151: stopped and paused are lumped. His rules differ: L31 "when I click a BPM clip with the tempo paused it waits for the bpm clock to return to the 1"; L12 "When stopped, ... any clip is the new 1"; L13 "Pause pauses ... the clips and everything that it controls with BPM". The paper's choice (preview plays at the tempo number in both cases) is a deliberate departure. Fix: B-2 TEXT names paused and stopped separately and says the preview departs from L13/L31. Keep LINE or make YES for the paused case.
- SHOULD P5: the variant "shows the output, dimmed, when nothing is cued" differs in what the app does, so the discipline says STATUS OPEN plus an @@ASSUME. Paper has DROPPED, resting on R152 "black" standing by inferred consent. Fix: STATUS OPEN with a one-line @@ASSUME (LINE): "With nothing cued the preview is black."
- SHOULD R170: the RULE states "A double-click previews only; it loads nothing into a deck", and TODAY says the present double-click path "has to go". Removing what a files double-click does now is Harmony's own choice and has no @@ASSUME. Fix: add a LINE assumption (double-click on a file only previews, nothing else).
- SHOULD NAME blocks (output monitor, files window, Cuepoints, transparency): SOURCE lines say "the on-screen name now is ...". The names list is meant for him, and L1 forbids "today" there. Fix: drop the "on-screen name now" clauses; keep them in TODAY.
- SHOULD R150 and R153: STATUS STANDS while CHANGED adds a new behaviour (name click switches the monitor to preview mode; the mode names). Fix: STATUS CORRECTED (added, nothing replaced).
- SHOULD B-4 IF-WRONG says trying a global action in the preview needs "a second copy of the show's state" as if clip actions did not. L36 already has a clip's actions play for the preview alone (apply-D.md:719 says so). Fix: say so, so the size of the build is not misstated.

## MISSING
- How the cue slider acts on the LOWEST cued layer: R149 (d) says it is "drawn as it is"; at 50 percent over what (black? nothing)? Not stated.
- What the preview monitor shows in preview mode when the previewed clip is waiting for its 1: stands on its first frame (stated in R151 as fact, only partly in B-1).
- A name click on the clip that is already previewed, and on a second clip while one is previewed (restart or replace): not stated.
- Whether master cue ON also covers layer/clip actions: only ALT d of B-4; fine, but the RULE of 177 should say "global actions only" if that is the reading.

## WHAT I CHECKED AND FOUND RIGHT (ids only)
176 (quotes L35/L37 verbatim, return rule B-6), 177 (quote L38; B-4 is rightly ASK: YES; B-5 LINE acceptable), R149 (b) (c) (d) (e) (g), R152 (mapping via Q206 default; B-7 LINE), R179 (a) (b) (d), D7 (L88 quote verbatim), D8, P3, P4. All quotes of L16, L29, L34-L38, L51, L88 are verbatim with correct L numbers except the L125 above. "Lipp's" and "queue" slips flagged as INFERRED. No "today" in any RULE. B-1 ASK: YES is right (L35 against L36). B-8, B-12, B-13, B-14, B-17, B-18 NO: acceptable.

## LINT OUTPUT
blocks: 16 ITEM (slice has 16 ids), 16 ASSUME (2 ask YES, 7 LINE), 0 ANSWER, 11 NAME
OK
