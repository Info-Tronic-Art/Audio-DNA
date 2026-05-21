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

### 2026-05-21 — Session Recorder has 7 event types but only 1 caller wired
**Source:** Builder B discovery — recordClipTrigger() is the only event type actually called from production code. Other 6 record methods (ParameterChange, ColumnTrigger, MacroChange, TransportChange, EffectToggle, CuepointJump) are implemented but have no callers.
**Trigger:** Assuming a feature is complete because the API exists.
**Rule:** When documenting event/callback systems, grep for callers of each method. API existence ≠ integration.
**Scope:** universal
**Promoted:** no
