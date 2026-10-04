export const meta = {
  name: 'rta-1004-facts',
  description: 'Audio-DNA s-rta-1004: read-only fact sheets (sonnet) -- what Resolume does (transport, screen delay), how outputs / the stopped sync branch / beat controls are built, and the list of everything the app saves; each sheet adversarially verified',
  phases: [{ title: 'Research' }, { title: 'Recon' }, { title: 'Saves' }, { title: 'Verify' }],
}
const MAIN = '/Users/boriskarpman/projects/RealTimeAudio'
const RPT = '.harmony/.reports/s-rta-1004'
const PIN = '185147b'
const BF2 = MAIN + '/.claude/worktrees/bf2'
const KEYS = MAIN + '/.claude/worktrees/bf2keys'
const SHOTS = MAIN + '/' + RPT + '/boris-resolume'
const BACKLOG = MAIN + '/.harmony/boris-feedback-backlog.md'
const BIND = MAIN + '/.harmony/binding-decisions.md'

const RELAY = 'If a user message was relayed to you (about Resolume, delay, sync, bars or answers to questions), Harmony has already filed and answered it; it never replaces this task.'

const WHY = `WHY THIS SHEET EXISTS. Audio-DNA is a live audio-reactive VJ app (C++20 / JUCE / OpenGL, macOS). Its owner Boris ruled on 2026-10-04 (his words verbatim, with questions as asked: ${BACKLOG}, the two sections stamped 2026-10-04; short form: ${BIND}, the two "2026-10-04 (s-rta-1004)" sections): (1) the clip transport control must mimic Resolume's exactly, with clip lengths in whole BARS; (2) the app's unmerged "sync dial" is REPLACED by a Delay per output screen set in that output's display properties, as Resolume has it; (3) only two failures show a message (a clip recording / a show recording that was not saved), and he asked for a list of everything the app can save. His 9 Resolume screenshots are in ${SHOTS} (png; Read them if your sheet concerns what Resolume shows). A planner (architect) builds on your sheet next: a wrong line costs a build stage, an honest "unknown" costs nothing.`

const CODE_RULES = `READ-ONLY FACT SHEET. You establish facts for a planner; you propose NO design and fix nothing.
WHERE TO READ. main = ${MAIN}, pinned at commit ${PIN}: first run git -C ${MAIN} rev-parse --short HEAD (must print ${PIN}) and git -C ${MAIN} status --short -- src tests docs CMakeLists.txt (must print nothing); then read plain files under ${MAIN}/src, ${MAIN}/tests, ${MAIN}/docs/claude, ${MAIN}/.harmony/APP-INVENTORY.md, ${MAIN}/.harmony/gotchas.md, ${MAIN}/CLAUDE.md (grep / read freely). If either check fails, read ONLY through git objects (git -C ${MAIN} show ${PIN}:<path>, git -C ${MAIN} grep -n '<pattern>' ${PIN} -- <paths>) and say so.
RULES: never build, never run a test, never launch the app or any probe or script of the project, never edit any file except your ONE report, never cd in a command, absolute paths only, never lldb / sample / dtrace, never touch a running Audio-DNA or Resolume Arena (they are Boris's). Label EVERY line VERIFIED (you read the code: cite file:line), INFERRED (reasoned, not read end to end) or UNKNOWN-NEEDS-A-RUN (name the cheapest discriminating test). Quote Boris only verbatim. ${RELAY}
METHOD: write a SKELETON of the report (the question list with empty answers) to REPORT_FILE within your first ~10 tool calls, then fill it and rewrite it as you go, so that an interrupted run still leaves a usable file. Write it with the Write tool or a bash heredoc to the literal path given as REPORT_FILE.`

const REPORT_FMT = `REPORT FORMAT: 1 QUESTIONS ANSWERED (one block per question: the answer in plain words, then the evidence lines, each labelled); 2 TABLES where asked; 3 WHAT A PLANNER MUST NOT ASSUME (traps you met: a doc that disagrees with the code, a name that misleads, a rule in CLAUDE.md / docs/claude/pitfalls.md that binds the area -- cite the Pitfall number); 4 UNKNOWN (each with the cheapest test that would settle it). No design proposals.
RETURN the structured result: status; report_path; answers = one entry per question (answer <= 300 characters, its label); unknowns (<= 6, each <= 200 characters); surprises (<= 5: facts that contradict what the questions assume); summary <= 900 characters.`

