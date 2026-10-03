# Facts: characters in deck names and file names (read-only recon)

Source read: /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/recon (commit a7491d4, lane/bf9b). Paths below are relative to that root unless marked `main:` (c6fbc14, read with `git show`) or `JUCE:` (JUCE 8.0.4, the version pinned at CMakeLists.txt:30; read from /Users/boriskarpman/projects/RealTimeAudio/build/_deps/juce-src/modules/...).
Nothing was built, run or launched. The only side effect was a scratch dir under $TMPDIR (touch/ls only), removed afterwards, see Q4.

Boris, verbatim (boris-feedback-backlog.md line 117): "If there are ‘/‘ characters in a decks file name, do not worry about adding sub folders. They are just characters. If there are characters that are no good, then let me know and we will create a fix together"

## 1 QUESTIONS ANSWERED

### Q1. Every place a deck name or composition name becomes a file name or path

Headline (VERIFIED): there is NO sanitizing anywhere in the deck or composition save path. `juce::File::createLegalFileName` is called exactly once in the whole tree, in the Record panel's take name (src/ui/RecordPanel.cpp:17). Every deck or composition default file name is `dir.getChildFile(name + ".json")`, and `File::getChildFile` treats "/" as a path separator, a leading "/" or "~" as an absolute path, and a leading "../" as "go up" (JUCE: juce_File.cpp:432-481, isAbsolutePath 424-433). That is the "/" bug.

Only two sites turn a deck or composition name into a path. All other sites are not name-based (table below).

**1a. Save Deck As (the known bug)** (VERIFIED)
- Default file name: src/MainComponent.cpp:3699-3702: `FileChooser("Save Deck As...", dir.getChildFile(juce::String(deck.name) + ".json"), "*.json")`. `dir` = `~/Library/Application Support/AudioDNA/decks` (CompDecksBrowser.cpp:328-332), created at :3697.
- Deck "A/B" gives startingFile `.../decks/A/B.json`. JUCE's mac chooser (JUCE: native/juce_FileChooser_mac.mm:144-155) does `startingFile.isDirectory()` (false), then `startingDirectory = parent = .../decks/A` (does not exist) and `filename = "B.json"`, then `setDirectoryURL(.../decks/A)` and `setNameFieldStringValue("B.json")`. So the dialog is asked to open a folder that does not exist ("A") and to propose only the tail. This matches the reported symptom (a sub-folder instead of the whole name). What NSSavePanel shows for a non-existent directoryURL is INFERRED (fallback folder) - see section 4.
- Sanitizing: none. Chooser result handling at MainComponent.cpp:3715-3716: `hasFileExtension(".json") ? file : file.withFileExtension("json")`. Note `withFileExtension` REPLACES everything after the last dot (JUCE: juce_File.cpp:719-733), so a typed "Mix 1.5" (if the panel did not add ".json") becomes "Mix 1.json". Whether the panel appends ".json" first is INFERRED (allowedFileTypes = ["json"] is set at JUCE: mac.mm:107; `setAllowsOtherFileTypes` is not called).
- After a successful write (MainComponent.cpp:3719-3726): `deck.sourceFile = saveFile; deck.name = saveFile.getFileNameWithoutExtension()`. The deck takes the FILE's name. So the invariant "deck name == file base name" is enforced (comment :3721, and :3617 "Save Deck As enforces name == file"). A "/" can never survive a round trip, because a POSIX file name cannot hold "/" (Q4).
- Write: `writeDeckFile` (:3733-3748) = `file.replaceWithText(JSON::toString(deck.toVar()))`. replaceWithText writes a hidden temp file first: `"." + base + "_temp" + <up to 8 hex> + ext` (JUCE: juce_TemporaryFile.cpp:55-69, File.cpp:798-803), then renames. Failure shows the alert "Save failed: <path>" (:3742-3746) unless test mode.
- Edge cases for 1a:

