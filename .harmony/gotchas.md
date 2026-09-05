# Project Gotchas — Audio-DNA

<!-- Captured by EOS from dispatch failures. EOD promotes universal entries. -->
<!-- Builder reads this via SELF-BRIEF. Harmony includes relevant entries in work packets. -->
<!-- Format: each entry has Source, Trigger, Lesson, Scope (repo|universal), Promoted (yes|no). -->

(No gotchas captured yet. Entries appear after Builder dispatches encounter failures.)

### 2026-05-18 — conftest autouse fixture blocks non-Eyes tests
Source: Builder A report — test_ax_inspector.py initially skipped all tests
Trigger: tests/visual/conftest.py has `autouse=True` fixture `reset_between_tests` that depends on the `app` fixture, which skips when Audio-DNA executable is not built
Rule: Always override `app` and `reset_between_tests` fixtures locally in any new test file in tests/visual/ that does NOT need the running app
Scope: repo
Promoted: no

### 2026-05-18 — FetchContent must pin versions for reproducible builds
Source: Tester CONCERN — melatonin_inspector used `origin/main` instead of SHA
Trigger: Builder followed melatonin_inspector README which recommends `origin/main`
Rule: Always pin FetchContent GIT_TAG to a specific SHA or release tag, with GIT_SHALLOW TRUE. Match existing pattern in CMakeLists.txt.
Scope: repo
Promoted: no

### 2026-05-19 — load_image renders black in production mode
Source: EOS test validation — effect tests PSNR=inf
Trigger: ApiServer load_image sets a texture but production renderer draws composition layers, not standalone images. Tests using load_image + render_frame get black frames.
Rule: Always use load_source with a procedural source (e.g., checkerboard) for effect comparison tests in production mode. Reserve load_image for test-mode only.
Scope: repo
Promoted: no
**REFINED 2026-07-30 (scout source-trace, INFERRED — not yet verified):** there is NO
mode gate anywhere in the render path — testMode_ gates exactly 4 things (alert
suppression, analysis-thread skip ×2, TestServer start; MainComponent.cpp:386,
1400, 1404, 1585), and both servers' handleLoadImage bodies are behaviorally
identical (renderer_.loadImage + sleep(100), no clearActiveSource in either). The
"production renders black" difference is therefore almost certainly SESSION-STATE
precedence, not mode: production sessions have an active source/clip that renders
over the standalone image (handleClipTrigger clears active source at
MainComponent.cpp:2931; load_image never does). Cheap verify when convenient:
POST /api/reset (calls clearActiveSource) → load_image → render_frame → non-black
expected. The practical Rule above still stands.

### 2026-05-19 — ApiServer load_image needs GL thread sleep
Source: EOS T1 fix — compared TestServer vs ApiServer load_image
Trigger: ApiServer handleLoadImage returned before GL thread processed the pending image load, causing subsequent render_frame to capture stale content.
Rule: Always add juce::Thread::sleep(100) after renderer_.loadImage() in HTTP handlers to give the GL thread time to process.
Scope: repo
Promoted: no

### 2026-05-21 — KeyingMode enum ≠ Layer::Type — verify enums directly
Source: Builder A HAD_TO_INFER — audit listed 4 "keying modes" (Opaque, Transparent, FXOnly, Mask) but those were Layer::Type values. Actual KeyingMode enum has 13 entries (Alpha through ChannelB).
Trigger: Confusing two related enums in the compositing pipeline. Layer::Type controls compositing behavior; Layer::KeyingMode controls transparency extraction.
Rule: Always verify enum names from header files, not from audit summaries. Two enums in the same class can have overlapping conceptual scope.
Scope: repo
Promoted: no

### 2026-05-21 — Composition transform fields are runtime-only (not serialized)
Source: Builder A discovery — compPositionX/Y, compScale, compRotation, compAnchorX/Y exist in Composition struct but are NOT in toVar() serialization.
Trigger: Assuming all model fields persist. These are runtime-only adjustments.
Rule: Check toVar()/fromVar() to confirm which fields actually serialize before documenting persistence behavior.
Scope: repo
Promoted: no
**FIXED 2026-07-17:** now serialized (Wave 1-C) — compPositionX/Y, compScale, compRotation, compAnchorX/Y and every other dropped Clip/Layer/Composition field now round-trip through toVar/fromVar (Clip.cpp / Layer.cpp / Composition.h). The general rule above still stands: verify toVar/fromVar before documenting persistence.

### 2026-05-21 — Session Recorder has 7 event types but only 1 caller wired
Source: Builder B discovery — recordClipTrigger() is the only event type actually called from production code. Other 6 record methods (ParameterChange, ColumnTrigger, MacroChange, TransportChange, EffectToggle, CuepointJump) are implemented but have no callers.
Trigger: Assuming a feature is complete because the API exists.
Rule: When documenting event/callback systems, grep for callers of each method. API existence ≠ integration.
Scope: universal
Promoted: yes → audit-workflow (2026-06-01)

