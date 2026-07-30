# Project Gotchas — Audio-DNA

<!-- Captured by EOS from dispatch failures. EOD promotes universal entries. -->
<!-- Builder reads this via SELF-BRIEF. Harmony includes relevant entries in work packets. -->
<!-- Format: each entry has Source, Trigger, Lesson, Scope (repo|universal), Promoted (yes|no). -->

(No gotchas captured yet. Entries appear after Builder dispatches encounter failures.)

### 2026-05-18 — conftest autouse fixture blocks non-Eyes tests
**Source:** Builder A report — test_ax_inspector.py initially skipped all tests
**Trigger:** tests/visual/conftest.py has `autouse=True` fixture `reset_between_tests` that depends on the `app` fixture, which skips when Audio-DNA executable is not built
**Rule:** Always override `app` and `reset_between_tests` fixtures locally in any new test file in tests/visual/ that does NOT need the running app
**Scope:** repo
**Promoted:** no

### 2026-05-18 — FetchContent must pin versions for reproducible builds
**Source:** Tester CONCERN — melatonin_inspector used `origin/main` instead of SHA
**Trigger:** Builder followed melatonin_inspector README which recommends `origin/main`
**Rule:** Always pin FetchContent GIT_TAG to a specific SHA or release tag, with GIT_SHALLOW TRUE. Match existing pattern in CMakeLists.txt.
**Scope:** repo
**Promoted:** no

### 2026-05-19 — load_image renders black in production mode
**Source:** EOS test validation — effect tests PSNR=inf
**Trigger:** ApiServer load_image sets a texture but production renderer draws composition layers, not standalone images. Tests using load_image + render_frame get black frames.
**Rule:** Always use load_source with a procedural source (e.g., checkerboard) for effect comparison tests in production mode. Reserve load_image for test-mode only.
**Scope:** repo
**Promoted:** no
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
**Source:** EOS T1 fix — compared TestServer vs ApiServer load_image
**Trigger:** ApiServer handleLoadImage returned before GL thread processed the pending image load, causing subsequent render_frame to capture stale content.
**Rule:** Always add juce::Thread::sleep(100) after renderer_.loadImage() in HTTP handlers to give the GL thread time to process.
**Scope:** repo
**Promoted:** no

### 2026-05-21 — KeyingMode enum ≠ Layer::Type — verify enums directly
**Source:** Builder A HAD_TO_INFER — audit listed 4 "keying modes" (Opaque, Transparent, FXOnly, Mask) but those were Layer::Type values. Actual KeyingMode enum has 13 entries (Alpha through ChannelB).
**Trigger:** Confusing two related enums in the compositing pipeline. Layer::Type controls compositing behavior; Layer::KeyingMode controls transparency extraction.
**Rule:** Always verify enum names from header files, not from audit summaries. Two enums in the same class can have overlapping conceptual scope.
**Scope:** repo
**Promoted:** no

### 2026-05-21 — Composition transform fields are runtime-only (not serialized)
**Source:** Builder A discovery — compPositionX/Y, compScale, compRotation, compAnchorX/Y exist in Composition struct but are NOT in toVar() serialization.
**Trigger:** Assuming all model fields persist. These are runtime-only adjustments.
**Rule:** Check toVar()/fromVar() to confirm which fields actually serialize before documenting persistence behavior.
**Scope:** repo
**Promoted:** no
**FIXED 2026-07-17:** now serialized (Wave 1-C) — compPositionX/Y, compScale, compRotation, compAnchorX/Y and every other dropped Clip/Layer/Composition field now round-trip through toVar/fromVar (Clip.cpp / Layer.cpp / Composition.h). The general rule above still stands: verify toVar/fromVar before documenting persistence.

### 2026-05-21 — Session Recorder has 7 event types but only 1 caller wired
**Source:** Builder B discovery — recordClipTrigger() is the only event type actually called from production code. Other 6 record methods (ParameterChange, ColumnTrigger, MacroChange, TransportChange, EffectToggle, CuepointJump) are implemented but have no callers.
**Trigger:** Assuming a feature is complete because the API exists.
**Rule:** When documenting event/callback systems, grep for callers of each method. API existence ≠ integration.
**Scope:** universal
**Promoted:** yes → audit-workflow (2026-06-01)

