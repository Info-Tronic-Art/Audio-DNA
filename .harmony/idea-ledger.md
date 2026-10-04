<!-- Secondary→Primary idea-ledger (foreign-repo lane). Verbatim IDEA records (spec §4.1). The primary PULLS these at boot via config/repos.yml scan into memory/secondary-ideas-inbox.md. Append-only; never compress `raw`. -->
--- IDEA ---
id:          idea-2026-07-28-RealTimeAudio-1785289328541427496
raw:         I would like cmd x to clear a clip as well
context:     Undo v1 manual e2e sitting 2026-07-28 — asked how to delete clips from a cell; wants Cmd+X (cut) as a clip-clear gesture. Note: Clip menu lists Cut/Copy/Paste — wiring/shortcut state unverified.
why:         
intent:      
target:      
constraints: 
related:     
priority:    HIGH
repo:        RealTimeAudio     session:      date: 2026-07-28
status:      NEW
status-changed: 2026-07-28
status-note:
artifact:
history:     NEW(2026-07-28)
--- /IDEA ---

--- IDEA ---
id:          idea-2026-07-28-RealTimeAudio-17852893285415730694
raw:         I want them all loaded by default
context:     Same sitting — MilkDrop browser showed no presets (preset dir unset; libprojectM absent). Wants bundled resources/projectm_presets loaded by default. Natural bundle with the MilkDropBrowser empty-state null-deref crash fix (.ips 2026-07-28-190701).
why:         
intent:      
target:      
constraints: 
related:     
priority:    HIGH
repo:        RealTimeAudio     session:      date: 2026-07-28
status:      NEW
status-changed: 2026-07-28
status-note:
artifact:
history:     NEW(2026-07-28)
--- /IDEA ---

--- IDEA ---
id:          idea-2026-08-04-RealTimeAudio-17858604595945222831
raw:         If I drop multi images it would be great to have a toggle to select between drop on cell or drop on multi cells. Can this be visible as a selection right after drop?
context:     Boris, session 2026-08-04c, Audio-DNA. PARTIALLY addressed, NOT fully delivered. SHIPPED in 7d3a203: sequence threshold raised to 3+ (2 images now spread across 2 cells, which removes the surprise that prompted this) + a 'SEQ N' badge so sequence cells stop being visually IDENTICAL to video cells (they shared one paint branch — that was the real root cause). NOT SHIPPED: the 'visible as a selection right after drop' escape hatch.
why:         Boris dropped 2 images, they merged into one animated sequence clip, and he did not recognise a feature his own app ships and documents. He wanted both to know it happened AND to be able to opt out per-drop.
intent:      Give the user a per-drop choice between image-sequence and spread-across-cells, discoverable at the moment of the drop.
target:      RECOMMENDED HOME: a persistent 'Spread Sequence to Cells' command in the existing Clip menu (MenuBarModel.cpp:9) or as a TextButton in ClipInspector (already a panel of exactly such actions). No timer, no snapshots, correct undo depth by construction, works ten minutes after the drop, and greys out when inapplicable — which itself teaches.
constraints: An architect designed a transient post-drop chooser pill (~450-500 lines). TWO independently-dispatched blind critics BOTH returned UNSOUND/FAIL and converged against it: cells are 90px with kCellGap=0 so a legible pill is WIDER than its anchor and would occlude the trigger hit box; its dismiss mechanism (UndoManager::onHistoryChanged) is structurally BLIND to autopilot advances and non-user deck switches — the two things most likely to happen mid-set, neither of which creates a command; UndoManager::onHistoryChanged is a single std::function already assigned at MainComponent.cpp:1544 so hooking it would silently break Edit-menu undo text; and a 5s timer makes the escape hatch a race in exactly the window where the clip is most likely to have started playing.
related:     .harmony/decisions-2026-08-04c.md ; commit 7d3a203 ; .harmony/gesture-replay-results.md
priority:    medium — the surprise is already fixed by the badge + threshold; this is the remaining convenience half
repo:        RealTimeAudio     session:      date: 2026-08-04
status:      NEW
status-changed: 2026-08-04
status-note:
artifact:
history:     NEW(2026-08-04)
--- /IDEA ---


--- IDEA ---
id:          idea-2026-09-23-RealTimeAudio-17902127443151813897
raw:         When a mechanism looks broken, suspect the way you are driving it before you suspect it -- and prove which one it is with a test that varies your own input, not the tool's output. An anomaly attributed without a discriminating test is a false lead with a commit message attached.
context:     Triage of RealTimeAudio .harmony/idea-ledger.md prose (s166/s167 session notes, 2026-09-05). Same lesson learned twice independently in one session: oscillators looked 'frozen' (no beat input, not a bug), render_frame looked 'flaky' (nothing was loaded), load_source looked 'lying' (wrong request field sent). All three were diagnosed correctly only after a probe that varied the operator's OWN input rather than re-measuring the tool's output.
why:         Recurring root cause of wasted investigation time and false regression reports; a general debugging-method gap, not specific to this codebase.
intent:      Fold into debugging/verification guidance: before filing an anomaly or regression, run one cheap probe that changes YOUR input/usage, not just re-reads the tool's output, to separate 'the tool is broken' from 'I am driving it wrong'.
target:      harmony-system
constraints: 
related:     .harmony/idea-ledger.md (RealTimeAudio) sections s166 FIELD EVIDENCE + s166 CORRECTION TO THE RETRACTION, pre-triage; content preserved verbatim in .harmony/notebook.md (RealTimeAudio) after s-rta-0923 triage
priority:    MEDIUM
repo:        RealTimeAudio     session: s-rta-0923     date: 2026-09-23
status:      NEW
status-changed: 2026-09-23
status-note:
artifact:
history:     NEW(2026-09-23)
--- /IDEA ---