### 2026-05-22 — §3.6 shorthand vs §6.7 full spec contradiction missed by vague cross-ref audit
Source: Tester catch on MOCKUP_BRIEF cleanup — Builder applied opacity-floor edits to §6.7 + §6.2 but left §3.6 reading "direct 1:1 mapping" (semantically contradictory but defensible). Edit 8 packet wording said "scan for conflicting Active triangle descriptions" but didn't list §3.6 explicitly. Builder rationalized leaving it.
Trigger: Cross-reference audit instructions that say "scan for X" without enumerating known candidate sites. Builder reads narrowly and skips defensible-but-contradictory sites.
Rule: When a packet edits canonical doc and authoritative spec changes (new floor, new state, new rule), enumerate EVERY site that paraphrases or shorthands that spec — not just direct contradictions. List candidate sections by number in the cross-ref audit instruction, not just by topic.
Scope: universal
Promoted: yes → feature-build (2026-06-01)

### 2026-05-22 — §6-tokens-win-over-§5-pixel-specs (MOCKUP_BRIEF authority hierarchy)
Source: Boris-locked decision during MOCKUP_BRIEF cleanup. §5 cites pixel specs from praised v9 mockups; §6 declares canonical production tokens. Pre-cleanup, §5 = 50px and §6 = 60px for signal column.
Trigger: Builder reading §5 quality-bar pixel specs and treating them as authoritative production values.
Rule: MOCKUP_BRIEF §6 production tokens always override §5 quality-bar pixel specs when they differ. §5 is inspiration, §6 is contract. New §8.5 Token Sync Verification enforces this on every v10 mockup.
Scope: repo
Promoted: no

### 2026-05-22 — No push to remote (CI failure on Audio-DNA)
Source: Boris session 15 — "I am getting messages that run failed in git remote. we should be making all of these locally. no need to push to remote at all."
Trigger: Auto-commit-and-push protocol pushing to Audio-DNA remote triggers CI workflow that's currently failing.
Rule: Commit locally to RTA, do NOT push to remote until Boris re-enables. Captured 2026-05-22 mid-session. May be temporary — verify with Boris before resuming pushes.
Scope: repo (RealTimeAudio / Audio-DNA only)
Promoted: no

### 2026-05-22 — B+ minimum quality bar for design-variation evaluation
Source: Boris session 15 — "Re-design anything that is not B+ level" then "refine all 20 to b+ and make the the default behavior in the future regardless of the task, you'll finish what I ask you for."
Trigger: When Harmony spawns a Critic agent to grade design variations (layer-rows, mockups, page studies), and Boris requests "evaluate and refine".
Rule: Anything graded below B+ gets refined in-place via Refiner Builder. Don't ask which to refine — refine all sub-B+ items. Re-Critic verifies B+ achieved. Iterate max 2 loops. Pattern: Critic (Tester + design-review methodology + Playwright) → Refiner (Builder with per-variation recommendations) → Re-Critic. Validated session 15: 13/13 refinements achieved B+ first pass.
Scope: universal
Promoted: yes → design-review (2026-08-30)

### 2026-07-16 — All shipped shaders are EMBEDDED; shaders/ dir files are dead + hot-reload is inert
Source: 7-lane re-norm audit (L2 render-effects)
Trigger: Assuming the 5 files in `shaders/` (hue_shift/rgb_split/ripple/vignette/passthrough) are live, or that ShaderManager hot-reload works.
Rule: Every effect/transition/source shader ships as an inline string in `src/render/EmbeddedShaders.h` and is compiled via `compileProgram` (275 compile() calls in Renderer.cpp). `ShaderManager::reloadAll()` only reloads programs compiled *from files* (empty vertFile/fragFile skipped), so hot-reload is inert for the entire shipped set. The 5 `shaders/*.frag|vert` disk files are dead duplicates. Edit shaders in EmbeddedShaders.h, not the disk files.
Scope: repo
Promoted: no

### 2026-07-16 — SourcesBrowser list is hand-maintained, NOT generated from SourceRegistry
Source: 7-lane re-norm audit (L3 sources-media, L5 ui-surfaces)
Trigger: Adding a procedural source to `SourceRegistry` and expecting it to appear in the GUI Sources browser.
Rule: `SourcesBrowser.cpp` uses ZERO `SourceRegistry` references — its ~103 source rows are a hand-maintained list. A new source must be added in BOTH places (registry + SourcesBrowser) or it is API-selectable but invisible in the GUI. 6 registered sources (strange_attractor, gravity_well, fluid_dynamics, text_animator, layer_router, projectm_visualizer) are currently missing from the browser for this reason.
Scope: repo
Promoted: no

### 2026-07-16 — 48kHz is hardcoded across analysis with no runtime validation
Source: 7-lane re-norm audit (L1 audio-analysis)
Trigger: Assuming the analysis pipeline adapts to the device sample rate.
Rule: `kSampleRate=48000` (AnalysisThread.h:46) plus the LoudnessAnalyzer K-weighting biquad coefficients (ITU-R BS.1770 48k, LoudnessAnalyzer.cpp:13-29) and all frequency math assume 48kHz with NO runtime SR check. A device running at 44.1k/96k silently produces wrong LUFS/frequency features. Do not assume SR-independence when touching analysis.
Scope: repo
Promoted: no

