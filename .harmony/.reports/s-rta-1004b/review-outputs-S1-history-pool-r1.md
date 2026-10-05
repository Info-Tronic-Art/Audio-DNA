# Reviewer Verdict -- outputs S1 history-pool lens, round 1
STATUS: DONE
VERDICT: PASS_WITH_NITS (REVIEW VERDICT: APPROVE; 0 blocking, 3 SHOULD, 3 NIT)
REVIEWED: lane/outputs-core @ a364bad (code head 0f4c73c) vs 8b464a6, read through git objects only; nothing built or run.
FILES: src/output/FrameHistory.h, OutputLook.h, SharedFrameSet.h (pool part), SurfacePool.cpp; tests/test_frame_history.cpp,
 test_output_look.cpp, test_surface_pool.cpp, tests/CMakeLists.txt; the report, mutant runner and log. The diff is 11 files, all
 inside the Owns list plus lane reports. No venv link, no mutant left in the tree, no on-screen text added
 (settingsStateText is the ruled A-12 text). VERIFIED (git diff --stat / --summary).

ISSUES
[SHOULD] frameClockUs() has no owner. A-4 puts "one function output::frameClockUs() ... a pointer tests replace" with the log.
 FrameHistory.h:97 names it, but it exists nowhere (grep of a364bad src + tests: that one comment only). S1 Owns FrameHistory.h and its
 list leaves the clock out; S2's Owns does not list FrameHistory.h. S2 must either edit outside its fence or invent a second home.
 Fix: Harmony decides now: add the function and its test pointer to FrameHistory.h (S1 round 2, three lines), or add FrameHistory.h to
 S2's Owns. VERIFIED (grep).