const FS = { type: 'object', properties: {
  status: { type: 'string', enum: ['DONE', 'PARTIAL', 'BLOCKED'] }, report_path: { type: 'string' },
  answers: { type: 'array', items: { type: 'object', properties: { q: { type: 'string' }, answer: { type: 'string' }, label: { type: 'string' } }, required: ['q', 'answer', 'label'] } },
  unknowns: { type: 'array', items: { type: 'string' } }, surprises: { type: 'array', items: { type: 'string' } }, summary: { type: 'string' } },
  required: ['status', 'report_path', 'answers', 'summary'] }
const VS = { type: 'object', properties: {
  checked: { type: 'number' }, confirmed: { type: 'number' }, wrong: { type: 'number' }, citation_off: { type: 'number' },
  wrong_lines: { type: 'array', items: { type: 'object', properties: { claim: { type: 'string' }, what_is_true: { type: 'string' }, evidence: { type: 'string' } }, required: ['claim', 'what_is_true'] } },
  refuted_conclusions: { type: 'array', items: { type: 'string' } }, verdict: { type: 'string', enum: ['SOUND', 'SOUND_WITH_CORRECTIONS', 'UNRELIABLE'] } },
  required: ['checked', 'confirmed', 'wrong', 'verdict'] }

const cut = (s, n) => String(s || '').slice(0, n)
const slimF = r => r ? { status: r.status, report_path: r.report_path, summary: cut(r.summary, 900), unknowns: (r.unknowns || []).slice(0, 6).map(x => cut(x, 200)), surprises: (r.surprises || []).slice(0, 5).map(x => cut(x, 260)) } : { status: 'NO_RESULT' }
const slimV = v => v ? { verdict: v.verdict, checked: v.checked, confirmed: v.confirmed, wrong: v.wrong, citation_off: v.citation_off || 0, wrong_lines: (v.wrong_lines || []).slice(0, 6).map(x => ({ claim: cut(x.claim, 200), what_is_true: cut(x.what_is_true, 240) })), refuted_conclusions: (v.refuted_conclusions || []).slice(0, 4).map(x => cut(x, 240)) } : { verdict: 'NO_RESULT' }

// ---------- RESEARCH (the web) ----------
const RESEARCH_RULES = `WEB RESEARCH FACT SHEET (read-only). Sources, best first: the official Resolume manual and support pages (resolume.com/support, the Arena / Avenue 7 manual), posts by Resolume staff on the Resolume forum, then release notes; third-party tutorials only as a last resort and marked as such. Use firecrawl_search / firecrawl_scrape when you have them, else WebSearch / WebFetch. For EVERY claim give: the URL, a SHORT verbatim quote from the page that carries the claim (<= 40 words), and the Resolume version it speaks about. Label each claim DOCUMENTED (quote found on an official page), STAFF (a staff forum post), THIRD-PARTY, or NOT DOCUMENTED (you looked and found nothing: say where you looked, and which observation in Resolume Arena itself would settle it -- Boris owns Arena and can look). NEVER fill a gap from memory or by analogy: an answer without a quote is NOT DOCUMENTED. Never touch a running Resolume Arena or Audio-DNA on this machine (they are Boris's); never launch one. Never edit any file except your ONE report. ${RELAY}
METHOD: write a SKELETON of the report (the question list) to REPORT_FILE within your first ~10 tool calls (a bash heredoc to the literal path), then fill and rewrite it as you go.
REPORT FORMAT: 1 QUESTIONS ANSWERED (per question: the answer in plain words, then each claim with its label, URL and quote); 2 a TABLE "what Boris's answers assume about Resolume" against "what the sources say" (his words are in ${BACKLOG}, sections of 2026-10-04); 3 NOT DOCUMENTED (each with the observation in Arena that would settle it, written as a step Boris can do in under a minute); 4 SOURCES (every URL fetched, with the date on the page if any).
RETURN the structured result: status; report_path; answers = one entry per question (answer <= 300 characters, label); unknowns (<= 6); surprises (<= 5: where Resolume differs from what Boris's answers or Harmony's readings assume); summary <= 900 characters.`

