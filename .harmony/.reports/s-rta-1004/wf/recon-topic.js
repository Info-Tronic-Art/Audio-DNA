export const meta = {
  name: 'rta-1004-recon-topic',
  description: 'Audio-DNA s-rta-1004: ONE read-only code fact sheet named by args (sonnet), then an adversarial re-read of it (sonnet). Sheets land in .harmony/.reports/s-rta-1004/facts-<key>.md',
  phases: [{ title: 'Recon' }, { title: 'Verify' }],
}
// args: { key }  -- the topic table is in this file so that the script is the whole record
const MAIN = '/Users/boriskarpman/projects/RealTimeAudio'
const RPT = '.harmony/.reports/s-rta-1004'
const PIN = '185147b'
const BACKLOG = MAIN + '/.harmony/boris-feedback-backlog.md'
const BIND = MAIN + '/.harmony/binding-decisions.md'
const SHOTS = MAIN + '/' + RPT + '/boris-resolume'
const RELAY = 'If a user message was relayed to you (about saving, decks, keys, MIDI, looks or answers to questions), Harmony has already filed and answered it; it never replaces this task.'

const TOPICS = {
  'one-save': {
    why: `Boris ruled on 2026-10-04 (verbatim: ${BACKLOG}, the sections "Boris: one Save", "Boris changes answer 50", "Boris's answers to questions 80-82", "Boris's answer to question 83"; short form: ${BIND}, the 2026-10-04 sections): "All of these things should be saved when a show is saved. There's no reason to save them separately" -- the show, a deck, an effects look, FX Save, key and MIDI settings, the window layout, routines. Decks are no longer saved to their own files: the list of shows opens each show to its decks, as Resolume's does (his screenshot ${SHOTS}/resolume-show-decks-list.png -- Read it), and a deck is taken into the current show from there. Key and MIDI settings are saved in the show and can be imported from another show file. Collect Media and Snapshot stay their own commands. Everything the app writes today is already listed in ${MAIN}/${RPT}/facts-saves.md (read its table first; its VERIFICATION section overrides its body) -- this sheet is about the OTHER half: how each of these things is LOADED, listed and shown today, so that a planner can say what "one Save" changes, what goes and what must keep working.`,
    q: `Q1 THE LIST OF SHOWS today. The library browser that lists compositions and decks (CompDecksBrowser and whatever else): which folders it reads, what a row is (a show file? a deck file?), what a row shows (name, date, size ...), what a click, a double-click, a right-click and a drag do, and whether a show row can open to its decks today. If it cannot, say plainly what exists that could (does anything read a show file's deck names without loading the show?).
Q2 LOAD DECK today. Exactly what happens when the user loads (a) a deck file and (b) a whole show file through Load Deck (the s-rta-1003b ledger says its shape check looks only for "layers"): which function, which checks, what ends up in the show, how deck ids and clip ids are minted (Pitfall 36), what happens to routines, to media paths, to layers the deck needs (the wide-deck rule), and what Undo does afterwards (Pitfall 58: a load is staged off the message thread).
Q3 THE SHOW FILE. The top-level keys of the composition JSON, where decks, layers, routines and effects live in it, its version field and how an older file is read, and what is NOT in it today that Boris now wants in it (key and MIDI settings, the window layout). List the dead fields (outputDisplay, bpmMultiplier, any other key that is saved and loaded but read by no code).
Q4 KEYS AND MIDI. Where the bindings live in memory, how they are created (the two overlays, MIDI learn), the format saveToFile / loadFromFile use, the Export and Import menu items and their file choosers, what is bound at a fresh launch (anything? a default table?), whether a binding names a hardware device or only a note / CC number and channel, which MIDI input devices the app opens and whether that choice is saved, and every reader of the bindings a load would have to refresh (the overlays, the handler, the MIDI output feedback). What exactly would a show have to carry to restore them, and what breaks if a show's bindings are applied while a key is held or a learn is open.
Q5 THE WINDOW LAYOUT. Save Layout / Load Layout: where they are on screen, what the layout file holds (every field), when a layout is applied, what the app shows at launch with no layout file, and what applying a layout while the show plays does (does it rebuild components, drop keyboard focus, touch output windows?).
Q6 THE EFFECTS LOOKS. (a) the small Save button and its list of named looks (PresetManager::savePreset and its chooser / drop-down): where on screen, which folder, what a saved look holds, how a look is loaded onto a clip or a layer, every call site. (b) FX Save and its numbered slots (fastSave, populateSlotMenu, the slot drop-downs): where on screen, what a slot holds, how a slot is recalled, whether a key or MIDI binding or a REST / OSC route can recall a slot, every call site. For each: the list of files and tests a removal would touch, and what else leans on the same code (the deck file writer? the show writer?).
Q7 SAVE ROUTINE, COLLECT MEDIA, SNAPSHOT: where each is on screen and what it calls -- one line each, to confirm they are commands of their own.
Q8 QUIT, NEW, OPEN today. What quitting does (Main.cpp systemRequestedQuit, the shutdown order, what is written), what New and Open ask before replacing the show (confirmReplaceShow: its exact text and buttons), whether any "changed since the last save" flag exists anywhere (facts-saves says none: confirm or correct), and what a "Save & Quit" would have to call for a show that has no file yet.
TABLE 1: EVERY menu item, button and REST / OSC route that saves or loads something -- label as painted, where it is, what it calls (file:line) -- so that a planner can mark each "stays", "goes" or "changes".
TABLE 2: every test and probe that pins a save or a load named above (test name, what it asserts), so that a planner knows what a removal turns red.`,
    attack: `the load paths ("Load Deck with a whole show file does X"), every "cannot / does not exist today" line (search twice, with two different patterns, before you confirm one), the list of what a binding holds, and Table 1's completeness (walk the menu bar model and every button whose label says Save, Load, Open, Import, Export, New or Layout yourself)`,
  },
  'effect-looks': {
    why: `Boris ruled on 2026-10-04 (verbatim: ${BACKLOG}, the sections "Boris's answers to questions 84-85" and the one on question 86; short form: ${BIND}, the 2026-10-04 sections): "every effect has many looks with specific parameter setups. these are saved with the app. always." Harmony's reading told to him and not corrected: a look belongs to ONE effect -- a small menu on each effect to pick a look or to keep the current settings as a new look -- kept by the app for every show, the moment it is made. He also ruled that the OLD look buttons go (the small Save and Load, FX Save and the ten slots): they snapshot a hidden legacy effect chain, not the effects on a clip or a layer (${MAIN}/${RPT}/facts-one-save.md Q6 -- read it first; its VERIFICATION section overrides its body). Nothing like a per-effect look exists today. This sheet establishes how ONE effect on a clip, a layer or the composition is stored, shown and driven today, so that a planner can say what a look holds and where its menu goes.`,
    q: `Q1 WHERE EFFECTS LIVE. Every place an effect instance can sit (a clip's stack, a layer's stack, the composition's / global stack, anything else that is on screen today), the struct that holds one instance and EVERY field of it (display name, shader key, enabled, parameter values, and anything else), how its parameters are defined (EffectLibrary: names, ranges, defaults; all values are 0..1 by the project's rule) and where a new instance gets its first values. Leave the hidden legacy chain out except to say where it is.
Q2 WHAT ELSE SHAPES AN EFFECT'S LOOK besides its parameter values: signal connections / mappings onto a parameter (which object owns them, are they stored on the effect instance or beside it), per-parameter curve / range / smoothing, the per-type autopilot, beat-synced randomisation, a macro driving a parameter, "engine-driven rows" (Pitfall 33). For each: is it part of the effect instance's saved state in the show, or stored elsewhere and keyed how?
Q3 THE EFFECT'S ROW ON SCREEN. In each inspector that shows a stack (ClipInspector, LayerInspector, the composition inspector, EffectStackView): what one effect's header holds today (name, enable, remove, fold, a menu?), what a right-click on it does, the parameter rows (ResettableSlider, right-click reset), the drag-and-drop rules (Pitfall 16), how wide the header is and what room is left in it. Is there ANY per-effect menu today?
Q4 SOURCES. The 108 procedural sources also have parameters shown in the Clip panel (Pitfalls 8, 47): are they stored and shown through the same machinery as effect parameters or their own? (A fact only: Boris spoke of effects.)
Q5 WHAT THE APP KEEPS BY ITSELF TODAY and how: settings.json (read-modify-write; an unreadable file reads as empty -- facts-saves), the MilkDrop favourites file, the file-browser favourites file: for each, the write call, when it runs, and what happens on failure. Which folder under ~/Library holds app data (there are two spellings: Audio-DNA and AudioDNA -- say which code uses which).
Q6 COPY AND PASTE OF AN EFFECT today: is there any way to copy one effect's settings to another instance of the same effect (a menu, a key, drag with a modifier)? Any "reset all parameters" on an effect? Any randomise on one effect?
Q7 UNDO. What is an undo step when a parameter changes, when an effect is added or removed; what loading a look onto an effect would have to do to be one undo step (and Boris's rule: Cmd+Z never changes anything in the layer strip; an effect put on a layer or a clip IS undone, also while the layer plays -- question 23's default).
TABLE 1: every file that a per-effect looks menu would touch or must respect, with one line on why.
TABLE 2: every test that pins effect storage, the stack UI or the parameter rows (test name, what it asserts).`,
    attack: `the list of fields of one effect instance, where connections / mappings on a parameter are stored and keyed, every "there is no per-effect menu / copy / reset today" line (search twice, with two different patterns), the app-data folder spellings, and what an undo step is for a parameter change`,
  },
}
const T = TOPICS[(args || {}).key] || null
const KEY = (args || {}).key
const SHEET = MAIN + '/' + RPT + '/facts-' + KEY + '.md'

