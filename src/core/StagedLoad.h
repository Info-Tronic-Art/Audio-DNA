#pragma once
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <optional>
#include <string>
#include <vector>

// s-rta-0929 asyncload (plan-asyncload.md 5.2 + HARMONY ADOPTION AL5 / AL7): the pure bookkeeping of a STAGED
// composition / deck load -- the one live open batch (Ledger), the file label while it is staged (labels, LabelHold),
// every media id the batch handed to the renderer (Adopted), and the FIFO of Append / Duplicate requests that arrive
// while a load is staged (admit, LoadQueue, resolveDuplicateSource). No JUCE, no model types: tests/test_staged_load.cpp.
namespace stagedload
{
enum class Kind : uint8_t { Composition, DeckAppend, DeckDuplicate };

// The ONE live asynchronous open batch. begin() cancels the live batch and mints a generation; land(gen) is Apply for
// the live generation (++done) and Stale otherwise; complete(gen) = live and done >= total; begin(0) is complete at once.
class Ledger
{
public:
    enum class Landing { Apply, Stale };

    uint64_t begin(int total)
    {
        cancel();
        ++gen_;
        ++batches_;
        total_ = std::max(0, total);
        done_ = 0;
        live_ = true;
        return gen_;
    }
    Landing land(uint64_t gen)
    {
        if (!live_ || gen != gen_ || done_ >= total_)
            return Landing::Stale;
        ++done_;
        return Landing::Apply;
    }
    bool complete(uint64_t gen) const { return live_ && gen == gen_ && done_ >= total_; }
    void cancel()
    {
        live_ = false;
        total_ = done_ = 0;
    }
    int pending() const { return live_ ? total_ - done_ : 0; }
    uint64_t liveGen() const { return live_ ? gen_ : 0; }
    uint64_t batches() const { return batches_; }

private:
    uint64_t gen_ = 0, batches_ = 0;
    int total_ = 0, done_ = 0;
    bool live_ = false;
};

// Labels (A2 / A5). The text while a batch is staged, the final text (today's three texts), and what a cancel shows.
inline std::string loadingLabel(Kind, const std::string& name) { return "Loading " + name + "..."; }
inline std::string doneLabel(Kind k, const std::string& name)
{
    switch (k)
    {
        case Kind::Composition:   return "Loaded: " + name;
        case Kind::DeckAppend:    return "Loaded deck: " + name;
        case Kind::DeckDuplicate: return "Duplicated deck: " + name;
    }
    return name;
}
// current == loadingText ? before : current (a writer that bypassed the hold keeps its text).
inline std::string labelAfterCancel(const std::string& current, const std::string& loadingText, const std::string& before)
{
    return current == loadingText ? before : current;
}
// AL7 label notes: a 9th queued request, and a queued Duplicate whose source deck is gone at dequeue.
inline std::string queueFullLabel(const std::string& name) { return "Too many loads waiting: " + name + " skipped"; }
inline std::string sourceGoneLabel(const std::string& name) { return "Duplicate skipped: " + name + " is gone"; }

// AL5: while a batch is staged the file label keeps "Loading <name>..."; every other label writer's text is held here
// (the latest wins) and a cancel shows it. hold() at staging (remembers the label's text), divert() from a writer,
// release() at a cancel, clear() at the completion (which writes the done label itself).
class LabelHold
{
public:
    const std::string& hold(const std::string& current, const std::string& loadingText)
    {
        restore_ = current;
        loading_ = loadingText;
        return loading_;
    }
    // A label writer's text while held: true = kept for a cancel (the caller must NOT write the label).
    bool divert(const std::string& text)
    {
        if (loading_.empty())
            return false;
        restore_ = text;
        return true;
    }
    bool held() const { return !loading_.empty(); }
    const std::string& loadingText() const { return loading_; }
    const std::string& restoreText() const { return restore_; }
    // Cancel: the text the label shows now; the hold ends.
    std::string release(const std::string& current)
    {
        auto t = labelAfterCancel(current, loading_, restore_);
        clear();
        return t;
    }
    void clear()
    {
        loading_.clear();
        restore_.clear();
    }

private:
    std::string loading_, restore_;
};

// Every media id a batch handed to the renderer (video at landing, sequence at completion); a cancel retires them all.
struct Adopted
{
    std::vector<uint32_t> ids;
    void add(uint32_t id) { ids.push_back(id); }
    std::vector<uint32_t> takeAll()
    {
        std::vector<uint32_t> out;
        out.swap(ids);
        return out;
    }
};

// AL7: the end state of any sequence of requests equals the synchronous app's. A Composition supersedes everything
// staged or queued; an Append / Duplicate begins when nothing is staged, else it queues (FIFO, <= kQueueMax) and is
// PREPARED when dequeued against the then-live composition; the (kQueueMax + 1)-th is refused.
constexpr std::size_t kQueueMax = 8;
enum class Admit : uint8_t { Begin, Supersede, Enqueue, Refuse };
inline Admit admit(Kind k, bool staged, std::size_t queued)
{
    if (k == Kind::Composition)
        return staged || queued > 0 ? Admit::Supersede : Admit::Begin;
    if (!staged)
        return Admit::Begin;
    return queued < kQueueMax ? Admit::Enqueue : Admit::Refuse;
}

// One queued request: DeckAppend names its file; DeckDuplicate names its source deck by id + the model epoch it was
// read in (a composition cut replaces every deck: ids of the old model mean nothing in the new one).
struct Queued
{
    Kind kind = Kind::DeckAppend;
    std::string file;          // DeckAppend: absolute path
    uint32_t deckId = 0;       // DeckDuplicate
    uint64_t epoch = 0;        // DeckDuplicate
    std::string name;          // for the label notes
};

class LoadQueue
{
public:
    bool push(Queued q)
    {
        if (q_.size() >= kQueueMax)
            return false;
        q_.push_back(std::move(q));
        return true;
    }
    std::optional<Queued> pop()
    {
        if (q_.empty())
            return std::nullopt;
        Queued q = std::move(q_.front());
        q_.pop_front();
        return q;
    }
    void clear() { q_.clear(); }
    std::size_t size() const { return q_.size(); }

private:
    std::deque<Queued> q_;
};

// Where a queued Duplicate's source deck is NOW: its index among the live deck ids, or -1 (skip: the deck was removed,
// or a composition cut replaced the model the id was read in).
inline int resolveDuplicateSource(uint64_t queuedEpoch, uint64_t epochNow, const std::vector<uint32_t>& liveDeckIds,
                                  uint32_t deckId)
{
    if (queuedEpoch != epochNow)
        return -1;
    const auto it = std::find(liveDeckIds.begin(), liveDeckIds.end(), deckId);
    return it == liveDeckIds.end() ? -1 : static_cast<int>(it - liveDeckIds.begin());
}
} // namespace stagedload