[SHOULD] U-P2's named RED arm M-P2 (bound = kMaxSlots) cannot bite. SurfacePool.cpp retainSurface keeps `slot >= g->slots || g->s[slot]
 == nullptr`; s[] beyond a generation's count is null (releaseGeneration zeroes it), so M-P2 changes no answer. The builder says so
 (concern 1) and the log shows `M-P2 NOT RED (EQUIVALENT)`. M-P2x (bound = kSlots) turns it RED, so the test is not toothless.
 Fix: Harmony re-states the row's arm as M-P2x in the gate list. Not a MUST: the case fails on a real wrong bound.
 VERIFIED (read SurfacePool.cpp:~185-195, log lines for M-P2 / M-P2x).
[SHOULD] "Exactly its named case RED" is not met by 9 mutants (log: M-H1 +4, M-H2 +3, M-H3 +4, M-H4/H5/H6/H6b +1 = the oracle case,
 M-P5 +U-P3, M-L1 +U-L4, M-P2x +U-P6). The oracle case failing for any pick change is inherent, and the named case does go RED
 each time, so no test is dead. Fix: Harmony reads the gate wording as "the named case is RED". VERIFIED (log).
[NIT] Edges the focus list names that have no case: a serial wrap, a clock that steps back, a publish rate above kSizingHz in the
 pick property tests (only reachMs covers 144), a truly empty log (writeSeq 0). Behaviour by reading: m = 0 gives lo = 1 > m, no loop,
 nothing; a stepped-back clock gives no gap clear (the difference is negative) and pick still returns a defined frame; stamps are int64
 microseconds, no wrap. Serial wrap (FrameHistory.h pick, `frontWordSerial(w1) < minSerial`, about 414 days at 120 a second, D-9): a
 reader whose minSerial is near 2^32 finds no candidate and shows front(). INFERRED for S2: the front() fallback must set the reader's
 floor to front's serial, or that reader keeps the no-delay picture for good. Fix: one line in S2's brief, or one case.
[NIT] A-4's cost line says "at most 15 entries"; pick visits slots - 2 = 16 at 18 slots (FrameHistory.h pick loop over [m - 15, m]; test
 line 127 asserts `reads <= slots - 2`). Harmless; S7's docs should say 16. VERIFIED.
[NIT] SharedFrameSet.h gained an include and a static_assert (lines 22, 61-62) outside "the SurfacePool part" in the strict sense;
 disclosed as concern 7, needed to pin the serial layout. Accept.

SLIM (authoritative over the builder's sweep)
 - EnsureResult::operator bool (SharedFrameSet.h): EXCESS_VESTIGIAL once S2 rewrites SharedFrameSet.cpp:19. DEBT_FILED -- S2 drops it
   (the builder's FOR S2 line says so). JUSTIFIED_KEEP for now, reason="SharedFrameSet.cpp:19 reads ensure() as a bool and is S2's
   file; a plain enum would read Failed as true there".
 - deepSlots, colourAdjusted, OutputLook operator==, Picked::reads, SurfacePool::releaseAll / slotCount / setCreateFn / allocBytes /
   retiredBytes: JUSTIFIED_KEEP reason="each is a named item of A-3 / A-7 / A-13 whose caller is S2 or S5; the ruling stages the
   build". Not under hooks/ or a config surface, so no harness-wiring check applies. No EXCESS_DEAD.

WHAT I CHECKED AND FOUND SOUND (VERIFIED by reading unless noted)
 - A-1 constants and the table: extraSlots 14/14/14/14/11/5; 5120x2880 = 640 MiB / 58,982,400 = 11; 7680x4320 = 5; reachMs 233, 116,
   97, 91, 83, 41 (integer floor of extra x interval); 16384x8192 = 1 fit = 0. kSizingHz 120 is right for M1's 98.96.
 - FrameLog against A-4: 32 entries, atomic word and stamp, seq_cst, writeSeq / floorSeq; noteWrite zeroes the entry BEFORE it
   publishes the index (builder D-2, an improvement on A-4's order: with the index first a reader could accept the pair from 32 writes
   earlier; the second section of U-H6 would catch it); offer = zero, stamp, word; a reader accepts only if the word reads the same
   twice; the window [max(floor, m - (slots - 3)), m] counts writes; newest with stamp <= now - delay, else the OLDEST candidate, else
   nothing. Wrap of the ring: the window of at most 16 is under 32 (static_assert). A clear after a gap puts the just-offered copy
   under the floor, as A-4's "stated consequence" says, and the test asserts it.
 - Torn-read proof in the header re-derived: sound under seq_cst, for serials that never repeat (serial_ is never reset: SharedFrameSet.cpp:58).
 - DepthPolicy: deep at once, shallow on the 600th quiet update, any ask resets the count; matches U-H9 exactly.
 - OutputLook: default is word 0; 9 + 7 + 5 x 8 = 56 bits; clamps on pack and on unpack; isDefault(word) goes through unpack; a 0..250
   Delay fits the 9 bits (question 51 = B needs no re-pack).
 - SurfacePool: no GL anywhere in the pool or the log. No new mutex: the new paths (trimRetired, releaseAll, refreshBytes) take the
   existing mutex on rare paths only; ensure() on the steady path returns Unchanged before any lock or create; tick() adds a loop over
   two ints. Surfaces are still created by the writer on its thread, as today. lastGen_ is its own counter, so a number is not reused
   after releaseAll (U-P6, M-P6 RED). trimRetired keeps the newest retired and keepGen, so the pool owns at most three generations;
   a reader's own CFRetain keeps a trimmed generation alive. The shown slot is never evicted by this stage because nothing calls
   trimRetired / releaseAll yet (grep: no src caller) and the base 4-slot path is byte-for-byte the old arithmetic. Pitfall 37: ensure
   takes (w, h) from its caller; nothing here reads a Component.
 - The one behaviour difference on the existing path: after a failed create the same (w, h, slots) is not retried for 300 ticks
   (5 s at 60 Hz). It is ruled (A-3 / GL-1), and the report names it. Not a finding.
 - Tests drive real code: Feed uses the real FrameLog and packFront; the oracle is an independent brute force; U-H10 runs the real
   stores one at a time through a spy cell and then a reader thread against 1,000,000 writes (a stamp that is a function of the serial);
   the pool cases create real IOSurfaces. Each mutant edit in outputs-S1-mutants.py applies exactly once or the run errors out.
   RED evidence is the raw log (21 of 22 RED, M-P2 recorded as equivalent). I did not re-run it (the task forbids it); Harmony re-witnesses.
 - The mutant runner reads only the scratch dir it is given and the shared Catch2 source; it does rmtree <scratch>/<name> and
   runs clang++ and its own test binary. No kill, no pkill, no app launch, no path under Boris's folders. VERIFIED.
 - Existing pool cases unedited: test_surface_pool.cpp is +242 -0 (builder's numstat; the diff hunk starts after the old last case).
 - Pitfall number: none written (S7). Docs: none touched, as the ruling says.

CONFIDENCE: VERIFIED for every item above marked so (read at a364bad). INFERRED: the unchanged app (no run), the serial-wrap consequence
 in S2, and "the existing 8 shared-frame GL cases stay green" (builder's number, not re-run).
METADATA: reviewer=independent-source-review lens history-pool r1, builder_packet=outputs-S1, date=2026-10-04
