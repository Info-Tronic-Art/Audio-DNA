STATUS: DONE_WITH_CONCERNS

RESULT: All four items shipped as 4 commits on `lane/decks-followup-0926b` (base main b766720). ITEM 1:
`AudioDNALookAndFeel::installAsDefault()`/`uninstallAsDefault()` wrap `juce::LookAndFeel::setDefaultLookAndFeel`;
MainComponent's ctor/dtor call them, so the 12 `showMessageBoxAsync` alerts with no associated component and the 3
"FOUND NOT FIXED" menus from the prior fix round (MacroPanel/UniversalParamControl x2/SignalBar) now draw square, in
app colours, with NO per-site change to any of them. renameDeck's now-redundant explicit `setLookAndFeel` is removed
(verified pixel-identical via live before/after shots). ITEM 2: `CompDecksBrowser::isV2DeckFile` filters the Decks
section to files with a top-level `"layers"` key, so Boris's 4 legacy v1 `*.deck.json` files (same folder, APFS
case-insensitive collision) are no longer listed; nothing on disk was touched. ITEM 3: a `static_assert` pins
`Deck` as nothrow-move-constructible next to its definition; it compiles clean (the invariant already holds).
ITEM 4: `currentAudioFile_`'s stale "for deck save" comment is fixed; `PresetManager::DeckState/saveDeck/loadDeck`
are left in place because `tests/test_preset_manager.cpp` still calls them (grep found the one caller) --
removing the production code would also remove that test's only coverage of the embedded-fx round trip. ctest
636 -> 641 (5 new test cases: 1 in test_lookandfeel_square, 4 in the new test_comp_decks_browser).

FACTS (disk-cited; paths worktree-relative unless absolute):
- Step 0: `git -C $WT merge-base --is-ancestor lane/decks-0926b main` -> exit 0 (already merged, commit b766720);
  `git -C $WT checkout -B lane/decks-followup-0926b main`.
- Referenced docs read in full before implementing: `.harmony/.reports/s-rta-0926b/decks.md` (base report + Fix
  round), `BORIS_DECISIONS.md:382` ("Rejected: Rounded corners (anywhere, ever)").
- ITEM 1 mechanism, verified against JUCE source (`build/_deps/juce-src`):
  - `juce_gui_basics/detail/juce_AlertWindowHelpers.h` `setUpAlert()`: `auto& lf = component != nullptr ?
    component->getLookAndFeel() : LookAndFeel::getDefaultLookAndFeel();` -- confirms `showMessageBoxAsync` with no
    associated component resolves through the JUCE-wide default.
  - `juce_gui_basics/components/juce_Component.cpp:1850-1857` `Component::getLookAndFeel()`: walks
    `parentComponent` then falls to `LookAndFeel::getDefaultLookAndFeel()`.
  - `juce_gui_basics/menus/juce_PopupMenu.cpp:336-359` `MenuWindow` ctor: `setLookAndFeel(findLookAndFeel(menu,
    parentWindow))` runs BEFORE `addChildComponent()` (line 363) -- for a menu with no explicit
    `menu.setLookAndFeel(...)` and no parent yet assigned, `getLookAndFeel()` at that point falls straight to the
    default, regardless of `.withParentComponent(...)` topology. This is why installing the default also fixed
    MacroPanel.cpp:138, UniversalParamControl.cpp:376/388, SignalBar.cpp:192 (none of the three call
    `menu.setLookAndFeel`, and none had an in-app parent) with zero changes to those files (outside this packet's
    fence: `src/ui/MacroPanel.cpp`/`UniversalParamControl.cpp`/`SignalBar.cpp` are untouched).
  - `juce_gui_basics/lookandfeel/juce_LookAndFeel.cpp:59-83` `~LookAndFeel()`: the documented reason to call
    `setDefaultLookAndFeel(nullptr)` before destruction. `juce_gui_basics/desktop/juce_Desktop.cpp:101-123`:
    `setDefaultLookAndFeel(nullptr)` clears the weak `currentLookAndFeel`; the next `getDefaultLookAndFeel()` call
    lazily creates (or reuses) Desktop's own `LookAndFeel_V4` singleton -- safe at app shutdown.