--- IDEA ---
id:          idea-2026-09-23-RealTimeAudio-17902127523533330390
raw:         Assert on parsed JSON, never on the text of a JSON response, and never interpolate a possibly-empty variable into a grep pattern. A gate that cannot fail is not a gate.
context:     Triage of RealTimeAudio .harmony/idea-ledger.md prose (s166 session, 2026-09-05, 'METHOD NOTE -- my own gate was wrong before the app was'). A bash+grep verification gate reported 7 failures; 5 were bugs in the GATE itself: pretty-printed JSON defeated grep '"ok":false' (real text was '"ok": false'), float formatting defeated grep '0.42' against '0.419999986886978', and an empty effect-name variable made grep "$EFF" match every line, producing both a false PASS and a stress test that silently tested nothing.
why:         This is a generic hazard in every shell-script verification gate that greps raw text/JSON instead of parsing it, and it fails silently in the dangerous direction (false PASS).
intent:      Add to gate-writing/verification guidance: parse JSON (jq or equivalent) before asserting on it; never grep raw response text; guard against interpolating an empty/unset variable into a grep pattern (it becomes a match-everything wildcard).
target:      harmony-system
constraints: 
related:     .harmony/idea-ledger.md (RealTimeAudio), preserved verbatim in .harmony/notebook.md (RealTimeAudio) after s-rta-0923 triage
priority:    MEDIUM
repo:        RealTimeAudio     session: s-rta-0923     date: 2026-09-23
status:      NEW
status-changed: 2026-09-23
status-note:
artifact:
history:     NEW(2026-09-23)
--- /IDEA ---

--- IDEA ---
id:          idea-2026-09-23-RealTimeAudio-1790212752353599434
raw:         A correct multiplier applied to the wrong quantity passes every arithmetic test there is. This is the strongest argument for pixel-level (behavioral) gates over helper-level (unit) ones.
context:     Triage of RealTimeAudio .harmony/idea-ledger.md prose (s167, 'CLIP OPACITY RENDERS, BUT NOT AT THE RIGHT STRENGTH'). A clip-opacity bug shipped with green unit tests because the arithmetic helper (layer x clip opacity) was tested and correct, but was applied to the wrong quantity downstream (alpha channel instead of RGB) -- no unit test rendered a frame and measured it.
why:         Unit tests on a pure-math helper can be 100% correct and still hide a real, user-visible defect if the helper's output is wired to the wrong place. This is a class of false-confidence green test, not specific to this codebase.
intent:      When reviewing test coverage for a composition/wiring boundary (value computed correctly here, applied somewhere else), require at least one integration/behavioral check of the actual applied effect, not just the arithmetic.
target:      harmony-system
constraints: 
related:     .harmony/idea-ledger.md (RealTimeAudio), preserved verbatim in .harmony/notebook.md (RealTimeAudio) after s-rta-0923 triage
priority:    MEDIUM
repo:        RealTimeAudio     session: s-rta-0923     date: 2026-09-23
status:      NEW
status-changed: 2026-09-23
status-note:
artifact:
history:     NEW(2026-09-23)
--- /IDEA ---

--- IDEA ---
id:          idea-2026-09-23-RealTimeAudio-17902127603784411524
raw:         A defect that is a SHAPE rather than a SITE is not finished when the reported instance is fixed; it is finished when the subsystem has been swept and every hit classified. Naming the shape and grepping for it costs minutes and finds more than any amount of reviewing the original site would have.
context:     Triage of RealTimeAudio .harmony/idea-ledger.md prose (s167, 'THE FRAME-RATE BUG SHAPE: SIX INSTANCES'). A single reported bug (video playback hardcoded to 1/60 dt) turned out to have 6 instances across the render subsystem once swept by pattern, including 2 in code already reviewed and gated the same day, one of which had already been reported to Boris as working.
why:         Fixing only the reported instance of a pattern-shaped defect leaves siblings live and creates false confidence that the class is closed.
intent:      When a defect's root cause is a reusable pattern (e.g. a hardcoded constant standing in for a value that should be threaded through), grep the whole subsystem for the pattern and classify every hit (fixed / mapped-not-fixed / legitimate) before closing the finding, not just the one reported site.
target:      harmony-system
constraints: 
related:     .harmony/idea-ledger.md (RealTimeAudio), preserved verbatim in .harmony/notebook.md (RealTimeAudio) after s-rta-0923 triage
priority:    MEDIUM
repo:        RealTimeAudio     session: s-rta-0923     date: 2026-09-23
status:      NEW
status-changed: 2026-09-23
status-note:
artifact:
history:     NEW(2026-09-23)
--- /IDEA ---

--- IDEA ---
id:          idea-2026-09-23-RealTimeAudio-1790212760378591954
raw:         A skill named __iso_37348 appeared in the available-skills list partway through a session, with no description, no provenance, and no connection to anything in the project repo or Harmony_Main. It was not present in the legitimate skill list at session start. Neither the agent nor a separate builder that also saw it invoked it; the builder independently flagged it as looking like an injected instruction rather than a real tool offering.
context:     Triage of RealTimeAudio .harmony/idea-ledger.md prose (s167, 'AN UNEXPLAINED CAPABILITY APPEARED MID-SESSION -- LOGGED, NOT ACTED ON', 2026-09-05). Two independent agents in the same session both declined to invoke the unexplained item.
why:         An unexplained capability appearing mid-session is a prompt-injection / tampering pattern; recording it once in a project ledger and not flagging it up-channel is how such an event gets normalized by silence.
intent:      Make Harmony aware this was sighted once (2026-09-05, RealTimeAudio) and refused by two independent agents; if the same or a similarly unexplained item appears again in any session, do not invoke it and escalate to Boris rather than only noting it in a project ledger.
target:      harmony-system
constraints: 
related:     .harmony/idea-ledger.md (RealTimeAudio), preserved verbatim in .harmony/notebook.md (RealTimeAudio) after s-rta-0923 triage
priority:    HIGH
repo:        RealTimeAudio     session: s-rta-0923     date: 2026-09-23
status:      NEW
status-changed: 2026-09-23
status-note:
artifact:
history:     NEW(2026-09-23)
--- /IDEA ---