const CODE_RULES = `READ-ONLY FACT SHEET. You establish facts for a planner; you propose NO design and fix nothing.
WHERE TO READ. main = ${MAIN}, pinned at commit ${PIN}: first run git -C ${MAIN} rev-parse --short HEAD (must print ${PIN}) and git -C ${MAIN} status --short -- src tests docs CMakeLists.txt (must print nothing); then read plain files under ${MAIN}/src, ${MAIN}/tests, ${MAIN}/docs/claude, ${MAIN}/.harmony/APP-INVENTORY.md, ${MAIN}/.harmony/gotchas.md, ${MAIN}/CLAUDE.md (grep / read freely). If either check fails, read ONLY through git objects (git -C ${MAIN} show ${PIN}:<path>, git -C ${MAIN} grep -n '<pattern>' ${PIN} -- <paths>) and say so.
RULES: never build, never run a test, never launch the app or any probe or script of the project, never edit any file except your ONE report, never cd in a command, absolute paths only, never lldb / sample / dtrace, never touch a running Audio-DNA or Resolume Arena (they are Boris's). Label EVERY line VERIFIED (you read the code: cite file:line), INFERRED (reasoned, not read end to end) or UNKNOWN-NEEDS-A-RUN (name the cheapest discriminating test). Quote Boris only verbatim. ${RELAY}
METHOD: write a SKELETON of the report (the question list with empty answers) to REPORT_FILE within your first ~10 tool calls, then fill it and rewrite it as you go, so that an interrupted run still leaves a usable file. Write it with the Write tool or a bash heredoc to the literal path given as REPORT_FILE.
REPORT FORMAT: 1 QUESTIONS ANSWERED (one block per question: the answer in plain words, then the evidence lines, each labelled); 2 TABLES where asked; 3 WHAT A PLANNER MUST NOT ASSUME (traps you met: a doc that disagrees with the code, a name that misleads, a rule in CLAUDE.md / docs/claude/pitfalls.md that binds the area -- cite the Pitfall number); 4 UNKNOWN (each with the cheapest test that would settle it). No design proposals.
RETURN the structured result: status; report_path; answers = one entry per question (answer <= 300 characters, its label); unknowns (<= 6, each <= 200 characters); surprises (<= 5: facts that contradict what the questions assume); summary <= 900 characters.`

