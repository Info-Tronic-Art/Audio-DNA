# Reviewer Verdict — rev-L1-media-leak-s-rta-0904
STATUS: PARTIAL
VERDICT: FAIL

## Summary
Mechanism is sound and well-built for the FOUR command classes it touches
(SetClipCmd, SwapClipsCmd, ClearLayerClipsCmd, RemoveColumnCmd): trap (a)
double-apply and trap (b) GL-thread-destroy are both correctly solved, with
good tests. But trap (c) family coverage has TWO real, live, user-reachable
gaps the packet never named and the builder did not find: RemoveLayerCmd and
RemoveDeckCmd both vacate cells (an entire layer's or deck's clips,
permanently) and neither calls the new dispose hook — confirmed with two
independent grep patterns (zero hits for `disposeHook`/`ClipMediaDisposeHook`
inside either class body). "Remove Layer" (#17) and "Remove Deck" (#22) are
ordinary menu commands, reachable any time >1 layer/deck exists, with no
requirement the target be empty first. This reproduces exactly the failure
mode the whole review was designed to catch: "a fix covering only one
path[-family] gates green and still leaks." Recommend sending back for a
narrow extension of the SAME pattern to these two classes, not a redesign.

## Trap (a) — DOUBLE-APPLY: PASS
SetClipCmd/SwapClipsCmd/ClearLayerClipsCmd/RemoveColumnCmd all changed
`apply()` to take an explicit `leaving` snapshot parameter (the command's own
`before_`/`after_`), and dispose decisions are keyed off that parameter, not
off live model state re-read through `resolver_`. Verified by reading every
`apply()` body (ClipCommands.h:80-131, 179-240; DeckCommands.h:120-138,
270-298) — the live-cell mutation (`cell = state` / `layer->clips =
state.clips` / `deck->removeColumn`) always runs first, then the dispose
decision consults the snapshot params, never the just-mutated live cell.
`SwapClipsCmd::disposeIfOrphaned` correctly treats a relocation (id moves to
the OTHER resulting cell) as "still present" and never disposes it — verified
by a dedicated test asserting `disposeCalls == 0` across swap/undo/redo
(tests/test_undo_commands.cpp:531-547).

## Trap (b) — GL-THREAD DESTROY: PASS for the covered paths, one gap found
- Retire list IS mutex-guarded (`retiredMediaMutex_`) and IS drained only on
  the GL thread (`drainRetiredMedia()`, called from `renderOpenGL()` —
  Renderer.cpp:144-150, 991-1013).
- Lock is NOT held across a GL call: `drainRetiredMedia()` swaps the lists out
  under lock, releases the lock, then calls `releaseGL()` on the local copies
  (Renderer.cpp:999-1009) — no lock held during `glDeleteTextures`. Verified
  `VideoPlayer::close()`/`ImageSequence::close()` (VideoPlayer.cpp:195-216,
  ImageSequence.cpp:89-100) contain zero GL calls, so `closeMediaForClip`
  holding `videoPlayerMutex_`/`imageSeqMutex_` while calling `->close()` is
  also safe — matches the code's own claim.
- `releaseGL()` is idempotent on both classes (guards `texture_ != 0` /
  iterates and zeroes `textures_`), so the double-release (explicit call in
  `drainRetiredMedia()`, then again implicitly via the retired vector's own
  destructor) is a documented, verified no-op.
- **GAP (OBSERVED): `openGLContextClosing()` was NOT extended to cover the new
  retire lists.** It releases GL for the live `videoPlayers_`/`imageSequences_`
  maps (Renderer.cpp:730-742, unchanged) but never touches
  `retiredVideoPlayers_`/`retiredImageSequences_`. If an item is retired
  (Clear happens) and the app quits/context closes before the NEXT GL frame
  drains it, that item's eventual destruction (when `Renderer`'s own member
  vectors are destroyed after context teardown) runs `~VideoPlayer()` →
  `releaseGL()` → `glDeleteTextures` with **no context current** —
  reproducing the exact bug class trap (b) exists to eliminate, just at
  shutdown instead of at Clear. The packet explicitly asked "is there a drain
  on shutdown, or do retired items leak at exit... say so explicitly" — no
  comment anywhere addresses this; it was not examined. In practice this is a
  narrow window (one Clear right before quit, before the next render tick)
  and GL texture handles are typically reclaimed by context teardown anyway,
  so I'd call it low-severity, but it is a real, previously-unexamined gap
  directly on the topic the packet asked about. Suggested fix: have
  `openGLContextClosing()` also call (or inline) the same
  swap-and-`releaseGL()` logic over the two retired vectors.
