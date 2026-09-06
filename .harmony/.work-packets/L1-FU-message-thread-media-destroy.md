# L1-FU-message-thread-media-destroy work packet

## VERDICT: BUILDABLE — reuse `closeMediaForClip()` inside `openVideoForClip`/`openImageSequenceForClip`; one line added to each, no signature change, no call-site sweep.

## SIZE: trivial — 1 file functionally touched (`src/render/Renderer.cpp`, ~6 net lines incl. comments across 2 functions), 1 file doc-comment-only (`src/render/Renderer.h`, 0 signature changes). Zero call sites to update (no constructor/signature changed). No headless test can drive the real fix (see PROVE IT WORKS) — this is a genuine gap, stated plainly, not glossed over.

## CURRENT BEHAVIOUR (verified)

L1 (commit `349f676`) built a mutex-guarded retire list (`retiredMediaMutex_`,
`retiredVideoPlayers_`, `retiredImageSequences_`, `drainRetiredMedia()` —
Renderer.h:434-437, Renderer.cpp:1040-1059) so that `closeMediaForClip()`
(Renderer.cpp:1005-1039), which **may run on the message thread**, never lets
a `VideoPlayer`/`ImageSequence` destruct in place there. It `close()`s the
media immediately (FFmpeg/CPU-only, thread-safe), then **moves** the
`unique_ptr` into a retired vector instead of erasing-in-place; only
`drainRetiredMedia()`, called every frame from `renderOpenGL()`
(Renderer.cpp:151, GL thread) and once more from `openGLContextClosing()`
(Renderer.cpp:756, GL thread), actually calls `releaseGL()` and lets the
object destruct.

`closeMediaForClip()` has exactly ONE call site today —
`MainComponent.cpp:3636`, inside `makeClipMediaDisposeHook()` (the L1
"detach" hook commands call when a clip is genuinely leaving a cell).
Confirmed via `grep -rn "closeMediaForClip"` (see CALL-SITE ENUMERATION):
1 declaration, 1 definition, 3 comment mentions, 1 real call site.

`openVideoForClip()` (Renderer.cpp:982-991) and `openImageSequenceForClip()`
(Renderer.cpp:993-1003) do **not** go through this mechanism at all. Each
does a bare `map[clipId] = std::move(newPtr)` (lines 989 and 1001). If
`clipId` is already a key in that map, `std::unordered_map::operator[]`
value-assigns over the existing `unique_ptr`, which destroys the OLD
`VideoPlayer`/`ImageSequence` synchronously, on whatever thread called
`openVideoForClip`/`openImageSequenceForClip` — with no GL context current
if that thread is the message thread. `~VideoPlayer()` unconditionally calls
`releaseGL()` → `glDeleteTextures` (verified, VideoPlayer.cpp:26-29,
344-348). `~ImageSequence()` only calls `close()`, not `releaseGL()`
(ImageSequence.cpp:12-14) — so an in-place ImageSequence overwrite would
leak GL textures rather than call `glDeleteTextures` with no context (still
wrong, different failure mode: leak instead of possible-UB call).

## ROOT CAUSE (verified)

`openVideoForClip`/`openImageSequenceForClip` are the ONLY two functions
that ever insert or overwrite an entry in `videoPlayers_`/`imageSequences_`
(exhaustive enumeration below — every other touch is `find`/`erase`/a
read-only loop, all already under the correct mutex). They were written
before L1's retire list existed and were never updated to use it. The
ledger's own citation (`Renderer.cpp:933-954`) has already drifted to
932→982-1003 today — re-derived, not trusted.

Of the **7** call sites into these two functions repo-wide (all in
`src/MainComponent.cpp`, zero in `tests/` — two independent grep patterns
agree, see below), exactly **2** can hit a clip id that already has a live
entry in that function's own map. Those 2, and only those 2, are the
message-thread-destroy hazard:

1. **`src/MainComponent.cpp:3595`** — inside `makeClipMediaHook()`
   (`ClipMediaHook MainComponent::makeClipMediaHook()`, decl at line 3580).
   Anchor text: `renderer.openVideoForClip(clip.id, clip.mediaFile);`
   Guarded by `needsVideoReopen(renderer.getVideoPlayerFile(clip.id),
   clip.mediaFile, exists)` (`src/core/MediaReconnect.h:12-17`), which
   returns `true` when `exists == true` AND the loaded file differs from
   the clip's current file — i.e. exactly the "id-stable content swap"
   case the header comment names. This hook is `mediaHook_` in
   `SetClipCmd::apply()` (`src/core/ClipCommands.h:96-108`, fires
   unconditionally whenever `state.has_value()`) and in
   `SwapClipsCmd::applyCell()` (`ClipCommands.h:230-244`). Reached on:
   - **Redo/undo of a "Replace Content" edit** (see site 2 below for how
     the SetClipCmd gets created) — undo restores the OLD `before_` clip
     with its OLD file while the NEW file's player is still live at that
     same id → mismatch → reopen → destroys the live (new) player in
     place. Redo does the mirror swap back.
   - `SwapClipsCmd`'s video branch is **provably safe** despite going
     through the same hook: a swap/move never changes a clip's
     `mediaFile`, only its cell coordinate, so `loadedFile == clipFile`
     always holds and `needsVideoReopen` returns `false` — verified by
     reading `SwapClipsCmd::apply`/`applyCell` (ClipCommands.h:196-249):
     it re-copies the SAME `Clip` value into a new cell; content is
     never mutated.

2. **`src/MainComponent.cpp:4570`** — inside the `kClipReplaceContent`
   menu-command handler (`case C::kClipReplaceContent:` at line 4529, the
   "Replace Content keeping effects" feature), inside the
   `fileChooser_->launchAsync(...)` completion lambda (chooser created at
   line 4535). Anchor text: `if (renderer.openVideoForClip(existing->id, file))`.
   This is a **direct, unguarded** call on `existing->id` — the id of
   whatever clip is already in the targeted cell — called BEFORE
   `existing->replaceContent(newContent)` (line 4595) even runs. If
   `existing` was already a Video clip (so `videoPlayers_[existing->id]`
   is live), this overwrites it in place, unguarded, on the very FIRST use
   of "Replace Content" — no undo/redo round-trip required. This is a
   **second, independent entry point** into the same hazard, not named in
   the ledger snippet quoted to this lane (which only named the
   reconnect-hook path) — exactly the kind of undercount the brief warned
   about.
   Bonus finding, not a threading hazard but a real leak from the same
   line: if `existing` was previously an **ImageSequence** clip (not
   Video), this call only touches `videoPlayers_`; the old
   `imageSequences_[existing->id]` entry is never found or closed by
   anything on this path — it is silently orphaned (still in the map,
   still holding CPU/GL state) forever. `MediaReconnect.h`'s own comment
   confirms sequences never get id-stable-swapped back the other way
   ("replace only produces Image/Video; sequences always get a fresh
   id"), so this leak is one-directional (ImageSequence → Video/Image via
   Replace Content) and does not require an undo/redo cycle either.

**Thread verified for both hazardous sites:**
`UndoManager::perform/undo` assert the message thread in debug builds —
`#define UNDO_ASSERT_MESSAGE_THREAD()` (`src/core/UndoManager.cpp:8-10`),
invoked at the top of `perform()` (line 14) and `undo()` (line 50). Since
`SetClipCmd`/`SwapClipsCmd::execute()/undo()` (and hence `mediaHook_`, site
1) only ever run inside `UndoManager::perform/undo`, site 1 is
message-thread VERIFIED, not inferred. Site 2 runs inside a
`juce::FileChooser::launchAsync` completion callback; JUCE dispatches that
callback on the message thread by contract, corroborated in-file by the
same lambda calling `undoService_.withDeckDetached(...)` (line 4595) and
`deckView_->rebuildGrid()`/`inspectorPanel_->refresh()` (lines 4604-4605) —
both message-thread-only operations elsewhere in this codebase. Called this
VERIFIED on the strength of that JUCE contract + corroborating same-lambda
calls, not by reading JUCE's own source in this repo.

## FILES TOUCHED