const FS = { type: 'object', properties: {
  status: { type: 'string', enum: ['DONE', 'PARTIAL', 'BLOCKED'] }, report_path: { type: 'string' },
  answers: { type: 'array', items: { type: 'object', properties: { q: { type: 'string' }, answer: { type: 'string' }, label: { type: 'string' } }, required: ['q', 'answer', 'label'] } },
  unknowns: { type: 'array', items: { type: 'string' } }, surprises: { type: 'array', items: { type: 'string' } }, summary: { type: 'string' } },
  required: ['status', 'report_path', 'answers', 'summary'] }
const VS = { type: 'object', properties: {
  checked: { type: 'number' }, confirmed: { type: 'number' }, wrong: { type: 'number' }, citation_off: { type: 'number' },
  wrong_lines: { type: 'array', items: { type: 'object', properties: { claim: { type: 'string' }, what_is_true: { type: 'string' }, evidence: { type: 'string' } }, required: ['claim', 'what_is_true'] } },
  refuted_conclusions: { type: 'array', items: { type: 'string' } }, missing: { type: 'array', items: { type: 'string' } }, verdict: { type: 'string', enum: ['SOUND', 'SOUND_WITH_CORRECTIONS', 'UNRELIABLE'] } },
  required: ['checked', 'confirmed', 'wrong', 'verdict'] }