- **Related pre-existing issue, NOT introduced by this diff (OBSERVED):**
  `Renderer::openVideoForClip`/`openImageSequenceForClip`
  (Renderer.cpp:933-954, unchanged) do `videoPlayers_[clipId] =
  std::move(player)` — a map-assignment overwrite that destroys any PRIOR
  entry at that key synchronously, on whatever thread the caller runs on.
  `ClipMediaHook`'s own reconnect logic (`needsVideoReopen`,
  MainComponent.cpp:3523-3527) deliberately re-opens when a clip's file
  changed under a stable id (replace-content undo/redo) — meaning this
  overwrite path can and does fire from the **message thread** whenever a
  clip with already-live media has its content replaced. That is the same
  "destroy with no GL context current" bug class as trap (b), via a
  different path this fix does not touch. This is old code, not a regression
  from this diff, and outside the packet's stated scope (Clip > Clear /
  ClearLayerClipsCmd family / RemoveColumnCmd) — flagging for a follow-up
  ticket, not blocking this one.

## Trap (c) — FAMILY COVERAGE: FAIL
Enumerated every vacate path in `src/core/` and `src/model/` (`clips.clear()`,
`.erase()` on a clips/layers/decks vector, `cell.reset()`):

| Path | Disposes? | Evidence |
|---|---|---|
| `SetClipCmd` (Clip > Clear, kClipClear) | YES | ClipCommands.h:114-119 |
| `SwapClipsCmd` (drag move/swap) | YES (correctly never on relocation) | ClipCommands.h:217-232 |
| `ClearLayerClipsCmd` (kDeckClearClips, kLayerClearClips) | YES | DeckCommands.h:283-298 |
| `RemoveColumnCmd` | YES | DeckCommands.h:127-138 |
| **`RemoveLayerCmd` (kLayerRemove, #17)** | **NO** | No `disposeHook_` member or call anywhere in the class (DeckCommands.h:470-526); confirmed with two grep patterns (`disposeHook` and `ClipMediaDisposeHook`, both zero hits inside the class body). `execute()` (line 481-494) does a bare `deck->layers.erase(...)` with no media interaction at all; `undo()` only reconnects. Removing the LAST layer (no emptiness precondition, MainComponent.cpp:4205-4223) permanently strands any video/image-sequence clips it held. |
| **`RemoveDeckCmd` (kDeckRemove, #22)** | **NO** | Class has no `mediaHook_` OR `disposeHook_` member at all (DeckCommands.h:676-747); `execute()` is a bare `comp->decks.erase(...)` (line 711). Removing the active deck (no emptiness precondition, MainComponent.cpp:4132-4147) permanently strands every clip in every layer of that deck. |
| `MoveLayerCmd`, `SetColumnCountCmd`, `AddLayerCmd`/`AddDeckCmd` undo | N/A | Not vacate paths — `SetColumnCountCmd::apply` is grow-only (comment: "ensureColumns never shrinks", DeckCommands.h:75); `MoveLayerCmd` relocates, doesn't remove; `Add*Cmd` undo only ever erases a just-created empty layer/deck. |
| `Layer::fromVar`'s `clips.clear()` (Layer.cpp:208) | N/A | Composition-load/JSON deserialize path — explicitly out of scope (L3, packet's own "OUT OF SCOPE" section). |

Both `RemoveLayerCmd` and `RemoveDeckCmd` are ordinary, always-available menu
commands (not edge cases), and both still exhibit the EXACT original defect
("strands an FFmpeg decoder + GL texture for the life of the process") after
this fix lands. This is squarely inside the packet's own title ("Clip > Clear
**and its sibling vacate commands**") even though the packet's own body only
named three siblings — the review brief explicitly instructed finding
uncovered family members myself rather than trusting the packet's list, and
these are exactly that.

## ALSO ADJUDICATED

**1. Liveness/unique-id guard:** `makeClipMediaDisposeHook`
(MainComponent.cpp:3540-3570) re-walks every deck/layer/cell before actually
closing, and skips the close if the id is found live anywhere — a genuine
second, defensive check independent of each command's own snapshot-based
orphan check. The future-fragile comment is present, load-bearing, and placed
exactly where a clipboard/duplicate feature would be added (directly above
the hook body, explicitly naming "READ BEFORE TOUCHING CLIPBOARD/DUPLICATE").

**2. `ClipMediaHook` cycle:** Confirmed unchanged in what it guarantees
(reconnect-if-missing / reconnect-on-content-swap), only its doc comment was
updated to describe the new "attach half" framing. Traced the full
clear→undo→redo cycle through SetClipCmd, RemoveColumnCmd, and
ClearLayerClipsCmd's `apply()`: on clear, `state` is empty so no reconnect and
`leaving` disposes; on undo, `state` is populated so mediaHook_ reconnects and
`leaving` (the cleared/empty side) doesn't dispose; on redo, symmetric to
clear — dispose fires again, safely idempotent because `closeMediaForClip`'s
`.find()` guard makes a second call on an already-erased id a no-op. Matches
the packet's required fail-first oracle exactly, for every path that has the
hook wired.

**3. Test call-site audit (arity-filler check):** Went through every changed
call site in `tests/test_undo_commands.cpp`.
- **Real assertions (not filler):** the new dedicated trap-a test
  (`SetClipCmd: dispose hook fires on clear/redo, NOT on undo`, lines
  352-386); the SwapClipsCmd swap test (disposeCalls asserted == 0 across
  execute/undo/redo, lines 511-548); the RemoveColumnCmd test (disposeCalls
  2→2→4, reconnectCalls 0→2, lines 763-800); the ClearLayerClipsCmd test
  (disposeCalls 2→2→4, reconnectCalls 0→2, lines 847-877); the "Multi-select
  clear composite" test (disposeCalls == 3 across three SetClipCmds, lines
  1026-1050).