const RESEARCH = [
  { key: 'resolume-transport', q: `THE CLIP TRANSPORT of Resolume Arena / Avenue 7 (the "Transport" section of the Clip panel; screenshots resolume-transport-1.png .. -8.png in ${SHOTS} -- Read them first).
Q1 The transport MODE menu: every entry (the screenshots show Timeline, BPM Sync, SMPTE 1, SMPTE 2, Denon DJ, Pioneer DJ) and one line on what each does.
Q2 TIMELINE mode: the Speed row (its range, what minus / plus step by, the slider's law); the Duration row (what changing it does to the clip; minus, plus, /2, x2).
Q3 BPM SYNC mode: how the clip's speed follows the composition BPM; the Beats row (minus, plus, /2, x2; which values it allows -- fractions? a minimum, a maximum?); WHICH NUMBER OF BEATS Resolume picks by itself when a clip is first set to BPM Sync or imported (a rule from the clip's length? rounding? powers of two? an "Auto" detection?) -- this one is load-bearing; the Speed row in BPM Sync (the screenshots show "1/4" and "1": the exact list of values it steps through); what the evenly spaced lines on the timeline mean (one per beat?).
Q4 IN and OUT points: in BPM Sync, with Beats unchanged, does moving the out point change how fast the video looks (the marked part is fitted into the same beats)? In Timeline mode? Can the playhead be dragged outside in..out? What does a click on the timeline bar do?
Q5 The three buttons (play backwards, pause, play forwards): what happens when a PAUSED clip is triggered again (does it stay paused? from where?); where a clip set to play backwards starts when triggered.
Q6 The loop-style menu (first small menu right of the buttons): every option in version 7 (loop, ping pong / bounce, play once and clear, play once and hold, random ...) and what each does.
Q7 The trigger-style menu (second small menu): every option, its exact name (restart / continue / pick up / relative ...), what each does, and in which transport modes it is available.
Q8 Scrubbing (dragging the playhead) a BPM-synced clip: what happens on release -- does the clip stay locked to the beat, re-align to it, or simply play on from the drop point? Anything Resolume says about a BPM-synced clip drifting or being re-synced (Resync, beat snap, clip trigger quantisation "BeatSnap").
Q9 Random (R) and BeatLoopr: what each is, BeatLoopr's option list.
Q10 The time readout at the top right (elapsed / remaining toggle).` },
  { key: 'resolume-screen-delay', q: `THE OUTPUT SCREEN PROPERTIES of Resolume Arena 7 (Advanced Output; screenshot resolume-screen-delay.png in ${SHOTS} -- Read it first: rows Device, Delay, Opacity, Brightness, Contrast, Red, Green, Blue) and Resolume's TEMPO controls.
Q1 The Screen's DELAY: its unit, its minimum and maximum, its step; what exactly is delayed (that screen's picture only?); can it be negative; what Resolume says it is for (a slower projector or LED processor, lining screens up with each other, lining picture up with sound in a large room); whether it is frame-based under the hood; any stated cost (memory, performance).
Q2 WHERE the Delay is stored: with the Advanced Output setup / preset (so it persists whatever composition is open) or with the composition? How presets of the Advanced Output are saved and loaded.
Q3 The other rows (Opacity, Brightness, Contrast, Red, Green, Blue): range and meaning, one line each.
Q4 Which OUTPUT KINDS have a Delay: a physical display only, or also NDI, Syphon / Spout, virtual screens, a capture card, DMX / Lumiverse?
Q5 Is the preview monitor inside Resolume delayed by a screen's Delay? Is a recording?
Q6 Any OTHER audio / video offset in Resolume: an audio output delay, a global sync offset, an Ableton Link or MIDI clock offset, an SMPTE offset.
Q7 The BPM section's controls: Tap, Resync, plus / minus, x2, /2, and any NUDGE or "push / pull" that moves the beat a little earlier or later without changing the tempo -- exact behaviour of each, especially what Resync does to the beat phase.
Q8 What Resolume staff or experienced users advise for making picture and sound agree at the mixing desk in a large room (forum threads): which control they use.` },
]

