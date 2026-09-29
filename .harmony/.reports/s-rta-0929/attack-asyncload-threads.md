# ATTACK — plan-asyncload.md (concurrency / lifetime seat)

VERDICT: the ticket/WeakReference/generation machinery is sound (`get()` is only ever
called on the message thread; `ReferenceCountedObject`'s refcount is atomic —
juce_ReferenceCountedObject.h:88-147 — so cross-thread capture of a `WeakReference` is
safe despite JUCE's doc calling it "not thread-safe"). Two load-bearing claims are wrong
or unaudited, and one shutdown path has no answer for a hang.

## MUST 1 — R3's core-pressure arithmetic omits FFmpeg's own frame-threading workers
`VideoPlayer.cpp:108` sets the decoder's `thread_count = 2` (`avcodec_open2`) — every
`open()` on a MediaOpener pool thread spawns up to 2 *additional* FFmpeg-internal
decode-worker threads for that one open. R3 (plan-asyncload.md:189-199) budgets only
"2 low-priority openers" against the old composition's `normal`-priority decode threads;
with 2 pool workers each driving up to 2 FFmpeg workers, real concurrent pressure is up
to ~4 threads, not 2 — the "3-4 P + 2 E cores free" claim and a2(g)/(h)'s xrun/late-frame
gates are sized against half the actual thread count. Fix: state the true worst case, or
set `thread_count = 1` for pool-driven opens (a one-line change) and remeasure.

## MUST 2 — no answer for a hung (not merely failing) `VideoPlayer::open()` at quit
`~MediaOpener` = `masterReference.clear(); pool_.removeAllJobs(true, 5000)` — a bounded
wait. a7 only exercises fast-failing files (bad pix_fmt, missing path); nothing exercises
an open that *hangs* (stalled network mount, device vanished mid-`avformat_open_input`,
plain blocking FFmpeg calls, no cooperative cancellation here). If the 5 s elapses with
the job still running, the plan never says what happens next: `pool_`'s own destructor
runs immediately after, in member teardown, and may forcibly stop/detach the OS thread
while it still holds the job's `unique_ptr<VideoPlayer>` and a raw `VideoStats*` into
`previewPanel_` — a use-after-free race, not just a slow quit. `end_quit_mid_load`'s bar
("terminated within 30 s") cannot catch this since a merely-slow path already passes.
Fix: an `AVIOInterruptCB` on `formatCtx_` keyed off an atomic MediaOpener sets before
waiting, so a stuck open aborts inside the window; add a hanging-open fixture to the
`end_quit_mid_load` row.

## SHOULD
- `makeThumbnail()` (`VideoPlayer.cpp:531`) builds a `juce::Image` via the plain ctor,
  which is `NativeImageType` (CoreGraphics-backed on macOS, `juce_Image.cpp:279-280`) —
  now constructed on a pool thread. The plan's sole safety argument ("No GL, no thread")
  never mentions this. Very likely fine — `ImageDecode.h:92` already builds `juce::Image`
  off-thread — but the plan should cite that precedent, and a ctest should pixel-diff a
  pool-built thumbnail against a message-thread-built one.
- R9's new RT-callback atomics are correctly gated (`AUDIODNA_BUILD_TEST_SERVER` default
  OFF, CMakeLists.txt:62/452-453), so shipped Sacred-Rule-1 compliance holds — but no
  ctest asserts `is_lock_free()` for them on the probe build's CPU; a non-lock-free
  fallback would silently add a lock to the audio callback in every probe run unnoticed.
- `removeAllJobs(false, 0)`'s "returns without waiting" is stated INFERRED (5.0(a)) and
  gates the whole non-blocking-cancel contract (R4, a3/a3b); the only fallback if JUCE
  blocks is prose, not code — needs a concrete Plan-B before commit 3.

## NIT
`/api/state video_players` counts staged-but-not-cut adopted players together with live
ones the instant `installVideoPlayer` runs; no row reads this field mid-window, so a
live-state reader would see an inflated, unlabeled count.

## Strongest counterargument to this seat's own position
Both MUSTs are boundary/degenerate cases, not demonstrated races in the mainline path:
the ledger + WeakReference design is provably correct for cancel/supersede/happy-path.
MUST 1 could just mean raising `swapMaxS`, not corrupting anything; MUST 2's 5 s ceiling
is generous against real ~68-100 ms opens. A reviewer could down-rank both to SHOULD on
those grounds. I hold MUST because both are exactly the failure class that happens once,
mid-show, on a flaky external drive, and the plan's own gates (a2(g)/(h),
end_quit_mid_load) are structurally unable to catch either.
