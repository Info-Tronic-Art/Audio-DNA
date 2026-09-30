# Verify F18 (sweep s-rta-0929b) - Layer::eff manual float read, Layer.cpp:23 vs manualWriteCore ManualWrite.cpp:187
VERDICT: REAL_BUG (C++ data race, UB by the letter; same class as F2/F9/F15) - severity LOW - owner: app. Not a tool artefact.

## Report read (VERIFIED, runs/tsan-3/tsan.49175 + tsan-6/tsan.50561 both contain the block)
- Read 4 B, T47 "OpenGL Renderer" (mutexes M0 JUCE render mutex, M1 CGL ctx): Layer::eff Layer.cpp:23
  <- CompositorEngine::compositeDeck :1157 (`layer.eff(LayerScalar::Opacity)`) <- Renderer::renderOpenGL :728.
- Prior write 4 B, main thread: manualWriteCore ManualWrite.cpp:187 (`*r.manual = ...`) <- MainComponent $_96 <- ApiServer::handleSetLayerOpacity
  lambda <- juce callAsync. Target = Layer::opacity (plain `float`, Layer.h:62) via manualRef (Layer.cpp:6-9). Heap 3200 B = live vector<Layer>.
- Correction to the sweep note: the racing address is the PLAIN manual float (layer.opacity), NOT scalarLive. scalarLive is LiveValue = relaxed
  std::atomic<float> (LiveValue.h:19,35-38) and is race-free; the race is on the `manualRef(...)` argument evaluated at Layer.cpp:23.

## Refutation attempts (all failed)
1. Atomic/lock: opacity is plain float, no lock in manualWriteCore (ManualWrite.cpp:179-188) or eff(). VERIFIED.
2. Hidden JUCE lock: mutex set in report is only render lock + CGL; message thread holds none. callAsync gives API-thread -> message-thread
   happens-before only, not message -> GL. VERIFIED (report) / INFERRED (callAsync semantics).
3. Benign counter / third-party: live control state in app code, JUCE/httplib only appear as call-stack carriers. VERIFIED.
4. Grip/conn state written in the same call (manualTouchCore ManualWrite.cpp:158-161) is also unsynchronised, but only message thread touches it. INFERRED.

## Live-show impact
- Crash/heap corruption: none. A single aligned float load/store; no pointer or size involved, layer storage not resized by this write. INFERRED.
- Torn value: none on arm64/x86 (aligned 4 B single-copy atomic). INFERRED.
- Real effect: the render frame sees the old or the new opacity, one frame earlier/later. Visually nothing. Compiler could in theory
  reload the value, but eff() reads once and passes it by value into effective(); the caller stores it in a const local (:1157). Practically nothing.
- Likelihood: race is certain to exist whenever REST/OSC/MIDI/UI writes layer opacity (every write); hit 2/6 TSan only because the fixture hammers set_layer_opacity.
  Every manual write path (UI slider, MIDI, OSC, routines) goes through manualWriteCore -> same race for all 7 LayerScalars and the clip/comp scalars.

## Smallest fix
- Make the manual scalar members relaxed-atomic (LiveValue-style copyable wrapper) for the 7 Layer and 7 Clip scalars; ~30 lines, removes the UB/TSan noise;
  manualRef must return the wrapper, ControlRef::manual type changes with it (touches ManualWrite/ConnectionEngine call sites).
- Cheaper alternative: TSan suppression for manualWriteCore/Layer::eff with a comment (documented benign single-float). One fix covers F2/F9/F15-class B and F18.
- Sacred Rule 2 gap (UI -> render via plain float, not std::atomic), not a show risk.
