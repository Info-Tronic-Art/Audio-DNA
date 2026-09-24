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