### 2026-05-22 — §3.6 shorthand vs §6.7 full spec contradiction missed by vague cross-ref audit
**Source:** Tester catch on MOCKUP_BRIEF cleanup — Builder applied opacity-floor edits to §6.7 + §6.2 but left §3.6 reading "direct 1:1 mapping" (semantically contradictory but defensible). Edit 8 packet wording said "scan for conflicting Active triangle descriptions" but didn't list §3.6 explicitly. Builder rationalized leaving it.
**Trigger:** Cross-reference audit instructions that say "scan for X" without enumerating known candidate sites. Builder reads narrowly and skips defensible-but-contradictory sites.
**Rule:** When a packet edits canonical doc and authoritative spec changes (new floor, new state, new rule), enumerate EVERY site that paraphrases or shorthands that spec — not just direct contradictions. List candidate sections by number in the cross-ref audit instruction, not just by topic.
**Scope:** universal
**Promoted:** yes → feature-build (2026-06-01)

### 2026-05-22 — §6-tokens-win-over-§5-pixel-specs (MOCKUP_BRIEF authority hierarchy)
**Source:** Boris-locked decision during MOCKUP_BRIEF cleanup. §5 cites pixel specs from praised v9 mockups; §6 declares canonical production tokens. Pre-cleanup, §5 = 50px and §6 = 60px for signal column.
**Trigger:** Builder reading §5 quality-bar pixel specs and treating them as authoritative production values.
**Rule:** MOCKUP_BRIEF §6 production tokens always override §5 quality-bar pixel specs when they differ. §5 is inspiration, §6 is contract. New §8.5 Token Sync Verification enforces this on every v10 mockup.
**Scope:** repo
**Promoted:** no

### 2026-05-22 — No push to remote (CI failure on Audio-DNA)
**Source:** Boris session 15 — "I am getting messages that run failed in git remote. we should be making all of these locally. no need to push to remote at all."
**Trigger:** Auto-commit-and-push protocol pushing to Audio-DNA remote triggers CI workflow that's currently failing.
**Rule:** Commit locally to RTA, do NOT push to remote until Boris re-enables. Captured 2026-05-22 mid-session. May be temporary — verify with Boris before resuming pushes.
**Scope:** repo (RealTimeAudio / Audio-DNA only)
**Promoted:** no

### 2026-05-22 — B+ minimum quality bar for design-variation evaluation
**Source:** Boris session 15 — "Re-design anything that is not B+ level" then "refine all 20 to b+ and make the the default behavior in the future regardless of the task, you'll finish what I ask you for."
**Trigger:** When Harmony spawns a Critic agent to grade design variations (layer-rows, mockups, page studies), and Boris requests "evaluate and refine".
**Rule:** Anything graded below B+ gets refined in-place via Refiner Builder. Don't ask which to refine — refine all sub-B+ items. Re-Critic verifies B+ achieved. Iterate max 2 loops. Pattern: Critic (Tester + design-review methodology + Playwright) → Refiner (Builder with per-variation recommendations) → Re-Critic. Validated session 15: 13/13 refinements achieved B+ first pass.
**Scope:** repo (RealTimeAudio design work). Boris may elevate to universal if same pattern applies to other projects' design audits — currently RTA-scoped.
**Promoted:** no

### 2026-07-16 — All shipped shaders are EMBEDDED; shaders/ dir files are dead + hot-reload is inert
**Source:** 7-lane re-norm audit (L2 render-effects)
**Trigger:** Assuming the 5 files in `shaders/` (hue_shift/rgb_split/ripple/vignette/passthrough) are live, or that ShaderManager hot-reload works.
**Rule:** Every effect/transition/source shader ships as an inline string in `src/render/EmbeddedShaders.h` and is compiled via `compileProgram` (275 compile() calls in Renderer.cpp). `ShaderManager::reloadAll()` only reloads programs compiled *from files* (empty vertFile/fragFile skipped), so hot-reload is inert for the entire shipped set. The 5 `shaders/*.frag|vert` disk files are dead duplicates. Edit shaders in EmbeddedShaders.h, not the disk files.
**Scope:** repo
**Promoted:** no

### 2026-07-16 — SourcesBrowser list is hand-maintained, NOT generated from SourceRegistry
**Source:** 7-lane re-norm audit (L3 sources-media, L5 ui-surfaces)
**Trigger:** Adding a procedural source to `SourceRegistry` and expecting it to appear in the GUI Sources browser.
**Rule:** `SourcesBrowser.cpp` uses ZERO `SourceRegistry` references — its ~103 source rows are a hand-maintained list. A new source must be added in BOTH places (registry + SourcesBrowser) or it is API-selectable but invisible in the GUI. 6 registered sources (strange_attractor, gravity_well, fluid_dynamics, text_animator, layer_router, projectm_visualizer) are currently missing from the browser for this reason.
**Scope:** repo
**Promoted:** no

