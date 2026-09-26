# Critic (blind, fidelity lens) — plan-routines-s1.md vs rulings 17/20/21/22/26/27 and D4/D5/D8/D9

Verdict: SOUND_WITH_FIXES. The scope cut, deferral list, and bank-vs-grid decision are faithful to
the rulings. One concrete mechanism (routine-vs-routine stacking) diverges from D9's stated rule
in a way the plan asserts is equivalent but is not, evidenced against the ALREADY-BUILT
`ManualWrite.cpp`. A second item (the "restore" reading) is a defensible interpretation the plan
states as flatly "RULED" rather than flagging as an inference, when the plan itself has a separate
section for exactly this kind of thing. A third is a silent scope-narrowing against the ruling's
own resolution text.

## MUST — "later begin wins" is not what the plan's mechanism implements

Claim in the plan (plan:341-344, section 4.3): "Two routines on ONE control: both write at
`Hand::Lane`; `manualWriteCore` 'equal rank: latest writer wins' (`ManualWrite.h:61-67`) and
`running_` is advanced in fire order, so the LATER-fired routine writes last each tick -- D9's
'the later begin wins' (`spec:495-499`) with no new mechanism; test 10 pins the order."

D9's actual text (spec:495-499, VERIFIED read this pass): "A routine is a hand, so conflicts use
D8's chain: two routines on one **control**: the later `begin` wins **for the rest of that
gesture**." This is a per-GESTURE rule (a gesture = one touch→set→release span on a continuous
lane, D5:292-297) — whichever routine's gesture on that control *began most recently* keeps the
control until *that gesture* ends; if the other routine later starts its own gesture on the same
control, the "later begin" flips to it.

