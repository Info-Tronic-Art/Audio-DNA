#pragma once
#include "ui/ThumbnailCache.h"
#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <juce_graphics/juce_graphics.h>
#include <functional>
#include <set>

// ClipThumbnails (s-rta-0928, restore-diag.md): the deck grid's IMAGE-clip thumbnails, decoded OFF the message thread
// once per file and shared by every ClipCell and LayerStrip (DeckView owns one). get() never decodes: a hit is a
// ThumbnailCache lookup (path + mtime); a miss queues ONE decode per file on a low-priority pool thread and returns an
// invalid Image; the result lands on the message thread and onLanded fires so the owner re-pulls. It replaces a decode
// from disk on EVERY DeckView::refresh -- once per discrete routine-restore entry and on every clip trigger -- which
// was 99.6-99.9% of a 54-178 ms message-thread hold. FilesBrowser's pattern (FilesBrowser.cpp:565-615), not a new one.
// A path whose decode is in flight is answered before any stat() of the file: a waiting cell re-pulls on every
// refresh, and that costs a set lookup, never file-system I/O.
class ClipThumbnails
{
public:
    static constexpr int kWidth = 90, kHeight = 72;   // the video / sequence Clip::thumbnail size (MainComponent.cpp:2929)

    using Decoder = std::function<juce::Image(const juce::File&)>;   // a pool thread: no Component, no model
    using Poster  = std::function<void(std::function<void()>)>;      // hands a completion to the message thread

    explicit ClipThumbnails(int maxEntries = ThumbnailCache::kDefaultMaxEntries)
        : cache_(maxEntries),
          decode_(&ClipThumbnails::decodeThumbnail),
          post_([](std::function<void()> fn) { juce::MessageManager::callAsync(std::move(fn)); })
    {
    }

    ~ClipThumbnails()
    {
        masterReference.clear();                        // a completion still queued finds nobody home
        pool_.removeAllJobs(true, kShutdownTimeoutMs);  // FIRST (FilesBrowser.cpp:332-345); jobs capture no `this`
    }

    // Message thread. The file's thumbnail; invalid while its decode is queued / running, or when it cannot be decoded
    // (a failure is retried only after the file's mtime changes). NEVER decodes on this thread.
    juce::Image get(const juce::File& imageFile)
    {
        jassert(juce::MessageManager::getInstanceWithoutCreating() == nullptr
                || juce::MessageManager::existsAndIsCurrentThread());
        ++lookups_;
        const auto path = imageFile.getFullPathName();
        if (inFlight_.count(path) != 0)
            return {};                                  // pending: no stat (Harmony adoption D4)
        ++fileStats_;
        const auto failedKey = path + "|" + juce::String(imageFile.getLastModificationTime().toMilliseconds());
        if (failed_.count(failedKey) != 0)
            return {};
        ++fileStats_;                                   // ThumbnailCache::get stats the file for its key
        if (auto hit = cache_.get(imageFile); hit.isValid())
            return hit;
        inFlight_.insert(path);
        ++queued_;
        juce::WeakReference<ClipThumbnails> self(this);
        pool_.addJob([self, imageFile, path, decode = decode_, post = post_] {
            const auto mtime = imageFile.getLastModificationTime();   // beside the decode (FilesBrowser.cpp:574-580)
            juce::Image image;
            try { image = decode(imageFile); } catch (...) {}
            post([self, imageFile, path, mtime, image] {
                if (auto* store = self.get())
                    store->landed(path, imageFile, mtime, image);
            });
        });
        return {};
    }

    // Message thread: a queued decode finished, valid or not (DeckView re-runs refresh(); waiting cells re-pull).
    std::function<void()> onLanded;

    // Tests only, before the first get(): a counting decoder and a manual poster (a ctest runs no message loop:
    // JUCE_MODAL_LOOPS_PERMITTED is 0).
    void setBackendsForTests(Decoder d, Poster p) { jassert(queued_ == 0); decode_ = std::move(d); post_ = std::move(p); }
    int decodesQueued() const noexcept { return queued_; }
    int lookups() const noexcept { return lookups_; }
    int fileStats() const noexcept { return fileStats_; }   // stat() calls get() made (tests)

    // A pool thread: today's ClipCell decode (ClipCell.cpp:408-410), moved off the message thread. Invalid on a missing
    // or undecodable file.
    static juce::Image decodeThumbnail(const juce::File& f)
    {
        auto img = juce::ImageFileFormat::loadFrom(f);
        return img.isValid() ? img.rescaled(kWidth, kHeight, juce::Graphics::lowResamplingQuality) : juce::Image();
    }

private:
    void landed(const juce::String& path, const juce::File& file, juce::Time mtimeAtDecode, const juce::Image& image)
    {
        inFlight_.erase(path);
        if (image.isValid())
            cache_.put(file, mtimeAtDecode, image);   // keyed by what was decoded (ThumbnailCache.h:50-57)
        else
            failed_.insert(path + "|" + juce::String(mtimeAtDecode.toMilliseconds()));
        if (onLanded)
            onLanded();
    }

    static constexpr int kShutdownTimeoutMs = 5000;
    ThumbnailCache cache_;
    std::set<juce::String> inFlight_;   // full paths: never queued twice, never stat'ed while pending
    std::set<juce::String> failed_;     // "path|mtimeMs": a failure waits for a new mtime
    Decoder decode_;
    Poster post_;
    int queued_ = 0, lookups_ = 0, fileStats_ = 0;
    juce::ThreadPool pool_ { juce::ThreadPoolOptions{}.withNumberOfThreads(2).withThreadName("ClipThumbs")
                                                     .withDesiredThreadPriority(juce::Thread::Priority::low) };
    JUCE_DECLARE_WEAK_REFERENCEABLE(ClipThumbnails)
    JUCE_DECLARE_NON_COPYABLE(ClipThumbnails)
};
