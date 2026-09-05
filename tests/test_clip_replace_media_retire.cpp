#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <unordered_map>

// S166-LEAK: kClipReplaceContent's IMAGE branch (src/MainComponent.cpp, case
// C::kClipReplaceContent) called neither an open nor a close on the OUTGOING
// clip id, so replacing a Video (or ImageSequence) clip's content with a
// still image orphaned the existing videoPlayers_/imageSequences_ entry —
// decoder and map entry both leaked forever. The VIDEO branch never had this
// problem: it calls Renderer::openVideoForClip(existing->id, file), which
// (as of L1-FU, commit c7247a9) retires whatever the outgoing id already
// held through Renderer::closeMediaForClip() before installing the new
// player. The fix makes the IMAGE branch call closeMediaForClip(existing->id)
// directly, since a plain Image has no persistent decoder of its own to
// install in its place.
//
// This is a message-thread bug fix inside MainComponent.cpp's FileChooser
// lambda, calling into Renderer's videoPlayers_/imageSequences_ retire
// mechanism (Renderer.h/.cpp). Neither file is linked into ANY ctest target
// in this repo: MainComponent.cpp needs the full JUCE GUI stack, and
// Renderer.cpp pulls in CompositorEngine/ProjectMSource(MilkDrop)/
// AnalysisThread/VideoRecorder/SyphonOutput and needs a live GL context for
// large parts of its surface — test_compositor.cpp and
// test_renderer_source_confinement.cpp document this exact limitation for
// the same subsystem. Following test_renderer_source_confinement.cpp's own
// precedent ("mirror the mechanism, not the subsystem"), this test
// reproduces the STATE MACHINE that matters here — which of
// videoPlayers_/imageSequences_ holds a given clip id, and what
// closeMediaForClip() does to it — and the exact per-branch call pattern
// kClipReplaceContent uses, rather than exercising MainComponent.cpp/
// Renderer.cpp's actual code.
//
// IMPORTANT, same honesty this repo already holds itself to elsewhere:
// mutating this file's own mirror functions proves THIS TEST has teeth
// against changes to the mirror. It does NOT prove regression coverage for
// the real one-line fix in src/MainComponent.cpp — the builder report for
// this lane says so explicitly, with the neutralize/rebuild/restore proof
// run against the real file to show exactly that gap.

namespace
{

// Mirrors Renderer::videoPlayers_ / imageSequences_ (Renderer.h): two maps,
// keyed by clip id, each holding at most one live entry per id. `bool`
// stands in for a live std::unique_ptr<VideoPlayer>/<ImageSequence> — this
// test only needs presence/absence, not decode behavior.
class MediaRetireSim
{
public:
    std::unordered_map<uint32_t, bool> videoPlayers_;
    std::unordered_map<uint32_t, bool> imageSequences_;
    int closeCount_ = 0;   // how many LIVE entries closeMediaForClip() actually retired

    // Mirrors Renderer::closeMediaForClip (Renderer.cpp): checks BOTH maps
    // for this id and retires whichever is present. Safe no-op if neither
    // map holds it (Image->Image replace; a fresh id that never had media).
    void closeMediaForClip(uint32_t id)
    {
        if (videoPlayers_.erase(id) > 0) ++closeCount_;
        if (imageSequences_.erase(id) > 0) ++closeCount_;
    }

    // Mirrors Renderer::openVideoForClip: retires whatever the outgoing id
    // already held FIRST, then installs the new player under that same id.
    bool openVideoForClip(uint32_t id)
    {
        closeMediaForClip(id);
        videoPlayers_[id] = true;
        return true;
    }

