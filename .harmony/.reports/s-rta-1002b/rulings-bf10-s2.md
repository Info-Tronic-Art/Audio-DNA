# Harmony rulings on bf10 stage S2 STOPs (s-rta-1002b, 2026-10-02 19:14:33) — binding for the S2 finish + S3
STOP 1 (T5 (b) zero-GL-error clause fails 5/5): ACCEPT the builder's recommendation. Evidence: projectm_create and the first
  load_preset_file raise GL_INVALID_ENUM (0x500) inside libprojectM 4.1.1 in BOTH the stock and the patched build
  (bf10-s2/errprobe.log) — library-internal, not our render path. Rule: on the INIT variant only, drain and tolerate exactly
  {0x500} raised inside projectm_create + the FIRST preset load (assert the drained set == {0x500} or empty); every other
  code, and ANY error after the first rendered frame, still FAILS. Record the drained codes as INFO.
STOP 2 (T6 mid-transition median clause passes 2/5 — projectM picks a random transition shader; a wipe never has a median
  between A and B): REPLACE the clause, not the bar values. New clause (covers cross-fade AND wipe): during the blend, at
  least one sampled read has (>= 5 % of pixels within 6 of A AND >= 5 % within 6 of B) OR (a median >= 10 from both A
  and B); AND the last read of the window has near-B >= 95 %. Re-run T6 >= 10 times: 10/10 PASS required.
FOUND 1 (presets without a comp shader write their own alpha < 1 into the FBO; bf10_circle alpha255 2.93 % / 9.16 %):
  Amendment 10 fix (i) ("a cheap opacity clear only if a test sees otherwise") NOW FIRES — a test saw it. Rule: after the
  library renders into outputFBO_, force alpha to 1 (glColorMask(false,false,false,true) + clear alpha 1 + restore the
  mask, inside the saved / restored GL state), and ASSERT in T4: bf10_circle alpha255 == 100 % at both sizes (RED on
  ab5cf4b). MilkDrop is a full-frame generator; an opaque output is what a layer expects.
Pitfall number for this lane: 66 (64 mkvidx, 65 ui).