- **`src/render/Renderer.cpp`** — the only functional change. Add one call,
  `closeMediaForClip(clipId);`, inside `openVideoForClip` (after the
  `player->open()` success check, before the `videoPlayers_[clipId] =`
  assignment) and inside `openImageSequenceForClip` (same shape), plus a
  comment on each explaining why (see THE CHANGE). No other line in this
  file needs to move.
- **`src/render/Renderer.h`** — doc-comment-only. The existing comment on
  `openVideoForClip`'s declaration (lines 173-175, "Call from message
  thread. The Renderer manages the VideoPlayer lifecycle.") and the bare
  one-liner on `openImageSequenceForClip` (line 178) should note that a
  pre-existing live entry at `clipId` is now retired through the same
  GL-thread-drained path `closeMediaForClip` uses, instead of being
  destroyed in place. No signature change.

Nothing else. No command constructor changes, no header changes to
`ClipCommands.h`/`DeckCommands.h`, no `MainComponent.cpp` changes — the fix
lives entirely inside the two `Renderer` functions that already own the
maps, so every one of the 7 existing call sites (hazardous or not) gets the
fix for free with zero call-site edits.

## CALL-SITE ENUMERATION (raw grep output)

Two independent patterns for each symbol, repo root, build dirs excluded —
both agree, so "7 call sites, 0 in tests" is not a single-pattern fluke.

```
$ grep -rn "openVideoForClip(" --include="*.cpp" --include="*.h" . | grep -v "^./build"
src/MainComponent.cpp:3361:                renderer.openVideoForClip(clip->id, clip->mediaFile);
src/MainComponent.cpp:3595:                renderer.openVideoForClip(clip.id, clip.mediaFile);
src/MainComponent.cpp:3827:        if (renderer.openVideoForClip(clip.id, file))
src/MainComponent.cpp:4570:                            if (renderer.openVideoForClip(existing->id, file))
src/render/Renderer.h:176:    bool openVideoForClip(uint32_t clipId, const juce::File& videoFile);
src/render/Renderer.cpp:982:bool Renderer::openVideoForClip(uint32_t clipId, const juce::File& videoFile)

$ grep -rn "openVideoForClip" --include="*.cpp" --include="*.h" . | grep -v "^./build"
[identical 6 lines — bare-symbol pattern agrees exactly with call-syntax pattern]

$ grep -rn "openImageSequenceForClip(" --include="*.cpp" --include="*.h" . | grep -v "^./build"
src/MainComponent.cpp:3374:                renderer.openImageSequenceForClip(clip->id, clip->sequenceFiles, clip->sequenceFps);
src/MainComponent.cpp:3603:                renderer.openImageSequenceForClip(clip.id, clip.sequenceFiles, clip.sequenceFps);
src/MainComponent.cpp:3909:    renderer.openImageSequenceForClip(clip.id, clip.sequenceFiles, clip.sequenceFps);
src/render/Renderer.h:179:    bool openImageSequenceForClip(uint32_t clipId, const std::vector<juce::File>& files, float fps);
src/render/Renderer.cpp:993:bool Renderer::openImageSequenceForClip(uint32_t clipId, const std::vector<juce::File>& files, float fps)

$ grep -rn "openImageSequenceForClip" --include="*.cpp" --include="*.h" . | grep -v "^./build"
[identical 5 lines — bare-symbol pattern agrees exactly with call-syntax pattern]

$ grep -rn "openVideoForClip(\|openImageSequenceForClip(" tests/    -> exit 1, no matches
$ grep -rln "openVideoForClip\|openImageSequenceForClip" tests/     -> exit 1, no matches
```

So: **4 call sites** for `openVideoForClip`, **3** for
`openImageSequenceForClip` — 7 total, all in `MainComponent.cpp`, 0 in
`tests/` (confirmed by two patterns each, all four runs agree). No
signature is changing, so this enumeration's only purpose is hazard
classification (next), not a call-site sweep.

**Full enumeration of every write path into the two maps** (the "counts
are claims" check — every touch of `videoPlayers_`/`imageSequences_` in
`Renderer.cpp`, not just the open functions):

