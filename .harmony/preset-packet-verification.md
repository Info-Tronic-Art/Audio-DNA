# PRESET RETARGET PACKET — VERIFICATION BEFORE BUILD (2026-08-04, session c)

**Verdict: GREEN TO BUILD.** `.harmony/.work-packets/preset-retarget-fix.md` is STILL VALID,
zero percent implemented at HEAD (`20fc53d`), and its line numbers land exactly on their
anchors — unusual for this repo, and worth noting as a point in the packet's favour.

Recon did the sweep; Harmony independently re-verified the load-bearing claims below by direct
read. Claims Harmony did NOT personally re-verify are marked [RECON-ONLY].

## Harmony-verified (read directly, quoted)
1. **Save writes a raw INDEX** — `PresetManager.cpp:113-114`
   `mObj->setProperty("targetEffect", static_cast<int>(m->targetEffectId));` (+ targetParam).
2. **Load reads it back unvalidated** — `PresetManager.cpp:212-213`. A stale but in-range index
   silently drives the WRONG effect; only out-of-range is skipped.
3. **Param VALUES restore BY POSITION** — `PresetManager.cpp:186-194`:
   `for (int p = 0; ...) { ... fx->setParamValue(p, val); }`. Names ARE saved (`:95`) and are
   never read back anywhere in the file. Second bug, same disease, confirmed.
4. **Route is genuinely out of scope — upgraded from recon's INFERRED to VERIFIED.**
   `Route::targetEffectIndex` (`src/routing/Route.h:23`) IS consumed against the global chain
   (`src/render/Renderer.cpp:234`), so it is a same-class consumer. BUT a grep of `src/routing/`
   for `save|load|toJson|fromJson|serial|ValueTree` returns **NOTHING** — routes are in-memory
   only and never serialized, so they cannot go stale inside a saved file. Exclusion holds on a
   decisive negative, not on an absence-of-evidence hunch. **Worth one line in the commit
   message; NOT a scope change.**
5. **BindingManager stays out of scope** — recon re-derived this independently of both the
   packet and the prior correction, and found the dormancy argument *structurally stronger*
   than previously stated: every `push_back` in `buildBindableTargets`
   (`MainComponent.cpp:4750-4826`) passes `effectIndex = 0` and no site emits
   `Action::ToggleEffectBypass`. The field is persisted but inert. [RECON-ONLY, but it agrees
   with the prior session's independent falsification — two independent reads now concur.]

## Notable: PresetManager has ZERO test coverage
`grep -rl "PresetManager\|savePreset\|loadPreset" tests/` returns nothing; no preset target in
`tests/CMakeLists.txt`. The packet's fail-first test would be the FIRST coverage this file has
ever had. That raises the value of the commit and lowers the risk of the change (nothing to
regress), but it also means **the 193 baseline proves nothing about this file** — do not treat
a green ctest as evidence the preset path works.

## Corrections a builder MUST receive (do not let these drift into the source)
- **Paths:** everything is under `src/` (`src/ui/PresetManager.cpp`, `src/effects/EffectLibrary.h`).
  The packet uses bare filenames / implies a `Source/` dir. No `Source/` exists.
- `Renderer.cpp:1511` cited as "the re-population guard"; it is `:1511-1512`. Cosmetic.
- The Route finding above belongs in the commit message as a documented exclusion, so the next
  session does not re-derive it a third time.
- **Serialization constraint is RELEASED.** The packet sequences itself behind the black-overlay
  fix; that landed (`5a580c8`). This lane no longer needs to wait.

## Gaps — named, NOT filled
[RECON-ONLY, unverified by Harmony] N1's mechanical audit (0 duplicate names/shaderNames across
135 defs / 333 params), the `git log -S` rename history, and the per-category counts. These
affect the SEVERITY story and the D1 design rationale — **not** whether the bug exists. The
ctest 193 figure is recon's TEST_CASE-count proxy, INFERRED; ctest was not run.