| Input in deck name | What happens at a7491d4 |
|---|---|
| "/" inside (A/B) | startingFile = decks/A/B.json; chooser opens a non-existent folder "A", name field "B.json" (VERIFIED code path; on-screen look INFERRED). |
| leading "/" (/x) | getChildFile returns the ABSOLUTE path `/x.json` (VERIFIED JUCE:isAbsolutePath). The chooser starts in "/" with "x.json". |
| leading "~" | `isAbsolutePath` is true for "~" on mac (VERIFIED JUCE:424-433); the File ctor expands "~/" to home, and "~user" via getpwnam (JUCE:parseAbsolutePath). "~name" with no such user stays a relative path (INFERRED from the code, not run). |
| leading "../" | getChildFile climbs out of decks/ (VERIFIED JUCE:446-476). |
| ":" | Passes through unchanged (File name is a plain string). Legal at POSIX level on this Mac (VERIFIED by touch, Q4). Finder display: INFERRED. Whether NSSavePanel's name field maps ":" is UNKNOWN-NEEDS-A-RUN. |
| leading "." (.x) | File `.x.json`. Legal (VERIFIED by touch). Hidden in Finder and in `ls`. The in-app library still lists it: scan uses `findChildFiles(findFiles, false, "*.json")` with no ignoreHiddenFiles flag (CompDecksBrowser.cpp:287,302; JUCE flag value 4 is separate). Whether the save panel shows hidden files is INFERRED no. |
| trailing space ("x ") | Rename and the box both `.trim()`, so a trailing space cannot be typed (see Q3). A name from a composition JSON could carry one. File `x .json` is legal (VERIFIED by touch). |
| empty | Rename refuses empty (MainComponent.cpp:3768; main:applyDeckRename). A deck name loaded INSIDE a composition JSON is whatever the JSON holds (Deck::fromVar, src/model/Deck.h:141), so "" is possible from a file. Then the default is ".json" (INFERRED; that file is legal, VERIFIED by touch of ".json"). |
| 255+ chars | No check. Limit on this volume: 255 characters, not bytes (VERIFIED by touch, Q4). A name whose `.json` file is 236+ characters fails earlier: the temp file name is `.`+base+`_temp`+8 hex+`.json` = base+19 characters, over 255 for base > 236 (arithmetic, INFERRED, not run). Failure path: alert "Save failed". |
| emoji | Legal and unchanged (VERIFIED by touch, 1 emoji + variation selector). |
| backslash | Legal on POSIX. JUCE has a debug-only `jassert` on a backslash in a raw path (JUCE: File.cpp parseAbsolutePath); release builds do nothing (INFERRED from the `jassert` macro). |

**1b. Save Deck (plain)** (VERIFIED) - src/MainComponent.cpp:3678-3689. Writes straight to `deck.sourceFile` (no chooser) ONLY when sourceFile is set, its folder exists, and `sourceFile.getFileNameWithoutExtension() == deck.name`; otherwise falls through to Save Deck As. A deck named with "/" never matches any file (a file name has no "/"), so Save Deck always opens the chooser for it. A Rename also breaks the link (comment :3674-3677).

**1c. Save Composition / Save Composition As** (VERIFIED)
- Default file name: MainComponent.cpp:3512-3515: `dir.getChildFile(juce::String(composition_.name) + ".json")`, `dir` = `.../AudioDNA/compositions` (CompDecksBrowser.cpp:322-326). Same `getChildFile` behaviour as 1a. Result handling :3526-3527 same as 1a.
- Plain Save (:3464-3488): if `composition_.filePath` is set and its folder exists, `saveToFile(filePath)` straight; else Save As.
- `composition_.name` can only come from: a file's base name on Load/Save As (:3454, :3499), "Untitled" (Composition.h:44, :202), or the JSON `name` (Composition.h:876), which load then overrides with the file base name (:3454). There is no UI to type a composition name (grep: the only assignments are those). So a "/" cannot normally reach it; ":" and the others can (they come from an existing file name).
- Collect Media (MainComponent.cpp:6553-6597): `chosenFolder.getChildFile(composition_.name + "_media")` and `.getParentDirectory().getChildFile(composition_.name + ".json")`. Same chars rule as above; no sanitizing.