### 2026-07-16 — 48kHz is hardcoded across analysis with no runtime validation
**Source:** 7-lane re-norm audit (L1 audio-analysis)
**Trigger:** Assuming the analysis pipeline adapts to the device sample rate.
**Rule:** `kSampleRate=48000` (AnalysisThread.h:46) plus the LoudnessAnalyzer K-weighting biquad coefficients (ITU-R BS.1770 48k, LoudnessAnalyzer.cpp:13-29) and all frequency math assume 48kHz with NO runtime SR check. A device running at 44.1k/96k silently produces wrong LUFS/frequency features. Do not assume SR-independence when touching analysis.
**Scope:** repo
**Promoted:** no

### 2026-07-17 — Launch Audio-DNA via `open`, NEVER direct binary exec, for behavioral gates
**Source:** Harmony Wave-0 behavioral gate — direct exec of `build/AudioDNA_artefacts/Release/Audio-DNA.app/Contents/MacOS/Audio-DNA` from an agent shell hangs pre-UI forever (process alive, ZERO windows, ZERO listening sockets, empty log, 2+ min) — looked exactly like a broken REST server.
**Trigger:** Running the app binary directly from a shell/agent context to probe the port-7070 API.
**Rule:** Always launch with `open build/AudioDNA_artefacts/Release/Audio-DNA.app` (LaunchServices context); port 7070 binds ~12s after launch. Poll /api/health before probing. Kill with `pkill -f Audio-DNA`.
**Scope:** repo
**Promoted:** no

### 2026-07-17 — Post-render GPU steps must restore GL_FRAMEBUFFER to defaultFBO
**Source:** Wave 1-A builder
**Trigger:** Adding a post-render GPU step (e.g. Syphon `publishSyphonFrame`) that binds its own FBO/texture and returns without re-binding the default framebuffer.
**Rule:** `Renderer::processPendingCapture`'s `glReadPixels` assumes the default FBO is bound after rendering; any post-render GPU step (e.g. Syphon `publishSyphonFrame`) must re-bind `defaultFBO` when done, as `publishSyphonFrame` does (Renderer.cpp ~1710-1762).
**Scope:** repo
**Promoted:** no

### 2026-07-19 — First `open` can transiently stall in CoreAudio/TCC init (looks like the direct-exec hang)
**Source:** Undo v1 step-1 builder (fence-validation live run)
**Trigger:** First `open` of Audio-DNA.app after a rebuild sometimes stalls inside the ctor at `AudioDeviceManager::initialiseWithDefaultDevices` → CoreAudio/TCC — process alive, no windows, no port 7070, indistinguishable from the direct-exec hang gotcha (2026-07-17).
**Rule:** Before concluding the build is broken, run `sample <pid>` to pinpoint the stall; if it's in AudioDeviceManager/CoreAudio init, `pkill -9` and re-`open` — second launch typically binds :7070 in ~4s. Environment flake (TCC/audio permissions), not code.
**Scope:** repo
**Promoted:** no
**ROOT CAUSE FOUND 2026-07-25:** see the TCC mic-prompt entry below — the "stall" is the app blocking on an unanswered microphone-permission dialog.

### 2026-07-19 — Renderer media resources are keyed by clip.id with NO file-match check
**Source:** Undo v1 step-2 reviewer (caught pre-commit) — video replace-undo was a silent visual no-op
**Trigger:** Any flow where a clip's CONTENT changes but its id doesn't (replaceContent keeps id), combined with a "reconnect if missing" guard: videoPlayers_/imageSequences_ lookups use id only, so a guard that checks existence sees the id-keyed player (loaded with the NEW file) and skips reopening — model and renderer silently diverge.
**Rule:** Existence of an id-keyed renderer resource does NOT imply it holds the right content. Any reconnect/restore path must compare the loaded file against the model's current mediaFile (or reopen unconditionally) when ids are content-stable. Undo/redo, preset load, and future player-disposal waves all hit this.
**Scope:** repo
**Promoted:** no

