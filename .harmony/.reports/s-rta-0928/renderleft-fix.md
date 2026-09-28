# renderleft-fix (s-rta-0928) -- fix-round report + the lane's full state

STATUS: DONE
RESULT: I checked all four review findings against the code; each one is real, and each is fixed.
- MUST 1: Pitfalls 48/49/50 are restored verbatim.
- MUST 2: C1 now also pauses a crossfade onto an image sequence whose first frame is still decoding. New row i2ms: RED on the pre-fix lane app, GREEN.
- MUST 3: the missing C2 snapshot row is added (i3n). RED on main, plus a teeth build; GREEN.
- SHOULD 4: imagePaths dedup is now O(n).

On the final app: probe-image-load GREEN twice (37/0), probe-crossfade GREEN (35/0), serial ctest 846/846.
The lane (R1/R2/R3/R5 shipped, R4 filed) is otherwise as in renderleft.md; its fix-round section is appended there.
INBOX-RECHECK: none

## FACTS (FSP = /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/cfd08720-0a05-463c-84b1-89ae4354a3fb/scratchpad/renderleft-fix; runs in FSP/runs)
- Apps:
  - PREFIX = scratchpad/renderleft/apps/final: the lane head before this round (code == c129b8d == 5f1536f).
  - MAIN = /Users/boriskarpman/projects/RealTimeAudio/build/AudioDNA_artefacts/Release/Audio-DNA.app, used read-only.
  - TEETH = FSP/apps/snapgate: takeSnapshot calls `captureFrame(outputFile, -1.0f, 0, 0, true)`.
  - FINAL = FSP/apps/final: the build-lane app after the teeth restore. Its code is HEAD's code.
- **MUST 1: verified.**
  - `git show 5f1536f:docs/claude/pitfalls.md` numbers entries 47 then 51; `git show 15c5f8d:...` has 48/49/50 at lines 105-110.
  - Fix 15f1ff8 inserts those 6 lines verbatim after line 104.
  - Now `git diff 15c5f8d HEAD -- docs/claude/pitfalls.md` shows only the lane's own lines: the 37 and 46 amendments, plus the new 52 and 53.
  - Content check, FSP/patchcmp.sh: for each file, the multiset of +/- lines of the lane patch before the first rebase (6db8d67..79bafe7) was compared with the patch now (15c5f8d..tree).
    - 29 files are SAME.
    - CLAUDE.md, pitfalls.md, rendering.md and testing-eyes.md are the same once the pitfall numbers are ignored (48/49 -> 52/53).
    - probe-image-load.py differs only in the intended restore-lane comment (c129b8d).
    - No other file lost lines to a rebase.
- **MUST 2: verified.**
  - `CompositorEngine::incomingImagePending` returned false for `mediaType != Image`.
  - For a fresh ImageSequence, `ImageSequence::getCurrentTexture` sets *pending until its first frame uploads, so the dissolve ran on during the hold.
  - Fix abefe7e:
    - `ImageSequence::firstFramePending() const` implements the same rule without side effects: open and non-empty; current frame not resident; nothing shown yet; not failed; empty per-frame vectors => pending.
    - `CompositorEngine::setSequencePendingProvider`: Renderer's lambda locks imageSeqMutex_ and asks the sequence.
    - incomingImagePending calls it for ImageSequence, after the existing fade / layer-type early-outs.
  - New row i2ms_seq_fade_start: i2m with col 1 = 3 fresh 7680x4320 sequence frames at 0.1 fps; no fillers, because a sequence is never prefetched.
  - RED on PREFIX (FSP/runs/red-prefix.log, red-prefix-2.log):
    `FAIL  i2ms_seq_fade_start: the dissolve starts when the image lands (first answered frame p 0.578 <= 0.08)`; run 2 p 0.553; p = [0.553, 0.776, 1.0, 1.0].
  - RED on MAIN (red-main.log, red-main-2.log): p 0.262 / 0.261, the dt of main's in-frame decode.
  - GREEN on FINAL (green-1.log, green-2.log):
    `PASS  i2ms_seq_fade_start: the dissolve starts when the image lands (first answered frame p 0.000 <= 0.08)`; p = [0.0, 0.09, 0.238, 0.425] and [0.0, 0.078, 0.2, 0.357].
  - Frame looked at: green-1's f2 (p 0.238), downscaled in FSP/runs/i2ms_seq_fade_start_f2_small.png. The warm orange is visibly blending into the R/G gradient, as intended.
