# MilkDrop crash #2 TRUE root cause — deterministic UAF (2026-07-30)
Scout: milkdrop-race-scout. Disassembly-verified, both .ips fit exactly. HIGH confidence.

## Mechanism (single-threaded, no race)
previewPanel_.setVisible(false) — e.g. SignalBar EXPANDED mode (MainComponent.cpp:1727)
or glHost_ collapsing to 0 width/height via dividers (juce_OpenGLContext.cpp:1103-1108) —
→ JUCE detaches GL SYNCHRONOUSLY on the message thread (ComponentMovementWatcher →
componentVisibilityChanged juce_OpenGLContext.cpp:1129 → detach:1095 → stop →
CachedImage::stop:176 → pause:194 → renderer->openGLContextClosing() at :205)
→ Renderer::openGLContextClosing() does activeSources_.clear() (Renderer.cpp:668-673)
→ ProjectMSource destroyed WITH its BY-VALUE ProjectMPresetManager (ProjectMSource.h:105)
→ MilkDropBrowser::presetManager_ = raw interior pointer (wired ONCE, MainComponent.cpp:1303-1334)
   now DANGLING; never re-wired
→ every later browser resized()/paint() = UAF read.

## Instruction-level proof
getCuratedPresets+128 = `ldr s0,[x23,#0x60]` = p.energy (PresetInfo 0x68 bytes, energy @0x60).
07-30: x23=0x133793fc0, far=+0x60 exact; 07-28 (+104 pre-guard codegen): base 0x1, far 0x61 exact;
both hold x24=0x3dcccccd (0.1f literal from `p.energy > 0.1f`). presets_.begin/end loaded from
manager+0x20 (onPresetChanged std::function precedes presets_, ProjectMPresetManager.h:79/82).
07-28 element ptr 0x1 = impossible for any live vector ⇒ object dead, memory reused.
GL thread idle in driver in both reports ⇒ no concurrent mutation. 9229f87 guards can't help
(pointer non-null, garbage size non-zero). Empty-state + race theories DEAD.

## 07-28 stack fit
SignalBar arrow (onSizeChanged → resized(), MainComponent.cpp:566; grow/shrink SignalBar.cpp:81/97)
→ MainComponent::resized() lays preview :1879-1880 then browser :1900 → BrowserPanel::resized():84
→ MilkDropBrowser::resized():693 → calculateContentHeight():940 → getCuratedPresets():927. Exact.

## Corroborating non-crash symptom
scanDirectory/loadManifest run ONCE (MainComponent ctor :1321/:1325/:1331) ⇒ after any detach the
RE-created ProjectMSource has ZERO presets forever — MilkDrop clips render nothing after one
expand/collapse. (Check with Boris — may explain part of the "browser/presets empty" history.)

## Fix ranking (scout)
1. ONE LINE: keep releaseGL() loop, DELETE activeSources_.clear() in openGLContextClosing
   (Renderer.cpp:668-673). Verified: releaseGL fully tears down GL (projectm_destroy+FBO);
   render() lazily re-inits on !glInitialized_ (ProceduralSource.cpp:92-93, ProjectMSource.cpp:111);
   no GL calls in ~ProjectMSource. Fixes UAF AND the presets-vanish bug.
2. Durable follow-up (~1h): MainComponent owns ProjectMPresetManager; sources hold pointer.
3. Browser snapshot copy — bigger, doesn't fix presets-vanish. 4. Locks/nulling — rejected.

## Residuals
- Fix 1 shifts source destruction to Renderer dtor — shutdown-crash lane must be told (same teardown).
- NEW SEPARATE FINDING → queue candidate (11): activeSources_ unordered_map mutated with NO mutex
  from message thread (MainComponent.cpp:1304,1340), GL thread (Renderer.cpp:322/336/347,762), and
  HTTP worker (TestServer.cpp:893) — concurrent operator[] on unordered_map is UB. Post-startup all
  hit find() fast path (not the cause of these crashes) but harden separately.
- INFERRED only: that the 07-30 session's arming event was the (Harmony) SignalBar expand at 13:02 —
  code path proven; the specific arming instance not directly observable post-hoc.

## Verification recipe (reproducible)
Browser→MilkDrop tab (presets listed) → SignalBar arrow to Expanded → back → repaint browser:
pre-fix = crash or garbage/empty list; post-fix = presets intact, no crash. MallocScribble=1 makes
the pre-fix read fault deterministically (0x55… pattern). Optional log pair: openGLContextClosing +
~ProjectMSource to prove the detach fires.