    // Mirrors Renderer::openImageSequenceForClip: same retire-before-replace
    // guarantee as openVideoForClip above.
    bool openImageSequenceForClip(uint32_t id)
    {
        closeMediaForClip(id);
        imageSequences_[id] = true;
        return true;
    }
};

// Mirrors kClipReplaceContent's VIDEO branch (the `else` block): always
// routes through openVideoForClip, which retires the outgoing id for free.
void replaceContentVideoBranch(MediaRetireSim& r, uint32_t outgoingId)
{
    r.openVideoForClip(outgoingId);
}

// Mirrors kClipReplaceContent's IMAGE branch AS FIXED by this lane: closes
// the outgoing id's media before handing back Image content, since Image
// itself installs no persistent decoder in its place.
void replaceContentImageBranch_Fixed(MediaRetireSim& r, uint32_t outgoingId)
{
    r.closeMediaForClip(outgoingId);
}

// Mirrors kClipReplaceContent's IMAGE branch BEFORE this lane's fix: did
// nothing to the outgoing id at all. This is the leak S166-LEAK describes.
void replaceContentImageBranch_Buggy(MediaRetireSim&, uint32_t)
{
    // intentionally empty
}

} // namespace

TEST_CASE("S166-LEAK: video->image replace retires the outgoing video decoder", "[media][clip-replace]")
{
    MediaRetireSim r;
    constexpr uint32_t clipId = 42;
    r.openVideoForClip(clipId);
    REQUIRE(r.videoPlayers_.count(clipId) == 1);

    replaceContentImageBranch_Fixed(r, clipId);

    CHECK(r.videoPlayers_.count(clipId) == 0);
    CHECK(r.imageSequences_.count(clipId) == 0);
    CHECK(r.closeCount_ == 1);
}

TEST_CASE("S166-LEAK: the pre-fix image branch reproduces the leak (proves the test above has teeth)", "[media][clip-replace]")
{
    MediaRetireSim r;
    constexpr uint32_t clipId = 42;
    r.openVideoForClip(clipId);
    REQUIRE(r.videoPlayers_.count(clipId) == 1);

    replaceContentImageBranch_Buggy(r, clipId);

    // Without the fix, the outgoing video entry is never touched: it leaks.
    CHECK(r.videoPlayers_.count(clipId) == 1);
    CHECK(r.closeCount_ == 0);
}

TEST_CASE("S166-LEAK: imageSequence->image replace retires the outgoing sequence (same leak family)", "[media][clip-replace]")
{
    MediaRetireSim r;
    constexpr uint32_t clipId = 7;
    r.openImageSequenceForClip(clipId);
    REQUIRE(r.imageSequences_.count(clipId) == 1);

    replaceContentImageBranch_Fixed(r, clipId);

    CHECK(r.imageSequences_.count(clipId) == 0);
    CHECK(r.closeCount_ == 1);
}

TEST_CASE("image->image replace is a safe no-op — nothing to retire", "[media][clip-replace]")
{
    MediaRetireSim r;
    constexpr uint32_t clipId = 99;

    replaceContentImageBranch_Fixed(r, clipId);

    CHECK(r.videoPlayers_.empty());
    CHECK(r.imageSequences_.empty());
    CHECK(r.closeCount_ == 0);
}

TEST_CASE("video->video replace retires the old decoder before installing the new one", "[media][clip-replace]")
{
    MediaRetireSim r;
    constexpr uint32_t clipId = 5;
    r.openVideoForClip(clipId);   // first video
    REQUIRE(r.closeCount_ == 0);  // nothing to retire yet — fresh id

    replaceContentVideoBranch(r, clipId);   // second video, same id

    CHECK(r.videoPlayers_.count(clipId) == 1);   // new player installed
    CHECK(r.closeCount_ == 1);                   // old one was retired first
}

TEST_CASE("image->video replace has nothing to retire and installs cleanly", "[media][clip-replace]")
{
    MediaRetireSim r;
    constexpr uint32_t clipId = 11;

    replaceContentVideoBranch(r, clipId);

    CHECK(r.videoPlayers_.count(clipId) == 1);
    CHECK(r.closeCount_ == 0);
}
