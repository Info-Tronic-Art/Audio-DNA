# Ghost Feature Decisions — 2026-05-19

Decided during Session 1 design audit with Boris.

| # | Feature | Decision | Notes |
|---|---------|----------|-------|
| 1 | MappingSuggester | **INCLUDE** | AI mapping suggestions. Expose as button in Mapping Editor |
| 2 | Per-scope MacroBanks | **INCLUDE** | Wire up Clip + Layer scopes (currently only Global). 24 total knobs |
| 3 | Crossfader | **EXCLUDE** | Boris decided not to have this feature |
| 4 | LUT Loader | **INCLUDE** | Expose as global post-process effect |
| 5 | Camera Input | **INCLUDE** | Enable live camera as clip source |
| 6 | Syphon Input | **DEFER** | Future build, not needed now |
| 7 | Chained Signals | **INCLUDE as MACRO CONDITIONS** | NOT a separate chain UI. Add Gate + ScaleRange conditions to existing macro source picker. See design/MACRO_CONDITIONS_PLAN.md |

## Macro Conditions Design Decision

Instead of building a separate chain/modulation panel, extend the existing MacroBank/MacroPanel:

- Each macro knob gets an optional **Condition** (Gate or ScaleRange)
- Condition = another signal + threshold/range that controls WHEN/HOW the macro's source signal passes through
- Gate: signal passes only when condition signal > threshold
- ScaleRange: condition signal controls the output range of the macro
- Multiply and Add modes deferred to v2
- This keeps chains INSIDE the macro UI — no separate routing panel
- Per-Clip + Per-Layer macros must be wired up first (decision #2)
- The macro dashboard becomes the performance intelligence interface