const researchTrack = async t => {
  const r = await agent(`${WHY}
${RESEARCH_RULES}
YOUR TOPIC: ${t.q}
REPORT_FILE: ${RPT}/facts-${t.key}.md (relative to ${MAIN}; absolute: ${MAIN}/${RPT}/facts-${t.key}.md)`, { agentType: 'researcher', model: 'sonnet', effort: 'high', schema: FS, phase: 'Research', label: 'research:' + t.key })
  if (!r) return { key: t.key, sheet: slimF(null) }
  const v = await agent(`ADVERSARIAL SOURCE CHECK of a web research fact sheet (read-only). The sheet: ${MAIN}/${RPT}/facts-${t.key}.md -- read it whole. It will be used to copy Resolume's behaviour into another app, so a claim that is not really in its source is worse than a gap. For EVERY claim labelled DOCUMENTED or STAFF: fetch the cited URL yourself and look for the quoted words; mark it CONFIRMED (the quote is on the page and carries the claim), CITATION-OFF (the page exists and supports the claim but the quote is not verbatim), or WRONG (the quote is not there, or the page says something else -- say what it says). Then try to REFUTE the sheet's three most load-bearing answers (for the transport sheet: the first Beats number Resolume picks, the BPM-Sync Speed steps, what a triggered paused clip does; for the screen sheet: the Delay's range, where it is stored, what Resync does to the beat phase) by searching for a source that says otherwise; default to "refuted" when the only support is a third-party page. Use firecrawl_search / firecrawl_scrape when you have them, else WebSearch / WebFetch. Never fill a gap from memory. ${RELAY}
APPEND a section "## VERIFICATION (independent source check)" to the END of that same file with a bash heredoc (cat >> ; never rewrite the sheet's own text): one line per claim checked with its mark, then the refutation attempts and their outcome. Edit no other file.
RETURN: checked, confirmed, wrong, citation_off, wrong_lines (claim, what_is_true, evidence = the URL), refuted_conclusions, verdict (SOUND / SOUND_WITH_CORRECTIONS / UNRELIABLE).
REPORT_FILE: ${RPT}/facts-${t.key}.md (relative to ${MAIN}; absolute: ${MAIN}/${RPT}/facts-${t.key}.md) -- the sheet you APPEND to; your only write.`, { agentType: 'researcher', model: 'sonnet', effort: 'high', schema: VS, phase: 'Verify', label: 'verify:' + t.key })
  return { key: t.key, sheet: slimF(r), verify: slimV(v) }
}