The plan's mechanism is not that. `running_` order is fixed at fire time (Running is
`push_back`-ed once, section 4.1's comment "at most one per slot" and section 4.2's "for each
Running r" iterate that fixed vector); `ManualWrite.cpp:178-190` (`manualTouchCore`,
`manualWriteCore`, VERIFIED read this pass) shows the grip is refused only when a STRICTLY higher
rank *actively* holds it — at equal rank (`Hand::Lane` for both routines) touch always succeeds and
the write always lands, so whichever routine is iterated LAST in the tick's fixed order overwrites
the other's value on that control, EVERY TICK, for as long as both keep touching it — not just for
"the rest of that gesture." Concretely: Routine A fires at t=0, Routine B fires at t=5 (B is
fire-order-later, so `running_` always processes B after A). At t=50 A begins a NEW gesture on
control X (A's gesture began most recently). Per D9, A should now own X until A's gesture ends. Per
the plan's mechanism, B still writes last every tick (fire order is fixed), so B's value (whatever
it was already writing, or nothing if B has no active gesture there — in which case A does win by
default, but only because B is silent, not because of the "later begin" rule) continues to
override A whenever B *also* has an active gesture there, regardless of which gesture began more
recently. Test 10 (plan:449, "two routines on one opacity key, fired A then B: each tick's `sets`
order is [A, B]") only exercises the case where both gestures are co-active from the start and
never re-derives the ordering after a later gesture begins on the earlier-fired routine — it proves
the fire-order mechanism does what the fire-order mechanism does, not that it matches D9's
gesture-begin rule. This is the exact scenario the dispatch's lens asks about: the plan claims
fidelity ("D9's 'the later begin wins' ... with no new mechanism") to a ruling-adjacent spec clause
it has not actually implemented, and ships it as if gate-complete via a test that cannot catch the
gap.

Fix: either (a) implement the gesture-begin comparison for real (each `LaneCursor`/`Running` records
the wall-clock or beat-clock time its CURRENT gesture on a given control began; on a tie at
`Hand::Lane`, the tick compares those begin-times, not vector order), or (b) if the coarser
"whichever routine fired later always wins for the whole overlap" behavior is intentionally chosen
as the slice-1 simplification, say so explicitly (next to the other named simplifications in
section 1's DEFERRED list and section 9's decided-here list) instead of citing it as satisfying D9,
and add a test that actually distinguishes the two mechanisms (fire order fixed vs. per-gesture
begin order) so a future slice can tell which one shipped.

## SHOULD — "restore the state it was recorded in" is asserted as RULED, not flagged as an inference

Ruling 26 (binding-decisions.md:427-430, VERIFIED): "A ROUTINE RESTORES THE STATE IT WAS RECORDED
IN. Boris: 'restore'. Firing a saved routine first puts the layers and knobs it uses back the way
they were **at record time**, then plays." Read alone, "at record time" is at least as naturally
read as "at the moment the WHOLE TAKE was originally recorded" (i.e., checkpoint 0 / take start)
as it is "at the moment during the original recording that corresponds to this slice's own start."
The plan (plan:207-214, section 3.3) picks the second reading, grounding it in D4 step 2's
definition of a routine's preamble as "the state at `beatFrom`... derived lane-by-lane... falling
back to checkpoint 0... only the controls the routine touches" — a reasonable and well-cited
reading, and probably the right one given D4 predates the ruling by one day. But the plan states
this as "RULED; tests 2-3 pin it" with no hedge, in a document that elsewhere (section 8, "Only
Boris can check") is careful to separate its own calls from Boris's. The two readings produce
materially different behavior — full-take-start snapshot restore vs. touched-controls-only,
slice-start-derived restore — and the plan's own section 9 already carries a list of "decided
here, Boris may overrule" items for exactly this class of gap. This item belongs there too, or at
minimum the plan should say plainly "this reading of ruling 26 is the architect's inference from
D4, not confirmed against Boris's literal words" rather than "RULED."

Fix: move this item into section 9's list (or add it as a seventh decided-here item) so a reviewer
or Boris can see it was interpreted, not answered.

## SHOULD — "direction" silently dropped from the Routine control set

Ruling 22's adopted resolution (binding-decisions.md:381-386, VERIFIED) states the settled shape of
a routine's signal-like controls as "loop / once / **direction** / quantize." The plan's `Routine`
struct (plan:90-99, section 3.1) carries `loop`, `restoreState`, `quantize`, `deckRelative` — no
direction field, and no reverse-playback path anywhere in `Player`/`RoutineEngine`. Elsewhere the
plan is scrupulous about naming every deferred capability in an explicit DEFERRED list (plan:40-48)
and in section 9's decided-here list — "direction" appears in neither. This may be a legitimate
slice-1 cut (a routine's `Player`/`Program` have no reverse-clock concept today either), but the
plan should say so by name next to the other deferrals rather than let it disappear silently,
since it is literally in the sentence the ruling used to describe the settled control set.

Fix: add "direction (reverse playback) — not in Player/Program today, deferred" to the DEFERRED
list.

## Not refuted (checked and held up)

- Ruling 17 (routine hits fire on the RECORDED layer): `deckRelative = true` resolves the DECK at
  fire time while the layer stays the one recorded — matches plan:98 and D2/D9's own text.
- Ruling 20 (own bank, not grid cells): `Composition::routines`/`routineBank`, `kRoutineBankSize`,
  section 5.4's bank-strip design — matches; D9's own rejection of the clip-`MediaType` alternative
  (spec:506-508) is correctly not revisited.
- Ruling 27 (next bar default, per-routine override, global Quantize wins when on): `quantize =
  Bar` default, `forcedSnap` override in 4.2 — matches `quantizeModeToForcedSnap`'s existing
  precedent (`MainComponent.cpp:22-31`, cited and consistent with how clip triggers already work).
- PerfState genuinely has no composition-scalar (master opacity / Master Signal) field — VERIFIED
  by reading `src/recording/PerfState.h` this pass (no `masterOpacity`/`masterSignal`/scalar map at
  the top-level `PerfState` struct) — so the plan's `preambleUnknown` bucket for those controls is
  real, not a hand-wave.
- `Program.h:21-24`'s `Range` is confirmed (read this pass) to be exactly the "no preamble
  synthesis" filter the plan says it is; `slice()`/`Routine` symbols are confirmed absent from
  `src/` by the plan's own grep, independently spot-checked.
- Section 9's explicit "product questions decided here, Boris may overrule" list (global Stop
  stopping routines, whole-bar rounding, deck-relative default, re-fire = restart, pad replace,
  nothing recorded into takes) is the right and transparent way to handle a product call that
  isn't covered by a ruling — the MUST/SHOULD above are about places that same discipline slipped,
  not about this list itself.

## Scope note
Read-only pass; no source edited, nothing built, app not launched, no lldb/gdb use. Cited files:
`.harmony/binding-decisions.md`, `.harmony/specs/s167-performance-log-and-routines.md` (D4/D5/D8/D9),
`.harmony/.reports/s-rta-0926/plan-routines-s1.md`, `src/recording/Program.h`, `src/recording/Player.cpp`,
`src/connect/ManualWrite.h`, `src/connect/ManualWrite.cpp`, `src/recording/PerfState.h`.