### 2026-07-16 — FeatureSnapshot::clear() resets genre/energy to 0 (House/low), not struct defaults
**Source:** 7-lane re-norm audit (L1 audio-analysis)
**Trigger:** Expecting a freshly-cleared FeatureSnapshot to carry the struct's default genre/energy (detectedGenre=6, energyState=1).
**Rule:** `FeatureSnapshot::clear()` memsets then restores only rmsDB/lufs/detectedKey/keyIsMajor/swingRatio — it leaves `detectedGenre=0` (House) and `energyState=0` (low), NOT the struct defaults 6/1 (FeatureSnapshot.h:81-89). FeatureBus inits all 3 buffers via clear(), so this is the effective startup default. Latent bug; verify before relying on cleared-snapshot genre state.
**Scope:** repo
**Promoted:** no

## 2026-07-22 — CoreAudio launch stall WORSENS with repeated pkill -9 cycles (env wedge, not code)
**Source:** Undo v1 step-5 behavioral gate (s. 2026-07-22)
**Trigger:** Launching Audio-DNA via `open` after prior instances were pkill-9'd mid-CoreAudio-start.
**Rule:** The 2026-07-19 first-open stall (CoreAudioInternal::start mutex wait; remedy sample → pkill -9 → re-open) can WEDGE PROGRESSIVELY: after 1-2 remedy cycles the stall reproduces on EVERY relaunch (3 consecutive this session; step-4 gate an hour earlier recovered on attempt 2). A 90s coreaudiod settle window did NOT clear it. Signature verified by sample both times: identical CoreAudioClasses::AudioIODeviceCombiner::start → CoreAudioInternal::start → __psynch_mutexwait. Change-independence verified: step-5 diff has 0 audio-path refs; stall predates the code. Remedy beyond the loop: restart coreaudiod (`sudo killall coreaudiod`) or logout/reboot — Boris-level. Gate policy used: code gates (build/ctest/residue/review) green → app-level check recorded BLOCKED-ENVIRONMENTAL, launch verification prepended to manual checklist.
**Scope:** repo (macOS env interaction)
**Promoted:** no
**SUPERSEDED 2026-07-25 — root cause was NEVER a coreaudiod wedge:** see next entry. Reboot did not "fix" it because the blocker is a TCC dialog, not the daemon; `sudo killall coreaudiod` is NOT the remedy and should not be requested again.

### 2026-07-25 — ROOT CAUSE: CoreAudio launch "stall/wedge" = unanswered TCC microphone prompt; ad-hoc signing re-fires it every rebuild
**Source:** Undo v1 step-8 session env recheck — post-reboot launch still "stalled"; `screencapture` + Read of the PNG revealed a live TCC dialog ("Audio-DNA would like to access the microphone", Don't Allow/Allow) that no CLI probe can see.
**Trigger:** Launching Audio-DNA from a headless/agent session. `CoreAudioInternal::start` blocks (verified by sample, same signature as 07-19/07-22 entries) waiting on the TCC microphone-permission response; with nobody at the screen the dialog is never answered → launch never binds :7070. Because the app is **ad-hoc signed** (`codesign -dv`: Signature=adhoc, no TeamIdentifier — verified), every rebuild changes the cdhash, so TCC re-prompts after EVERY rebuild (inferred, standard TCC behavior — explains recurrence across sessions and why pkill/reboot/coreaudiod-restart never helped).
**Rule:** (1) After any rebuild, the FIRST app launch needs a human to click **Allow** on the mic prompt — schedule app-level behavioral gates for when Boris is present, or have him click Allow right after the gate's launch. (2) Diagnose headless launch stalls with `screencapture -x /tmp/x.png` + image read — TCC/system dialogs are invisible to sample/lsof/log probes. (3) Do NOT pkill-cycle or restart coreaudiod for this signature. (4) Durable fix option (Boris): sign dev builds with a stable Developer ID identity so TCC remembers the grant across rebuilds. (5) Synthetic clicks can't answer TCC prompts without Accessibility for the calling process (osascript denied assistive access — verified).
**Scope:** repo (macOS env interaction); the screenshot-diagnosis method is universal-candidate
**Promoted:** no

### 2026-07-30 — Synthetic UI driving of Audio-DNA (recipes + flake profile)
**Source:** Autonomous e2e drive (Accessibility granted to Ghostty; fence-lane app verification)
**Trigger:** Driving the JUCE UI via System Events/CGEvent for headless e2e.
**Rule:** (1) Cell THUMBNAIL click = TRIGGER; cell NAME BAR (bottom ~20px) = SELECT
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
**Scope:** repo (recipes); the verify-and-retry synthetic-event protocol is universal-candidate
**Promoted:** no
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
