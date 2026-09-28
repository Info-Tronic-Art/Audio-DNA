// test_image_tex_cache -- s-rta-0928 renderleft R1.2 / R1.5: the bookkeeping of the compositor's image textures
// (src/render/ImageTexCache.h), decoded off the GL thread. Pure: no GL, no JUCE. The GL wrapper
// (CompositorEngine::pumpImages / getKeyTexture) does exactly what these results name.
#include <catch2/catch_test_macros.hpp>
#include "render/ImageTexCache.h"

using namespace ImageTexCache;

namespace
{
const Stamp kS1{ 1000, 42 };
const Stamp kS2{ 2000, 43 };

// A path goes through lookup -> Decoded -> upload -> Resident, as pumpImages does it.
void makeResident(Cache& c, const std::string& p, uint32_t tex, size_t bytes = 100, Stamp s = kS1)
{
    c.lookup(p);
    REQUIRE(c.onResult(p, Kind::Decoded, s) == Act::Upload);
    REQUIRE(c.onUploaded(p, tex, 10, 10, s, bytes) == 0u);
}
} // namespace

TEST_CASE("(1) a miss is Pending and requests exactly once", "[image_tex_cache][s-rta-0928]")
{
    Cache c;
    const auto a = c.lookup("/a.png");
    CHECK(a.pending); CHECK(a.request); CHECK(a.tex == 0u);
    const auto b = c.lookup("/a.png");
    CHECK(b.pending); CHECK_FALSE(b.request); CHECK(b.tex == 0u);
    CHECK(c.pendingCount() == 1);
}

TEST_CASE("(2) Decoded on Pending -> Upload; onUploaded -> Resident; lookup -> that texture", "[image_tex_cache][s-rta-0928]")
{
    Cache c;
    c.lookup("/a.png");
    CHECK(c.onResult("/a.png", Kind::Decoded, kS1) == Act::Upload);
    CHECK(c.lookup("/a.png").pending);                 // still pending until the upload
    CHECK_FALSE(c.lookup("/a.png").request);
    CHECK(c.onUploaded("/a.png", 7u, 10, 10, kS1, 400) == 0u);
    const auto l = c.lookup("/a.png");
    CHECK(l.tex == 7u); CHECK_FALSE(l.pending); CHECK_FALSE(l.request);
    CHECK(c.residentCount() == 1); CHECK(c.residentBytes() == 400u); CHECK(c.pendingCount() == 0);
}

TEST_CASE("(3) Failed -> MarkFailed; then tex 0, not pending, no request (sticky)", "[image_tex_cache][s-rta-0928]")
{
    Cache c;
    c.lookup("/bad.png");
    CHECK(c.onResult("/bad.png", Kind::Failed, kS1) == Act::MarkFailed);
    for (int i = 0; i < 3; ++i)
    {
        const auto l = c.lookup("/bad.png");
        CHECK(l.tex == 0u); CHECK_FALSE(l.pending); CHECK_FALSE(l.request);
    }
}

TEST_CASE("(4) a result for a path with no entry (cleared or evicted) is dropped", "[image_tex_cache][s-rta-0928]")
{
    Cache c;
    CHECK(c.onResult("/never.png", Kind::Decoded, kS1) == Act::Drop);
    c.lookup("/a.png");
    c.clearAll();
    CHECK(c.onResult("/a.png", Kind::Decoded, kS1) == Act::Drop);
    CHECK_FALSE(c.awaitingUpload("/a.png"));
    // a second result for a resident image is dropped too
    makeResident(c, "/b.png", 3u);
    CHECK(c.onResult("/b.png", Kind::Decoded, kS2) == Act::Drop);
}

TEST_CASE("(5) clearAll returns every texture and each later lookup requests again", "[image_tex_cache][s-rta-0928]")
{
    Cache c;
    makeResident(c, "/a.png", 3u);
    makeResident(c, "/b.png", 4u);
    c.lookup("/c.png");   // pending: no texture
    auto dead = c.clearAll();
    std::sort(dead.begin(), dead.end());
    CHECK(dead == std::vector<uint32_t>{ 3u, 4u });
    CHECK(c.residentCount() == 0); CHECK(c.residentBytes() == 0u); CHECK(c.pendingCount() == 0);
    for (const char* p : { "/a.png", "/b.png", "/c.png" })
    {
        const auto l = c.lookup(p);
        CHECK(l.pending); CHECK(l.request);
    }
}

TEST_CASE("(6) isDemand after a lookup; a prefetch entry is not demand until a frame asks", "[image_tex_cache][s-rta-0928]")
{
    Cache c;
    CHECK_FALSE(c.isDemand("/a.png"));
    c.lookup("/a.png");
    CHECK(c.isDemand("/a.png"));
    c.applyImageSet({ "/p.png" });
    const auto j = c.nextPrefetch(1u << 30);
    REQUIRE(j.has_value());
    CHECK(j->path == "/p.png");
    CHECK_FALSE(c.isDemand("/p.png"));
    const auto l = c.lookup("/p.png");   // the prefetch is already in flight: pending, no second request
    CHECK(l.pending); CHECK_FALSE(l.request);
    CHECK(c.isDemand("/p.png"));
}

TEST_CASE("(7) UploadBudget: the first take is always granted, later ones only within the cap; reset",
          "[image_tex_cache][s-rta-0928]")
{
    UploadBudget b;
    b.cap = 100;
    CHECK(b.take(250));     // first: granted even over the cap
    CHECK_FALSE(b.take(1));
    b.reset();
    CHECK(b.take(60));
    CHECK(b.take(40));      // exactly the cap
    CHECK_FALSE(b.take(1));
    b.reset();
    CHECK(b.take(1));
}

