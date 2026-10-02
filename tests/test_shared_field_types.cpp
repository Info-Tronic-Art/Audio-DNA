// test_shared_field_types -- lane tsan (s-rta-1002; ruling .harmony/.reports/s-rta-0930/ruling-tsan.md amendment 11).
// Normal-build TYPE pins for the model fields another thread reads: a refactor that turns one back into a plain
// field fails to COMPILE here, without needing a TSan build. The tuple pin lands at T2; T4 (clip runtime fields),
// T5 (the manualRef scalars) and T6 (activeDeckIndex) extend this file.
#include <catch2/catch_test_macros.hpp>
#include "model/Layer.h"
#include <type_traits>

namespace
{
// The Layer trigger tuple is ONE atomic word behind the Layer API (runtime() / setRuntime() / casRuntime() /
// updateRuntime()), never five loose fields a thread could read or write one at a time.
template <class L>
concept LooseTuple = requires(L& l) { l.activeClipColumn; };
template <class L>
concept LooseTupleAny = requires(L& l) { l.previousClipColumn; } || requires(L& l) { l.crossfadeProgress; }
                     || requires(L& l) { l.pendingTriggerColumn; } || requires(L& l) { l.pendingTriggerSnapOverride; };
} // namespace

static_assert(!LooseTuple<Layer>, "Layer::activeClipColumn is back as a loose field (Pitfall 63)");
static_assert(!LooseTupleAny<Layer>, "a Layer trigger-tuple field is back as a loose field (Pitfall 63)");
static_assert(std::is_same_v<decltype(std::declval<const Layer&>().runtime()), LayerRuntimeSnapshot>);
static_assert(sizeof(LayerRuntimeCell::Word) == 16);

TEST_CASE("shared model field types are pinned at compile time", "[tsan_lint][types]")
{
    // The static_asserts above are the test; this case registers the target with ctest.
    CHECK(LooseTuple<LayerRuntimeSnapshot>);   // the value copy keeps the five named fields
}