```
$ grep -n "videoPlayers_\b" src/render/Renderer.cpp src/render/Renderer.h
Renderer.h:411   (member declaration)
Renderer.cpp:734  for (auto& [id, player] : videoPlayers_)      <- READ, GL thread (openGLContextClosing), releaseGL() loop, under videoPlayerMutex_
Renderer.cpp:989  videoPlayers_[clipId] = std::move(player);    <- WRITE (insert/overwrite) — openVideoForClip — THE bug
Renderer.cpp:1018 auto it = videoPlayers_.find(clipId);         <- closeMediaForClip
Renderer.cpp:1019 if (it != videoPlayers_.end())
Renderer.cpp:1024 videoPlayers_.erase(it);                      <- WRITE (erase, into retire list) — closeMediaForClip
Renderer.cpp:1069 auto it = videoPlayers_.find(clipId);         <- READ — getVideoPlayer
Renderer.cpp:1076 auto it = videoPlayers_.find(clipId);         <- READ — getVideoPlayerFile
Renderer.cpp:1095 auto it = videoPlayers_.find(clip->id);       <- READ — getVideoFrameTexture (GL thread)

$ grep -n "imageSequences_\b" src/render/Renderer.cpp
Renderer.cpp:741  for (auto& [id, seq] : imageSequences_)       <- READ, GL thread, releaseGL() loop
Renderer.cpp:1001 imageSequences_[clipId] = std::move(seq);     <- WRITE (insert/overwrite) — openImageSequenceForClip — THE bug
Renderer.cpp:1029/1030/1035  find/erase                          <- WRITE (erase, into retire list) — closeMediaForClip
Renderer.cpp:1083/1084       find                                <- READ — getImageSequence
Renderer.cpp:1161/1162       find                                <- READ — getVideoFrameTexture, ImageSequence branch (GL thread)
```

This confirms lines 989 and 1001 are the ONLY two lines in the entire
codebase that can insert-or-overwrite into these maps. Every other touch
is read-only or the existing, already-safe `closeMediaForClip` erase path.
That is why the fix is exactly 2 call sites inside `Renderer.cpp`, not a
sweep across `MainComponent.cpp`.

`closeMediaForClip` call sites today (to show the fix does not need to
create any new mechanism, only add 2 more callers of an existing one):

```
$ grep -rn "closeMediaForClip" --include="*.cpp" --include="*.h" . | grep -v "^./build"
src/MainComponent.cpp:3583  (comment)
src/MainComponent.cpp:3636  previewPanel_.getRenderer().closeMediaForClip(clip.id);   <- the ONE real call site (makeClipMediaDisposeHook)
src/render/Renderer.h:188   void closeMediaForClip(uint32_t clipId);                   <- declaration
src/render/Renderer.h:417,422 (comments)
src/render/Renderer.cpp:147,745,1047 (comments)
src/render/Renderer.cpp:1005 void Renderer::closeMediaForClip(uint32_t clipId)         <- definition
```

## THE CHANGE

Reuse `closeMediaForClip()` — which already knows how to retire an entry
from EITHER map through the mutex-guarded retire list — as the very first
thing both open functions do once they know the new media opened
successfully. No new members, no new mutex, no new vectors: literally the
mechanism L1 built, called twice more.

