# S166-LEAK — Replacing a video clip with an image leaks the decoder, permanently.

REPORT_FILE: /Users/boriskarpman/projects/RealTimeAudio/.harmony/.reports/s166/builder-media-leak.md

## THE LANE IN ONE LINE
`kClipReplaceContent`'s IMAGE branch calls neither open nor close on the OUTGOING
clip id, so replacing a video clip with an image orphans the existing media entry —
the decoder is never released and the map entry is never cleaned up.

## PROVENANCE — inherited, so verify it before you act
Found by lane L1-FU's independent reviewer in session s-rta-0905, recorded as
pre-existing and outside that lane's fence, and never fixed. It is item 7 of that
session's loose-ends ledger in `.harmony/HANDOFF.md`. **This is an INHERITED claim.
Re-derive it from source before changing anything** — this repo has a documented
history of prescribed fixes that were already implemented, and of inherited facts
that were wrong.

## THE SHAPE OF THE CORRECT FIX (a claim, not an instruction)
The VIDEO branch of the same command presumably does the right thing. Read both
branches, establish what the video path does with the outgoing id, and make the image
path consistent with it. Session s-rta-0905's lane L1-FU routed exactly this hazard
class through an existing **retire list** rather than destroying GL objects on the
message thread — `~VideoPlayer()` calls `glDeleteTextures`, so releasing a decoder on
the wrong thread is itself a bug. Find that retire mechanism (start from commit
`c7247a9`) and REUSE it. Do not invent a second release path.

## WHAT DONE MEANS
1. Replacing a clip's content releases the outgoing media exactly once, through the
   existing retire mechanism, on the correct thread.
2. Video→image, image→video, video→video and image→image all behave consistently.
3. No double-release, no use of a released handle, no release on the message thread.
4. A test covers at least the video→image transition that is broken today.

## FENCE — you may edit ONLY these
- the file implementing `kClipReplaceContent` (name it in your report before editing)
- the media open/close/retire helpers it must call
- a test file under `tests/`
Anything else: STOP and report.

## BUILD AND TEST RULES — this repo's documented false-green traps
- **Capture the build exit code and READ it before quoting any test number.**
- ctest baseline **222/222**, re-run on disk at s166 boot. Re-run it; never inherit.
- Do NOT launch the app. Build + ctest only.
- Prove any new test load-bearing: neutralize the fix, rebuild, confirm FAIL, restore
  byte-identically (md5), confirm PASS. Report both numbers.

## REPORTING
STATUS (DONE / DONE_WITH_CONCERNS / BLOCKED), whether the inherited claim was
accurate, what you changed, build exit code, before/after ctest, load-bearing proof,
PACKET QUALITY, and anything outside the fence.
