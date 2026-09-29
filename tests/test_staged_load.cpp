// s-rta-0929 asyncload (plan-asyncload.md 5.2 / 5.8 test 2 + HARMONY ADOPTION AL5 / AL7): the pure bookkeeping of a
// staged composition / deck load -- the batch Ledger (a landing of a superseded batch is Stale), the file-label texts,
// the label hold (while staged the label keeps "Loading <name>..."; a cancel shows the latest held text), the adopted
// media ids a cancel retires (video AND sequence), and the Append / Duplicate FIFO (admission, bound, source lookup).
#include <catch2/catch_test_macros.hpp>

#include "core/StagedLoad.h"

using namespace stagedload;
using Landing = Ledger::Landing;

TEST_CASE("a batch completes only at its last landing", "[asyncload][ledger]")
{
    Ledger l;
    const auto g = l.begin(3);
    CHECK(l.pending() == 3);
    CHECK_FALSE(l.complete(g));
    CHECK(l.land(g) == Landing::Apply);
    CHECK_FALSE(l.complete(g));
    CHECK(l.land(g) == Landing::Apply);
    CHECK_FALSE(l.complete(g));
    CHECK(l.land(g) == Landing::Apply);
    CHECK(l.complete(g));
    CHECK(l.pending() == 0);
}

TEST_CASE("a landing of a superseded batch is Stale and does not count", "[asyncload][ledger]")
{
    Ledger l;
    const auto g1 = l.begin(2);
    const auto g2 = l.begin(1);
    REQUIRE(g1 != g2);
    CHECK(l.land(g1) == Landing::Stale);
    CHECK(l.pending() == 1);
    CHECK_FALSE(l.complete(g2));
    CHECK(l.land(g2) == Landing::Apply);
    CHECK(l.complete(g2));
    CHECK_FALSE(l.complete(g1));
}

TEST_CASE("an empty batch is complete at once", "[asyncload][ledger]")
{
    Ledger l;
    const auto g = l.begin(0);
    CHECK(l.complete(g));
    CHECK(l.pending() == 0);
}

TEST_CASE("a cancelled batch lands Stale", "[asyncload][ledger]")
{
    Ledger l;
    const auto g = l.begin(2);
    l.cancel();
    CHECK(l.land(g) == Landing::Stale);
    CHECK(l.pending() == 0);
    CHECK(l.liveGen() == 0);
    CHECK_FALSE(l.complete(g));
}

TEST_CASE("batches() counts every begin", "[asyncload][ledger]")
{
    Ledger l;
    l.begin(2);
    l.begin(0);
    l.cancel();
    l.begin(1);
    CHECK(l.batches() == 3);
}

TEST_CASE("the label texts are today's three texts", "[asyncload][label]")
{
    CHECK(loadingLabel(Kind::DeckDuplicate, "A copy") == "Loading A copy...");
    CHECK(loadingLabel(Kind::Composition, "show") == "Loading show...");
    CHECK(doneLabel(Kind::Composition, "show") == "Loaded: show");              // MainComponent loadComposition step 7
    CHECK(doneLabel(Kind::DeckAppend, "deck16") == "Loaded deck: deck16");      // appendDeckFromFile step 7
    CHECK(doneLabel(Kind::DeckDuplicate, "A copy") == "Duplicated deck: A copy");   // duplicateDeck
}

TEST_CASE("a cancel restores the label only while it still shows the loading text", "[asyncload][label]")
{
    CHECK(labelAfterCancel("Loading X...", "Loading X...", "Loaded: W") == "Loaded: W");
    CHECK(labelAfterCancel("clip.mp4", "Loading X...", "Loaded: W") == "clip.mp4");
}

TEST_CASE("AL5: while held the label keeps the loading text and a cancel shows the latest writer's text", "[asyncload][label]")
{
    LabelHold h;
    CHECK_FALSE(h.divert("before staging"));   // not held: the writer writes the label itself
    CHECK(h.hold("Loaded: W", "Loading X...") == "Loading X...");
    CHECK(h.held());
    CHECK(h.divert("top.png"));                 // a trigger during the window: kept, the label is not written
    CHECK(h.divert("base.png"));                // the latest wins
    CHECK(h.restoreText() == "base.png");
    CHECK(h.release("Loading X...") == "base.png");
    CHECK_FALSE(h.held());
    CHECK_FALSE(h.divert("after"));

    LabelHold nothingWritten;
    nothingWritten.hold("Loaded: W", "Loading X...");
    CHECK(nothingWritten.release("Loading X...") == "Loaded: W");
}

TEST_CASE("a cancel retires every adopted id, video and sequence", "[asyncload][adopted]")
{
    Adopted a;
    a.add(1001);   // a video, at its landing
    a.add(1002);   // a sequence, at the completion
    const auto ids = a.takeAll();
    REQUIRE(ids.size() == 2);
    CHECK(ids[0] == 1001);
    CHECK(ids[1] == 1002);
    CHECK(a.takeAll().empty());
}

TEST_CASE("AL7: a Composition supersedes, an Append / Duplicate queues behind a staged load", "[asyncload][queue]")
{
    CHECK(admit(Kind::Composition, false, 0) == Admit::Begin);
    CHECK(admit(Kind::Composition, true, 0) == Admit::Supersede);
    CHECK(admit(Kind::Composition, true, 3) == Admit::Supersede);
    CHECK(admit(Kind::DeckAppend, false, 0) == Admit::Begin);
    CHECK(admit(Kind::DeckDuplicate, true, 0) == Admit::Enqueue);
    CHECK(admit(Kind::DeckAppend, true, kQueueMax - 1) == Admit::Enqueue);
    CHECK(admit(Kind::DeckDuplicate, true, kQueueMax) == Admit::Refuse);
}

TEST_CASE("AL7: the queue is FIFO and bounded", "[asyncload][queue]")
{
    LoadQueue q;
    for (std::size_t i = 0; i < kQueueMax; ++i)
        CHECK(q.push({ Kind::DeckDuplicate, "", static_cast<uint32_t>(i), 1, "d" + std::to_string(i) }));
    CHECK_FALSE(q.push({ Kind::DeckAppend, "/x.json", 0, 0, "x" }));   // the 9th is refused
    CHECK(q.size() == kQueueMax);
    for (std::size_t i = 0; i < kQueueMax; ++i)
    {
        auto p = q.pop();
        REQUIRE(p.has_value());
        CHECK(p->deckId == static_cast<uint32_t>(i));   // in order
    }
    CHECK_FALSE(q.pop().has_value());
    q.push({ Kind::DeckAppend, "/x.json", 0, 0, "x" });
    q.clear();
    CHECK(q.size() == 0);
}

TEST_CASE("AL7: a queued Duplicate finds its source deck by id, or is skipped", "[asyncload][queue]")
{
    const std::vector<uint32_t> ids{ 4, 7, 9 };
    CHECK(resolveDuplicateSource(2, 2, ids, 7) == 1);
    CHECK(resolveDuplicateSource(2, 2, ids, 5) == -1);   // the deck was removed meanwhile
    CHECK(resolveDuplicateSource(2, 3, ids, 7) == -1);   // a composition cut replaced the model the id was read in
    CHECK(queueFullLabel("deck16") == "Too many loads waiting: deck16 skipped");
    CHECK(sourceGoneLabel("A") == "Duplicate skipped: A is gone");
}