**1d. POST /api/debug/save_composition** (VERIFIED) - src/api/ApiServer.cpp:2173-2192. Takes an ABSOLUTE file path (rejects non-absolute and a parent folder that is not a directory). It is not name-based: the path is used as given, then `saveCompositionTo` (MainComponent.cpp:3490-3505) sets `composition_.name = base name`. Test-server build only (`#if AUDIODNA_TEST_SERVER`, ApiServer.cpp:299/335). Not available on the production port unless that build flag is on.

**1e. Autosave / restore** (VERIFIED by absence) - grep for autosave / auto-save / lastSession / recent composition in src returns nothing. The only "restore" is Output > Restore Last Outputs (MainComponent.cpp kOutputRestoreLast). The app writes no composition or deck file by itself.

**1f. Audio store** (VERIFIED) - not name-based. Folder = `~/Documents/Audio-DNA/Audio/<id><kAssetSuffix>` where id = `juce::Uuid().toString()` (src/recording/AudioStore.cpp:89-97, :115-119). Files inside: fixed names `audio.wav`, `audio.json` (:99-107).

**1g. Recordings, snapshots** (VERIFIED) - not name-based: `recording_<YYYYmmdd_HHMMSS>.mp4` (MainComponent.cpp:7141, :7860); `snapshot_<stamp>.png` (src/render/Renderer.cpp:2420-2425).

**1h. Takes (performance recorder)** (VERIFIED) - name-based but not from a deck name.
- Folder = `~/Documents/Audio-DNA/Takes/<name>.adna-take` (MainComponent.cpp:5727-5731, :5771-5774). `name` = the REST body's "name" or the Record panel's name box, else a timestamp.
- Record panel: `juce::File::createLegalFileName(nameEditor.getText().trim())` (RecordPanel.cpp:17). That removes `" # @ , ; : < > * ^ | ? \ /` and cuts to 128 characters, keeping the extension if it sits in the last 12 (JUCE: File.cpp:855-876). So "/" and ":" are REMOVED, not replaced.
- REST `POST /api/perf/record` (ApiServer.cpp:1657-1664): `name` goes in unsanitized to `takesRoot().getChildFile(name + ".adna-take")`. So "a/b" makes Takes/a/b.adna-take (Take::save calls `createDirectory()`, which creates parents; Take.cpp:288-293), "../x" leaves Takes/, and a leading "/" gives an absolute folder. This is the same class of bug as 1a, in a different place (INFERRED for the nesting, VERIFIED for the missing sanitizing).
- Files inside a take: fixed `take.json` (Take.cpp:292).

**1i. Routines** (VERIFIED) - a routine's name is stored inside the take/composition JSON (`Routine::toVar` "name", src/model/Routine.h:122; RoutineSlice.cpp:288 `routine.name = req.name`). No file or folder is made from a routine name. No "export" of takes or routines to a named file exists (grep "export": only Bindings export, 7180-7192, which uses a chooser).

**1j. Others that use a chooser name (not deck names, for completeness)** - FX preset Save (MainComponent.cpp:2862-2885) and bindings export (:7180-7192) take whatever the chooser returns; no default name is built from a deck name. `PresetManager::saveDeck` (ui/PresetManager.cpp:480) is the legacy v1 deck writer; nothing in src calls it (only tests/test_preset_manager.cpp).