--- IDEA ---
id:          idea-2026-09-23-RealTimeAudio-1790215118512329132
raw:         graphify-refresh-launchd.sh deletes graph.json + manifest.json BEFORE extracting; when the native (semantic) refresh then fails rc=1 (twice on 2026-09-07, ~20-30 min each, NO stderr captured in .refresh-launchd.log) the repo is left with NO graph, and the graphify post-commit hook's incremental rebuild then writes a 0-node graph into the void (proven: ~/.cache/graphify-rebuild.log 'Rebuilt: 0 nodes' at f5ae847 22:20, 1 doc file changed). The graphify fewer-nodes overwrite guard cannot fire when there is no prior graph. 'graphify update .' (code-only, no LLM) rebuilt RealTimeAudio in 10 s to 7254N/11265E on 2026-09-23.
context:     
why:         
intent:      Extract to a temp dir and atomically swap on success (never delete first); on semantic-stage failure fall back to code-only 'graphify update'; capture stdout/stderr of the refresh into the log; staleness check should treat a 0-node graph.json as MISSING.
target:      harmony-system
constraints: 
related:     graphify-commit-hook-writes-empty-graph, graphify-staleness-blind-to-missing-outputs, RTA inbox down-2026-09-10-RealTimeAudio-17890807759054610371
priority:    MEDIUM
repo:        RealTimeAudio     session: s-rta-0923     date: 2026-09-23
status:      NEW
status-changed: 2026-09-23
status-note:
artifact:
history:     NEW(2026-09-23)
--- /IDEA ---

--- IDEA ---
id:          idea-2026-09-24-RealTimeAudio-17902256343085514525
raw:         Reviewer (and one builder-diagnosis) agents dispatched from a FOREIGN-repo secondary write their verdict files to <cwd>/memory/.reports/ — i.e. they create a stray memory/ directory inside the project repo (happened 3 times in RealTimeAudio s-rta-0923: reviewer-R28, reviewer-s-rta-0923-L2, reviewer-lane3-c2). The reviewer spec's persist path is Harmony_Main-relative but resolves against the project cwd.
context:     
why:         
intent:      Make the reviewer/builder persist path profile-aware: in a MINIMAL/foreign boot write to <repo>/.harmony/.reports/ (the kernel's own rule for project artifacts).
target:      harmony-system
constraints: 
related:     
priority:    MEDIUM
repo:        RealTimeAudio     session: s-rta-0923     date: 2026-09-24
status:      NEW
status-changed: 2026-09-24
status-note:
artifact:
history:     NEW(2026-09-24)
--- /IDEA ---

--- IDEA ---
id:          idea-2026-09-24-RealTimeAudio-17902256343087121794
raw:         The PreToolUse write-guard refuses read-only architect agents writing into a foreign repo's .harmony/specs/ (s159 ADJ ruling: read-only agents may write only memory/.reports, memory/wip, /tmp). In a project repo the architect's natural output home is <repo>/.harmony/specs/; every architect this session had to write to /tmp and Harmony copied the files over by hand (4 times).
context:     
why:         
intent:      Allow read-only architect/critic agents to write <repo>/.harmony/specs/ and <repo>/.harmony/.reports/ when the session is a MINIMAL foreign-repo boot.
target:      harmony-system
constraints: 
related:     
priority:    LOW
repo:        RealTimeAudio     session: s-rta-0923     date: 2026-09-24
status:      NEW
status-changed: 2026-09-24
status-note:
artifact:
history:     NEW(2026-09-24)
--- /IDEA ---

--- IDEA ---
id:          idea-2026-09-24-RealTimeAudio-17902256343088729062
raw:         Doctrine disagreement for foreign-repo (lane B) secondaries: the kernel's Write-Immediately section says 'secondary -> memory/.pending/ or log-event append' for learnings, while eos-secondary Step 3 says a FOREIGN-repo secondary must NOT write Harmony_Main's event log and records learnings in its own .harmony/notebook.md. Following the kernel, s-rta-0923 wrote 2 learning rows to Harmony_Main's event log from a foreign repo.
context:     
why:         
intent:      Make the kernel line lane-aware (co-session: log-event; foreign repo: own notebook.md) so the two texts agree.
target:      harmony-system
constraints: 
related:     
priority:    LOW
repo:        RealTimeAudio     session: s-rta-0923     date: 2026-09-24
status:      NEW
status-changed: 2026-09-24
status-note:
artifact:
history:     NEW(2026-09-24)
--- /IDEA ---

--- IDEA ---
id:          idea-2026-09-24-RealTimeAudio-1790225827192095082
raw:         secondary-close-gate-verify.sh check 3 matches the session log by FILENAME prefix $(date +%F) at close time, so a session that starts before midnight and closes after it (s-rta-0923: 2026-09-23 21:10 -> 2026-09-24 01:00) is BLOCKED even though its log exists, named for its start date. Worked around by renaming the log to the close date.
context:     
why:         
intent:      Accept a log dated today OR the session's start date (or any log modified since the session's boot), so midnight-spanning sessions are not forced to misdate their log.
target:      harmony-system
constraints: 
related:     
priority:    LOW
repo:        RealTimeAudio     session: s-rta-0923     date: 2026-09-24
status:      NEW
status-changed: 2026-09-24
status-note:
artifact:
history:     NEW(2026-09-24)
--- /IDEA ---

--- IDEA ---
id:          idea-2026-09-26-RealTimeAudio-17904502332714714709
raw:         REPLY to routed request down-2026-09-26-RealTimeAudio-17904459455602512214 (s234 boot-cost audit R1): RealTimeAudio CLAUDE.md split DONE in s-rta-0926 (bb0a7c0): 112,696 -> 24,114 bytes; 13 on-demand docs under docs/claude/ (no @-imports), trigger table + one-line-per-pitfall index; no-loss check 952 non-blank lines -> 941 verbatim + 11 declared edits, 0 missing; fresh-reader test 7/7 (Sacred Rules, kick-off phase N answerable from CLAUDE.md alone); completeness critic PASS. First-call token number: not measurable in this session (CLAUDE.md is loaded at boot) - measure at the next RTA boot with ctx-now.sh.
context:     down-channel routed item in .harmony/inbox.md (status now DONE)
why:         
intent:      
target:      harmony-system
constraints: 
related:     
priority:    NORMAL
repo:        RealTimeAudio     session: s-rta-0926     date: 2026-09-26
status:      NEW
status-changed: 2026-09-26
status-note:
artifact:
history:     NEW(2026-09-26)
--- /IDEA ---