TEST_CASE("(7b) budget exhausted mid-drain: the decoded bytes wait, no re-decode, uploaded on a later frame (B3)",
          "[image_tex_cache][s-rta-0928]")
{
    Cache c;
    c.lookup("/a.png"); c.lookup("/b.png");
    CHECK(c.onResult("/a.png", Kind::Decoded, kS1) == Act::Upload);
    CHECK(c.onResult("/b.png", Kind::Decoded, kS1) == Act::Upload);
    UploadBudget budget; budget.cap = 100;
    CHECK(budget.take(80));
    CHECK(c.onUploaded("/a.png", 1u, 10, 10, kS1, 80) == 0u);
    CHECK_FALSE(budget.take(80));                        // b misses this frame's budget
    CHECK(c.awaitingUpload("/b.png"));
    const auto l = c.lookup("/b.png");                   // the next frame: still pending, NOT re-requested
    CHECK(l.pending); CHECK_FALSE(l.request);
    budget.reset();
    CHECK(budget.take(80));
    CHECK(c.onUploaded("/b.png", 2u, 10, 10, kS1, 80) == 0u);
    CHECK(c.lookup("/b.png").tex == 2u);
}

// ---- R1.5: prefetch at a composition swap + release of dropped images ----

TEST_CASE("(8) applyImageSet drops non-members: their textures are returned and late results Drop",
          "[image_tex_cache][s-rta-0928]")
{
    Cache c;
    makeResident(c, "/keep.png", 1u, 100);
    makeResident(c, "/gone.png", 2u, 100);
    c.lookup("/inflight.png");                              // a demand decode still running
    const auto dead = c.applyImageSet({ "/keep.png" });
    CHECK(dead == std::vector<uint32_t>{ 2u });
    CHECK(c.residentCount() == 1); CHECK(c.residentBytes() == 100u);
    CHECK(c.onResult("/inflight.png", Kind::Decoded, kS1) == Act::Drop);
    CHECK(c.lookup("/keep.png").tex == 1u);
}

TEST_CASE("(9) P = 1: the second queued path is issued only after the first's result", "[image_tex_cache][s-rta-0928]")
{
    Cache c;
    c.applyImageSet({ "/p1.png", "/p2.png" });
    const auto j1 = c.nextPrefetch(1u << 30);
    REQUIRE(j1.has_value()); CHECK(j1->path == "/p1.png"); CHECK_FALSE(j1->known.has_value());
    CHECK_FALSE(c.nextPrefetch(1u << 30).has_value());       // one in flight
    CHECK(c.onResult("/p1.png", Kind::Decoded, kS1) == Act::Upload);
    const auto j2 = c.nextPrefetch(1u << 30);
    REQUIRE(j2.has_value()); CHECK(j2->path == "/p2.png");
    CHECK_FALSE(c.nextPrefetch(1u << 30).has_value());
}

TEST_CASE("(10) budget: no NEW prefetch at or over it; re-validations still issued; demand still requests",
          "[image_tex_cache][s-rta-0928]")
{
    Cache c;
    makeResident(c, "/big.png", 1u, 1000);
    c.applyImageSet({ "/new.png", "/big.png" });
    const auto j = c.nextPrefetch(1000);                       // resident 1000 >= budget 1000
    REQUIRE(j.has_value());
    CHECK(j->path == "/big.png");                              // /new.png skipped, the re-validation issued
    REQUIRE(j->known.has_value()); CHECK(*j->known == kS1);
    CHECK(c.onResult("/big.png", Kind::Unchanged, kS1) == Act::Drop);
    CHECK_FALSE(c.nextPrefetch(1000).has_value());
    const auto l = c.lookup("/new.png");                       // a frame needs it: requested regardless
    CHECK(l.pending); CHECK(l.request);
}

TEST_CASE("(11) re-validation: Unchanged keeps the texture; Decoded with a new stamp uploads and returns the old one",
          "[image_tex_cache][s-rta-0928]")
{
    Cache c;
    makeResident(c, "/a.png", 5u, 100, kS1);
    c.applyImageSet({ "/a.png" });
    auto j = c.nextPrefetch(1u << 30);
    REQUIRE(j.has_value()); REQUIRE(j->known.has_value());
    CHECK(c.onResult("/a.png", Kind::Unchanged, kS1) == Act::Drop);
    CHECK(c.lookup("/a.png").tex == 5u);

    c.applyImageSet({ "/a.png" });
    j = c.nextPrefetch(1u << 30);
    REQUIRE(j.has_value());
    CHECK(c.lookup("/a.png").tex == 5u);                       // still shown while it re-validates
    CHECK(c.onResult("/a.png", Kind::Decoded, kS2) == Act::Upload);
    CHECK(c.onUploaded("/a.png", 9u, 10, 10, kS2, 120) == 5u); // the replaced texture, to delete
    CHECK(c.lookup("/a.png").tex == 9u);
    CHECK(c.residentBytes() == 120u);
}

TEST_CASE("(11b) a Failed image is retried after the next composition swap (R-7)", "[image_tex_cache][s-rta-0928]")
{
    Cache c;
    c.lookup("/bad.png");
    CHECK(c.onResult("/bad.png", Kind::Failed, kS1) == Act::MarkFailed);
    c.applyImageSet({ "/bad.png" });
    const auto j = c.nextPrefetch(1u << 30);
    REQUIRE(j.has_value()); CHECK(j->path == "/bad.png");
}
