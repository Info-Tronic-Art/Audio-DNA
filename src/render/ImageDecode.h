#pragma once
#include <juce_graphics/juce_graphics.h>
#include "core/LogLine.h"
#include "render/ImageTexCache.h"
#include "render/PixelConvert.h"
#include <chrono>
#include <cstdint>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

// ImageDecode: image files decoded and converted to GL-ready RGBA rows OFF the GL thread (s-rta-0928 renderleft R1).
// No GL call anywhere in this header (tests/test_image_decode.cpp runs it without a context). The GL thread only
// uploads what a Mailbox delivers.
//
// Jobs capture no `this`: each delivers into a consumer-owned Mailbox through a std::weak_ptr, so a consumer that died
// meanwhile (a retired image sequence, a released compositor) simply drops the result. Every job delivers exactly one
// Result -- Decoded, Unchanged or Failed, also when the decode throws -- so a consumer waiting for one never waits
// forever. The Decoder's destructor drops queued jobs and waits (<= 5 s) for the running ones (FilesBrowser precedent).
namespace ImageDecode
{
enum class Layout { StraightRGBA, PremultipliedRGBA };

struct Result
{
    std::string path;
    ImageTexCache::Stamp stamp;
    uint64_t tag = 0;
    ImageTexCache::Kind kind = ImageTexCache::Kind::Failed;
    int w = 0, h = 0;
    std::vector<uint8_t> rgba;   // GL-ready RGBA8, bottom-up, tightly packed w*4 (Decoded only)
    double decodeMs = 0.0, convertMs = 0.0;
};

// Producer side (decoder threads): push takes the mutex for an O(1) move. Consumer side (the GL thread): tryDrain
// uses try_lock and returns false when contended or empty -- the GL thread NEVER blocks on it.
class Mailbox
{
public:
    void push(Result&& r)
    {
        std::lock_guard<std::mutex> lock(m_);
        box_.push_back(std::move(r));
    }

    bool tryDrain(std::vector<Result>& out)
    {
        std::unique_lock<std::mutex> lock(m_, std::try_to_lock);
        if (!lock.owns_lock() || box_.empty())
            return false;
        for (auto& r : box_)
            out.push_back(std::move(r));
        box_.clear();
        return true;
    }

    int pending()
    {
        std::lock_guard<std::mutex> lock(m_);
        return static_cast<int>(box_.size());
    }

private:
    std::mutex m_;
    std::vector<Result> box_;
};

inline ImageTexCache::Stamp stampOf(const juce::File& f)
{
    return { f.getLastModificationTime().toMilliseconds(), f.getSize() };
}

// Decode + convert one file. The stamp {mtime ms, size} is read FIRST (on this thread, next to the decode --
// FilesBrowser rule); known == stamp -> Unchanged without decoding. StraightRGBA = the bytes the old getPixelColour
// loops uploaded (CompositorEngine, ImageSequence); PremultipliedRGBA = TextureManager::uploadImage's raw swizzle.
inline Result decodeFile(const juce::File& file, Layout layout, uint64_t tag,
                         std::optional<ImageTexCache::Stamp> known)
{
    using Clock = std::chrono::steady_clock;
    Result r;
    r.path = file.getFullPathName().toStdString();
    r.tag = tag;
    r.stamp = stampOf(file);
    if (known.has_value() && *known == r.stamp)
    {
        r.kind = ImageTexCache::Kind::Unchanged;
        return r;
    }
    const auto t0 = Clock::now();
    juce::Image img = juce::ImageFileFormat::loadFrom(file);
    if (!img.isValid())
    {
        r.kind = ImageTexCache::Kind::Failed;
        logLine("[Image] decode FAILED ", r.path);
        return r;
    }
    img = img.convertedToFormat(juce::Image::ARGB);
    const auto t1 = Clock::now();
    r.w = img.getWidth();
    r.h = img.getHeight();
    r.rgba.resize(static_cast<size_t>(r.w) * static_cast<size_t>(r.h) * 4);
    {
        const juce::Image::BitmapData bmp(img, juce::Image::BitmapData::readOnly);
        PixelConvert::argbToGlRgbaBottomUp(bmp, r.rgba.data(), layout == Layout::StraightRGBA);
    }
    const auto t2 = Clock::now();
    r.decodeMs = std::chrono::duration<double, std::milli>(t1 - t0).count();
    r.convertMs = std::chrono::duration<double, std::milli>(t2 - t1).count();
    r.kind = ImageTexCache::Kind::Decoded;
    logLine("[Image] decoded ", r.path, " (", r.w, "x", r.h, ") decode=", juce::String(r.decodeMs, 1),
              " convert=", juce::String(r.convertMs, 1), " ms");
    return r;
}

class Decoder
{
public:
    explicit Decoder(int threads = 3)
        : pool_(juce::ThreadPoolOptions{}
                    .withThreadName("ImageDecode")
                    .withNumberOfThreads(threads)
                    .withDesiredThreadPriority(juce::Thread::Priority::low))
    {
    }

    // Queued jobs are dropped; running decodes finish (a decode cannot be interrupted), at most 5 s.
    ~Decoder() { pool_.removeAllJobs(true, 5000); }

    void request(const juce::File& file, Layout layout, uint64_t tag, std::optional<ImageTexCache::Stamp> known,
                 std::weak_ptr<Mailbox> box)
    {
        pool_.addJob([file, layout, tag, known, box]
        {
            Result r;
            try
            {
                r = decodeFile(file, layout, tag, known);
            }
            catch (...)
            {
                r = Result{};
                r.path = file.getFullPathName().toStdString();
                r.tag = tag;
                r.kind = ImageTexCache::Kind::Failed;
            }
            if (auto b = box.lock())
                b->push(std::move(r));
        });
    }

    int queuedOrRunning() const { return pool_.getNumJobs(); }

    Decoder(const Decoder&) = delete;
    Decoder& operator=(const Decoder&) = delete;

private:
    juce::ThreadPool pool_;
};
} // namespace ImageDecode