```cpp
// src/render/Renderer.cpp

bool Renderer::openVideoForClip(uint32_t clipId, const juce::File& videoFile)
{
    auto player = std::make_unique<VideoPlayer>();
    if (!player->open(videoFile))
        return false;

    // Retire whatever media (video OR image-sequence) currently occupies
    // this clip id through closeMediaForClip()'s existing GL-thread-drained
    // retire list, instead of letting the operator[] assignment below
    // destroy a live VideoPlayer in place (message-thread destroy, L1-FU,
    // 2026-09 — same hazard class L1's retire list exists to fix, reached
    // via reconnect-on-replace (makeClipMediaHook) and "Replace Content"
    // rather than Clip>Clear). Only done AFTER the new player has opened
    // successfully, so a failed open leaves the currently-live media
    // untouched, matching this function's existing no-op-on-failure
    // contract.
    closeMediaForClip(clipId);

    std::lock_guard<std::mutex> lock(videoPlayerMutex_);
    videoPlayers_[clipId] = std::move(player);
    return true;
}

bool Renderer::openImageSequenceForClip(uint32_t clipId, const std::vector<juce::File>& files, float fps)
{
    auto seq = std::make_unique<ImageSequence>();
    seq->setFps(fps);
    if (!seq->open(files))
        return false;

    // See openVideoForClip's comment above — same reuse of the retire list,
    // and this direction also closes the "Replace Content on an
    // ImageSequence clip" leak (old imageSequences_[clipId] entry would
    // otherwise never be found by anything once the id starts being used
    // as a video id instead).
    closeMediaForClip(clipId);

    std::lock_guard<std::mutex> lock(imageSeqMutex_);
    imageSequences_[clipId] = std::move(seq);
    return true;
}
```

Step by step, why this is correct and sufficient:

1. `player->open()`/`seq->open()` run BEFORE `closeMediaForClip`, unchanged
   from today — a failed open still returns `false` with zero side effects
   on existing state, exactly like today.
2. `closeMediaForClip(clipId)` internally takes `videoPlayerMutex_` then
   (nested) `retiredMediaMutex_` for the video half, and separately
   `imageSeqMutex_` then `retiredMediaMutex_` for the sequence half — each
   pair fully acquired-and-released in its own scope block
   (Renderer.cpp:1005-1039) BEFORE `closeMediaForClip` returns. By the time
   `openVideoForClip` takes `videoPlayerMutex_` itself for the final
   assignment, `closeMediaForClip`'s lock on that same mutex has already
   been released — no nested/reentrant lock, no new lock-ordering to
   reason about.
3. For the 5 already-safe call sites (3361, 3374, 3603, 3827, 3909), no
   live entry exists at `clipId` when they call — `closeMediaForClip`
   becomes a pure no-op (both `.find()` calls miss) at negligible extra
   cost (two mutex lock/unlock pairs, two map lookups). Zero behavior
   change for those 5.
4. For the 2 hazardous sites (3595, 4570), the OLD entry is now `close()`d
   and handed to `retiredVideoPlayers_`/`retiredImageSequences_` instead of
   being destroyed by the map assignment — `drainRetiredMedia()` picks it
   up and calls `releaseGL()` + lets it destruct on the very next
   `renderOpenGL()` frame, on the GL thread, exactly like a normal
   Clip>Clear today.
5. Update the two declaration comments in `Renderer.h` (lines 173-175,
   178) to say so — doc-only, no signature change, so nothing else in the
   repo needs to change to match.

## FENCE

The builder for this lane may WRITE only:
- `src/render/Renderer.cpp`
- `src/render/Renderer.h`

No other file needs to change. In particular: no file under `src/core/`,
no `MainComponent.cpp`, no `MainComponent.h`, and nothing under `tests/`
is required by the fix itself (see PROVE IT WORKS for why a new test is
not being fenced in here).

## TRAPS

- **Do not reorder the `closeMediaForClip(clipId)` call before the
  `open()` success check.** Retiring the old media before confirming the
  new media loaded would leave the cell's old, still-wanted media
  destroyed even on a failed replace/reconnect attempt — strictly worse
  than today's behavior (today a failed open just leaves the OLD, correct
  media in place and returns `false`; the caller keeps showing/playing the
  old clip). Order matters: open-new, THEN retire-old, THEN install-new.
- **`closeMediaForClip` retires from BOTH maps unconditionally** — it
  doesn't know or care which map's entry (if any) is being replaced. That
  is exactly what makes it reusable here (see bonus fix on the ImageSequence
  leak), but it does mean a call to `openVideoForClip` will also close out
  an ImageSequence living at that id, and vice versa. That is the CORRECT
  behavior for this codebase's invariant ("a clip id holds at most one live
  media object" — same invariant `makeClipMediaDisposeHook`'s load-bearing
  comment at MainComponent.cpp:3628-3634 already documents and depends on)
  — do not narrow it to "only retire same-type" thinking that's safer; it
  would resurrect the cross-type leak this fix also closes.