### Q2. Loading a deck file: is the deck's name from the file name or from inside the JSON? (VERIFIED)
- The file name wins. `Deck::fromVar` does read `"name"` from the JSON (src/model/Deck.h:141), but `stageDeckAppend` then overwrites it: `incoming.name = file.getFileNameWithoutExtension().toStdString(); incoming.sourceFile = file;` (src/MainComponent.cpp:3616-3621; comment: "ALWAYS the file's base name (plan6 §6.4)"). The label uses the same base name (:3629).
- Composition load does the same for the composition's own name (MainComponent.cpp:3450-3458). The decks INSIDE a composition keep the names stored in the JSON (Composition::fromVar -> Deck::fromVar; no override). So a deck whose JSON name contains "/" comes back from a composition file with its "/" intact.
- `getFileNameWithoutExtension` cuts at the LAST dot (JUCE: File.cpp:389-398): "My.Deck.json" gives "My.Deck"; ".json" gives ".json" (the dot is the first character, so it is treated as no extension).
- Shape check on load: top-level "layers" key only (MainComponent.cpp:3576-3585); the Decks library list uses the same test (CompDecksBrowser::isV2DeckFile, CompDecksBrowser.cpp:315-320) and is non-recursive, so a deck saved into a sub-folder of decks/ would NOT appear in the library (VERIFIED, CompDecksBrowser.cpp:297-303, `findChildFiles(findFiles, false, ...)`).

### Q3. Rename in place: what characters does the rename box accept? (VERIFIED on main c6fbc14; recon has only the dialog)
- main: `DeckView::DeckNameEditor : juce::TextEditor` (main:src/ui/DeckView.h:200-206); setup at main:src/ui/DeckView.cpp:29-39 sets popup menu off, font, justification and tooltip. NO `setInputRestrictions`, NO length limit (`git grep setInputRestrictions main -- src` finds only RecordPanel bar boxes and TopBar BPM). So every character is accepted, including "/", ":", a leading ".", emoji and any length (a default `juce::TextEditor` is single-line; line breaks are not typeable, INFERRED from the JUCE default, not read).
- Commit: `finishRename` (main:DeckView.cpp:656-678) does `renameEditor_.getText().trim()`; keeps it only if non-empty and different; then `applyDeckRename` (main:src/MainComponent.cpp:3804-3820) trims again and pushes `RenameDeckCmd`. Nothing else is filtered. Trim removes leading/trailing spaces (and tabs/newlines per JUCE `String::trim`, INFERRED), so a name cannot start or end with a space through the UI.
- The right-click "Rename Deck..." dialog (`renameDeck`, main:MainComponent.cpp:3781-3801; recon MainComponent.cpp:3750-3778) is a `juce::AlertWindow::addTextEditor`, with no restrictions and the same trim-and-empty rule.
- On the recon branch (a7491d4) only the dialog exists (MainComponent.cpp:3750-3778); the in-place box (Pitfall 65, BF8) is on main and arrives with the merge (docs: main:docs/claude/performance-controls.md line 53).
- Other deck-name sources: "Deck N" for new decks (Composition.h:217,578), "<name> copy" for Duplicate (comment MainComponent.cpp:3809-3818), the file base name on Load, the JSON name inside a composition. None sanitize.

### Q4. macOS facts a fix needs (run in my own $TMPDIR scratch dir, since removed)
Test run (VERIFIED by running touch/ls in `$TMPDIR/dn.2hsdqc`, volume `/dev/disk3s5` = APFS Data volume):
- `touch "a:b.json"`: works. ":" is a legal byte in a file name.
- `touch "a/b.json"`: "No such file or directory" (a "/" is a path separator, so it looks for a folder "a"). A "/" can never be inside a name; NUL cannot either (INFERRED, standard POSIX; not tried).
- `.hidden.json` and `.json` (empty base): both created; hidden from plain `ls`.
- Trailing space (`trail.json␣`) and a space before the dot (`trail␣.json`): both created and kept.
- Emoji `emoji🎛️.json`: created.
- Length: 250 "a" + ".json" = 255 chars OK; 251 "a" + ".json" = 256 chars: "File name too long". 128 "é" + ".json" (261 bytes) OK; 250 "é" + ".json" (505 bytes, 255 chars) OK. So the limit is 255 CHARACTERS on this APFS volume, not 255 bytes (`getconf NAME_MAX` prints 255).
- `mdls -name kMDItemDisplayName a:b.json` prints "a:b.json" - that is Spotlight's stored name, NOT Finder's drawing.