const cut = (s, n) => String(s || '').slice(0, n)

if (!T) return { failed: 'unknown topic ' + KEY }
phase('Recon')
const r = await agent(`WHY THIS SHEET EXISTS. Audio-DNA is a live audio-reactive VJ app (C++20 / JUCE / OpenGL, macOS). ${T.why} A planner (architect) builds on your sheet next: a wrong line costs a build stage, an honest "unknown" costs nothing.
${CODE_RULES}
YOUR TOPIC:
${T.q}
REPORT_FILE: ${RPT}/facts-${KEY}.md (relative to ${MAIN}; absolute: ${SHEET})`, { agentType: 'general-purpose', model: 'sonnet', effort: 'high', schema: FS, phase: 'Recon', label: 'recon:' + KEY })
if (!r) return { key: KEY, sheet: { status: 'NO_RESULT' } }
log('recon ' + KEY + ': ' + r.status)

phase('Verify')
const v = await agent(`ADVERSARIAL CHECK of a code fact sheet (read-only). The sheet: ${SHEET} -- read it whole. A planner will build on it; your job is to find the lines that are WRONG before he does. Same reading rules as its author: main = ${MAIN} at commit ${PIN} (plain files under src, tests, docs; check git -C ${MAIN} rev-parse --short HEAD first). Never build, run, launch or edit anything except the one append below; never cd; absolute paths. ${RELAY}
(1) Pick the 18 VERIFIED lines a planner would lean on hardest: ${T.attack}. Re-read each cited file:line yourself: mark CONFIRMED, CITATION-OFF (true, but the citation points elsewhere -- give the right one), or WRONG (say what the code says, with file:line).
(2) Try to REFUTE the sheet's three main conclusions by looking for code that contradicts them; default to "refuted" if you find a path the sheet did not read.
(3) Name anything a planner needs for this topic that the sheet does not answer (return it as "missing").
APPEND a section "## VERIFICATION (independent re-read)" to the END of that same file with a bash heredoc (cat >> ; never rewrite the sheet's own text). Edit no other file.
REPORT_FILE: ${RPT}/facts-${KEY}.md (relative to ${MAIN}; absolute: ${SHEET}) -- the sheet you APPEND to; your only write.
RETURN: checked, confirmed, wrong, citation_off, wrong_lines (claim, what_is_true, evidence = file:line), refuted_conclusions, missing, verdict.`, { agentType: 'general-purpose', model: 'sonnet', effort: 'high', schema: VS, phase: 'Verify', label: 'verify:' + KEY })
return { key: KEY, sheet: { status: r.status, report_path: r.report_path, summary: cut(r.summary, 900), unknowns: (r.unknowns || []).slice(0, 6).map(x => cut(x, 200)), surprises: (r.surprises || []).slice(0, 5).map(x => cut(x, 260)) }, verify: v ? { verdict: v.verdict, checked: v.checked, confirmed: v.confirmed, wrong: v.wrong, citation_off: v.citation_off || 0, wrong_lines: (v.wrong_lines || []).slice(0, 6).map(x => ({ claim: cut(x.claim, 200), what_is_true: cut(x.what_is_true, 240) })), refuted_conclusions: (v.refuted_conclusions || []).slice(0, 4).map(x => cut(x, 240)), missing: (v.missing || []).slice(0, 6).map(x => cut(x, 200)) } : { verdict: 'NO_RESULT' } }