### 2026-07-17 — Launch Audio-DNA via `open`, NEVER direct binary exec, for behavioral gates
Source: Harmony Wave-0 behavioral gate — direct exec of `build/AudioDNA_artefacts/Release/Audio-DNA.app/Contents/MacOS/Audio-DNA` from an agent shell hangs pre-UI forever (process alive, ZERO windows, ZERO listening sockets, empty log, 2+ min) — looked exactly like a broken REST server.
Trigger: Running the app binary directly from a shell/agent context to probe the port-7070 API.
Rule: Always launch with `open build/AudioDNA_artefacts/Release/Audio-DNA.app` (LaunchServices context); port 7070 binds ~12s after launch. Poll /api/health before probing. Kill with `pkill -f Audio-DNA`.
Scope: repo
Promoted: no

### 2026-07-17 — Post-render GPU steps must restore GL_FRAMEBUFFER to defaultFBO
Source: Wave 1-A builder
Trigger: Adding a post-render GPU step (e.g. Syphon `publishSyphonFrame`) that binds its own FBO/texture and returns without re-binding the default framebuffer.
Rule: `Renderer::processPendingCapture`'s `glReadPixels` assumes the default FBO is bound after rendering; any post-render GPU step (e.g. Syphon `publishSyphonFrame`) must re-bind `defaultFBO` when done, as `publishSyphonFrame` does (Renderer.cpp ~1710-1762).
Scope: repo
Promoted: no

### 2026-07-19 — First `open` can transiently stall in CoreAudio/TCC init (looks like the direct-exec hang)
Source: Undo v1 step-1 builder (fence-validation live run)
Trigger: First `open` of Audio-DNA.app after a rebuild sometimes stalls inside the ctor at `AudioDeviceManager::initialiseWithDefaultDevices` → CoreAudio/TCC — process alive, no windows, no port 7070, indistinguishable from the direct-exec hang gotcha (2026-07-17).
Rule: Before concluding the build is broken, run `sample <pid>` to pinpoint the stall; if it's in AudioDeviceManager/CoreAudio init, `pkill -9` and re-`open` — second launch typically binds :7070 in ~4s. Environment flake (TCC/audio permissions), not code.
Scope: repo
Promoted: no
**ROOT CAUSE FOUND 2026-07-25:** see the TCC mic-prompt entry below — the "stall" is the app blocking on an unanswered microphone-permission dialog.

### 2026-07-19 — Renderer media resources are keyed by clip.id with NO file-match check
Source: Undo v1 step-2 reviewer (caught pre-commit) — video replace-undo was a silent visual no-op
Trigger: Any flow where a clip's CONTENT changes but its id doesn't (replaceContent keeps id), combined with a "reconnect if missing" guard: videoPlayers_/imageSequences_ lookups use id only, so a guard that checks existence sees the id-keyed player (loaded with the NEW file) and skips reopening — model and renderer silently diverge.
Rule: Existence of an id-keyed renderer resource does NOT imply it holds the right content. Any reconnect/restore path must compare the loaded file against the model's current mediaFile (or reopen unconditionally) when ids are content-stable. Undo/redo, preset load, and future player-disposal waves all hit this.
Scope: repo
Promoted: no

### 2026-07-16 — FeatureSnapshot::clear() resets genre/energy to 0 (House/low), not struct defaults
Source: 7-lane re-norm audit (L1 audio-analysis)
Trigger: Expecting a freshly-cleared FeatureSnapshot to carry the struct's default genre/energy (detectedGenre=6, energyState=1).
Rule: `FeatureSnapshot::clear()` memsets then restores only rmsDB/lufs/detectedKey/keyIsMajor/swingRatio — it leaves `detectedGenre=0` (House) and `energyState=0` (low), NOT the struct defaults 6/1 (FeatureSnapshot.h:81-89). FeatureBus inits all 3 buffers via clear(), so this is the effective startup default. Latent bug; verify before relying on cleared-snapshot genre state.
Scope: repo
Promoted: no

## 2026-07-22 — CoreAudio launch stall WORSENS with repeated pkill -9 cycles (env wedge, not code)
Source: Undo v1 step-5 behavioral gate (s. 2026-07-22)
Trigger: Launching Audio-DNA via `open` after prior instances were pkill-9'd mid-CoreAudio-start.
Rule: The 2026-07-19 first-open stall (CoreAudioInternal::start mutex wait; remedy sample → pkill -9 → re-open) can WEDGE PROGRESSIVELY: after 1-2 remedy cycles the stall reproduces on EVERY relaunch (3 consecutive this session; step-4 gate an hour earlier recovered on attempt 2). A 90s coreaudiod settle window did NOT clear it. Signature verified by sample both times: identical CoreAudioClasses::AudioIODeviceCombiner::start → CoreAudioInternal::start → __psynch_mutexwait. Change-independence verified: step-5 diff has 0 audio-path refs; stall predates the code. Remedy beyond the loop: restart coreaudiod (`sudo killall coreaudiod`) or logout/reboot — Boris-level. Gate policy used: code gates (build/ctest/residue/review) green → app-level check recorded BLOCKED-ENVIRONMENTAL, launch verification prepended to manual checklist.
Scope: repo (macOS env interaction)
Promoted: no
**SUPERSEDED 2026-07-25 — root cause was NEVER a coreaudiod wedge:** see next entry. Reboot did not "fix" it because the blocker is a TCC dialog, not the daemon; `sudo killall coreaudiod` is NOT the remedy and should not be requested again.

