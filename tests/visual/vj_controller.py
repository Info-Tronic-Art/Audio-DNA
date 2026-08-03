"""
VJ App Controller — Python client for the Eyes test harness HTTP API.

Wraps the HTTP endpoints exposed by TestServer (C++) into a clean Python API
for use in pytest tests.
"""

import subprocess
import time
import requests
import signal
import os


class VJAppController:
    """Controls Audio-DNA via the Eyes HTTP API."""

    def __init__(self, port: int = 8080, executable: str = None):
        self.base_url = f"http://localhost:{port}"
        self.port = port
        self.executable = executable
        self.process = None

    def start(self, timeout: float = 20.0):
        """Spawn the app in test mode and wait for it to become ready.

        Args:
            timeout: Maximum seconds to wait for the app to start.

        Raises:
            TimeoutError: If the app doesn't respond to health checks in time.
            FileNotFoundError: If the executable doesn't exist.
        """
        if self.executable and not os.path.exists(self.executable):
            raise FileNotFoundError(f"Executable not found: {self.executable}")

        if self.executable:
            cmd = [self.executable, "--test-mode", f"--test-port={self.port}"]
            self.process = subprocess.Popen(
                cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
            )

        deadline = time.time() + timeout
        last_error = None
        while time.time() < deadline:
            try:
                r = requests.get(f"{self.base_url}/api/health", timeout=2)
                if r.status_code == 200:
                    data = r.json()
                    print(f"[Eyes] App ready: {data}")
                    return True
            except requests.ConnectionError as e:
                last_error = e
                time.sleep(0.5)
            except Exception as e:
                last_error = e
                time.sleep(0.5)

        raise TimeoutError(
            f"App did not become ready within {timeout}s. Last error: {last_error}"
        )

    def stop(self):
        """Stop the app subprocess gracefully."""
        if self.process:
            self.process.send_signal(signal.SIGTERM)
            try:
                self.process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                self.process.kill()
                self.process.wait()
            self.process = None

    def health(self) -> dict:
        """Check if the app is running and ready."""
        r = requests.get(f"{self.base_url}/api/health", timeout=5)
        r.raise_for_status()
        return r.json()

    def load_image(self, filepath: str) -> dict:
        """Load an image file into the renderer.

        Args:
            filepath: Absolute path to the image file.
        """
        return self._post("/api/load_image", {"filepath": filepath})

    def set_effect(
        self, name: str, enabled: bool = True, params: dict = None
    ) -> dict:
        """Enable/disable an effect and optionally set parameters.

        Args:
            name: Effect display name (e.g., "Ripple", "Hue Shift").
            enabled: Whether to enable the effect.
            params: Dict of param_name -> value (all [0, 1]).
        """
        body = {"name": name, "enabled": enabled}
        if params:
            body["params"] = params
        return self._post("/api/set_effect", body)

    def set_effect_chain(self, effects: list) -> dict:
        """Configure the entire effect chain.

        Disables all existing effects, then enables the specified ones.

        Args:
            effects: List of dicts, each with "name" and optional "params".
                     e.g., [{"name": "Ripple", "params": {"intensity": 0.5}}]
        """
        return self._post("/api/set_effect_chain", {"effects": effects})

    def inject_features(self, features: dict) -> dict:
        """Inject synthetic audio features into the FeatureBus.

        Args:
            features: Dict of feature_name -> value.
                      e.g., {"rms": 0.8, "beatPhase": 0.5, "bandEnergies": [0.1]*7}
        """
        return self._post("/api/inject_features", features)

    def render_frame(
        self,
        output_path: str,
        time_val: float = 0.0,
        width: int = 0,
        height: int = 0,
    ) -> dict:
        """Render a single frame and save it to disk.

        Args:
            output_path: Absolute path for the output PNG.
            time_val: Override u_time for deterministic rendering.
            width: Render width (0 = use current).
            height: Render height (0 = use current).

        Returns:
            Response dict with "ok", "path", "width", "height".
        """
        return self._post(
            "/api/render_frame",
            {
                "output_path": output_path,
                "time": time_val,
                "width": width,
                "height": height,
            },
        )

    def state(self) -> dict:
        """Get the current engine state (effects, FPS, etc.)."""
        r = requests.get(f"{self.base_url}/api/state", timeout=10)
        r.raise_for_status()
        return r.json()

    def reset(self) -> dict:
        """Reset all effects, clear images, restore defaults."""
        return self._post("/api/reset", {})

    def load_source(self, source_type: str, params: dict = None) -> dict:
        """Load a procedural source into the active clip.

        Args:
            source_type: Source ID (e.g., "mandelbrot", "julia_set", "mandelbulb").
            params: Dict of uniform_name -> value (all [0, 1]).
        """
        body = {"source_type": source_type}
        if params:
            body["params"] = params
        return self._post("/api/load_source", body)

    def update_source_params(self, params: dict) -> dict:
        """Update parameters on the currently active source.

        Args:
            params: Dict of uniform_name -> value.
        """
        return self._post("/api/update_source_params", {"params": params})

    def list_sources(self) -> dict:
        """Get all registered procedural sources with their parameters."""
        r = requests.get(f"{self.base_url}/api/sources", timeout=10)
        r.raise_for_status()
        return r.json()

    # === P16: Signal/Routing Methods ===

    def list_signals(self) -> dict:
        """Get all signals with cached values."""
        r = requests.get(f"{self.base_url}/api/signals", timeout=10)
        r.raise_for_status()
        return r.json()

    def add_route(self, route: dict) -> dict:
        """Create a signal→parameter route.

        Args:
            route: Dict with keys: source_signal_id, target_effect, target_param,
                   output_min, output_max, threshold, gain, inverted.
        """
        return self._post("/api/add_route", route)

    def remove_route(self, route_id: int) -> dict:
        """Remove a signal route by ID."""
        return self._post("/api/remove_route", {"id": route_id})

    def list_routes(self) -> dict:
        """Get all active routes with current output values."""
        r = requests.get(f"{self.base_url}/api/routes", timeout=10)
        r.raise_for_status()
        return r.json()

    def set_macro(self, scope: str, index: int, **kwargs) -> dict:
        """Set a macro knob value or source.

        Args:
            scope: "global", "layer", or "clip"
            index: Macro index (0-7)
            **kwargs: Either source_signal_id=int or manual_value=float
        """
        body = {"scope": scope, "index": index}
        body.update(kwargs)
        return self._post("/api/set_macro", body)

    # === W6 (outputwindow-arc): test-mode mapping add/remove ===

    def add_mapping(self, target_effect: str, target_param: str, **kwargs) -> dict:
        """Create an RMS→param mapping (test-mode enabler, TestServer only).

        Args:
            target_effect: Effect display name (as shown in /api/state).
            target_param: Param name on that effect.
            **kwargs: Optional input_min, input_max, output_min, output_max,
                      smoothing (EMA alpha; 1.0 = no smoothing).
        """
        body = {"target_effect": target_effect, "target_param": target_param}
        body.update(kwargs)
        return self._post("/api/add_mapping", body)

    def remove_mapping(self, index: int = 0) -> dict:
        """Remove a mapping by index. The apply is async on the app's message
        thread; the response's num_mappings_before lets callers drain by
        repeating remove_mapping(0) until it reports 0."""
        return self._post("/api/remove_mapping", {"index": index})

    def _post(self, path: str, data: dict) -> dict:
        """Send a POST request with JSON body."""
        r = requests.post(f"{self.base_url}{path}", json=data, timeout=15)
        r.raise_for_status()
        return r.json()
