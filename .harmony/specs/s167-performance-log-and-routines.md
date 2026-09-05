
---

## 9. ADDENDUM — timelines and connection sources (answer to the team-lead's four questions; §D7 has the long form)

### 9.1 HOLDS or BREAKS

**BREAKS at the owner level; HOLDS at the curve level.** Your reading — "a recorded lane plugs into
`ConnSource` where `Signal` and `Oscillator` do" — is wrong, and the reconciliation is exactly
the one D6/D8 imply: **the `Player` is a WRITER through `manualWrite`, one more hand in the grip
chain; a lane is not a connection source.** Plainly: two features do not collapse into one
`ConnSource` kind. They collapse into one **curve type** (9.2), which is the part worth taking
into Lane 2 today.

The evaluation shape is *not* what separates them — a continuous lane's gesture IS a function
evaluated per tick (the `Player` samples its `AutomationCurve` at 120 Hz, D3/D5), the same
shape as `ConnectionEngine::evaluate`. What separates them is ownership, and on four properties
a connection has and a lane cannot (D7, one line each):

1. **Exclusivity.** One `ParamConnection` per parameter, enforced by the type (conn spec §2.6).
   A knob can be under the set replay AND two routines AND a hand in the same minute; two of
   them at once is legal (D9). A lane-as-owner would make a routine trigger *unplug* the knob's
   bass connection and plug it back after — the opposite of what was recorded (a hand on top of
   the bass).
2. **Gaps.** A lane is silent between gestures; the control belongs to its owner, and the
   gesture's end is a hand-back glide. An envelope is a total function over its cycle.
3. **Clock and trigger.** An envelope cycles on the beat grid or the clip playhead and restarts
   on *clip* trigger (`Clock {Beats, ClipPosition}`, §2.4). A lane runs from the *take/routine*
   trigger, quantized, stopped and looped as a unit with every other lane in that routine — an
   identity that must live outside any one parameter regardless.
4. **Coverage.** Half the lanes (clip hits, deck switches, tap tempo, enable/bypass) are buttons
   — not connectable by Ruling B.1. "Lane = source" would cover sliders only and need a second
   model for the rest.

**Do `Program`/`Player` and a per-parameter owner coexist, or fight?** Coexist, by the rule that
already exists. They never write the same field: the `ConnectionEngine` publishes the owner's
value into the `LiveValue` twin (§3.1); the `Player` writes the *manual* field through
`manualWrite`, which takes a `Held` grip, so the engine stores `NaN` in the twin and the
renderer reads the manual field (§2.3) — the lane's value. On the gesture's `end` the player
releases, and the engine glides from the last manual value to the owner's value
(`handBackGlideMs`). A human grabbing the knob outranks the player (D8 chain), and the player's
next gesture re-takes. **One ordering requirement for the builder (new, belongs in §4):** within
the 120 Hz tick, `Player::advanceTo` runs BEFORE `ConnectionEngine::tick`, so grip state is
current when the engine decides whether to publish; otherwise a gesture's first tick renders one
frame late.

### 9.2 What HOLDS, and the minimal Lane 2 change

The hand-drawn "Timeline" (`ConnSource::Kind::Envelope`, owner D6/D8 "build") and a recorded
continuous lane are the **same curve** — one drawn, one captured — and must share one struct,
one editor, one evaluator:

- `src/model/AutomationCurve.h` (PROPOSED, D7): `Breakpoint { double x; float y; Interp
  {Linear, Hold, Smooth} }` + `AutomationCurve { pts; eval(x); xMin(); xMax() }`.
- `ConnSource::Envelope::points` (a bare `vector<pair<float,float>>` today, §2.1) becomes
  `AutomationCurve curve` with `x ∈ [0,1]`. Semantics — `playbackXform`, `loop`, `Clock`,
  `cycleBeats` — unchanged. Cost: a type rename and one enum. `Interp::Hold` is the step mode a
  drawn Timeline needs for strobe-like shapes anyway.
- **Reserve nothing else.** Do NOT add `Kind::Lane`/`Kind::Performance`, and do NOT add a
  performance clock to `Envelope::Clock` (that couples composition state to a transient player).
  The seam you want already exists: `ConnSource` kinds and `Clock` values are serialized as
  **strings** and the loader maps unknown kinds to `None` and reports them (§2.7) — so a new
  kind or clock value later is *additive*, not a format break. The only thing that WOULD be a
  break later is the shape of `points` — which is why the curve type goes in now.
- The two bridges (LATER, plain curve copies once the type is shared): **Print to knob** — a
  lane gesture → `Envelope { curve rescaled to [0,1], cycleBeats = gesture length, loop, Clock::
  Beats }` on that knob; **Record into take** — a knob's Timeline → a lane gesture. Same editor
  component in both places.

### 9.3 How the hand-drawn curve gets built, and what Boris will SEE that differs

Built **as a connection source** — `ConnSource::Kind::Envelope`, exactly as the connection spec
already specifies for Lane 2/L5 — with recorded lanes as writers. He experiences both as "a
shape over time driving a knob"; the difference that will land on him is *where the shape lives*
and *when it plays*, which is a distinction he already uses (a clip's settings vs a performance):

| | Timeline on a knob (connection source) | Recorded lane (writer, in a take or routine) |
|---|---|---|
| lives in | the knob — saved and copied with the clip; shows in the knob's source picker next to Bass and LFO | the take or routine — shows in the take's lane view, never in the knob's picker |
| plays when | whenever the clip/beat runs; loops per its own setting; restarts on clip trigger; stays until he disconnects it | only while that take/routine is playing; starts when he fires it, ends when it ends; several can stack |
| between/after | it IS the owner — there is no "between" | the knob's own owner (Bass, LFO, or its Timeline) resumes between gestures and after the take |
| grab the knob | he wins; let go → glides back to the curve (Ruling A) | he wins; let go → glides back to the lane's gesture (D8 TOUCH) — same feel |
| range / invert / direction | apply (it is a source shaped by `ConnShape`) | do NOT apply — a lane replays the hand's literal positions; shaping it would replay a different performance than the one recorded. Want it shaped? Print it to the knob and shape the Timeline |
| bridge | "Record into take" | "Print to knob" |

Same curve editor in both places, so drawing and editing feel identical; only the header says
"Ripple › Amplitude › Timeline" versus "Friday take › Layer 2 › Opacity".

### 9.4 The per-control lane model — structural, already done, not a view

Revision 2 already made lanes the **structural on-disk form** (D3: `Take = map<ControlPath,
Lane>`), not an index over an event stream — because Boris's sentence is an *editing*
requirement ("can be changed similar to Ableton"), not a presentation one. A chronological
stream with a per-control index gives the *picture* for free but not the *editing*: every move,
retime or span-delete on one control re-sorts the global list and invalidates the index, and
gestures (begin/points/end on one control) are only implicit. With lanes as storage an edit
touches one vector; a gesture is one object holding one `AutomationCurve` (9.2); cross-lane
ordering — the one thing a stream gives — is needed only at playback and the `Program` compile
produces it by a k-way merge on `(at, seq)` with capture order preserved by the global `seq`
and simultaneity by `group`. The chronological list survives as a derived view
(`Take::chronological()`) for a history list and debugging, never as the authority. D4/D5's
document-vs-program split is about safety; D3's lanes are about editing; they compose.

REPORT_FILE: /Users/boriskarpman/Harmony_Main/memory/.reports/s167-rta-performance-log-and-routines.md
STATUS: COMPLETE