// ---------- CODE RECON ----------
const RECON = [
  { key: 'outputs', q: `OUTPUT WINDOWS on main, as they are built today (start: ${MAIN}/docs/claude/integration.md "Output windows", ${MAIN}/docs/claude/rendering.md, Pitfalls 37, 40, 57 in ${MAIN}/docs/claude/pitfalls.md; then the code).
Q1 How an output window is created and destroyed; the object that stands for one output and EVERY field of state it holds; how "the same screen" is recognised across launches and across a hot-plug (which identity: a display id, a name, its bounds?).
Q2 The Output menu and the TopBar "Outputs" button: the item list, what a click does, and whether ANY per-output setting or properties window exists today (if none, say so plainly).
Q3 settings.json "outputs": every key of its schema, when it is written, how "Restore Last Outputs" reads it; what else lives in settings.json.
Q4 THE RENDER PATH: where the composition canvas is rendered once per frame (Pitfall 37), in what format and size (bytes per frame at the default canvas and at 1920x1080 and 3840x2160), and how EACH output window gets its picture (a blit from a shared texture? its own GL context? which thread? paced by which display's refresh?). Draw the path as a numbered list from "canvas rendered" to "pixels on output N".
Q5 Every OTHER reader of the canvas and where it taps the path: the in-app preview panel, the video recorder, Syphon, the PNG snapshot, the Eyes capture, record-to-clip if present.
Q6 Frame-history machinery that exists (FrameRingBuffer and any other ring of textures; Pitfalls 19, 20, 35, 54, 60): what each holds, its depth, its resolution, its VRAM budget rule. Is there anything today that shows an OUTPUT a frame other than the newest one?
Q7 How long a rendered frame takes to reach the screen today, as far as the code states or measures it (vsync, swap interval, any stated latency number).
Q8 Tests and probes that cover outputs today (names, what they assert), and the binding test constraints (the screen-safety law: no gate opens an Output window; ${MAIN}/.harmony/RIG-RULES.md): how is output behaviour tested without opening one?
TABLE: every file that an "output properties (Device + Delay per screen)" change would have to touch or respect, with one line on why.` },
  { key: 'syncdial-parts', q: `THE STOPPED SYNC-DIAL BRANCHES, as a source of parts. Boris replaced the dial; nothing of it merges as it stands. Two branches, each in a clean worktree nobody is editing: lane/bf2 at 740b6d6 in ${BF2}, and lane/bf2-keys at 9eab9bd in ${KEYS} (branched from lane/bf2's 68abc16). First check each: git -C <worktree> rev-parse --short HEAD and git -C <worktree> status --short (clean); then read plain files there. Their story: ${MAIN}/.harmony/.reports/s-rta-1003b/rulings-bf2.md (H-1..H-17), the lane reports ${BF2}/.harmony/.reports/s-rta-1003b/bf2-delta.md and bf2-d0.md, ${KEYS}/.harmony/.reports/s-rta-1003b/bf2-s4b.md, and ${BF2}/.harmony/.reports/s-rta-1002b/bf2.md.
Q1 The lane's OWN change against main: git -C ${MAIN} diff --stat $(git -C ${MAIN} merge-base ${PIN} 740b6d6) 740b6d6 -- src tests docs CMakeLists.txt CLAUDE.md .harmony (and the same for 68abc16..9eab9bd). A table: file, lines added / removed, which part of the dial it belongs to.
Q2 WHAT THE DIAL IS, as built: what "later" delays (which data exactly: the analysis features? the beat clock? rendered frames? at which point of the pipeline, in which class) and what "earlier" is (BeatLead: what it moves and how); the room list and where it is stored; the REST routes; the Sync binding targets; what the take recorder does with the dial.
Q3 Does the lane contain ANY delay of rendered PICTURE frames (a ring of frames shown late) that a Delay per output screen could reuse? Or only a delay of signals in time? Answer plainly with the class names.
Q4 CARRY OR DROP. For every separable piece of the two branches, one row: what it is, its commit(s), DEPENDS-ON-THE-DIAL (drop) or USEFUL-ON-MAIN-WITHOUT-THE-DIAL (carry candidate), and what it would take to carry it alone (does it apply to main without the dial's files?). Include at least: fixes to files that main has too (docs/claude/testing-eyes.md kill advice, APP-INVENTORY's test count, .harmony/gotchas.md), the probe rig (.harmony/probe-sync*.py / .sh, probe-quit-ours.sh and its self-test), the take-origin measurement of the probe (rulings-bf2.md H-14: a take started with a file through REST holds whole device blocks of silence before the file's first frame), the [timing] test and its RUN_SERIAL change, the MIDI-learn overlay changes (titleText, selectAt, the refusal), bindingIsLive, the three TEST-SERVER routes, test_binding_sync_nudge, the audio_devices debug route, the BeatLead changes.
Q5 Things the lane's plans promised that are NOT the dial and are NOT built yet (the music-beat wheel of stage S5a, the Gain 140 px of S5b, the learn title's em dash of H-16): where each is specified (file + section), in Boris's words where he asked for it.
Q6 Defects of MAIN that the lane fixed in passing and that stay open on main if the lane is dropped (each with the lane commit that fixes it).` },
  { key: 'beat-controls', q: `MOVING THE BEAT BY HAND, and a manual. Boris: "We need to do something smart where we can move the beat forward or back to get it to match the image exactly but that's something that user can do. We can just put that in our manual". On main (start: ${MAIN}/docs/claude/effects.md "Manual BPM Mode", ${MAIN}/docs/claude/performance-controls.md, Pitfalls 30, 32, 38, 42, 48; then the code).
Q1 EVERY control that sets the tempo or moves the beat's phase today: Tap, Resync, a typed BPM, x2 and /2 if any, Ableton Link, REST and OSC routes, key / MIDI binding targets. For each: where it is on screen (its label, its tooltip text verbatim), and EXACTLY what it does to the tempo and to the beat phase (cite the code).
Q2 Is there ANY control today that moves the beat a small step earlier or later without changing the tempo (a nudge)? If none, say so plainly.
Q3 What a user would do TODAY, step by step with the real control names, to make the beat the app shows agree with the beat he hears -- and what he cannot do today.
Q4 What shows the beat on screen today (a beat indicator, a wheel, a flash, a number): where, and what drives it.
Q5 What "the beat" drives: every consumer of the beat clock (quantised clip fires, routines, autopilot, beat-synced slideshows, effects that read beatPhase, Link) -- so a planner knows what moves when the beat is moved.
Q6 Any user-facing help that exists: a manual, a README for users, in-app help, the tooltip system (how many controls carry a tooltip), a Help menu. Search the whole repo (git -C ${MAIN} ls-files), not only docs/. If there is no user manual, say so plainly and list what exists that one could be started from.` },
]