- **MUST 3: verified.** `grep -n snapshot .harmony/probe-image-load.py .harmony/probe-capture.py` found no row on 5f1536f.
  - New row i3n_snapshot_while_pending (7f56369, f6b7e0a). The steps: fillers, a warm image, a baseline /api/snapshot, trig to a slow cold image, a /api/snapshot at once (timed), then render_frame (timed).
  - PASS needs all of these:
    - the snapshot answers within snapshotMaxMs = 500;
    - the snapshot is the held warm picture (it did not wait for the decode);
    - the render_frame right after shows the new picture (the gate still waits).
  - The row deletes its snapshot files. `ls ~/Documents/Audio-DNA/Snapshots` was identical before and after every batch.
  - RED on MAIN (red-main-2.log):
    `FAIL  i3n_snapshot_while_pending: a snapshot while the cold image decodes answers (ok True) in 1422 ms <= 500 (baseline 73 ms)`
    and `FAIL  i3n_snapshot_while_pending: the snapshot is the picture of its moment -- the held picture, it did not wait for the decode (dbox(snap, capA) 76.72 <= 3.0)`.
  - TEETH (teeth-snapgate-2.log): 887 ms with dbox(snap, capA) 76.72, so 2 FAIL.
    - Restore: sha256 143677da...5da1 was the same before and after, and `git diff --quiet -- src/render/Renderer.cpp` was clean (FSP/teeth-snapgate.sh output).
  - GREEN:
    - FINAL: `PASS ... answers (ok True) in 78 ms <= 500 (baseline 75 ms)` and 72 ms; dbox(snap, held) 0.00; render_frame after it 665 / 686 ms, dbox 0.00 to the new picture.
    - PREFIX, which already did C2 right in code: 82-84 ms.
  - Calibration for 500 ms: ungated 72-84 ms (lane and main); gated 887-1442 ms.