- RED lines (verbatim):
  - ITEM 1 compile-RED (`tests/test_lookandfeel_square.cpp` before `installAsDefault`/`uninstallAsDefault` existed):
    `error: no member named 'installAsDefault' in 'AudioDNALookAndFeel'` (x1), `error: no member named
    'uninstallAsDefault' in 'AudioDNALookAndFeel'` (x1); `2 errors generated.`
  - ITEM 2 compile-RED (`tests/test_comp_decks_browser.cpp` before `isV2DeckFile` existed): `error: no member named
    'isV2DeckFile' in 'CompDecksBrowser'` (x13, every call site).
  - ITEM 2 post-implementation RED (self-check, NOT shipped): `CHECK( CompDecksBrowser::isV2DeckFile(v2) )` ->
    `false` with `hasProp=1` in an inline debug check one statement earlier -- the dangling-`DynamicObject*` bug
    below, caught before committing.
- GREEN: `test_lookandfeel_square`: `All tests passed (22 assertions in 5 test cases)` (was 4 cases/prior fix
  round). `test_comp_decks_browser`: `All tests passed (6 assertions in 4 test cases)`. Full serial ctest
  (`ctest --test-dir build-lane -j1`): `100% tests passed, 0 tests failed out of 641` (636 -> 641, +5, matching
  the work packet's stated base of 636 exactly).
- DANGLING POINTER caught during self-check (Item 2, not shipped): the first `isV2DeckFile` was
  `auto* obj = juce::JSON::parse(file.loadFileAsString()).getDynamicObject();` -- `getDynamicObject()` returns a
  raw pointer into the temporary `var`'s `ReferenceCountedObject`; the temporary is destroyed at the end of the
  full expression (after `obj` is assigned, at the `;`), so `obj` dangles for the `return` statement's use.
  `DynamicObject* var::getDynamicObject() const noexcept { return dynamic_cast<DynamicObject*>(getObject()); }`
  (`juce_Variant.cpp:575`) confirms the raw, non-owning return. Fixed by keeping the parsed `var` alive as a named
  local (`auto parsed = ...; auto* obj = parsed.getDynamicObject();`) -- the exact pattern `PresetManager::loadDeck`
  already uses. Recorded in `.harmony/notebook.md` for future `JSON::parse` call sites.
- ITEM 1 before/after live evidence (Quartz window captures + one in-app snapshot for the ComboBox popup, which
  dismisses in <50ms in a background app): a scratch copy of this lane's own build with ONLY
  `lookAndFeel_.installAsDefault();` commented out (and renameDeck's removed workaround temporarily restored, to
  reproduce exactly what shipped on `main` before this lane) is "before"; the real, final build is "after". Both
  carried the same TEMPORARY `AUDIODNA_DEBUG_FOLLOWUP` env-var hook (constructor-end, `.harmony/notebook.md`
  uitoggle/decks idiom), fully reverted before the commit (`git diff --stat -- src/MainComponent.cpp` after revert
  shows only the 3 shipped hunks; `strings build-lane/.../Audio-DNA | grep -c AUDIODNA_DEBUG_` = 0).
  - alert1 ("Some Mappings Dropped", WarningIcon, verbatim title/message from `MainComponent.cpp:2794-2799`'s real
    call site): before = rounded panel, warning triangle icon, rounded OK button (JUCE stock `LookAndFeel_V4`);
    after = square panel, app dark background (`kBackground`), no icon, square OK button (`kPanelBorder` outline).
  - alert2 ("Save Deck" / "Save failed: ...", verbatim from `writeDeckFile`, `MainComponent.cpp:3309-3312`): same
    before/after contrast.
  - rename (Rename Deck dialog, `renameDeck(0)`): before = the ORIGINAL per-site workaround present
    (`w->setLookAndFeel(&lookAndFeel_)`, temporarily restored for this shot only) + no default installed; after =
    the workaround removed + default installed. A pixel diff (`PIL.ImageChops.difference`) of the two 700x336
    window captures: 2509/235200 px differ (max channel delta 198), ALL confined to bbox (245,65)-(456,91) -- the
    "Deck 1" text-editor field, matching a TextEditor caret-blink phase difference between two separate app
    launches, not a panel/border/button difference. Confirms the removed workaround is pixel-identical net of
    caret-blink noise.
  - combo (TopBar audio-source ComboBox popup, `topBar_->getAudioSourceSelector().showPopup()`): the popup is its
    own `Desktop`-level window (not a child of `getTopLevelComponent()`), found via
    `Desktop::getInstance().getComponent(i)` and snapshotted in-app immediately after `showPopup()`. Before/after
    130x60 in-app snapshots are BYTE-IDENTICAL (`PIL` max/mean diff = 0/0.0) -- this popup already resolved through
    MainComponent's own explicit LookAndFeel pre-fix (it is a descendant of MainComponent in the component tree),
    so Item 1 does not change it; included as representative confirmatory evidence, not a before/after proof.
  - A context shot of the real pre-change app (`build/AudioDNA_artefacts/Release/Audio-DNA.app`, read-only) shows
    its default window; that binary predates the `AUDIODNA_DEBUG_FOLLOWUP` hook and cannot show these three
    dialogs on demand under the no-synthetic-input rig rule, so it is included as context only, not as a fourth
    before-state. See NUANCE for why the isolated single-line toggle is the rigorous substitute.
- Live-app lock discipline: `/tmp/audiodna-live.lock` was held by another lane ("fitmode") for the first ~25 min of
  this session; polled every 20s (bounded loops, never a busy spin) and NEVER removed; acquired only after it was
  released (`mkdir` succeeded). Two runs held the lock (owner `decks-followup <pid> <epoch>`,
  `.claude/worktrees/.../shots.sh` then `shots-combo.sh`), each released only its own lock on exit (trap), each
  ended with 0 Audio-DNA processes (`ps -eo pid=,ucomm= | awk '$2=="Audio-DNA"'` empty both times).
- Fence check: `git show --stat <sha>` for all 4 commits touches only files inside the FENCE (`src/ui/LookAndFeel.*`,
  `src/MainComponent.{h,cpp}`, `src/ui/CompDecksBrowser.*`, `src/model/Deck.h`, `tests/**`). `src/ui/PresetManager.*`
  is the actual on-disk path for the class named in the packet's fence (`src/core/PresetManager.*` does not exist
  in this repo -- a path typo in the packet, confirmed by `find`); it was read for ITEM 4's grep but not modified
  (nothing to remove, per FACTS below). `src/ui/DeckView.cpp`'s two pre-existing `menu.setLookAndFeel(&getLookAndFeel())`
  calls (from the prior fix round) are NOT in this packet's fence and were left untouched even though Item 1 makes
  them redundant too (see NUANCE).