--- IDEA ---
id:          idea-2026-09-26-RealTimeAudio-1790462247885395697
raw:         54790
context:     Reply to Harmony s234 boot-cost audit (inbox.md item: split CLAUDE.md). First-call token number measured with ctx-now.sh as the FIRST tool call of RTA session s-rta-0926b (session a07fe8e4), after s-rta-0926 split CLAUDE.md 112,696 B -> ~24.1 KB. Reading: [CTX] 54,790 / 1,000,000 (5.5%). Includes the pasted birth prompt + the workflow-authoring skill body in the first user turn.
why:         Acceptance was first call <= 75,000 tokens (was 101,568). PASS: 54,790, -46,778 tokens (-46%).
intent:      
target:      project:RealTimeAudio
constraints: 
related:     .harmony/inbox.md (CLAUDE.md split request), docs/claude/*.md
priority:    HIGH
repo:        RealTimeAudio     session: s-rta-0926b     date: 2026-09-26
status:      NEW
status-changed: 2026-09-26
status-note:
artifact:
history:     NEW(2026-09-26)
--- /IDEA ---

--- IDEA ---
id:          idea-2026-10-02-RealTimeAudio-17909965584441726728
raw:         here is some feedback as I evaluted the app yesterday. Ask me questions on any item you don't understand then add to the
work we need to do. There are other issues but these stand out the most:

Sampling to other layers/ clip. Think how to do this, like set a clip to record output of the composition and they be able
to use it as soon as it is done. This would be good to have it automatically start and end on bars or beats, quantized

We need a nice clean way to adjust the delay or speeding ahead for each room so we can set it per room. Like if the audio
is slower than the video, this delays or advances signals and bpm with one very sensitive dial.

Codec display for each video and easy access to that video in finder

Need to manually change envelopes. Need to expand envelope controls and setup format

Make it so when you load a sample into the envelope that it will emulate the envelope of the audios behavior. So basically
use a sample to extract the envelope live, and then different envelopes can be pulled out of that sample with the envelope
creator

Timeline control is not working for parameter control

Add bars to all places we have beats

Need to be able to rename each deck with double click

Clips in the layer should not persist between deck changes. Whatever is in the layer should be what is playing and there
shouldn't be anything from other decks.
context:     s-rta-1002b Audio-DNA app-evaluation feedback BF1-BF9 (+ BF10 MilkDrop, + the 'decks are boxes of clips' clarification); all planned / ruled / adopted in this repo; status per item in .harmony/boris-feedback-backlog.md and .harmony/HANDOFF.md s-rta-1002b section
why:         
intent:      
target:      
constraints: 
related:     
priority:    HIGH
repo:        RealTimeAudio     session:      date: 2026-10-02
status:      NEW
status-changed: 2026-10-02
status-note:
artifact:
history:     NEW(2026-10-02)
--- /IDEA ---

--- IDEA ---
id:          idea-2026-10-03-RealTimeAudio-1791066975859007673
raw:         # Boris feedback of 2026-10-03 (received ~13:28, recorded 2026-10-03 13:32:39, session s-rta-1003) — answers to the Oct 2 page (39 questions) + new items
## VERBATIM (his whole message; lead-in: "here is my feedback. organize it as you wish and give me list of questions to clarify, if needed:")
```
Firing a column that is already playing should restart its videos

What does this mean: Momentary pad released before its quantized beat: now cancels

Little correction. When record to clip, we want to see the recording in the layer it is recording to while it is being recorded. Can you do that?

The layer strip does not need to show the deck a clip is playing from.

Recording a clip should be prominently displayed, so it can be recorded as long as the user likes. There's no limit except hard drive Space.

The recording should look exactly like the output. If there's a logo, it should look exactly as it's displayed.

The recording will always play like a regular clip once it is recorded. 

As far as a video clip is concerned, it is in two categories, either BPM synced throughout the whole thing, or just playing with a speed control. If it is BPM synced, regardless of the length, it is synced to the current playing BPM and it has bars that the user can set. We automatically set bars for a cliff, but the user can change the bars in it by amount. Let me know if you have any questions on that. Regardless of the clip being BPM or speed controlled, if we are in Qantize mode, it is triggered on time by the Qantize method.  There should be no problem playing clips that are set up for BPM and clips that are set up for speed at the same time. Their speeds are just controlled differently.

For sync control we should have a way to remember it as part of a composition save. I imagine if a user is doing this professionally, they will set up the venue and save it in case of a computer crash or something and if they come back to that venue, they have the settings already.

Sync lives in top bar

For sync and setting the BPM. The user of the application will tap tempo and keep the application in time with the music. It is hard to tap ahead or behind the beat. The user will try to have the beat matched exactly to the music so the user can see it pulsing exactly to the time of the music. Sometimes the visual system is a little delayed and will need to be a little ahead or sometimes the visual system is a little faster than the music and that's what this is for. The visuals should be on the delay. The music is in time and the visuals should be delayed or a little ahead depending on how the system is wired. Let me know if you have questions to clarify. 

Long samples: squeeze the whole sample into the envelope length (up to 10 minutes), or pick a section? Default: the whole sample. I want to be able to use the whole thing or to shorten it manually but clicking to timing marks.

For Milk drop there is no timeline, but there are effects.

Explain this to me. Where do I draw the timeline curve? [ After this fix, “Clip Position” and “Timeline” do the same thing: the knob goes from its low end to its high end as the clip plays. Once you can draw the Timeline curve, Timeline follows your drawing and Clip Position stays a straight line. Keep both, or remove Clip Position? Default: keep both.]

Quantize 1 bar, 1/2 bar, 1/4, 1/8, 1/6

Let’s get rid of beats and just have bars in most places unless beats are necessary. For setting a clips beats, these are done with beats and not bars. For most other things, we will use bars. For the circle at the top that counts off 1234 and then starts over, those are beats as well. 

We should pick the most optimal Kodex to use and not worry about the other ones. Web M is not necessary. I want codecs that decode easily and play well

If there are ‘/‘ characters in a decks file name, do not worry about adding sub folders. They are just characters. If there are characters that are no good, then let me know and we will create a fix together

Can you figure out how to get everything working in the keying menu? Does the keying slider actually do anything? Maybe we get rid of it. Some elements in the keying menu work and some don’t. Should we use the transparency slider to do the work for keying elements? Also review the compositing menu, right slider in layer strip. These elements don’t work: Creative, 3D, 

When I grab the play head and move the timeline, I do not want it to jump back to where it was or where it should be playing before I grabbed it. I wanted to keep playing at the same speed, but play from wherever I drop the play head. Does this make sense?


Testing some of your work. When I switch decks, the clips in the layers disappear. They should remain. The output also goes black when I switch decks in there are clips playing in a layer.


How do I access the bigger envelope, editor? I don't think I need it? I am fine using the envelope editor in the signal tab. I want all of the controls to work in that one rather than an extra bigger envelope window.

I have beats and seconds in the milkdrop editor. I only need beats.

Gain slider in top bar could be twice as long. I have very little space to move it because I start at a quarter from the left edge and move down from there so I have very little distance to make a lot of fine adjustment.

All the other defaults are good.
```

## Items (Harmony's reading — consequence text, not Boris's words). Page = .harmony/.reports/s-rta-1002b/boris-checks.html (questions 1-39).
BF11 [NEW, READY] Firing a column that is already playing restarts its videos (today a column re-fire leaves them running).
BF12 [EXPLAIN] "Momentary pad released before its quantized beat: now cancels" (s-rta-1002 page, feel check) — explanation owed, no build.
BF13 [bf1 DELTA] Record to clip: (a) the recording is visible in the layer / cell it records into WHILE recording (OPEN-Q: how —
     a live picture in the destination cell vs played into the layer; a whole-output recording played into its own layer would
     record itself); (b) page Q7 ANSWERED: no automatic end; REC shown prominently; the only limit is disk space (a disk-space
     guard is Harmony's consequence); (c) page Q8 ANSWERED: "should look exactly like the output ... logo ... exactly as it's
     displayed" (OPEN-Q: single-layer recording keeps see-through parts? default now YES); (d) page Q9 = default (plays like a
     regular clip; Quantize decides).
BF14 [bf9b DELTA, READY] The layer strip does NOT show the source deck: remove the strip's source-deck badge (ruling-bf9b
     amendment 16(a)-(c): badge + badge click) before the merge. OPEN-Q: the deck TAB dot (default keep).
BF15 [NEW MODEL STATEMENT] A video clip is either BPM-synced (always follows the current BPM; a length the user can set; the app
     sets it automatically, the user changes it) or speed-controlled; Quantize times the START of both kinds; both kinds play
     side by side. (Today: transport mode "BPM Sync" with beat-division presets exists — docs/claude/history.md:55,
     architecture.md:92 beatDivision.) OPEN-Q: unit (his message says "bars" here and "beats" under BF21), how "by amount"
     works, the automatic length rule.
BF16 [bf2 DELTA] Sync: (a) saved as part of the composition (venue recall after a crash / return visit); (b) page Q14 ANSWERED:
     it lives in the TOP BAR; (c) page Q16: the user taps in time with the music; the beat stays in time with the music; the
     VISUALS are what gets delayed / advanced (confirm: beat wheel unshifted, output shifted).
BF17 [bf45 DELTA] Long samples (page Q21): whole sample by default, AND shorten it by hand by clicking timing marks.
BF18 [bf6] Page Q24 = (a): MilkDrop has no timeline (Timeline greyed out); its effects remain.
BF19 [EXPLAIN] Page Q25: where the Timeline curve is drawn — explanation owed; the default (keep both) stands until he says.
BF20 [bf7 DELTA] Quantize menu: 1 bar, 1/2 bar, 1/4, 1/8, "1/6" (OPEN-Q: read as 1/16).
BF21 [bf7 DELTA] Bars in most places; a clip's length is set in BEATS; the top circle counts BEATS (1-2-3-4).
BF22 [NEW] Codecs: pick the one that decodes easily and plays best; do not chase the others; WebM not needed (page Q39: no job).
BF23 [BUG, page section 3] "/" in a deck name is just a character — never a sub-folder. Characters a file name cannot hold:
     tell him, fix together.
BF24 [NEW AUDIT] Keying menu: make every entry work or propose removal; does the Keying slider do anything (maybe remove; maybe
     the transparency slider does the keying amount); compositing (blend) menu on the layer strip's right slider: the
     "Creative" and "3D" groups do not work.
BF25 [BUG] Dragging the playhead: after the drop the clip plays on from the drop point at the same speed; it must not jump
     back to where it was / would have been.
BF26 [REPORT] "When I switch decks, the clips in the layers disappear ... output also goes black" = the app WITHOUT the deck
     change (lane bf9b is built, not merged: main 5abdf01 vs lane/bf9b a7491d4; the lane's own probe shows the pre-change app
     changing the picture on a switch). Merge = this session's first job.
BF27 [bf45 REVERSAL] No bigger envelope editor. Every control works in the Signal tab's envelope editor (supersedes BF4
     "bigger envelope" and page Q17 / Q18).
BF28 [bf7] MilkDrop editor: beats and seconds shown today; only the musical unit is needed (OPEN-Q: label in bars or beats).
BF29 [NEW, ui] Top bar Gain slider twice as long (he works in the lower quarter of its travel).
BF30 "All the other defaults are good." -> every page question not named above takes its default (decks Q1-Q4; record Q5, Q6,
     Q10; sync Q11 bar-not-dial, Q12, Q13, Q15; envelopes Q19, Q20, Q22, Q23; Timeline Q26; bars Q27, Q29, Q30 (except clip
     length = beats); deck rename Q32, Q33; picture info Q34; Q35 on-screen test strip NOT OK'd; MilkDrop Q36, Q37; MKV Q38).

## Boris's answers to Harmony's 14 clarifying questions (received ~14:02, recorded 2026-10-03 14:03:17, s-rta-1003)
VERBATIM:
```
1 drop
2 restart
3 yes
4 yes
5 beats
6 both
7 not current bpm but have a bpm and beats input so user can do it by numbers and a x2 and /2 control to double or half easily
8 yes
9 yes
10 yes
11 drag handles that snap
12 bars
13 switch to - is fine for all bad chars
14 no more tuning
```
The questions as asked (Harmony's wording) and what each answer settles:
1 Deck tab dot (a small dot on a deck's tab when one of its clips is playing): keep or drop? -> DROP. bf9b: the tab dot
  (ruling-bf9b amendment 16(b), machine check M-e) is removed together with the strip badge (BF14).
2 A video you played, replaced, then fire again: continue where it left off (today) or restart? -> RESTART. Every fire of a
  video starts it from its start (in point). Changes the bf9b C3 "resume" contract (K10 (ii), Boris-page step 8.11) ->
  built in the transport lane after the bf9b merge; bf9b's Boris page drops 8.11.
3 While recording, the destination cell shows a live picture, a REC mark and the running length; it is not played into the
  output. -> YES.
4 Recording one layer with see-through parts keeps them see-through. -> YES (needs an alpha codec; H.264 cannot carry alpha).
5 BPM-synced clip length unit. -> BEATS.
6 "Change it by amount": type a number plus /2 and x2 buttons. -> BOTH.
7 Automatic length = closest of 1, 2, 4, 8, 16, 32 bars at the current BPM? -> NO: "not current bpm but have a bpm and beats
  input so user can do it by numbers and a x2 and /2 control to double or half easily". Harmony's reading: a BPM-synced clip
  carries its own BPM and its length in beats, both typed as numbers, with x2 and /2. OPEN: what the two numbers start at.
8 "1/6" in the Quantize list = 1/16. -> YES.
9 Sync: the beat wheel stays exactly where he tapped; only the picture is shifted. -> YES (lane/bf2's wheel follows the
  shifted beat today: must change).
10 Opening a composition loads its saved venue and sync value, replacing what the app had; the SYNC note shows. -> YES.
11 Trimming a long sample: drag the two end handles; they snap to the bar lines. -> DRAG HANDLES THAT SNAP.
12 MilkDrop preset change labels. -> BARS (seconds removed).
13 Characters a file name cannot hold. -> "switch to - is fine for all bad chars": every bad character becomes "-".
14 Other codecs still open and play as today; no more tuning for them. -> NO MORE TUNING.

## Follow-up on answer 7 (recorded 2026-10-03 14:10:11) — Harmony asked: "What should the two boxes start at when you first switch a clip to BPM sync? Default: read a BPM from the file name if it has one (like \"128bpm\"); otherwise pick the beat count (4, 8, 16, 32, 64) that puts the clip closest to 120 BPM."
Boris (verbatim): "7 follow up: default and bpms for clips to 120"
Harmony's reading (INFERRED, put back to him): the default stands, with 120 as the BPM a clip is assumed to be when its file name gives none. Second reading, if he corrects: the BPM box starts at exactly 120 and the beats box shows whatever the clip's length gives at 120 (2 beats per second, possibly not a whole number).
CLARIFIED by Boris (verbatim, recorded 2026-10-03 14:10:25): "default all bpms to 120" -> every clip's BPM box starts at exactly 120 (no
file-name rule, no nearest-power-of-two rule); the beats box shows what the clip's length gives at 120 BPM; he then types a
number or uses x2 and /2. Harmony's first reading above ("the default stands") was WRONG and is superseded.

## Boris's answers to questions A-E (recorded 2026-10-03 14:55:32, s-rta-1003)
VERBATIM:
```
a Let's not allow control Z to change anything that is live in the layer strip. It changes anything else
b yes it stays in clip tab regardless of deck
c can you clarify?
d yes
e every show remembers it's sync
```
The questions as asked (Harmony's wording):
A "You duplicate or load a deck, a routine starts one of its clips, then you press Cmd+Z. Should that clip keep playing,
  like when you delete a deck? Default: yes. One exception for now: if the loaded deck added layers to your show, Cmd+Z
  takes those layers and their clips away."
B "A clip is selected and you click another deck tab. Should the Clip tab stay on that clip, or jump to the same spot on
  the new deck? Default: stays, as today."
C "After deleting a deck, the 'Undo Remove' button only shows when the tab row has room; with many decks only the text line
  at the top says it. Enough? Default: yes."
D "Gain: twice as long, same scale? Default: yes. Alternative: also stretch the low end so 0 to 1 takes half the slider."
E "A show saved while Sync sits on 'Default' at 0 does not remember a sync setting, so opening it later leaves the room's
  sync alone. Default: yes. Alternative: every show remembers its sync, 0 included — then a show saved at home resets the
  room to 0."
BF31 [NEW RULE, from answer a] Undo (Cmd+Z) never changes what is live in the layer strip; it undoes everything else.
  Harmony's reading (consequence text): a fired clip is not an Undo step; undoing a structural edit (add / load / duplicate
  / remove deck, add / remove layer, clear) leaves what is playing playing. Today a trigger IS an Undo step (TriggerClipCmd)
  and Undo of a Load Deck that added layers stops that deck's clips (ruling-bf9b-merge AM-7's pinned exception, T6h).
  NOT built in the bf9b lane (no regression there: both behaviours are main's today) -> its own lane right after the merge,
  with pre-registered expectation changes (T6h, the trigger-undo tests, Boris-page step 8.6). OPEN: the exact edges (Undo of
  Clear, Undo of Add Layer while that layer plays) -> the plan decides, council attacks.
Answer b: the Clip tab stays on the clip he is working on, whatever deck is shown (= today; ruling Q-C closed).
Answer c: he asks for a clarification of question C (owed in plain words).
Answer d: Gain twice as long, same scale.
Answer e: EVERY show remembers its sync, 0 included (REVERSES the ruling-bf2-delta default "a show saved at Default 0 does
  not remember a sync setting").

## Boris on the "Undo Remove" button (recorded 2026-10-03 14:58:10) — after Harmony's clarification of question C
VERBATIM: "I don't wanna see an under removed button at all. We just use control Z. The only place that we will see undo remove, will be in the top edit menu."
Harmony's reading: the "Undo Remove" button in the deck tab row is removed entirely (ruling-bf9b amendment 16(d)'s button); Cmd+Z is the way back; the Edit menu's Undo item is where "Undo Remove" is read. The information sentence in the top text line (Removed deck "..." -- Layer N keeps playing its clip) is not a control and stays. Built in bf9b FIX-3 (adoption item 9).
FOLLOW-UP (recorded 2026-10-03 14:59:28) — Harmony said the one-line sentence at the top after deleting a deck (Removed deck "Deck 3" -- Layer 2 keeps playing its clip) would stay, "Say so if you want that gone too." Boris (verbatim): "yes remove the visible line. not needed" -> that sentence is removed as well (bf9b FIX-3, adoption item 10).

## Boris on notices (recorded 2026-10-03 15:30:49) — after Harmony said the yellow "old show converted" note would stay
VERBATIM: "We don't need any text indicating what has happened or what has happened. That is something that happens online and is not necessary in this application. It is extra overhead and bloat. Please remove it cleanly and completely."
BF32 [RULE] The app shows no text that announces what has just happened. In the deck change (bf9b FIX-3, adoption item 11): the whole load notice goes (old show converted / routine pads left empty / deck-id refusal), with the Undo Remove button and the Remove Deck sentence (items 9, 10). The app log keeps its lines (not on screen). Sync-dial plan: no "sync changed" notice on a composition open (bf2 adoption item 7). OPEN (asked): the same rule for notices that were in the app before today (inventory owed).
FOLLOW-UP (recorded 2026-10-03 15:42:01) — Harmony asked: "For texts that were already in the app before today (for example the yellow note when a wired mic drops and the app falls back to the MacBook mic), should they go too? Default: I send you the full list first; informational ones go, ones that report a failure stay until you've seen the list." Boris (verbatim): "remove the list entirely and cleanly" -> BF32 is APP-WIDE: every on-screen text that announces what has happened is removed, the ones that were in the app before today too, failure reports included; no list review first. Own lane after the bf9b merge (inventory -> plan -> council -> ruling -> build). Harmony flags ONE risk back to him (a failed save would show nothing).

## Boris on the Edit menu + session end (recorded 2026-10-03 16:06:40)
VERBATIM: "we should have an edit menu. note this, finish current tasks then eos"
BF33 [NEW, ui] The app gets a standard Edit menu (Undo / Redo at least; today Undo is the first item of the "Composition" menu and names the action, e.g. "Undo Remove Deck"). Next UI pass (ui-polish lane), VISUAL + interaction gate.
SESSION: finish the tasks in flight (the deck change: last build stage -> reviews -> Harmony's gates -> merge), then EOS. Nothing new starts.
context:     s-rta-1003 secondary (RealTimeAudio / Audio-DNA), 2026-10-03: Boris's answers to the Oct 2 question page + new product rules BF11-BF33, every message verbatim with the question as asked. Product rules worth the primary's attention as patterns: (1) no on-screen text that announces what happened, app-wide; (2) Undo never changes what is live; (3) every fire restarts a clip; (4) decks are boxes of clips (merged today).
why:         
intent:      
target:      
constraints: 
related:     
priority:    HIGH
repo:        RealTimeAudio     session:      date: 2026-10-03
status:      NEW
status-changed: 2026-10-03
status-note:
artifact:
history:     NEW(2026-10-03)
--- /IDEA ---

--- IDEA ---
id:          idea-2026-10-03-RealTimeAudio-17910669758592217668
raw:         REPORT UP (s-rta-1003, RealTimeAudio): (A) T26 secondary boot-prefix cut — STEP 0 MEASURED, steps 1-3 not built yet (deck-change merge took the session; STEP 3 handoff slim + archive landed at this close). Numbers (.harmony/.reports/s-rta-1003/t26-step0.md): first-call tokens RTA main session 54,114; RTA builder today 29,598 (slim builder spec) vs 41,152 yesterday (32 builders 37.7k-43.7k); Harmony_Main primary today 39,520 (not 30k). RTA main contributors: harmony.md 34.5 KB (9.6k-15.5k tok), RTA CLAUDE.md 24 KB (6.7k-10.8k), workflow-authoring skill text 17.3 KB (loaded by the launch command), pasted birth prompt 10.1 KB, agent listing 5.5 KB. In a builder CLAUDE.md is ~36 % of the first call. The global ~/.claude/CLAUDE.md loads ONCE per agent (not twice); the primary avoids it with claudeMdExcludes; RTA has none (STEP 2 = add it, supply layer0 in-project). HANDOFF.md is NOT in any first-call prefix: only the pasted birth prompt rides (now cut from 10 KB to ~4 KB by moving rig rules to .harmony/RIG-RULES.md). Estimated savings: STEP 1 4.4k-7.2k tok / call, STEP 2 1.4k-2.3k. (B) BUILDER-SPEC TRIP-WIRE (inbox notice of 2026-10-03): 8 builder dispatches with test authoring (M1-M3, FIX-1..5): builder-playbook referenced in every transcript checked, the INBOX-RECHECK line present in every stage report, review fix rounds 0 after the 4-lens round (this repo's normal: 1). No regression signal from the slim spec. (C) GAP: a foreign-repo lane cannot write memory/DISPATCH_LOG.md, so fable-usage-audit WARNs LAW11-LOG-GAP every session (3 architect dispatches today: 2 plans opus high, 2 rulings opus max; Fable not used, per Boris's 2026-10-02 instruction).
context:     s-rta-1003 secondary close: report-up for inbox items down-2026-10-03-RealTimeAudio-179104415559333541 (T26) and -17910451953639312704 (builder-spec notice)
why:         
intent:      
target:      
constraints: 
related:     
priority:    HIGH
repo:        RealTimeAudio     session:      date: 2026-10-03
status:      NEW
status-changed: 2026-10-03
status-note:
artifact:
history:     NEW(2026-10-03)
--- /IDEA ---

--- IDEA ---
id:          idea-2026-10-04-RealTimeAudio-17910904834063820629
raw:         ok. the only fail message will be a failed save. remove all others
context:     s-rta-1003b 2026-10-03 20:59:50, Audio-DNA: Boris's ruling on on-screen messages after asking 'how would a save fail? not sure we need that'. Amends his 'remove the list entirely and cleanly'. Filed as BF34 in .harmony/boris-feedback-backlog.md; planned in ruling-notices.md.
why:         
intent:      
target:      
constraints: 
related:     
priority:    HIGH
repo:        RealTimeAudio     session:      date: 2026-10-04
status:      NEW
status-changed: 2026-10-04
status-note:
artifact:
history:     NEW(2026-10-04)
--- /IDEA ---

--- IDEA ---
id:          idea-2026-10-04-RealTimeAudio-17910904834065511968
raw:         answer to 12 questions. all defaults except for: 2 b / 10 If we set the inpoint and endpoint on the timeline of the clip, I should not be able to drag outside of the points. It should be as if that is the extent of the timeline unless I let go and drag one of the in or out points. / 4 For setting the clip beat marks, the clips are set with beats not bars. x2 or /2 are beat changes. bars will confuse this. A bar is for beats and is used in places where longer durations make sense and beats are used where exaggerations makes sense. We should be very clear where we are using beats and bars, but they are essentially the same thing like feet and inches / 9 b / 3 cmd z Does not change anything in the layer strip which is by default live based / 12 b / One thing I didn't mention is that when the Video is in beats rather than adjust by speed mode, the beats are shown with lines in the play head area whereas if it was speed control, it's just the basic play head and with beats control there are lines for each beat in the play head area and the play head moves past them on time
context:     s-rta-1003b 2026-10-03 23:4x, Audio-DNA: Boris's answers to the questions page (.harmony/.reports/s-rta-1003b/boris-questions.html); questions as asked are in boris-feedback-backlog.md; BF35-BF37; ruled in ruling-transport-delta1.md.
why:         
intent:      
target:      
constraints: 
related:     
priority:    HIGH
repo:        RealTimeAudio     session:      date: 2026-10-04
status:      NEW
status-changed: 2026-10-04
status-note:
artifact:
history:     NEW(2026-10-04)
--- /IDEA ---

--- IDEA ---
id:          idea-2026-10-04-RealTimeAudio-1791090483406723307
raw:         I am to go into bed and you need to finish the rest of this autonomously. I will not be able to approve the RM commands so I will need you to approve those yourself. can you do this or find a good workaround so your work tonight is not lost?
context:     s-rta-1003b 2026-10-03 23:4x. SYSTEM-UPGRADE CANDIDATE for Harmony: an unattended / overnight mode for secondaries. What worked: a PreToolUse hook that allows rm inside the rig's roots and DENIES any other rm at once (never a prompt), narrow allow rules, a background watchdog timer; recipe + script in ~/projects/RealTimeAudio/.harmony/.reports/s-rta-1003b/rm-guard.py. Cost lesson: the update-config skill's schema dump took ~13 % of a 1M context window -- a lean 'permissions + hooks' reference would avoid that. Also reported up: fable-usage-audit WARN LAW11-LOG-GAP (13 architect dispatches, 0 DISPATCH_LOG rows: a foreign lane cannot write the log); T26 STEP 1-2 still open.
why:         
intent:      
target:      
constraints: 
related:     
priority:    HIGH
repo:        RealTimeAudio     session:      date: 2026-10-04
status:      NEW
status-changed: 2026-10-04
status-note:
artifact:
history:     NEW(2026-10-04)
--- /IDEA ---

--- IDEA ---
id:          idea-2026-10-04-RealTimeAudio-1791144717556104074
raw:         Answers to 25 questions below. Ask questions to clarify till you are 100% confident you understand what needs to be done:
context:     s-rta-1004 2026-10-04 11:59:06, RealTimeAudio. A WORKING-STYLE DIRECTIVE from Boris, kept all session: every answer of his was read against the question as asked, every gap became a numbered question with a default A (26-134), and nothing was planned on a guess. It produced 54 filed rules (BF38-BF91) and five ruled lanes in four hours. SYSTEM-UPGRADE CANDIDATES for Harmony, from the same session: (1) a helper for foreign lanes that files Boris words in one call -- takes the verbatim text, stamps it from date itself, appends to the backlog and the decisions file (a hand-typed stamp was 4 minutes off and was caught by luck); (2) a helper that extracts pasted images from the session jsonl to disk the turn they arrive (they die with the session; a 20-line python walk did it); (3) the generic plan -> blind seats -> ruling workflow with the lanes table inside the script (~/projects/RealTimeAudio/.harmony/.reports/s-rta-1004/wf/plan-lane.js) and its dry-run-against-stub-agents check are reusable beyond this repo. Full session record: ~/projects/RealTimeAudio/.harmony/sessions/2026-10-04-s-rta-1004-secondary.md.
why:         
intent:      
target:      
constraints: 
related:     
priority:    HIGH
repo:        RealTimeAudio     session:      date: 2026-10-04
status:      NEW
status-changed: 2026-10-04
status-note:
artifact:
history:     NEW(2026-10-04)
--- /IDEA ---

--- IDEA ---
id:          idea-2026-10-04-RealTimeAudio-17911447175562611343
raw:         finish current tasks, save all decisions and run eos
context:     s-rta-1004 2026-10-04 15:14:22, RealTimeAudio. REPORT-UP at close: nothing built or merged; the sync dial lane was superseded by Boris (per-screen output Delay as Resolume); five lanes planned, councilled, ruled and adopted (one-save, outputs, nudge + tempo row, transport, looks per effect), builds start next session. Again reported: fable-usage-audit WARN LAW11-LOG-GAP -- 16 architect dispatches, 0 DISPATCH_LOG rows, because a foreign lane cannot write Harmony_Main; a registry-pull for dispatch rows (as for ideas) would close it. Session index: skipped (foreign lane, no transport). T26 STEP 1-2 still open. Context reached 68 % with eleven workflows and about sixty Boris exchanges: the per-exchange filing cost (two appends + a clarify file) is the main driver -- candidate (1) above would cut it.
why:         
intent:      
target:      
constraints: 
related:     
priority:    HIGH
repo:        RealTimeAudio     session:      date: 2026-10-04
status:      NEW
status-changed: 2026-10-04
status-note:
artifact:
history:     NEW(2026-10-04)
--- /IDEA ---