INFERRED (not verifiable here without launching Finder or a Cocoa program; `python3 -c "import Foundation"` fails, no pyobjc):
- Finder shows a ":" in a POSIX name as "/" and turns a typed "/" into ":" on disk (the classic HFS path-separator swap). Same treatment in the save panel's name field is likely but UNKNOWN-NEEDS-A-RUN.
- Characters bad at the POSIX level: "/" and NUL only. Characters that are legal but that Finder or Windows treat specially: ":" (display swap), a leading "." (hidden), a trailing space or dot (Windows-illegal), and `\ * ? " < > |` (Windows-illegal; JUCE's `createLegalFileName` removes those plus `# @ , ; ^`). The project is mac-primary but cross-platform (CLAUDE.md), so a deck file saved on mac with ":" or `?` would be illegal if copied to Windows (INFERRED).
- Unicode normalization: APFS stores names as given and compares normalization-insensitively; the app does no normalization (INFERRED, not tested; a precomposed vs decomposed accent could look identical).
- Case: APFS here is case-INSENSITIVE (CompDecksBrowser.h:44 notes "decks" and "Decks" collide), so "Deck.json" and "deck.json" are the same file (documented in repo; not re-tested).

Scratch dir `$TMPDIR/dn.2hsdqc` was removed after the test.

## 2 TABLES

### Where a name becomes a path

| # | Site | Source of the name | File:line (recon) | Default name built by | Sanitized? |
|---|---|---|---|---|---|
| 1 | Save Deck As | deck.name | MainComponent.cpp:3699-3702 | `getChildFile(name + ".json")` | No |
| 2 | Save Deck (plain) | deck.name must equal sourceFile base | :3678-3689 | no new name; overwrites sourceFile | n/a |
| 3 | Save Composition As | composition_.name (never typed) | :3512-3515 | `getChildFile(name + ".json")` | No |
| 4 | Save Composition (plain) | composition_.filePath | :3464-3488 | none | n/a |
| 5 | Collect Media | composition_.name | :6564-6565, :6593-6594 | `getChildFile(name + "_media")`, `(name + ".json")` | No |
| 6 | POST /api/debug/save_composition | request "path" (absolute) | ApiServer.cpp:2173-2192 | none (path as given) | only "absolute, parent exists" |
| 7 | Record panel take name | text box | RecordPanel.cpp:17 | `createLegalFileName` then `getChildFile(name + ".adna-take")` | Yes (removes bad characters, cuts to 128) |
| 8 | POST /api/perf/record | request "name" | ApiServer.cpp:1657-1664; MainComponent.cpp:5771-5774 | `getChildFile(name + ".adna-take")` | No |
| 9 | Audio store | uuid | AudioStore.cpp:96,119 | uuid | n/a |
| 10 | Recordings / snapshots | timestamp | MainComponent.cpp:7141,7860; Renderer.cpp:2420 | timestamp | n/a |
| 11 | Routines | routine.name | RoutineSlice.cpp:288 | none (JSON field) | n/a |
| 12 | Autosave / restore | - | - | none exists | n/a |

### Reverse direction (file to name)

| Load site | Name taken from | Line |
|---|---|---|
| Load Deck / library row / /api/debug/load_deck | file base name (overrides JSON) | MainComponent.cpp:3620 |
| Open Composition / /api/load_composition | file base name for the composition; deck names from JSON | :3454; Deck.h:141 |

### What the rename box accepts

| Character | Rename box (main) | Rename dialog | Result in Save Deck As default |
|---|---|---|---|
| "/" | accepted | accepted | sub-path (bug) |
| ":" | accepted | accepted | passes through |
| leading "." | accepted | accepted | hidden file |
| leading/trailing space | trimmed away | trimmed away | n/a |
| empty / spaces only | rejected (name kept) | rejected | n/a |
| 255+ chars | accepted | accepted | save fails at 236+ (INFERRED) |
| emoji | accepted | accepted | passes through |

## 3 WHAT EXISTS TODAY vs WHAT BORIS ASKED (gaps, no design)

- Boris: "/" in a deck file name is "just a character", "do not worry about adding sub folders". Today (VERIFIED): the code path turns "/" into a sub-folder (getChildFile), and the chooser is pointed at a folder that does not exist.
- Boris: "If there are characters that are no good, then let me know and we will create a fix together". Today: nothing in the app lists or reports bad characters; there is no check in rename, in Save Deck As, in Save Composition As or in the perf REST route. The facts for that conversation: "/" and NUL cannot exist in a POSIX name; ":" is legal on disk but Finder shows "/" (INFERRED); a leading "." hides the file; 255 characters is the cap; the temp-file suffix lowers the usable cap to about 236 (INFERRED).
- The app's own invariant "deck name == file base name" (MainComponent.cpp:3617-3621, :3685, :3724) means a deck named "A/B" cannot be saved under that exact name and cannot come back from its file as "A/B". Something has to give: the name, the file name, or the invariant. That is a decision for the planner and Boris; this sheet only states it.
- The in-composition deck names can hold "/" (a composition JSON stores them as strings, no file involved) - so "/" in a deck name is already legal inside a composition save and reload.
- REST perf record name is unsanitized (path traversal class): separate from decks but the same root cause; it is not in Boris's words.

## 4 UNKNOWN-NEEDS-A-RUN

1. What the NSSavePanel shows when `setDirectoryURL` names a folder that does not exist (".../decks/A") and the name field is "B.json": which folder it opens, and whether the user sees "B.json" or something else. Cheapest test: on main/recon app with the test server, rename a deck to `A/B` (on main: `POST 8080 /api/debug/deck_rename` ops begin/type/enter; on recon there is no such route, load a composition file whose deck "name" is "A/B" with `POST 7070 /api/load_composition {"path": ...}`), then open Deck menu > Save Deck As on screen and take a screenshot of the dialog.
2. What the save panel returns when the user types "A:B" or "A/B" in the name field (does it convert, refuse, or make a folder), and what file lands on disk. Same run, then `ls` the decks dir.
3. Whether NSSavePanel appends ".json" to a typed name that contains a dot ("Mix 1.5"), versus the app's `withFileExtension` rewriting it to "Mix 1.json". Same run.
4. Whether a file with a leading "." or a ":" shows up in the Decks library list and loads (scan code says yes, VERIFIED reading; the on-screen result is not seen). Test: create `~/Library/Application Support/AudioDNA/decks/.hid.json` and `a:b.json` containing a v2 deck, `GET`-less: look at the Compositions browser tab, or `POST 8080 /api/debug/load_deck {"path": ...}` then `GET 7070 /api/state` and read `decks[].name`.
5. Failure behaviour for a 237-255 character deck name (the temp-file arithmetic, INFERRED). Test: rename to a 240-character name, Save Deck As, see if "Save failed" appears; needs the on-screen dialog.
6. Finder's display of ":" (INFERRED to show "/"). Test: `ls` plus a Finder screenshot of a file named `a:b.json` in a scratch folder.

## 5 FILES A BUILDER WOULD TOUCH (paths only; no design implied)

- /Users/boriskarpman/projects/RealTimeAudio/src/MainComponent.cpp (saveDeckAs ~3691-3728, saveDeck ~3678, saveCompositionAs ~3507, Collect Media ~6553, renameDeck/applyDeckRename on main, perfRecord ~5748)
- /Users/boriskarpman/projects/RealTimeAudio/src/MainComponent.h
- /Users/boriskarpman/projects/RealTimeAudio/src/ui/DeckView.cpp and /Users/boriskarpman/projects/RealTimeAudio/src/ui/DeckView.h (rename box, main only)
- /Users/boriskarpman/projects/RealTimeAudio/src/ui/DeckTabRow.h (menu/geometry helpers, pure; main)
- /Users/boriskarpman/projects/RealTimeAudio/src/ui/CompDecksBrowser.cpp and .h (library scan, dirs)
- /Users/boriskarpman/projects/RealTimeAudio/src/api/ApiServer.cpp and ApiServer.h (perf record name; debug routes)
- /Users/boriskarpman/projects/RealTimeAudio/src/ui/RecordPanel.cpp (the one existing createLegalFileName)
- /Users/boriskarpman/projects/RealTimeAudio/src/model/Deck.h (Deck::fromVar name), /Users/boriskarpman/projects/RealTimeAudio/src/model/Composition.h (name at 876)
- /Users/boriskarpman/projects/RealTimeAudio/src/core/DeckCommands.h (RenameDeckCmd, main)
- Docs: /Users/boriskarpman/projects/RealTimeAudio/docs/claude/performance-controls.md (Deck tab row / rename in place), /Users/boriskarpman/projects/RealTimeAudio/docs/claude/integration.md (REST), /Users/boriskarpman/projects/RealTimeAudio/.harmony/APP-INVENTORY.md

## 6 EXISTING TESTS / PROBES AND REST ROUTES

Tests (names only):
- tests/test_deck_tab_row.cpp (menu rows incl. "Save Deck As...", line 77; geometry)
- tests/test_comp_decks_browser.cpp (isV2DeckFile, library scan with temp-dir fixtures)
- tests/test_staged_load.cpp (Load Deck label "Loaded deck: deck16", line 75)
- tests/test_preset_manager.cpp (legacy PresetManager::saveDeck/loadDeck only)
- main only: tests/test_deck_tab_rename.cpp; probe `.harmony/probe-ui-files-rename.sh` R1-R10 (rename box); `tests/probe_deck_tab_dispatch.cpp` (on-screen)
- Recon: `.harmony/probe-deck-tabs.sh` (deck tab menus; places a `_probe-deck-tabs.json` fixture in the app's library), `.harmony/probe-boxes.py` (uses save_composition at ~1146-1157 and load_deck at ~558-568), `.harmony/probe-async-load.py`.
- Gap (VERIFIED by grep): no test or probe calls `saveDeckAs`, `writeDeckFile`, or builds a Save Deck As default name; none uses a deck name with "/" or ":".

REST routes that can drive this without synthetic input:
- 7070 production: `GET /api/composition` and `GET /api/state` (read deck names; ApiServer.cpp:173, :280, :456); `POST /api/load_composition {"path"}` (ApiServer.cpp:251, handler :1091) - the cheapest way to get a deck named "A/B" on recon: write a composition JSON whose `decks[0].name` is "A/B" and load it; `POST /api/perf/record {"name"}` (:289, :1657) for the take-name path.
- 8080 / test-server build (`#if AUDIODNA_TEST_SERVER`, ApiServer.cpp:299): `POST /api/debug/save_composition {"path": <absolute>}` (:2173); `POST /api/debug/load_deck {"path"}` (:320, :2099); `POST /api/debug/duplicate_deck`; `POST /api/debug/remove_deck`; `POST /api/debug/undo`; `GET /api/debug/ui_text` (the status line text, e.g. "Saved deck: <name>").
- main only: `POST /api/debug/deck_rename {"deck","op","text"}` (main:ApiServer.cpp:331, handler :2196) types a name into the rename box path.
- Not drivable by any route: the Save Deck As / Save Composition As chooser itself and `writeDeckFile` (no route on recon or main). Seeing the dialog needs the app on screen and a screenshot (testing-eyes.md).