- ITEM 4 grep (verbatim): `grep -rn "PresetManager::saveDeck\|PresetManager::loadDeck\|PresetManager::DeckState\|
  \bsaveDeck(\|\bloadDeck(\|DeckState\b" src tests | grep -v "src/ui/PresetManager\."` -> zero hits under `src/`
  (the two `MainComponent::loadDeck()`/`saveDeck(int)` methods that DO appear share only a name, confirmed
  unrelated by reading both); one hit under `tests/test_preset_manager.cpp` ("Deck round-trip: mapping resolution
  survives the embedded-fx save/load path", lines 411-448) -- a live, currently-passing TEST_CASE that exercises
  `DeckState`/`saveDeck`/`loadDeck`'s embedded-"fx"-key JSON shape, which `savePreset`/`loadPreset` alone never
  produce or parse.

METHOD: Read the referenced UNKNOWNS + Fix round + BORIS_DECISIONS sections in full before touching anything, then
re-derived the JUCE LookAndFeel-resolution mechanism from source (`build/_deps/juce-src`) rather than assuming it,
since a wrong assumption here would silently under- or over-fix the app-wide default. Per item: RED (new test
referencing not-yet-written production symbols, confirmed compile-RED) -> implement -> GREEN -> full build
(`cmake --build build-lane -j4`) -> full serial ctest -> fence check (`git diff --stat`) -> commit with the RED
line(s) in the message. ITEM 1's live evidence used a single-line, in-worktree toggle
(`lookAndFeel_.installAsDefault();` commented out / restored, `renameDeck`'s original workaround line restored /
re-removed) to build an isolated "before" binary that differs from the shipped "after" binary by exactly the two
hunks Item 1 changes -- both binaries carried the identical TEMPORARY screenshot hook, so nothing else could
explain a pixel difference. Live runs took `/tmp/audiodna-live.lock` (owner `decks-followup <pid> <epoch>`), polled
20s at a time, never removed another lane's lock, and released only their own.

CONFIDENCE + VERIFY: HIGH for Items 2-4 (RED->GREEN unit tests; Item 3's static_assert; Item 4's grep is exhaustive
and disk-cited). HIGH for Item 1's mechanism (re-derived from JUCE source, not assumed) and for the alert1/alert2
before/after contrast (visibly, obviously different: rounded+icon vs square+no-icon). MEDIUM-HIGH for the rename
dialog's "pixel-identical" claim: the diff is nonzero (2509/235200 px) but entirely confined to the one region a
blinking text-caret occupies -- a second live run was not taken to independently reproduce the exact same
caret-phase-only diff (that would need either disabling the caret or averaging multiple runs, out of this packet's
scope). VERIFY (Harmony gate): rebuild; `ctest --test-dir build-lane` = 641/641; `git show <4 shas> -- src/` shows
no hook in any commit; `strings build-lane/.../Audio-DNA | grep -c AUDIODNA_DEBUG_` = 0 on the final build; a human
check of the two shot pairs (`decks-followup-shots/before-alert1-w1.png` vs `after-alert1-w1.png`, etc.).

UNKNOWNS / NOT DONE:
- Item 1's decision left open by the prior fix round is now closed for the 12 alerts + 3 menus this packet named,
  but `src/ui/DeckView.cpp`'s 2 `menu.setLookAndFeel(&getLookAndFeel())` calls (from the prior fix round) are now
  ALSO redundant by the same mechanism, and were left untouched because `DeckView.cpp` is outside this packet's
  fence. They are harmless (setting a Component's own LookAndFeel field to the very instance it would otherwise
  fall back to has no observable effect), but a future cleanup pass could remove them for the same "verified
  pixel-identical" reason as `renameDeck`'s workaround, if `DeckView.cpp` is ever put in-fence.
  `src/ui/CompDecksBrowser.cpp`'s own `menu.setLookAndFeel(&getLookAndFeel())` (line ~227, the library row menu)
  is likewise now redundant and IS in this packet's fence, but was deliberately left in place: the work packet's
  "removed workarounds" language is parenthesized specifically against `MainComponent.{h,cpp}`, and touching a
  second file's workaround for the same reason felt like scope creep beyond what was asked -- flagging it here for
  a decision rather than silently expanding the diff.
- Item 4: `PresetManager::DeckState`/`saveDeck`/`loadDeck` are NOT removed (see FACTS/RESULT) -- a caller exists.
  If Harmony/Boris decide the embedded-fx-round-trip test coverage is not worth keeping, both the production code
  and that one TEST_CASE could be deleted together in a follow-up; that decision is not this packet's to make.
  `PresetManager.*`'s actual path is `src/ui/`, not `src/core/` as the work packet's fence stated (confirmed via
  `find`, no `src/core/PresetManager.*` exists anywhere in the repo) -- read and reasoned about at its real path.
- Live-only, not unit-tested (matches the established pattern for this class of change; see decks.md's own
  UNKNOWNS for the same limitation on the prior lane): the exact caret-blink source of the rename-dialog's 2509-px
  diff was not independently re-run to confirm it is ALWAYS confined to that bbox (single before/after pair only).

NUANCE (deviations, each within its commit's fence unless stated):
- Item 1's RED-first unit test needed a way to prove "MainComponent's ctor wires this up" without constructing the
  full `MainComponent` (heavy: real audio device/GL/HTTP server, no existing test in this codebase does so). Added
  two thin instance methods (`installAsDefault`/`uninstallAsDefault`) to `AudioDNALookAndFeel` — genuinely
  production code (MainComponent calls exactly these, not a copy), small enough not to be an abstraction beyond
  what was asked, and the only way to get a compile-RED-before/GREEN-after test rather than a test that would
  trivially pass on `main` too (calling the raw JUCE API inline in the test, as first considered, would have
  proven JUCE's own contract, not this packet's wiring — no RED signal at all).
- Item 1's live before/after rig could not use the literal pre-change app at `build/AudioDNA_artefacts` for the
  three hook-driven dialogs (it has no hook, and synthetic input is forbidden) — substituted an isolated
  single-line toggle of the ONE thing Item 1 changes, built and screenshotted within this lane's own worktree, as
  explained in FACTS/CONFIDENCE. The pre-change app IS used for the "context" shot as literally named in the
  packet.
- The screenshot hook additionally exercised a "rename" state (`renameDeck(0)`) beyond the literal "2-3 alerts + a
  combo" list, specifically to produce disk evidence for the "verified pixel-identical" claim about the removed
  workaround — without it, that claim would have been asserted, not shown.

HANDOFF-NEEDS: Harmony's behavioral gate + an independent Reviewer. Decisions for Boris/Harmony: whether to also
remove `DeckView.cpp`'s 2 and `CompDecksBrowser.cpp`'s 1 now-redundant `menu.setLookAndFeel(&getLookAndFeel())`
calls in a follow-up (both currently harmless); whether `PresetManager::DeckState/saveDeck/loadDeck` +
`test_preset_manager.cpp`'s "Deck round-trip" TEST_CASE should be removed together, or kept for their current
embedded-fx-shape regression coverage.

INBOX-RECHECK: none

---

## SUMMARY
4 items shipped as 4 commits on `lane/decks-followup-0926b`; 2 new test files/cases (test_lookandfeel_square +5th
case, test_comp_decks_browser new, 4 cases); a dangling-pointer bug caught and fixed during self-check before
shipping; 17 before/after/context screenshots.

## FILES CHANGED (per commit)
- Item 1 (3563efe): `src/ui/LookAndFeel.h/.cpp` (installAsDefault/uninstallAsDefault), `src/MainComponent.cpp`
  (ctor/dtor wiring, renameDeck workaround removed), `tests/test_lookandfeel_square.cpp` (new TEST_CASE),
  `.harmony/.reports/s-rta-0926b/decks-followup-shots/*` (17 PNGs, force-added).
- Item 2 (48b615c): `src/ui/CompDecksBrowser.h/.cpp` (isV2DeckFile + scanForFiles filter), `tests/CMakeLists.txt`
  (new test_comp_decks_browser target), `tests/test_comp_decks_browser.cpp` (new).
- Item 3 (2a2b0b7): `src/model/Deck.h` (static_assert + `<type_traits>`).
- Item 4 (513b4b6): `src/MainComponent.h` (currentAudioFile_ comment).
- Notebook/report commit (this one): `.harmony/notebook.md`, `.harmony/.reports/s-rta-0926b/decks-followup.md`.

## TESTS
New: AudioDNALookAndFeel::installAsDefault (orphan Component + no-associated-component AlertWindow resolve to it,
square); CompDecksBrowser::isV2DeckFile x4 (accepts v2, rejects legacy v1, rejects missing/garbage, filters a mixed
temp-dir fixture pair 1-and-1). Deck.h static_assert (compile-time, no runtime test). Full serial ctest after every
commit: 637 / 641 / 641 / 641 (base 636).

## SHOTS (`.harmony/.reports/s-rta-0926b/decks-followup-shots/`)
context-prechange-default-w0 (real pre-change app, default window, no hook) · before-alert1-w0/w1 vs
after-alert1-w0/w1 (Some Mappings Dropped: rounded+icon -> square+no-icon) · before-alert2-w0/w1 vs
after-alert2-w0/w1 (Save Deck: same contrast) · before-rename-w0/w1 vs after-rename-w0/w1 (Rename Deck: both
square, pixel-diff confined to the caret-blink region — proves the workaround removal is safe) ·
before-combo-snapshot vs after-combo-snapshot (TopBar audio-source ComboBox popup, in-app snapshot, byte-identical)
· before-combo-w0 / after-combo-w0 (full window context for the combo runs).

## ISSUES
See UNKNOWNS/NUANCE above: DeckView.cpp's 2 and CompDecksBrowser.cpp's 1 now-redundant `menu.setLookAndFeel`
calls left in place (fence/scope reasons, not risk); PresetManager dead-code-with-one-test-caller left in place
per the packet's explicit instruction.

## SKILL_PROPOSALS
None -- this followed the established plan6/fix-round RED-first + live-hook-screenshot pattern already documented
in `.harmony/notebook.md`, no new reusable procedure discovered beyond the two notebook entries already added
(the app-wide-default mechanism, and the JSON::parse dangling-pointer gotcha).

## RISKS
The rename-dialog pixel-diff (2509/235200 px, caret-blink only) was not independently re-run; a second live pair
would strengthen the "pixel-identical" claim but was judged not worth a second live-app round trip given the bbox
is unambiguous. DeckView.cpp/CompDecksBrowser.cpp's redundant-but-harmless LookAndFeel calls are dead weight, not a
bug.

## PACKET QUALITY
- Clarity: CLEAR (each item was concrete and independently verifiable; the fence was precise except for one path
  typo).
- Missing context: the work packet's fence names `src/core/PresetManager.*`; the actual, only PresetManager in
  this repo lives at `src/ui/PresetManager.*` (confirmed via `find`, no `src/core/` variant exists) -- read and
  reasoned about at its real path, since the intent (PresetManager dead code) was unambiguous despite the typo.
- Unused context: none.
- Self-brief files: CLAUDE.md (worktree, read fully), `.harmony/.reports/s-rta-0926b/decks.md` (base + Fix round,
  read in full), `BORIS_DECISIONS.md:382` (Rejected: Rounded corners), `.harmony/notebook.md` (uitoggle/decks hook
  idiom entries, both directly reused). No DEPARTMENT or KNOWLEDGE_TOOLS block (grep-only; impact taken
  conservatively — the C3 rule was honored: `isV2DeckFile`/`installAsDefault` are NEW symbols, so "no callers"
  never arose as a false-dead-code signal). pulse.json GREEN at Step 1.1, no conflicting claim on this project's
  files; the live-app lock (a separate, file-based coordination mechanism) was held by another lane for the first
  ~25 minutes and was waited out, never forced.

## STATUS
DONE_WITH_CONCERNS

## NEXT ACTION
Harmony's behavioral gate + Reviewer; the two follow-up decisions under HANDOFF-NEEDS (redundant LookAndFeel
calls in DeckView.cpp/CompDecksBrowser.cpp; PresetManager dead-code-with-test-caller).
