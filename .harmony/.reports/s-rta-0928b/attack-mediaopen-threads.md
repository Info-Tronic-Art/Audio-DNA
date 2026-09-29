# Attack: plan-mediaopen — message-thread/GL-thread race & lifecycle

VERDICT: MUST fix a real resource leak in staged-load cancellation (image
sequences opened synchronously are never retired on cancel); SHOULD tighten
the fence's flag/deck ordering (TOCTOU, not proof); NIT the loading label is
unspecified for one Kind. None of the three is caught by the plan's own gates.

## MUST — `cancelStagedOpen` leaks `Renderer::imageSequences_` entries
`beginStagedOpen` (plan §4.4) opens EVERY staged sequence clip synchronously
during staging: "ImageSequence -> `renderer.openImageSequenceForClip` ...
synchronously". `openImageSequenceForClip` (verified, `Renderer.cpp:1455-1472`)
calls `closeMediaForClip(clipId)` then inserts into `imageSequences_` under
`imageSeqMutex_` — a committed insert into the renderer's LIVE map, under the
freshly re-minted staged id, before the batch (or its swap) ever completes.
`cancelStagedOpen` (§4.4) only walks `staged_->adopted`: "`for (id :
staged_->adopted) renderer.closeMediaForClip(id);`". `adopted` is populated
ONLY by `onStagedLanded` for VIDEO landings ("`adoptVideoPlayer(id, ...);
staged_->adopted.push_back(id);`") — never by the synchronous sequence-open
path. A staged load with sequence clips that gets canceled (a second load
begins, `swapCompositionModel` runs for an unrelated reason, or quit mid-load
— R-6's own three cancel triggers) leaves those `ImageSequence` objects
resident forever: no id is in `adopted`, `staged_.reset()` destroys only the
model copy (the Deck/Composition), and `drainRetiredMedia`
(`Renderer.cpp:1509-1533`) never sees an id that was never handed to
`closeMediaForClip`. Every canceled sequence-bearing load leaks its
`ImageSequence` objects for the process lifetime (REST-load-spam or rapid
re-drag turns this unbounded); no §4.9 ctest or §4.10 probe row cancels a
sequence-bearing batch (m4 lets both sequences land). Fix: push sequence ids
into `staged_->adopted` (or a second list) at open time, not only video ids
at landing time. Counter: the objects are cheap (seqvram keeps
`frameBytesHint_` at 0 until first GL upload, so no VRAM window is ever
allocated for a never-swapped-in sequence) — a slow host-memory leak, not a
GPU/correctness hazard — but it stands as MUST because the fix is a one-line
addition to a list the plan already exists to maintain, and the plan tracks
other leaks deliberately (§4.2 "a prepared drop whose commit never runs...
would leak one player... stated, accepted") — this one was missed, not
accepted.

## SHOULD — the fence's deck/flag ordering is TOCTOU, not proof
§4.1's reader does `fenced = load(fenced_)`, `deck = load(activeDeck_)`, then
`if (deck==nullptr && !fenced) fenced = load(fenced_)` again — a second load
of the SAME atomic, no new synchronization edge vs. the first. The bad case
(GL thread reads `fenced_==false`, then reads `activeDeck_==null`, straddling
the writer's two back-to-back release stores) can in principle repeat on the
retry too, since it is still just two loads, not a barrier. R2 asks
`fence_black_frames == 0` to serve as the correctness witness — that proves
"no repro in N runs," not "cannot happen," for a gap one combined atomic
would close (e.g. a single `atomic<Deck*>` with a sentinel "fenced-null"
value, read once). Counter: the window is two back-to-back stores vs. a
~16 ms frame period — astronomically narrow, and a hit just reproduces one of
today's already-tolerated black frames, not corruption; low urgency.

## NIT — DeckDuplicate has no `file` for the "Loading <name>..." label
`duplicateDeck` (`MainComponent.cpp:3333,3341`) builds `copy` with no source
`juce::File`; §4.4's `StagedLoad` shares one `file` member the other two
Kinds use to build the label. `beginStagedOpen`'s label text
("`fileLabel_.setText("Loading " + name + "...")`") doesn't say what `name`
is for `Kind::DeckDuplicate`; the natural read (`staged->file...`) is empty
for this Kind only. Fix: label from `deck.name` for DeckDuplicate.

REPORT_FILE: .harmony/.reports/s-rta-0928b/attack-mediaopen-threads.md