- **SHOULD 4: verified** (std::find per clip). Fixed in a3691e1 with an unordered_set; the output order is unchanged. `compload::imagePaths: ... deduplicated` Passed (ctest #232).
- Gates on FINAL, locked runs, load avg 4.0-7.5:
  - probe-image-load `PY 37 PASS / 0 FAIL`, `PROBE-IMAGE-LOAD GREEN`, twice (green-1.log, green-2.log). i1/i2/i5/i6/i7 are all under 16.7 ms (max callback 9.77 ms).
  - probe-crossfade `PY 35 PASS / 0 FAIL`, `PROBE-CROSSFADE GREEN` (final-xfade.log; main was 35/0).
- ctest, serial: `ctest --test-dir build-lane -j1` -> `100% tests passed, 0 tests failed out of 846` (FSP/ctest-final.log).
- Screen safety: outwins after every probe run reported `audio-dna windows 0, Output-named 0`. The lock was released after each of the 3 batches, and quit_app reported `app running after quit: no`.

## METHOD
I first checked each finding against disk (git show / grep / a code read), then fixed it. Each fix is its own commit.
The new probe rows went RED first on the pre-change app for that item: the pre-fix lane app for C1 sequences, and main for
C2. C2 also got a teeth build, mutated in place and restored with the sha checked. The rows then went GREEN on the
final app, twice. The existing gates that touch the changed paths (image-load, crossfade) were re-run without
re-thresholding. The one new threshold, snapshotMaxMs, is calibrated from this round's own RED/GREEN numbers.

## CONFIDENCE + VERIFY
High: every finding has a RED -> GREEN pair or a disk diff. Verify:
- `git diff 15c5f8d HEAD -- docs/claude/pitfalls.md` should show only the 37/46 amendments and the new 52/53.
- `bash .harmony/probe-image-load.sh /tmp i2ms_seq_fade_start,i3n_snapshot_while_pending` with the lock held and IMGLOAD_APP set.

## UNKNOWNS / NOT DONE
- MUST 2 was fixed rather than narrowed in scope (option b), so no sign-off is needed for a narrowing. The pause covers sequences on the active deck and on persistent layers, the two callers of incomingImagePending. An off-screen deck's DeckClock advances crossfades without any pause. That was already so for images, and nothing is drawn there.
- In the first RED batch, i3n's own snapshots tripped the .sh's foreign-traffic check. A snapshot also logs `[Eyes] Captured frame:`. The row now lists its snapshots in own-captures.txt and the check skips them (f6b7e0a). Every run reported above comes after that fix, except red-prefix / red-main / teeth-snapgate run 1. Their row verdicts are the same as in run 2; the numbers differ slightly.
- Per-commit builds: only HEAD's code was built (FINAL) and ctest'd; the 4 intermediate commits were not built one by one. abefe7e and 7f56369 / f6b7e0a touch disjoint files from a3691e1.

## NUANCE
- `firstFramePending` is read before getCurrentTexture drains the mailbox. On the frame where the first frame arrives, the dissolve therefore stays at its starting value for that one frame, then runs its full length. The GREEN p(f0) of 0.000 confirms it.
- The snapshot row writes into the app's real Snapshots dir (~/Documents/Audio-DNA/Snapshots, because snapshotDir_ is never set) and removes exactly the files the app reports.

## HANDOFF-NEEDS
Harmony: independent review of 15f1ff8..HEAD, the behavioral gate, then the merge (lane/renderleft-0928 fast-forwards on
15c5f8d). Append the notebook notes below.

### SUMMARY
- Pitfalls 48/49/50 restored.
- C1 extended to image sequences (a side-effect-free peek), with row i2ms.
- C2 snapshot row i3n.
- imagePaths dedup is O(n).

### FILES CHANGED (this round)
- docs/claude/pitfalls.md: 48/49/50 restored.
- docs/claude/rendering.md: the C1 sentence now covers sequences.
- src/media/ImageSequence.h/.cpp: firstFramePending.
- src/render/CompositorEngine.h/.cpp: SequencePendingFn / setSequencePendingProvider; incomingImagePending covers sequences.
- src/render/Renderer.cpp: wires the provider.
- src/core/CompositionLoad.h: unordered_set dedup.
- .harmony/probe-image-load.{py,json,sh}: rows i2ms + i3n; own-captures exclusion; snapshotMaxMs; layer ids 75/76.
- .harmony/.reports/s-rta-0928/renderleft.md: Fix round appended.
- This file.

### TESTS
See FACTS for the RED/GREEN/teeth lines. ctest 846/846 serial.

### ISSUES / DEVIATIONS
- The i2ms row does not load fillers (a sequence is never prefetched). The i3n row does (like i3).

### FOUND, NOT FIXED
- Carried from renderleft.md: F16 (video decodes on the GL thread); ImageSequence keeps every frame's texture; existsAsFile per image clip per frame; ImageSequence::open decodes frame 0 on the message thread; the R4 4K excess; the unexplained i5 readings.

### RISKS
- A mutex lock (imageSeqMutex_) per frame, only while a layer crossfades onto an image sequence. It is the same mutex syncMedia takes in that frame anyway.

### METRICS
Lane: 18 commits after 15c5f8d, including this round's report commit. 35 files, +3689 / -267 before the report commit. CLAUDE.md is 24,482 bytes (unchanged).

### PACKET QUALITY
- Clarity: CLEAR. Each finding named its evidence and an acceptable fix.
- Missing context: none. The snapshot's `[Eyes] Captured frame:` log line colliding with the foreign-traffic check was found in the RED run.
- Unused context: none.
- Self-brief files: renderleft.md and the renderleft scratchpad (apps, batch scripts): useful.
- Self-assembly: no DEPARTMENT field.

### KNOWLEDGE CONTEXT
- Tools used: grep, git.
- Impact authority: grep, which is not authoritative, so I took a conservative posture. incomingImagePending has 2 callers, both read.
- Risk level: NORMAL (a narrow change on the C1 path).
- Queries made: 0.

### NOTEBOOK NOTES (for Harmony to append)
- After a rebase, compare each file's lane patch before vs after (the +/- line multiset of base..head per file), not just the conflict files. A resolved doc conflict silently dropped three other-lane pitfall entries (48/49/50) and passed a numbering-only check.
- `Renderer::captureFrame` logs `[Eyes] Captured frame:` for user snapshots too. A probe's foreign-traffic check must exclude the snapshots the run itself took (probe-image-load: own-captures.txt).
- /api/snapshot writes to ~/Documents/Audio-DNA/Snapshots (snapshotDir_ is never set). A probe that takes snapshots must delete the files the response names.

### BORIS CHECKS
1. A dissolve onto an image sequence never shown before now starts when its first frame appears and then runs its full length, instead of jumping part-way in.

### STATUS
DONE

### NEXT ACTION
Harmony: review + behavioral gate + merge.
