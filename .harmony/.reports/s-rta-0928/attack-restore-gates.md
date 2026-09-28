# Attack: plan-restore.md gates (GATE + REGRESSION SKEPTIC seat)

VERDICT: SOUND_WITH_FIXES. The cause-2 gate (Player.cpp fix plus C1-C5/E1) is a genuine
RED-to-GREEN, teeth-checked, minimal-diff gate. The cause-1 gate (holdMs/5h/8h/11h/7m) is real
wall-clock measurement, not mocked, but its scope and a residual cost are under-stated, and one
teeth check and one citation are incomplete. Cheap to close before a builder starts.

## Verified against HEAD (6db8d67)
- Player::advanceTo's close branch matches F5 exactly (src/recording/Player.cpp:162-170: once
  pos reaches g.x1 it releases with no set()), so C1-C5/E1 are genuinely RED today and the
  one-line fix is minimal and independent of the measurement code (holdMs).
- G5/J2 current expectations match F7 exactly (tests/test_routine_engine.cpp:1137-1142,
  1519-1525: Release then a fresh Touch/Set, no end-value Set), so the plan's deltas are real
  behavior changes the fix causes, not test-massaging to match unrelated code.
- probe-routines.sh:373-380 (8g) and :427-435 (11g) use wide bands (strictly between 0.92 and
  0.98, or a max above 0.75, both non-decreasing), so the fix's small glide-start shift will not
  break them; the "MUST NOT CHANGE" claim holds functionally.

## MUST
none. No defect that ships and no gate that structurally cannot fail.

## SHOULD
1. Teeth gap on the exact tests a skeptic most doubts. Step 2.5 (plan-restore.md:315-316)
   reverts the sink.set condition and requires only C1/C2/C3/C5/E1 to fail; it never re-checks
   that the just-edited G5/J2 (:287-311) also flip red under the same revert. Almost certainly
   they would (identical branch), but "almost certainly" is exactly what the plan's own teeth
   discipline elsewhere refuses to accept. Fix: add G5/J2 to the Step 2.5 revert check.
2. Citation slip in "MUST NOT CHANGE" (plan-restore.md:789): it names rows 5g/8g/9g/11j. The
   file's own header (.harmony/probe-routines.sh:12) and the actual Ease-glide set is
   5g/8g/9g/11g (:427); 11j (:894-921) is a different, unrelated Jump-hard-cut check. A builder
   checking "the right rows did not move" could check the wrong row.
3. holdMs's scope is narrower than the live rows' plain claim of at most 16 ms. It wraps only
   startNow and the tick loop end, not ClipCell::paint's per-paint existsAsFile() stat (filed by
   the plan itself as R-4c, :860-861) or the F8 legacy-image-lock hold (R-4a, :856-858,
   deferred to renderleft R1.3). Disclosed, but the row text (5h/8h/11h/7m) does not say "of the
   two diagnosed causes only"; a GREEN could be misread as "no restore hold exists."
4. Untested residual cost inside the "fixed" path: ClipThumbnails::get() calls
   ThumbnailCache::get(), which calls file.getLastModificationTime() (src/ui/ThumbnailCache.h:30),
   a stat() syscall, on the message thread. Per ClipCell's planned memo (plan-restore.md:601-604)
   the early-return guard needs thumbnail_.isValid(), so while a decode is in flight every
   refresh() re-enters get() and re-stats the file -- bounded but not zero, absent from the
   sub-1ms target derivation and unasserted by S1/B1/B2 (which check decode counts and thread
   ids, never wall time of the pending window).
5. "Smallest change" is not clearly met: Option A (cache Clip::thumbnail per creation site) is
   rejected for a new ~100-line async subsystem plus a two-field memo, a defensible call
   (:151), but the plan never quantifies Option A's actual diff size (about 8 sites named
   qualitatively) to let a reader weigh the two costs numerically.

## Strongest counterargument, and why I still hold it
The plan is unusually disciplined about this exact kind of skepticism: it names its own residual
holds (R-4a-d), makes row 7m's inconclusive case a FAIL never a false PASS (:397-399), runs teeth
for the store (t1-t5, :544-547) and for the H1 hold-measurement itself (:364), and keeps the
temporary heartbeat protocol (T1/T2, never committed) separate from the narrower permanent metric
(holdMs) so a diagnostic technique never becomes a mis-scoped gate. That is why every finding
above is SHOULD, not MUST: the gaps are in what the plan asserts about its gate's coverage, not
in whether the gate goes RED and back GREEN for the two causes it actually targets.
