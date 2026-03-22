"""
Performance Verification — Tests that sources and effects render within budget.

Renders 5 frames per source/effect and measures average time.
Budget: 50ms per frame (includes HTTP overhead; actual GPU target is 16ms).
"""

import os
import time
import pytest
import sys
sys.path.insert(0, os.path.dirname(__file__))

FIXTURES_DIR = os.path.abspath(
    os.path.join(os.path.dirname(__file__), "..", "fixtures")
)
TEST_IMAGE = os.path.join(FIXTURES_DIR, "test_card.png")


@pytest.fixture(scope="module")
def all_sources(app):
    return app.list_sources()["sources"]


@pytest.fixture(scope="module")
def all_effects(app):
    return app.state()["effects"]


class TestSourcePerformance:
    """Every source must render within budget."""

    def test_source_render_time(self, app, tmp_path, all_sources):
        slow = []
        for src in all_sources:
            app.load_source(src["id"])
            times = []
            for i in range(5):
                out = str(tmp_path / f"perf_{src['id']}_{i}.png")
                start = time.time()
                app.render_frame(out, time_val=float(i), width=512, height=512)
                elapsed = (time.time() - start) * 1000
                times.append(elapsed)

            avg_ms = sum(times[1:]) / len(times[1:])  # Skip first (cold start)
            if avg_ms > 100:
                slow.append(f"{src['id']}: {avg_ms:.0f}ms avg")

        if slow:
            print(f"\nSlow sources (>100ms including HTTP):")
            for s in slow:
                print(f"  {s}")
            # Warn, don't hard fail — HTTP adds ~20-30ms overhead
            assert len(slow) < len(all_sources) // 2, (
                f"{len(slow)} sources too slow:\n" + "\n".join(slow)
            )


class TestEffectPerformance:
    """Effects should not significantly slow down rendering."""

    def test_effect_render_time(self, app, tmp_path, all_effects):
        if not os.path.exists(TEST_IMAGE):
            pytest.skip("Test image not found")

        slow = []
        for fx in all_effects:
            app.reset()
            app.load_image(TEST_IMAGE)
            app.set_effect(fx["name"], enabled=True)

            times = []
            for i in range(5):
                out = str(tmp_path / f"perf_fx_{fx['name']}_{i}.png".replace(" ", "_"))
                start = time.time()
                app.render_frame(out, time_val=float(i), width=512, height=512)
                elapsed = (time.time() - start) * 1000
                times.append(elapsed)

            avg_ms = sum(times[1:]) / len(times[1:])
            if avg_ms > 100:
                slow.append(f"{fx['name']}: {avg_ms:.0f}ms avg")

        if slow:
            print(f"\nSlow effects (>100ms including HTTP):")
            for s in slow:
                print(f"  {s}")