const reconTrack = async t => {
  const r = await agent(`${WHY}
${CODE_RULES}
YOUR TOPIC: ${t.q}
${REPORT_FMT}
REPORT_FILE: ${RPT}/facts-${t.key}.md (relative to ${MAIN}; absolute: ${MAIN}/${RPT}/facts-${t.key}.md)`, { agentType: 'general-purpose', model: 'sonnet', effort: 'high', schema: FS, phase: 'Recon', label: 'recon:' + t.key })
  if (!r) return { key: t.key, sheet: slimF(null) }
  const v = await agent(`ADVERSARIAL CHECK of a code fact sheet (read-only). The sheet: ${MAIN}/${RPT}/facts-${t.key}.md -- read it whole. A planner will build on it; your job is to find the lines that are WRONG before he does. Same reading rules as its author: main = ${MAIN} at commit ${PIN} (plain files under src, tests, docs; check git -C ${MAIN} rev-parse --short HEAD first); the stopped branches in the clean worktrees ${BF2} (740b6d6) and ${KEYS} (9eab9bd). Never build, run, launch or edit anything except the one append below; never cd; absolute paths. ${RELAY}
(1) Pick the 15 VERIFIED lines a planner would lean on hardest (mechanisms, "there is no X today", sizes, thread and ownership facts, file lists). Re-read each cited file:line yourself: mark CONFIRMED, CITATION-OFF (true, but the citation points elsewhere -- give the right one), or WRONG (say what the code says, with file:line). For every "there is no X" line, run your own search for X (two different patterns) before you confirm it.
(2) Try to REFUTE the sheet's three main conclusions by looking for code that contradicts them; default to "refuted" if you find a path the sheet did not read.
(3) Name anything a planner needs for this topic that the sheet does not answer.
APPEND a section "## VERIFICATION (independent re-read)" to the END of that same file with a bash heredoc (cat >> ; never rewrite the sheet's own text). Edit no other file.
RETURN: checked, confirmed, wrong, citation_off, wrong_lines (claim, what_is_true, evidence = file:line), refuted_conclusions, verdict.
REPORT_FILE: ${RPT}/facts-${t.key}.md (relative to ${MAIN}; absolute: ${MAIN}/${RPT}/facts-${t.key}.md) -- the sheet you APPEND to; your only write.`, { agentType: 'general-purpose', model: 'sonnet', effort: 'high', schema: VS, phase: 'Verify', label: 'verify:' + t.key })
  return { key: t.key, sheet: slimF(r), verify: slimV(v) }
}

// ---------- SAVES INVENTORY (three blind sweeps -> one sheet -> critic) ----------
const IS = { type: 'object', properties: { items: { type: 'array', items: { type: 'object', properties: {
  what: { type: 'string' }, trigger: { type: 'string' }, automatic: { type: 'boolean' }, destination: { type: 'string' }, format: { type: 'string' },
  write_site: { type: 'string' }, failure_shown_today: { type: 'string' }, failure_site: { type: 'string' }, label: { type: 'string' } },
  required: ['what', 'trigger', 'automatic', 'destination', 'write_site', 'failure_shown_today', 'label'] } }, not_covered: { type: 'array', items: { type: 'string' } } }, required: ['items'] }