- **`ImageSequence`'s destructor does not call `releaseGL()`** (only
  `close()` does the CPU-side reset; textures_ is explicitly left alone —
  ImageSequence.cpp:88-98, with the file's own comment "Don't clear
  textures_ here — releaseGL() handles that"). This means the EXISTING
  `drainRetiredMedia()` calling `seq->releaseGL()` before letting it
  destruct (Renderer.cpp:1057-1058) is load-bearing for image sequences,
  not just for video — do not "simplify" `drainRetiredMedia` to skip the
  explicit `releaseGL()` calls on the theory that the destructor handles
  it; for `ImageSequence` it does not, and skipping it would silently leak
  GL textures instead of crashing (a worse-to-diagnose failure than a
  crash would be).
- **The `apiServer_->onTriggerClip` callback runs on the API server's own
  background thread with no `MessageManager::callAsync` marshal**
  (`src/api/ApiServer.h:30-32` doc comment: "Runs on a background thread";
  wired directly at `MainComponent.cpp:1678`, contrast with
  `oscHandler_.onTriggerClip` at `MainComponent.cpp:1706-1708`, which DOES
  wrap in `callAsync`). `handleClipTrigger` reached this way can execute
  the SAFE guarded opens (site 3361/3374) off the message thread. This
  fix's `closeMediaForClip` no-ops correctly there too (no live entry to
  retire in the guarded case), so it does not change safety either way —
  but it means a THEORETICAL TOCTOU race exists independent of this fix:
  two threads racing the same "if (!exists) open" pattern for the same
  clip id could both pass the check before either inserts, and the second
  insert would then destroy the first thread's freshly-created player via
  the exact `operator[]` hazard this packet is about — just racy rather
  than deterministic, and not through either of the 2 hazardous sites this
  packet fixes. Flagged as OPEN QUESTION below; NOT fixed by this lane
  (would need a compare-and-swap-shaped API change, out of this lane's
  size).
- **Line numbers here WILL drift.** Every anchor above is quoted with its
  exact text; re-grep before trusting a line number.

## HOW TO PROVE IT WORKS

**What a headless `ctest` run CAN prove** (pure logic, already covered,
unaffected by this change): `needsVideoReopen`'s 4 branches
(`tests/test_undo_commands.cpp:655-668`) and the hook-call-counting tests
for `SetClipCmd` (lines 358-390ish) still pass unchanged — this fix touches
neither `needsVideoReopen` nor the `ClipMediaHook`/`ClipMediaDisposeHook`
call sequence, only what `Renderer::openVideoForClip`/
`openImageSequenceForClip` do internally, which none of those tests
construct (they use no-op counting lambdas, never a real `Renderer`).

