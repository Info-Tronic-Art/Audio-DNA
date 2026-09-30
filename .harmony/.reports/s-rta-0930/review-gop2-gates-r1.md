# Reviewer Verdict — gop2 gates r1
STATUS: DONE
VERDICT: APPROVE (PASS_WITH_NITS)
REVIEWED: lane/gop2 head 48dd81b433a3aa67bdcc379f865c9e6012b30306 (base 655d232), via git diff / git show / git archive only
FILES: src/media/GopCache.h, VideoPlayer.{h,cpp}, tests/test_gop_cache.cpp, tests/test_gop_cache_store.cpp, 3 fixtures, .harmony/probe-vupload-ab.{py,sh}, rendering.md, pitfalls.md, testing-eyes.md, CLAUDE.md
VERIFIED (executed):
- RED on base: built the committed store test (keyRels accessor/check stripped) against 655d232 src/media in a $TMPDIR relink rig. T1b a,b,d,e,f FAIL with late 69/56/403/69/69; T1c pause 33 and slew 38 FAIL; T3 (a) 118/120 resident, (b) 169 seeks / 77 misses / 3321 decodes FAIL. Values equal the builder's. T1b(c), T1c speed/shrink/flip, T1d pass on base (guards, as ruled). T1a/predicate do not compile on base (new API).
- T1d tooth: scratch mutant m5 (Lead keys = nullptr) on the 48dd81b src FAILS T1d at 1388 / 1690 decodes (> main's 1213 / 1461), others stay green.
- G2: build-lane-tsan binaries (built after c3): test_gop_cache 148/13, test_video_decode_trace 510/5, test_gop_cache_store 662/26 all pass, 0 TSan warnings.
- G3: --selftest SELFTEST PASS (A13's exact counts incl. exit codes, run from a git-show copy). ab256 replay: new summarizer prints INFO "cap 0.0: pre-lane arm only -- no rule" and five cap-256 lines identical to summary.txt's; 655d232's summarizer prints the six bogus "cap 0.0" FAILs (RED).
- ctest -N (build-lane) = 1055, six new gop2 cases registered, 0 new targets. Test diff is additive only; probe-vupload.json untouched; the .py change is confined to the u8 block: no bar re-thresholded.
- CLAUDE.md blob = 23,996 B (<= 25,000). Docs match ruling DOCS (a)-(e), Pitfall 62 rule-3 text + six guard names, testing-eyes :15.
- Stray: no .venv, no env hook / instrumentation / cerr in the added src lines; all seeks funnel through seekToTimestamp so the landing flags cover every seek path; no audio-thread, mutex, or render-wait change.
ISSUES: 0 blocking / 3 suggestions
- NIT rendering.md: placeholder "[G4's medians, filled in by Harmony]" is still in the committed text; Harmony must fill it (or strike it) after G4.
- NIT T1d is green on main by design; its only tooth is m5 (verified), not a RED-on-base test; accepted by ruling A16.
- NIT probe-vupload-ab.sh: burners sampled only while Audio-DNA runs (builder's A9 refinement; pre-round-1 orphan check as ruled). adna() comes from the external LOCK_LIB, not in-repo; any future probe step that runs ffmpeg while the app is up would taint every launch.
METADATA: reviewer=gates-lens, round=1, date=2026-09-30