- **Neutral placeholder (`noopDispose()`), and legitimately so:** every other
  call site — fence-firing tests, resolver/stale-coordinate tests, the
  column-growth swap test, the composite-grouping tests, and the property-based
  random-sequence test. None of these were ABOUT the media path before this
  diff and none claim to test it now; `noopDispose()` there is exactly the
  same pattern as the pre-existing `noopMedia()` used throughout for
  unrelated hooks. I did not find a single case where a test that WAS
  testing disposal used a placeholder to dodge the new parameter.

**4. Does the new test test the leak, or just that a function was called?**
The dedicated trap-a test and the four enhanced tests all assert exact call
COUNTS across execute/undo/redo, keyed to specific clip ids
(`REQUIRE(c.id == 7)` in the trap-a test) — this catches: dispose firing zero
times (regression to the original bug), firing on the wrong cycle phase
(e.g. firing on undo, which would wrongly close media that should still play),
double-firing where it shouldn't (e.g. a swap wrongly disposing), or firing
for the wrong clip. It does NOT and CANNOT exercise the actual Renderer-level
mechanics (retire list, GL-thread drain, real FFmpeg/GL handle closure) — this
suite is headless (no Renderer linked) and only exercises the Command-layer
hook contract. The retire-list/drain mechanism itself has zero automated
coverage; its correctness rests entirely on source reading (verified above)
plus whatever manual/lsof check Harmony runs as the fail-first oracle.

## WHAT I COULD NOT DETERMINE FROM SOURCE ALONE
- Whether `RemoveLayerCmd`/`RemoveDeckCmd`'s gap was a conscious scope
  decision (e.g. deferred to a later wave) versus an oversight — the packet
  and builder-facing docs I have access to are silent on both classes.
- Actual runtime behavior of `glDeleteTextures` with no context current on
  this project's target GL implementation/driver (silent no-op vs. crash) —
  relevant to how urgently the shutdown-drain gap should be prioritized.
- Whether Harmony's lsof fail-first oracle, run once per lane, would have
  exercised Remove Layer/Remove Deck at all (the packet's own fail-first
  description only mentions Clear).

## DEFECTS (ranked)
1. **MAJOR — `RemoveLayerCmd::execute()` never disposes media for clips in
   the removed layer.** DeckCommands.h:481-494. Fix: add `disposeHook_`
   (mirroring RemoveColumnCmd's shape — dispose every occupied cell in
   `removed_.clips` on execute, reconnect via existing `mediaHook_` on undo,
   which is already correct).
2. **MAJOR — `RemoveDeckCmd` has no media hooks at all; removing a deck
   permanently strands every clip in every layer it held.** DeckCommands.h:
   676-747. Fix: thread both `mediaHook_` and a new `disposeHook_` through the
   constructor, dispose every occupied cell across all layers on execute,
   reconnect on undo.
3. **LOW — `openGLContextClosing()` doesn't drain/release the two new retire
   lists**, leaving a narrow shutdown-time GL-call-with-no-context-current
   window for anything retired but not yet drained. Renderer.cpp:705-742 vs.
   991-1013. Fix: extend `openGLContextClosing()`'s existing release loop (or
   call a shared helper) over `retiredVideoPlayers_`/`retiredImageSequences_`
   too.
4. **INFO, not a regression — pre-existing, out of this lane's scope —
   `openVideoForClip`/`openImageSequenceForClip`'s map-assignment overwrite
   (Renderer.cpp:933-954) can destroy a live VideoPlayer/ImageSequence on the
   message thread outside the retire-list path**, when reconnect-on-replace
   fires for an id whose media is already open. Worth a follow-up ticket; not
   introduced by and not blocking this change.
METADATA: reviewer=rev-L1-media-leak, builder_packet=media-leak-fix.md, date=2026-09-05