### 2026-07-25 — ROOT CAUSE: CoreAudio launch "stall/wedge" = unanswered TCC microphone prompt; ad-hoc signing re-fires it every rebuild
Source: Undo v1 step-8 session env recheck — post-reboot launch still "stalled"; `screencapture` + Read of the PNG revealed a live TCC dialog ("Audio-DNA would like to access the microphone", Don't Allow/Allow) that no CLI probe can see.
Trigger: Launching Audio-DNA from a headless/agent session. `CoreAudioInternal::start` blocks (verified by sample, same signature as 07-19/07-22 entries) waiting on the TCC microphone-permission response; with nobody at the screen the dialog is never answered → launch never binds :7070. Because the app is **ad-hoc signed** (`codesign -dv`: Signature=adhoc, no TeamIdentifier — verified), every rebuild changes the cdhash, so TCC re-prompts after EVERY rebuild (inferred, standard TCC behavior — explains recurrence across sessions and why pkill/reboot/coreaudiod-restart never helped).
Rule: (1) After any rebuild, the FIRST app launch needs a human to click **Allow** on the mic prompt — schedule app-level behavioral gates for when Boris is present, or have him click Allow right after the gate's launch. (2) Diagnose headless launch stalls with `screencapture -x /tmp/x.png` + image read — TCC/system dialogs are invisible to sample/lsof/log probes. (3) Do NOT pkill-cycle or restart coreaudiod for this signature. (4) Durable fix option (Boris): sign dev builds with a stable Developer ID identity so TCC remembers the grant across rebuilds. (5) Synthetic clicks can't answer TCC prompts without Accessibility for the calling process (osascript denied assistive access — verified).
Scope: universal
Promoted: yes → debug-mode (2026-08-30)

### 2026-07-30 — Synthetic UI driving of Audio-DNA (recipes + flake profile)
Source: Autonomous e2e drive (Accessibility granted to Ghostty; fence-lane app verification)
Trigger: Driving the JUCE UI via System Events/CGEvent for headless e2e.
Rule: (1) Cell THUMBNAIL click = TRIGGER; cell NAME BAR (bottom ~20px) = SELECT
(ClipCell.cpp:180-194) — drag-move must START from the name bar or you trigger
instead. (2) Layer selection = strip click OUTSIDE the transport band
(LayerStrip.cpp:706-719); Move Up requires idx>0 (bottom strip is idx 0), Move
Down requires idx<top; Layer-Clear silently skips empty layers (content guard).
(3) Synthetic keystrokes + menu AXPress DROP ~15% of events — always
verify-and-retry against a state+menu-label fingerprint (/api/composition +
Composition-menu Undo item name); never fire blind undos after an unverified op.
(4) Window capture without focus steal: CGWindowList id + `screencapture -o -x
-l<id>`; coordinate map: screen_pt = display_coord×0.864, y+38 (window at (0,38),
2x Retina). (5) /api/composition omits clip effects — FX verification is
undo-label + visual capture. (6) An empty ACTIVE deck collapses the grid and
relocates deck tabs to the top — recapture coordinates after structural layout
changes. Menu bar: Undo/Redo live in the COMPOSITION menu (no Edit menu).
Scope: universal
Promoted: yes → feature-build (2026-08-30)
**ADDENDUM 2026-07-30 PM:** (7) FULL-SCREEN captures (`screencapture -x` without
-l): screen_pt = displayed_2000px_coord × 0.864 directly, NO +38 (menu bar
included in frame — the +38 applies only to WINDOW-relative coords from -l
captures). (8) Brightness-heuristic verification of layout state LIES (meter
colors defeat it) — always eyeball the capture for layout claims. (9) SignalBar
mode is NOT governed by View>Reset Layout; the ▲▼ cycler is a ~15px target and
resisted 3 synthetic click attempts in expanded mode — leave restoring it to a
human rather than looping. (10) Graceful quit (osascript/TERM) traverses a teardown
path that DISCOVERS latent heap corruption (shutdown SIGBUS was single-word
damage in a live Label, state/timing-dependent — NOT an every-quit crash; see
scout-shutdown-sigbus.md). SIGKILL for disposable instances remains the right
call: skips the discovery path, no misleading .ips. (11) SYNTHETIC-CLICK
PREFLIGHT (MANDATORY, learned 07-30 the hard way — a click landed in the
user's Firefox): immediately before EVERY synthetic event burst, in the SAME
command, check (a) HID idle is fresh-high AND (b) frontmost app is Audio-DNA:
`osascript -e 'tell application "System Events" to get name of first process
whose frontmost is true'`. A stale idle check (even 2 min) is worthless — the
user can return silently; HID-idle alone also fails while the user is READING.
If frontmost ≠ Audio-DNA → abort the burst entirely.

### 2026-08-02 — .harmony/ is gitignored BUT tracked-by-convention (force-add new knowledge files)
Source: Syphon lane fix round — builder declined to commit a new dossier because .gitignore:62 ignores `.harmony/`; its "not committed here" inference was wrong at directory scope (20+ .harmony files sit in history; EOS commits ship ledgers + dossiers).
Trigger: Committing a NEW .harmony/ knowledge file — `git check-ignore` flags it, plain `git add` refuses, and a builder will stall or skip the capture.
Rule: Tracked .harmony files travel in git normally (tracked overrides ignore). NEW knowledge files (dossiers, session ledgers) need a one-time `git add -f`, then normal commits. Run `git ls-files .harmony/ | head` before concluding a path "isn't committed here." Harmony owns these commits (knowledge layer), not builders.
Scope: repo
Promoted: no

## 2026-08-02 — TestServer (8080) listens on IPv6 ::1 — probe with `localhost`, never `127.0.0.1`
Trigger: S2 gate: curl 127.0.0.1:8080 timed out 50s ("no bind") while the server was up and healthy on [::1]:8080 (lsof).
Rule: TestServer binds ::1 (pre-existing, S2 diff clean of bind changes). Health-poll and probe 8080 via `localhost` (curl tries both families) or literal `[::1]`. ApiServer 7070 is the opposite: binds IPv4 127.0.0.1 (R8 default) — `localhost` works for both, so use `localhost` everywhere in probes. vj_controller.py/e2e unaffected (already uses localhost).

## 2026-08-03 — TSan APP gate recipe (build-tsan) + traps
Trigger: OW-arc C1 gate: 120s "no bind", Cmd+F dead, VJController import error.
Rule: (1) build-tsan is configured AUDIODNA_BUILD_TEST_SERVER=OFF — TSan app runs are PRODUCTION-mode only (no 8080, mic-driven features; reconfigure cache if injection needed). (2) Launch with `open --stdout F --stderr G app.app --args ...` — captures TSan + programID stderr reliably; first /api/health probe during the startup window can return EMPTY on a live listener — poll patiently before concluding no-bind; steady-state handlers answer in ~0.1s even under TSan. (3) Cmd+F is swallowed when a text field holds focus — open the output window via System Events menu click: item "Fullscreen: <res> (main)" of menu "Output" (AXPress path, ~15% drop → fingerprint-verify on "[OutputRenderer]" stderr lines, retry). (4) e2e class is VJAppController (not VJController); works against 7070 for list_sources/load_source/set_effect_chain; sources schema keys are id/name (no "type"). (5) Debug app emits a finite juce_LookAndFeel.cpp:54 jassert burst at startup — benign noise, not a hang.
Scope: repo (gate mechanics).
Promoted: no

## 2026-08-03 — App UI gates: activation can FAIL silently; use menu AXPress, and .ips lands LATE
Trigger: OW-arc C2 gate: coordinate click for the SignalBar cycler landed in Ghostty; `tell app "Audio-DNA" to activate` left ghostty frontmost; a stale ReportCrash dialog ate Escape keystrokes.
Rule: (1) VERIFY frontmost (`get name of first application process whose frontmost is true`) before ANY coordinate click — activate can no-op, and `click at {x,y}` then hits whatever window owns that point. (2) Menu-item AXPress works WITHOUT frontmost and is the reliable path: Output>"Fullscreen: <res> (main)" opens, Output>"Disabled" closes the output window (Escape is unreliable — a system dialog steals it). (3) A TSan-instrumented app that reported races ABORTS at exit (NSApplication terminate -> exit -> __cxa_finalize -> __tsan::finalize -> Die -> abort, SIGABRT) and writes an .ips — that is the sanitizer, NOT a product crash; check the crashed-thread frames before alarm. (4) .ips files are written with a DELAY (tens of seconds+) — an immediate post-quit `ls DiagnosticReports` proves NOTHING; re-check later or verify by parsing the newest report. (5) Escape/keystroke gates: kill lingering ReportCrash first (it respawns; pkill -9 may need repeating).
Scope: universal
Promoted: yes → feature-build (2026-08-30)

### 2026-08-03 — The /api/health "fps" field FREEZES on GL detach; it is an INVALID detach oracle
Source: OutputWindow arc C3 gate step 1. A handoff instructed "use /api/status fps as the detach oracle — it collapses when the preview GL context detaches." Both halves were wrong.
Trigger: Trying to prove the main-window preview GL context is detached (SignalBar expanded) before capturing fail-first evidence.
Rule: (1) `currentFps_` is stored at exactly ONE site, `Renderer.cpp:181`, which executes only INSIDE `renderOpenGL()`. On detach the render callback stops, so the last value PERSISTS INDEFINITELY — fps reads a healthy ~110 while the context is dead, i.e. it actively asserts liveness for a dead context. `openGLContextClosing()` (Renderer.cpp:695) does not reset it. Same defect in `/api/status` `frameTimeMs` (ApiServer.cpp:242 -> Renderer.cpp:634-635). (2) The VALID detach oracle is `POST /api/render_frame`: it blocks on a promise fulfilled only inside `renderOpenGL()` (TestServer.cpp:517 -> Renderer.cpp:1543-1583). ATTACHED => HTTP 200 in <0.1s (measured 0.08s). DETACHED => ~5s timeout then HTTP 500 "Frame capture failed" + stderr "[Eyes] Frame capture timed out after 5s". Works with no content loaded. (3) ALWAYS prove an oracle fires POSITIVE in the known-good state before trusting a negative reading from it.
**Bonus:** a startup fps reading of ~2.4e-06 is NOT a detach — `fpsTimer_` initialises to 0.0 (Renderer.h:211) and JUCE's hi-res counter is ms-since-boot, so the first-ever frame computes 1/uptime. 1/2.403e-06 = 416,090s = 4.82 days uptime. Read nothing into it.

### 2026-08-03 — Release build needs `--test-mode` or port 8080 never binds (and 7070 binding masks it)
Source: Same session. App looked perfectly healthy — process alive, audio analysis logging, 7070 bound, no TCC stall — but every 8080 probe returned empty.
Rule: (1) Launch for any Eyes/TestServer work with `open --stdout F --stderr G build/AudioDNA_artefacts/Release/Audio-DNA.app --args --test-mode`. Without the flag the ApiServer still binds 7070, so a "server is up" check against 7070 gives a FALSE GREEN while 8080 is absent. (2) TestServer binds `::1` ONLY — probe `http://[::1]:8080` or `localhost`, NEVER `127.0.0.1`, which returns empty and looks identical to a dead server. (3) The health route on 8080 is `/api/health`, NOT `/api/status` (which returns empty there). `/api/state` gives the full effect/param dump. (4) `--test-mode` and `--test-port=` are the only CLI flags; `AUDIODNA_API_BIND` is the only env var the app itself reads (Main.cpp:20-23, ApiServer.cpp:57-58).

### 2026-08-03 — AppleScript cannot reach JUCE's nested AX elements; use the AX C API
Source: Same session, trying to press the 20x14px SignalBar grow button without a flaky synthetic click.
Rule: JUCE buttons DO expose working macOS accessibility (verified: 91 AXButton nodes in the live tree, with exactly one "▼" and one "▲" — unique titles, so title lookup is unambiguous). But `System Events`' `entire contents of window 1` returns only the window's ~245 DIRECT children and does not recurse into JUCE's nested AXGroups, so both `whose title is "▼"` (error -1700) and an explicit repeat-loop over `entire contents` FAIL TO FIND IT. Do not conclude "no AX tree" from an empty AppleScript enumeration — that misdiagnosis cost a prior session. Dump the real tree with `tests/visual/ax_inspector.py --output /tmp/axtree.json` (root is under the `tree` key) and drive presses through the Accessibility C API via pyobjc instead.

### 2026-08-03 — Driving the output window in a gate can leave a black overlay on the user's other screens
Source: Boris reported mid-session that Audio-DNA was "putting a black overlay on all my screens except for the ones that are full screen" — after a C3 gate that opened/closed the fullscreen output window 3x across 2 launches and pkill'd the app several times.
Rule: (1) The output window is a REAL fullscreen window on Boris's actual displays — driving it in an automated gate has real, visible consequences on his machine, unlike headless test artifacts. Treat opening it as a user-affecting action: close it and verify the close before ending a session, and say in the handoff what state the app was left in. (2) The overlay SURVIVED a graceful app exit (stderr showed clean "HTTP server stopped"/"OSC Stopped listening", no .ips, no process left), so this is a teardown/ownership defect, not crash debris. (3) Diagnostic that halves the search space before reading any source: ask whether the overlay persists with NO process running. Persists => orphaned WindowServer/JUCE-shield artifact. Clears => live-window bug reproducible on demand. (4) When a session leaves an app running, killing it is not enough — verify the SCREEN state too, which no CLI probe can see (use `screencapture -x` + read the image).

### 2026-08-03 — LAW: verify the SCREEN before session close, never just the process
Source: Boris reported a black overlay on all non-fullscreen displays after a C3 gate session that drove the fullscreen output window 3x across 2 launches and pkill'd the app repeatedly. The overlay OUTLIVED a clean app exit (stderr showed graceful shutdown, no crash report, no process remaining).
Rule (MANDATORY, see SCREEN-SAFETY LAW in HANDOFF.md): (1) Audio-DNA's output window is a real fullscreen window on Boris's actual monitors — driving it in a gate has visible consequences on the machine he is using. Never end a session with it open. (2) Never pkill/SIGKILL the app while the output window is open — close the window first (`Output > "Disabled"`), let it tear down, then quit. (3) `pgrep` empty does NOT mean the screen is clean — this incident proves it. Run `screencapture -x` and READ THE IMAGE before declaring safe-to-close. Same class as the 2026-07-25 TCC-dialog gotcha: the screen holds state no socket or process probe can see. (4) Every session that launches the app must state in its handoff what window/app state it left behind and whether the screen was visually verified. (5) A Boris-reported screen artifact OUTRANKS the current lane — his machine is not a test rig.

### 2026-09-05 — an "only Boris can check" item may only need someone to LOOK
Source: s-rta-0904 — the MilkDrop Favorites empty-state string, carried as owner-only for 5 sessions
Trigger: an item is parked as ONLY BORIS CAN CHECK because it is described in perceptual terms ("does the tab say X")
Rule: before parking anything as owner-only, separate perceptual JUDGEMENT (his — taste, density, does-this-read-right) from perceptual ACCESS (yours — `screencapture -x` plus actually READING the image, or a cropped region via `sips -c H W --cropOffset Y X`). Access is not judgement. This item sat open for a month and took one screenshot; the answer ("No presets loaded." on Favorites, not "No favorites yet") confirmed a root cause that had been blocked on it.
Scope: universal
Promoted: no

### 2026-09-05 — a scheduled job can be silently serving the STALE DUPLICATE repo
Source: s-rta-0904 — `com.harmony.graphify-refresh.realtimeaudio-copy.plist`
Trigger: you are about to cite an automated pipeline (launchd job, cron, hook) as evidence that some data for THIS repo is fresh or self-maintaining
Rule: open the job definition and read its actual path strings before believing it serves this repo. The loaded graphify refresh job points every path at `~/projects/RealTimeAudio copy` (HEAD f128bdc, Jul 11 — the documented HAZARD duplicate), and there is no equivalent job for the real repo. An agent this session cited that job as proof the real repo's graph pipeline was alive; it was not. The existence of a job named after your project is not evidence it runs on your project.
Scope: universal
Promoted: no

### 2026-09-05 — graphify-out was force-tracked against this repo's own .gitignore
Source: s-rta-0904 — 271 tracked files / 105MB under a directory .gitignore already excluded
Trigger: `git status` shows hundreds of dirty paths under `graphify-out/` and you are tempted to ignore the noise
Rule: 258 of those files were `cache/ast/*.json` — pure derived cache the tool writes and cleans itself — force-tracked historically despite `graphify-out/` sitting in .gitignore. They are now untracked; the ~13 top-level graph outputs stay tracked so a fresh clone still gets a usable graph. A permanently dirty tree is not cosmetic: it destroys "is the tree clean?" as a signal, which is the check every close depends on. Expect a small residual: the post-commit hook launches a background regen, so after the last commit of a session a few graph files will read modified. That is bounded and explainable; 265 was not.
Scope: repo
Promoted: no

### 2026-09-05 — ctest reports 203/203 GREEN on top of a FAILED build
Source: s-rta-0904 — observed live while gating lane L1 (media-leak)
Trigger: you run `cmake --build . && ctest` (or run them as separate steps) and read the ctest number
Rule: `cmake --build .` returned **BUILD_RC=2 with 14 compile errors**, and the `ctest` run immediately after still printed `100% tests passed, 0 tests failed out of 203` — because it executed the PREVIOUS build's stale binaries. A green suite sitting on a failed compile is the most convincing wrong answer available in this repo. ALWAYS capture the build's exit code and treat a non-zero one as terminal: do not run ctest, and never quote a ctest number, until the build that produced those binaries actually succeeded. This is the concrete instance of the packet's standing warning "FORCED REBUILD before any ctest claim (stale-binary false-green is a documented trap here)" — it is not theoretical.
Scope: universal
Promoted: no

### 2026-09-05 — changing a command constructor signature: sweep tests/ too, or lose three rounds
Source: s-rta-0904 — lane L1 (media-leak) took FOUR compile rounds, three failing for this one reason
Trigger: you add or remove a parameter on any `Command` subclass in `src/core/*Commands.h`
Rule: `tests/test_undo_commands.cpp` constructs these commands as heavily as production does — it held 8 of the 10 call sites for `RemoveLayerCmd`/`RemoveDeckCmd`. Before considering the edit finished, run `grep -rn '<CmdName>' src/ tests/` and update EVERY site. The compiler stops reporting once a translation unit fails, so a clean-looking "only 2 errors" can hide more behind them. Related trap: when a test call site needs the new argument, passing a noop/placeholder to satisfy arity silently converts a covered surface into a falsely-covered one — at least one test per changed command must pass a real hook and assert on it.
Scope: repo
Promoted: no

### 2026-09-05 — the incremental build can emit a FALSE LINK ERROR from a stale object
Source: s-rta-0904 — lane L7, nearly reverted a correct lane on this
Trigger: the build fails with `Undefined symbols ... "Class::method()", referenced from: ... in <File>.cpp.o` and you have just added that method's body
Rule: CHECK THE DEFINITION EXISTS AND IS CORRECTLY SCOPED BEFORE BELIEVING THE LINKER. In this repo `cmake --build .` did not recompile `src/ui/PreferencesDialog.cpp` after its body was added — the object kept the call sites from an earlier compile and lacked the definition, so the link failed against code that was already correct on disk. `grep -n '<Class>::<method>' src/` showed the definition present, correctly scoped, with no preprocessor guard around it. `touch`ing the .cpp and .h and rebuilding linked cleanly first try. This is the MIRROR IMAGE of the documented stale-binary false-GREEN (ctest passing on binaries from a failed build): the same staleness produces a false RED at link time. Both come from trusting an incremental build's verdict over the source on disk. When a build result contradicts what you can read in the file, force the rebuild before you act on it — and never revert a lane on an unreproduced failure.
Scope: universal
Promoted: no

### 2026-09-05 — Audio-DNA enforces ONE instance: a running app blocks EVERY behavioural gate
Source: s-rta-0905 — Boris had the app open (GUI-launched 3 min before the session started) for the whole session
Trigger: you are about to launch the app for any gate — health check, menu drive, TSan run, screenshot
Rule: the app self-terminates a second instance ("Another instance is running - quitting...", visible on stderr). Worse, it is silent from the caller's side: `open` on EITHER bundle (`build/AudioDNA_artefacts/Release` or `build-tsan/AudioDNA_artefacts/Debug`) does not launch anything at all — LaunchServices matches the bundle id and just ACTIVATES the already-running window, so you get exit code 0, an empty stderr file, and no new process. `open -n` does start a process, which then immediately self-quits. **Run `pgrep -fl 'MacOS/Audio-DNA'` BEFORE you plan any gate, not after you have built.** If an instance is up and it is not yours, you cannot gate behaviourally at all this session: say so plainly rather than substituting a source review and calling the lane gated. A user's own running instance is not yours to quit (SCREEN-SAFETY LAW) — and note it may be running a PRE-FIX binary, so anything they observe in it is not evidence about your change.
Scope: repo
Promoted: no

### 2026-09-05 — a work packet's prescribed ORDER is a claim, exactly like its counts
Source: s-rta-0905 — lane L3 Step 2; the Fable-authored packet's own §1 sequence contained a use-after-free
Trigger: you are implementing a step-ordered sequence from a packet that swaps or destroys a model
Rule: this repo already knows a packet's COUNT is a claim ("both layer loops" was three; "three vacate paths" was six). Its ORDER is too. L3's packet ordered `deckView_->setActiveColumn(-1)` BEFORE `rebuildGrid()`; `setActiveColumn` calls `refresh()`, which walks `LayerStrip::refresh()`, which dereferences a raw `Layer*` into the just-destroyed deck — and that function's `if (!layer_)` guard catches NULL but not DANGLING. The builder reordered it after `rebuildGrid()` and the independent reviewer re-traced the chain and vindicated the deviation. **Generalisation worth carrying: never call a widget-container's own refresh between destroying a model and rebuilding the children that hold pointers into it — nulling the model-holding widgets one layer up (the inspectors) is not enough, because the container walks its children directly.**
Scope: universal
Promoted: no

### 2026-09-05 — a DECLARED uniform is not a USED uniform: the "56 of 57 shaders are phase-driven" figure was a declaration count
Source: s166 recon (recon-tempo-and-timed-sources.md) re-deriving a load-bearing number from the s-rta-0905 handoff, which used it to argue a lane was not worth building
Trigger: you are about to quote any count of "shaders that use X", "files that reference Y", or any figure derived from grepping a uniform/symbol NAME
Rule: 51 of the 243 embedded shaders DECLARE `u_beatPhase` (56 raw hits, one shader declares it six times) but only **8 actually reference it in GLSL math** — the other 43 carry a copy-pasted boilerplate uniform block. The real ratio against `u_bpm` is ~8:1, not 56:1. A declaration is boilerplate; a use is evidence. **Grep for the symbol in an EXPRESSION context, not just anywhere in the file, before any count of "how many things use this" is allowed to inform a build/no-build decision.** This number had already been carried forward across two sessions and used as an argument.
Scope: universal
Promoted: no

### 2026-09-05 — the same "connect a parameter to a signal" feature has been built FOUR times and finished zero times
Source: s166 recon (recon-universal-connect.md, recon-effects-rack.md, recon-timing-and-compinspector.md), three independent passes converging
Trigger: you are about to build, extend, or delete anything that binds a parameter to an audio feature, an oscillator, a macro or a route
Rule: this repo contains at least FOUR partial implementations of one idea: (1) `MappingEngine` v1 — owns the only real curve set (~20 curves, `src/mapping/CurveTransforms.h`), still ticks every frame, still round-trips through preset save/load, UI (`EffectsRackPanel`/`MappingEditor`) unreachable; (2) `RoutingEngine`/`Route` (`src/routing/`) — thread-correct, GL-thread tick, richest shaping and fully generalized addressing, `addRoute()` has ONE caller and it is `TestServer.cpp`; (3) the two live `tickModulation()` loops (`EffectStackView.cpp`, `ClipInspector.cpp`) — the only path a user can actually reach, unsynchronized message-write/GL-read; (4) `previewPanel_`'s own `MappingEngine`/`EffectChain` on a separate global chain, still ticked. **Before adding a fifth, establish which one you are extending and what happens to the other three — and check preset-format compatibility, because v1 mappings on disk still apply silently.**
Scope: repo
Promoted: no

### 2026-09-05 — parallel lanes in this repo share ONE cmake build dir, and a file fence does not protect it
Source: s166 — three builders dispatched in parallel (favorites persistence, global-effects compositing, clip-replace media leak); caught by a routine `git status` fence check before any lane committed
Trigger: you are about to dispatch more than one builder into this repo at the same time
Rule: two collisions that a per-file fence cannot see. (1) **Open-ended fences overlap.** One packet's fence said "the call-site file(s) you must touch — name them in your report", which resolved to `src/MainComponent.cpp` — the same file another lane's fence named explicitly. Both lanes' hunks ended up in the working tree together, and either lane committing would have swept in the other's unreviewed in-flight work. **Resolve every fence to concrete paths before dispatch and intersect them pairwise; an open-ended fence is not a fence.** `src/MainComponent.cpp` is ~5000+ lines and is the natural landing site for a large fraction of lanes, so it is the collision magnet here — assume any two lanes touch it until proven otherwise. (2) **One `build/` directory.** Concurrent `cmake --build build` runs corrupt each other's objects and yield failures attributable to no lane, invalidating every build exit code and ctest number quoted while they overlapped. Either serialize build slots (grant one lane at a time an exclusive slot and say so in the packet) or give each lane its own build dir. A file fence says nothing about shared toolchain state.
Scope: universal
Promoted: no