**What a headless `ctest` run CANNOT prove, and why**: no test target in
this repo links `Renderer.cpp` (confirmed: `grep -rln "Renderer"
tests/` finds only `test_renderer_source_confinement.cpp`, whose own
header comment states plainly — "Driving the real Renderer/JUCE
OpenGLContext headless is infeasible: it requires a live, attached GL
context with a running render thread... no target links CompositorEngine.cpp
or Renderer.cpp for exactly this reason"). That test instead mirrors the
OWNERSHIP-CONFINEMENT PATTERN in a standalone harness class, not
`Renderer` itself. The same limitation applies here: there is no existing
or newly-buildable headless path to construct a real `Renderer`, call
`openVideoForClip` twice on the same id, and assert that the second call's
old-`VideoPlayer` destruction happened via the retire list rather than
in-place. A new test COULD mirror the pattern the way
`test_renderer_source_confinement.cpp` does (a standalone map + retire-list
class exercising the same shape under TSan) — that would prove the
MECHANISM again, not this specific fix, and this lane is NOT adding one
because it would be testing a re-implementation of code that already has
one working, reviewed instance (`drainRetiredMedia`), not the actual 2-line
change. Recommend: if a ctest gate is wanted for this fix specifically, it
would have to be a targeted addition of a minimal real-GL-context test
harness for `Renderer` — a materially larger lane than this one, flagged
under OUT OF SCOPE.

**What a human must check in the running app** (this is the actual proof
for this fix):
1. Drop a video onto a cell (id X gets a live `VideoPlayer`).
2. Right-click → "Replace Content" → pick a DIFFERENT video file for the
   same cell. Confirm: no crash, no glitch frame, new video plays. This
   exercises hazardous site 4570 directly — before the fix, this is the
   line that can destroy the first `VideoPlayer` in place on the message
   thread.
3. Press Cmd+Z (undo the replace). Confirm: no crash, original video
   reappears and plays. This exercises hazardous site 4570's `SetClipCmd`
   round-trip through hazardous site 3595 (`makeClipMediaHook`) on undo.
4. Press Cmd+Shift+Z (redo). Confirm: no crash, replaced video reappears.
   Exercises site 3595 again, redo direction.
5. Repeat steps 1-4 rapidly several times in a row (stress the timing
   window between `close()`-on-message-thread and `releaseGL()`-on-next-
   GL-frame) — no crash, no visual corruption, no growing memory/GPU
   handle count over many repetitions (watch a GPU/process memory monitor
   across ~50 replace+undo+redo cycles; flat is pass, climbing is the bug
   still present or a new leak this fix introduced).
6. Drop an image SEQUENCE onto a cell, then "Replace Content" with a
   video file for that same cell. Confirm no crash (this is the
   cross-type path, hazardous only in the "old ImageSequence never
   closed" leak sense pre-fix, not a crash sense — the visible proof here
   is the same repeated-cycles memory-flatness check in step 5, not a
   single visible symptom).
7. Ideally run the above under AddressSanitizer/ThreadSanitizer builds
   (`build-asan`/`build-tsan` targets already exist in this repo per the
   `find` output above) if the app can be launched under those builds
   interactively — a `glDeleteTextures` call with no context current is
   the kind of GL-API misuse ASan/the GL debug layer may flag even when it
   doesn't visibly crash.

## OUT OF SCOPE

- **The TOCTOU race on `apiServer_->onTriggerClip`'s un-marshaled thread**
  (see TRAPS) — real but orthogonal (a race on the GUARDED, already-safe
  call sites, not the 2 hazardous ones this packet targets) and would need
  an API shape change, not a retire-list reuse.
- **Building a headless GL-context test harness for `Renderer`** so this
  fix (or any future `Renderer` fix) can get a real `ctest` proof instead
  of only human verification — flagged as valuable but clearly a separate,
  larger lane (`test_renderer_source_confinement.cpp` already documents
  this gap for a different `Renderer` mechanism; it is the same gap here).
- **Auditing every OTHER `Renderer` map/resource for the same
  "operator[]-overwrites-a-live-GL-owning-object" shape** beyond
  `videoPlayers_`/`imageSequences_` (e.g. `activeSources_` — though that
  one already has its OWN documented, tested confinement mechanism per
  `test_renderer_source_confinement.cpp`, which is a different, already-
  solved case, not left over). Doing a full-`Renderer` sweep for this
  shape is a reasonable follow-up audit but is explicitly not what this
  ledger item scoped ("openVideoForClip/openImageSequenceForClip... every
  remaining site that can destroy a media object off the GL thread" reads
  as scoped to the video/image-sequence media maps specifically, which is
  what got exhaustively enumerated above).

## OPEN QUESTIONS

- Should `Renderer.h`'s doc comment on `openVideoForClip` (line 174,
  "Call from message thread.") be strengthened to explicitly say "safe to
  call even when clipId already has live media — the old one is retired,
  not destroyed in place" now that it's true? Recommend yes (see THE
  CHANGE step 5) but leaving the exact wording to the builder/reviewer
  rather than prescribing it here.
- Is the `apiServer_->onTriggerClip` un-marshaled background-thread call
  (TRAPS, above) itself a known/accepted design choice (HTTP trigger
  latency matters more than the marshal round-trip) or an oversight that
  should ALSO get a `callAsync` wrap to match `oscHandler_`'s pattern? Not
  answerable from source alone — flagging for whoever owns the API-server
  lane; not this one.
