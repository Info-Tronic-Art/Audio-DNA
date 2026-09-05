# L0-MD FAIL-FIRST — captured 2026-09-04, session s-rta-0904 (secondary)

Captured by HARMONY HERSELF against **unmodified code at HEAD**, before any builder
was dispatched at the lane. Required by the packet, and required by Iron Law #6:
a gate that never saw the failure state cannot prove the flip.

## Tree state at capture
- HEAD: `5ab9010`-line (docs/.harmony only above `3b940a0`); **`src/` unchanged since `3b940a0`**.
- Build: `cmake --build . -j8` → `BUILD_RC=0` (already current, no source drift).
- Baseline ctest: **203/203 passed, 0 failed** — RE-RUN, not inherited (`CTEST_RC=0`).

## Launch
```
open --stdout /tmp/adna-failfirst-out.log --stderr /tmp/adna-failfirst-err.log \
  build/AudioDNA_artefacts/Release/Audio-DNA.app --args --test-mode
```
Health probe on the ONLY valid address (`::1`, never 127.0.0.1):
```
$ curl -s 'http://[::1]:8080/api/health'
{ "status": "ready", "gl_version": "4.1", "fps": 109.09, "effects_count": 135 }
```
8080 bound ⇒ this is a REAL green, not the documented 7070 false-green.

## THE ORACLE — the failure, observed
```
$ grep -ic milkdrop /tmp/adna-failfirst-err.log /tmp/adna-failfirst-out.log
/tmp/adna-failfirst-out.log:0
/tmp/adna-failfirst-err.log:0

$ grep -c 'Eyes' /tmp/adna-failfirst-err.log
1
```
**ZERO `[MilkDrop]` lines. `Eyes` present.** `Eyes` is emitted ~77 lines LATER in the SAME
MainComponent constructor, so the constructor demonstrably ran past the MilkDrop wiring
block and the block was skipped — exactly the `if (pmSource)` root cause. stderr carried
289 lines, so this is not an empty-log artifact.

## Post-fix gate (what must flip)
Same launch, same grep: `grep -ic milkdrop` must become **non-zero**, and the `[MilkDrop]`
line must carry a **non-zero preset count**. Plus ctest re-run 203/203 and a clean Release build.