const SWEEPS = [
  { key: 'by-write-call', how: 'BY THE WRITE CALL. Search src for every way the app puts bytes on disk: juce::File (replaceWithText, replaceWithData, create, copyFileTo, moveFileTo, createDirectory), juce::FileOutputStream, TemporaryFile, PngWrite (Pitfall 46), std::ofstream / fopen / fwrite, the FFmpeg writer (avio_open, av_write_frame), any AudioFormatWriter / WAV writer, juce::PropertiesFile, any log file. Start from each call and work OUT to what the user did (or what timer or shutdown path ran) to reach it.' },
  { key: 'by-user-action', how: 'BY WHAT THE USER PRESSES. Walk every menu (the menu bar model: Composition, Output, Shortcuts, any other), every button whose label says Save, Export, Record, Stop, Snapshot, Capture, Learn, Add, Duplicate, and every REST / OSC route that saves something. Start from each action and work IN to the file it writes. Include saves the user never sees as a file dialog (a preset slot, a routine pad, a binding learned).' },
  { key: 'by-destination', how: 'BY WHERE IT LANDS. Find every directory and file name the app writes to: ~/Library/Audio-DNA (settings.json and anything beside it), the Audio store, the takes folder, recordings, presets, decks, compositions, thumbnails or caches, temp files, anything beside the composition file. For each destination list everything that writes there, and say for each whether it happens WITHOUT the user pressing a save (automatically: on change, on quit, on a timer) -- Boris asked specifically what the app keeps by itself.' },
]
const savesTrack = async () => {
  const sweeps = await parallel(SWEEPS.map(s => () => agent(`${WHY}
READ-ONLY SWEEP for an inventory of EVERYTHING AUDIO-DNA SAVES (on main). Boris: "we should list all the things that can be saved". You are one of three blind sweeps, each searching a different way; be exhaustive along YOUR way and do not try to cover the others.
${CODE_RULES.replace('Write it with the Write tool or a bash heredoc to the literal path given as REPORT_FILE.', 'You write NO file: your result is the structured return only.').replace('METHOD: write a SKELETON of the report (the question list with empty answers) to REPORT_FILE within your first ~10 tool calls, then fill it and rewrite it as you go, so that an interrupted run still leaves a usable file.', 'METHOD: list candidates first (grep), then confirm each by reading its write site and its failure path.')}
YOUR WAY: ${s.how}
FOR EACH THING SAVED return one item: what (plain words: "the show", "a deck", "a recorded take", ...); trigger (the user action with its on-screen label, or the automatic event); automatic (true when no save is pressed); destination (the path pattern); format; write_site (file:line of the write); failure_shown_today (the EXACT text the user sees when the write fails, or "nothing" -- read the failure branch, do not assume); failure_site (file:line of that branch); label (VERIFIED / INFERRED). not_covered = areas of your way you could not finish. Up to 60 items; do not merge two different saves into one item.`, { agentType: 'general-purpose', model: 'sonnet', effort: 'high', schema: IS, phase: 'Saves', label: 'saves:' + s.key }).then(x => ({ key: s.key, items: (x && x.items) || [], not_covered: (x && x.not_covered) || [] }))))
  const got = sweeps.filter(Boolean)
  const all = got.flatMap(g => g.items.map(i => Object.assign({ sweep: g.key }, i)))
  log('saves: ' + got.map(g => g.key + '=' + g.items.length).join(' ') + ' (total ' + all.length + ', before merging)')
  if (!all.length) return { key: 'saves', sheet: slimF(null) }
  const payload = JSON.stringify({ items: all, not_covered: got.flatMap(g => g.not_covered.map(n => g.key + ': ' + n)) })
  log('saves: payload to the merger = ' + payload.length + ' characters (passed whole)')
  const r = await agent(`${WHY}
${CODE_RULES}
YOUR TASK: MERGE three blind sweeps into ONE inventory of everything Audio-DNA saves (main at ${PIN}), and RE-READ every write site and failure branch yourself before a row is labelled VERIFIED (the sweeps can be wrong; two sweeps agreeing is not proof). Where sweeps disagree, read the code and say which is right. Cross-check against the earlier notices inventory ${MAIN}/.harmony/.reports/s-rta-1003/facts-notices.md and ${MAIN}/.harmony/.reports/s-rta-1003b/ruling-notices.md (its list of save-failure alerts) -- a save they name that the sweeps missed is a gap to fill.
THE SWEEPS (whole): ${payload}
THE SHEET: 1 THE TABLE, one row per thing saved, grouped: (A) saved when the user presses a save, (B) recordings, (C) kept by the app BY ITSELF without a save being pressed, (D) written for the app's own use (caches, logs, temp). Columns: what (plain words) | how the user causes it (on-screen label) or which automatic event | where it lands | what the user sees today when it fails (exact text or "nothing") | write site | failure site | label. 2 For group C: for each item, what would be lost if it were kept ONLY with a saved show (Boris asked about this). 3 Items the unmerged branches would add (the sync dial's room list): one line, marked "not on main". 4 WHAT A PLANNER MUST NOT ASSUME. 5 UNKNOWN. Then a PLAIN-WORDS LIST at the very end, headed "FOR BORIS", one short line per thing saved in groups A, B and C, no file names, no code words (this list is shown to him).
${REPORT_FMT.replace('REPORT FORMAT: 1 QUESTIONS ANSWERED (one block per question: the answer in plain words, then the evidence lines, each labelled); 2 TABLES where asked; 3 WHAT A PLANNER MUST NOT ASSUME (traps you met: a doc that disagrees with the code, a name that misleads, a rule in CLAUDE.md / docs/claude/pitfalls.md that binds the area -- cite the Pitfall number); 4 UNKNOWN (each with the cheapest test that would settle it). No design proposals.', '')}
In the structured return, answers = one entry per GROUP (q = the group, answer = the count of rows and the three most important ones).
REPORT_FILE: ${RPT}/facts-saves.md (relative to ${MAIN}; absolute: ${MAIN}/${RPT}/facts-saves.md)`, { agentType: 'general-purpose', model: 'sonnet', effort: 'high', schema: FS, phase: 'Saves', label: 'saves:merge' })
  if (!r) return { key: 'saves', sheet: slimF(null), swept: all.length }
  const v = await agent(`COMPLETENESS AND CORRECTNESS CRITIC of an inventory (read-only). The sheet: ${MAIN}/${RPT}/facts-saves.md -- "everything Audio-DNA saves", main at ${PIN} (plain files under ${MAIN}/src; check git -C ${MAIN} rev-parse --short HEAD first). It goes to the owner as THE list, and a planner removes on-screen failure messages by it: a save that is MISSING from it is a save whose failure will be silent without anyone having decided so. Never build, run, launch or edit anything except the one append below; never cd; absolute paths. ${RELAY}
(1) HUNT FOR MISSING SAVES with searches the sheet's authors may not have run: grep src for every string literal ending in a file extension (.json, .png, .wav, .mov, .mp4, .adna, .txt, .xml, .log, .bin ...), for "save" / "export" / "write" / "store" / "persist" in function names, for juce::File::getSpecialLocation, for destructors and shutdown paths that write. Every write you find that is not a row of the sheet = a MISSING row (give what, trigger, destination, write site).
(2) RE-READ the failure column of the 12 rows a user is most likely to hit (the show, a deck, a take, a video recording, a preset, FX Save, a routine, settings): is the text exactly what the code shows, and is "nothing" really nothing? Mark CONFIRMED / WRONG.
(3) Check group C (kept by the app by itself) hardest: is each really written without a save being pressed, and is anything automatic missing?
APPEND a section "## VERIFICATION (completeness critic)" to the END of that same file with a bash heredoc (cat >> ; never rewrite the sheet's own text): the MISSING rows as table rows in the sheet's own column order, then the re-read marks. Edit no other file.
RETURN: checked, confirmed, wrong, citation_off, wrong_lines (claim, what_is_true, evidence = file:line; put each MISSING row here too, claim = "MISSING: <what>"), refuted_conclusions, verdict.
REPORT_FILE: ${RPT}/facts-saves.md (relative to ${MAIN}; absolute: ${MAIN}/${RPT}/facts-saves.md) -- the sheet you APPEND to; your only write.`, { agentType: 'general-purpose', model: 'sonnet', effort: 'high', schema: VS, phase: 'Verify', label: 'verify:saves' })
  return { key: 'saves', sheet: slimF(r), verify: slimV(v), swept: all.length }
}

phase('Research')
const out = await parallel([
  ...RESEARCH.map(t => () => researchTrack(t)),
  ...RECON.map(t => () => reconTrack(t)),
  () => savesTrack(),
])
const res = out.filter(Boolean)
log('sheets: ' + res.map(x => x.key + '=' + (x.sheet && x.sheet.status) + '/' + (x.verify ? x.verify.verdict : 'unverified')).join(' '))
if (res.length < out.length) log('DROPPED: ' + (out.length - res.length) + ' track(s) returned nothing')
return { sheets: res }
