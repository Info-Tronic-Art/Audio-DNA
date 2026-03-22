# Signal Routing Test Specification

Defines the API endpoints and test coverage needed to comprehensively test the signal routing system.

## Current State

The signal architecture is built (`SignalRegistry`, `RoutingEngine`, `MacroBank`, `Route`, `Signal` types) and unit-tested (`test_routing_engine.cpp` — 15+ tests). But:

1. **RoutingEngine is not wired into the render pipeline** — `Renderer::renderOpenGL()` doesn't call `routingEngine.processFrame()` yet
2. **TestServer doesn't expose signal/routing APIs** — can't create routes or query signal values from Python
3. **Feature injection works** — FeatureBus injection is implemented and sources use `u_rms`, `u_beatPhase` etc. directly

## What Works Now (Testable Today)

`test_signals.py::TestFeatureInjectionChangesOutput` — inject audio features via `/api/inject_features`, verify that sources and effects respond visually. This tests the direct `u_rms` → shader uniform path.

## What Needs API Endpoints

### Required Endpoints (add to TestServer)

```
GET  /api/signals
  → { "signals": [
       { "id": 1, "name": "Volume", "category": "Amplitude",
         "type": "audio", "value": 0.72 },
       { "id": 14, "name": "Mod 1", "category": "Modulation",
         "type": "oscillator", "shape": "sine", "beatDuration": 1.0 },
       ...
     ]}

POST /api/add_route
  Body: { "source_signal_id": 1, "target_effect": "Ripple", "target_param": "intensity",
          "output_min": 0.0, "output_max": 0.8, "threshold": 0.0, "gain": 1.0,
          "inverted": false }
  → { "ok": true, "route_id": 42 }

DELETE /api/remove_route/{id}
  → { "ok": true }

GET  /api/routes
  → { "routes": [
       { "id": 42, "source_signal_id": 1, "source_name": "Volume",
         "target_effect": "Ripple", "target_param": "intensity",
         "output_min": 0.0, "output_max": 0.8, "current_output": 0.58,
         "threshold": 0.0, "gain": 1.0, "inverted": false },
       ...
     ]}

POST /api/set_macro
  Body: { "scope": "global", "index": 0, "source_signal_id": 1 }
  or:   { "scope": "global", "index": 0, "manual_value": 0.75 }
  → { "ok": true }

GET  /api/macros
  → { "macros": { "global": [...], "layer": [...], "clip": [...] } }
```

### Implementation Requirements

1. TestServer needs `SignalRegistry&` and `RoutingEngine&` references (add to constructor)
2. MainComponent needs to pass them when creating TestServer
3. Route creation needs to resolve effect/param names to indices via EffectLibrary
4. Signal evaluation must happen before route processing in the render loop

### Integration Prerequisites

Before signal routes can produce visual changes, `Renderer::renderOpenGL()` needs:

```cpp
// After acquiring FeatureSnapshot:
signalRegistry_.evaluateAll(snapshot);
macroBank_.updateValues(signalRegistry_);
routingEngine_.processFrame(signalRegistry_, [this](const Route& route, float value) {
    // Write value to the target effect parameter
    if (auto* effect = effectChain_.getEffect(route.targetEffectIndex))
        effect->setParamValue(route.targetParamIndex, value);
});
```

## Test Coverage Matrix

### Tier 1: Signal Basics (unit tests — exist in test_routing_engine.cpp)

| Test | Status | What It Verifies |
|------|--------|-----------------|
| AudioSignal extracts RMS from snapshot | PASS | Signal.getValue() |
| AudioSignal extracts bandEnergies | PASS | Array field extraction |
| OscillatorSignal sine at various phases | PASS | Waveform math |
| OscillatorSignal all 5 shapes | PASS | SawUp/SawDown/Triangle/Square |
| SignalRegistry evaluateAll caches values | PASS | Cache correctness |
| Route processes with gain | PASS | Gain multiplier |
| Route processes with threshold | PASS | Below-threshold decay |
| Route processes with invert | PASS | Value flip |
| Route processes with output range | PASS | Min/max scaling |
| Route smoother dampens rapid changes | PASS | EMA filter |
| Route dial range narrows input | PASS | Input sensitivity |

### Tier 2: Feature Injection → Visual Change (exist in test_signals.py)

| Test | Status | What It Verifies |
|------|--------|-----------------|
| RMS 0 vs 0.9 changes source output | RUNNABLE | u_rms uniform works |
| Beat phase 0 vs 0.5 changes output | RUNNABLE | u_beatPhase uniform works |
| Bass vs treble band energy differs | RUNNABLE | Band energy uniforms |
| Spectral centroid low vs high | RUNNABLE | u_spectralCentroid uniform |
| Onset detected changes output | RUNNABLE | u_onsetStrength uniform |
| RMS changes effect on image | RUNNABLE | Effects respond to audio |

### Tier 3: Signal→Route→Parameter→Shader (need API — specification in test_signals.py)

| Test | Status | What It Verifies |
|------|--------|-----------------|
| Create route Volume→Ripple.intensity | SPEC | Route creation and wiring |
| High RMS → more ripple via route | SPEC | Full end-to-end signal flow |
| Route threshold blocks low values | SPEC | Threshold + falloff logic |
| Route invert flips output | SPEC | Inversion |
| Route output range clamps | SPEC | Min/max scaling |
| Macro drives multiple params | SPEC | MacroBank aggregation |
| Oscillator creates time-varying output | SPEC | Modulation signal |
| Signal registry has 14+ signals | SPEC | Registry completeness |
| Signal values update after inject | SPEC | Evaluation + caching |

### Tier 4: Complex Routing (future)

| Test | What It Verifies |
|------|-----------------|
| Two routes to same parameter sum | Route stacking |
| Route to source param (not just effect) | Source parameter routing |
| Macro chain: Signal→Macro→Route→Param | Multi-hop routing |
| 20 simultaneous routes don't lag | Performance under load |
| Route serialization roundtrip | Save/load |
| Route removal cleans up smoothers | Memory management |

## When to Activate

1. **Today**: Run `test_signals.py::TestFeatureInjectionChangesOutput` — tests direct feature→shader path
2. **When RoutingEngine is wired into Renderer**: Activate Tier 3 unit tests by adding signal API endpoints
3. **When signal API is built**: Remove `pytest.skip()` from test_signals.py route tests
4. **P16+**: Add feedback/temporal signal tests as those features are built

## VJAppController Methods Needed

```python
def list_signals(self) -> dict:
    """Get all signals with cached values."""
    r = requests.get(f"{self.base_url}/api/signals", timeout=10)
    r.raise_for_status()
    return r.json()

def add_route(self, route: dict) -> dict:
    """Create a signal→parameter route."""
    return self._post("/api/add_route", route)

def remove_route(self, route_id: int) -> dict:
    """Remove a signal route."""
    return self._post(f"/api/remove_route", {"id": route_id})

def list_routes(self) -> dict:
    """Get all active routes."""
    r = requests.get(f"{self.base_url}/api/routes", timeout=10)
    r.raise_for_status()
    return r.json()

def set_macro(self, scope: str, index: int, **kwargs) -> dict:
    """Set a macro knob value or source."""
    body = {"scope": scope, "index": index}
    body.update(kwargs)
    return self._post("/api/set_macro", body)
```
