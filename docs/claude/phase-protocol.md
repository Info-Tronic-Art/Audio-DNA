# "Kick off phase N" Protocol (v2)

> Moved verbatim from CLAUDE.md (s-rta-0927) to keep CLAUDE.md under its 25,000-byte cap. All phases P1-P25 are
> complete; this is the procedure to follow if Boris says "kick off phase N".

When the user says **"kick off phase N"**, follow this exact sequence:

1. **Read** these files in order:
   - `CLAUDE.md` (this file) — sacred rules, project context
   - `PHASE_GUIDE.md` — find Phase N, read its specific instructions
   - **For phases 1-12**: `ARCHITECTURE_V2.md` + `TASKPLAN_V2.md`
   - **For phases 13-25**: `research/UNIFIED_BUILD_PLAN.md` — this is the ONLY file needed. It contains all tasks, all files to read, the 3-step shader process, validation criteria, and references to detailed GLSL specs in `research/resolumeEffectSourceIntegration.md` and `research/archaosEffectSourceIntegration.md`.

2. **Read** all source files listed in the phase guide for that phase before changing anything

3. **Execute ALL tasks** in the phase without stopping between tasks. Batch everything.

4. **Self-validate** after completing all tasks:
   - Build: `cmake --build build --config Release` exits 0
   - Tests: all existing + new tests pass
   - Grep: no RT violations (no `new`/`malloc` in audio callback or analysis steady-state, no `std::mutex` on hot paths)
   - **Shader verification (MANDATORY if sources/effects/shaders changed)**: Follow the 4-tier system in `tests/visual/SHADER_VERIFICATION.md`:
     - **Tier 1 — Sources**: `AUDIODNA_NO_SPAWN=1 pytest tests/visual/test_sources.py -v` — auto-discovers ALL sources, sweeps every param, checks non-black + has-effect + no-discontinuity
     - **Tier 1 — Effects**: `AUDIODNA_NO_SPAWN=1 pytest tests/visual/test_effects.py -v` — auto-discovers ALL 112 effects, verifies each param changes output
     - **Tier 1 — Audio**: `AUDIODNA_NO_SPAWN=1 pytest tests/visual/test_audio_reactivity.py -v` — verifies injected audio features change source/effect output
     - **Tier 1 — Time**: `AUDIODNA_NO_SPAWN=1 pytest tests/visual/test_time_sweep.py -v` — verifies animated sources change over time
     - **Tier 1 — Performance**: `AUDIODNA_NO_SPAWN=1 pytest tests/visual/test_performance.py -v` — verifies render time within budget
     - **Tier 2**: `AUDIODNA_NO_SPAWN=1 pytest tests/visual/test_range_quality.py -v` — 11-position sweep, CSV reports, 70%+ useful range, no dead zones
     - **Tier 3**: Open `tests/visual/shader_preview.html` in browser, move every slider end-to-end
     - **Tier 4**: User tests in the actual app
     - Fix ALL Tier 1 failures before reporting. Tier 2 for tuning. Tier 3 is Claude's visual check. Tier 4 is user's.
   - **Quick run all visual tests**: `AUDIODNA_NO_SPAWN=1 pytest tests/visual/ -v --ignore=tests/visual/test_range_quality.py` (range quality is slow, run separately for tuning)
   - Phase-specific checks listed in PHASE_GUIDE.md

5. **Decision point — does this phase have UI changes?**
   - **NO UI changes** (P1, P3): Commit to git, update PHASE_GUIDE.md status to COMPLETE, report done. User does NOT need to validate.
   - **YES UI changes** (P2, P4-P20): Report to user with:

     ```text
     ## Phase N Complete
     **What changed**: <summary>
     **What to look for**: <specific UI elements to verify by launching the app>
     ```

     Wait for user to confirm.

6. **On human PASS**: Commit to git, update PHASE_GUIDE.md status to COMPLETE, then run Step 8.
7. **On human FAIL**: Fix the issue, rebuild, re-validate, report again. After final PASS, run Step 8.

8. **Post-phase documentation (MANDATORY after every phase)**:
   - Update `CLAUDE.md`:
     - Effect/source counts if changed
     - Any new architectural patterns, rendering pipeline changes, or data model changes
     - Add new entries to `docs/claude/pitfalls.md` (and its one-line index in CLAUDE.md) if bugs were discovered and fixed
     - Add new entries to "UI Patterns" if new interaction conventions were established
   - Update `research/UNIFIED_BUILD_PLAN.md`: mark phase COMPLETE with summary of what shipped
   - Update `PHASE_GUIDE.md`: mark phase COMPLETE
   - Write a project memory file summarizing what was built and any non-obvious lessons
   - Write feedback memory files for any user preferences discovered during testing
   - **Ask**: "Did we learn anything this phase that should change how future phases work?" If yes, update the relevant docs. If Claude identified patterns (common bug classes, UI conventions the user validated, architectural shortcuts), capture them proactively.

