# plan-sampleat (s-rta-0928b START HERE item 4) — execute an ALREADY-AUTHORED recipe

Source plan: `.harmony/.reports/s-rta-0928/plan-tempo.md` §8 "TempoMap::sampleAt -- FILED, not fixed in this lane"
(lines 770-804) and its HARMONY ADOPTION bullet A7; the tempo lane's report `.harmony/.reports/s-rta-0928/tempo.md`
found_not_fixed 1 (recipe + RED). Read both in full before touching code. Law #11 cost gate: executing an authored plan
with a deterministic ctest gate -> no new Fable plan (row 3 execution).

## HARMONY ADOPTION (s-rta-0928b) — OVERRIDES THE SOURCES WHERE THEY DIFFER
- A1 Execute tempo.md found_not_fixed 1 as written: `compile()` passes the resolved asset rate into `convertBeatX`;
  `sampleAt(t, nominalRate)` uses a segment's own delivered-sample slope only when that segment spans >= 1 s of t, and
  `nominalRate` otherwise; a single-anchor map uses `nominalRate` (never rate 0).
- A2 RED first, on the base tree, verbatim lines in the report: a one-anchor map {t 0, sample 150016, bpm 120} plus a
  stampless gesture at beats {0, 4, 8} compiled on DriveClock::Sample must give x = {150016, 246016, 342016} (48 kHz
  nominal); today all three are 150016.
- A3 A second RED case gives the >= 1 s rule teeth: a two-anchor map whose last segment is the diagnosis's 12 ms
  start/lock pair (today ~85,050 samples/s extrapolated) must extrapolate at the nominal rate.
- A4 Keep a case where a long (>= 1 s) segment's own slope IS used (so the rule is not "always nominal").
- A5 Update `tests/test_program_stamps.cpp:31-35`, which documents rate 0.
- A6 Verify in source (cite file:line) the claim "Program.cpp's Sample-clock fallback is the only caller and no take the
  app writes reaches it"; if false, STOP and report BLOCKED with the path that reaches it.
- A7 No live probe (A6 true -> nothing observable live). Full serial ctest at the end (`ctest --test-dir <wt>/build-lane -j1`).
- A8 Docs: if docs/claude/recording.md describes TempoMap / Program's stampless Sample-clock fallback, correct it in one
  line; CLAUDE.md untouched. No pitfall.
