#pragma once
#include "media/GopCache.h"
#include "media/VideoStats.h"

#include <cstdint>
#include <cstdlib>
#include <vector>

extern "C" {
#include <libavutil/frame.h>
#include <libavutil/imgutils.h>
}

#if defined(__APPLE__)
 #include <dispatch/dispatch.h>
 #include <sys/sysctl.h>
#endif
#if defined(__APPLE__) || defined(__linux__)
 #include <sys/mman.h>
 #include <unistd.h>
#endif

// s-rta-0929b gopcache (plan-gopcache.md 3.4 + HARMONY ADOPTION GC3 / GC6 / GC7 / GC10): the FFmpeg-facing half of a
// VideoPlayer's GOP cache -- a slot table of AVFrame COPIES in the decoder's native pixel format (never the ring's BGRA:
// VideoPlayer::convertInto turns a cached frame into a ring slot exactly as it turns a freshly decoded one) with the shared
// budget's accounting. Used by the player's decode thread only (configured in open(), which allocates nothing); header-only
// (included by VideoPlayer.cpp and the ctests). Pitfall 54's shape: a slot table + a budget, evict-before-store, drop
// outside a full window, the served / wanted frames protected (the caller's verdicts), everything freed on clear().
namespace GopCache
{
constexpr int kHole = -2;   // index value: a frame the decoder never output (GC3: skipped over, never sought again)

// GC7: the process-wide budget -- min(2 GiB, RAM / 16); TEST-SERVER builds: ADNA_GOPCACHE_BUDGET_MB=<n> sets it (the u8
// cap-256 row). A memory-pressure source on the main queue (the message thread) sets `pressure`: warn -> half the budget,
// critical -> the floors only, normal -> back; the decode threads evict down to the new cap at their next steps (GC6).
inline Budget& sharedBudget()
{
    static Budget* b = [] {
        auto* x = new Budget();   // process lifetime (the pressure source refers to it)
        uint64_t mem = 0;
#if defined(__APPLE__)
        size_t len = sizeof(mem);
        if (sysctlbyname("hw.memsize", &mem, &len, nullptr, 0) != 0)
            mem = 0;
#endif
        x->total.store(budgetFor(mem), std::memory_order_relaxed);
#if AUDIODNA_TEST_SERVER
        if (const char* e = std::getenv("ADNA_GOPCACHE_BUDGET_MB"))   // TEST-ONLY (test-server builds): u8's cap row
            if (std::atoll(e) > 0)
                x->total.store(static_cast<int64_t>(std::atoll(e)) << 20, std::memory_order_relaxed);
#endif
#if defined(__APPLE__)
        dispatch_source_t src = dispatch_source_create(DISPATCH_SOURCE_TYPE_MEMORYPRESSURE, 0,
                                                       DISPATCH_MEMORYPRESSURE_NORMAL | DISPATCH_MEMORYPRESSURE_WARN
                                                           | DISPATCH_MEMORYPRESSURE_CRITICAL,
                                                       dispatch_get_main_queue());
        if (src != nullptr)
        {
            dispatch_source_set_event_handler(src, ^{
                const unsigned long level = dispatch_source_get_data(src);
                x->pressure.store((level & DISPATCH_MEMORYPRESSURE_CRITICAL) != 0 ? 2
                                  : (level & DISPATCH_MEMORYPRESSURE_WARN) != 0   ? 1
                                                                                  : 0,
                                  std::memory_order_relaxed);
            });
            dispatch_resume(src);
        }
#endif
        return x;
    }();
    return *b;
}

class Store
{
public:
    Store() = default;
    ~Store() { clear(); }
    Store(const Store&) = delete;
    Store& operator=(const Store&) = delete;

    // open(): the decoder's format and size, the budget, the counters. Allocates nothing (Pitfall 58: the first store
    // allocates, on the decode thread).
    void configure(int pixFmt, int w, int h, Budget* budget, VideoStats* stats)
    {
        fmt_ = pixFmt;
        w_ = w;
        h_ = h;
        budget_ = budget;
        stats_ = stats;
        const int est = av_image_get_buffer_size(static_cast<AVPixelFormat>(pixFmt), w, h, 64);
        frameBytes_ = est > 0 ? est : 0;
#if defined(__APPLE__) || defined(__linux__)
        const int64_t page = static_cast<int64_t>(sysconf(_SC_PAGESIZE));   // mapBuffer's real size: whole pages
        frameBytes_ = (frameBytes_ + page - 1) / page * page;
#endif
    }
    void setBudget(Budget* b) { budget_ = b; }   // ctests (before anything is stored)
    Budget* budget() const { return budget_; }

    int slots() const { return static_cast<int>(frames_.size()); }
    int resident() const { return resident_; }
    int64_t bytes() const { return bytes_; }
    int64_t frameBytes() const { return frameBytes_; }   // one slot's real bytes once a slot exists (the estimate before)
    std::vector<Slot>& view() { return view_; }
    const std::vector<Slot>& view() const { return view_; }
    const std::vector<int>& index() const { return index_; }

    void ensureIndex(int n)
    {
        if (n > static_cast<int>(index_.size()))
            index_.resize(static_cast<size_t>(n), -1);
    }
    int slotOf(int rel) const
    {
        return (rel >= 0 && rel < static_cast<int>(index_.size())) ? index_[static_cast<size_t>(rel)] : -1;
    }
    bool isHole(int rel) const
    {
        return rel >= 0 && rel < static_cast<int>(index_.size()) && index_[static_cast<size_t>(rel)] == kHole;
    }
    void markHole(int rel)
    {
        ensureIndex(rel + 1);
        if (rel >= 0 && index_[static_cast<size_t>(rel)] == -1)
            index_[static_cast<size_t>(rel)] = kHole;
    }
    const AVFrame* frameAt(int slot) const { return frames_[static_cast<size_t>(slot)]; }
    double ptsAt(int slot) const { return pts_[static_cast<size_t>(slot)]; }

    // A frame worth keeping: the decoder's format and size, not flagged corrupt (GC3).
    bool storable(const AVFrame* src) const
    {
        return src != nullptr && src->format == fmt_ && src->width == w_ && src->height == h_
               && (src->flags & AV_FRAME_FLAG_CORRUPT) == 0 && src->decode_error_flags == 0;
    }

    // Copy `src` (relative frame rel, its pts) per the verdict: Store into v.slot (a free slot whose memory is kept) or a NEW
    // slot (-1: allocated only while the budget allows -- floorBytes wins), Replace = overwrite v.slot's frame (an
    // eviction). False = not stored (the budget refused, the frame is not storable, an allocation failed).
    bool store(const AVFrame* src, int rel, double pts, const Verdict& v, const Slot& policy, int64_t floorBytes)
    {
        if (v.keep == Keep::Drop || !storable(src) || rel < 0)
            return false;
        int slot = v.slot;
        if (v.keep == Keep::Store && slot < 0)
        {
            slot = allocate(floorBytes);
            if (slot < 0)
                return false;
        }
        if (slot < 0 || slot >= slots())
            return false;
        AVFrame* dst = frames_[static_cast<size_t>(slot)];
        if (av_frame_copy(dst, src) < 0)
            return false;
        auto& s = view_[static_cast<size_t>(slot)];
        if (s.frame >= 0)
        {
            if (slotOf(s.frame) == slot)
                index_[static_cast<size_t>(s.frame)] = -1;
            addResident(-1);
            if (stats_ != nullptr)
                ++stats_->gopCacheEvictions;
        }
        ensureIndex(rel + 1);
        s = policy;
        s.frame = rel;
        index_[static_cast<size_t>(rel)] = slot;
        pts_[static_cast<size_t>(slot)] = pts;
        addResident(1);
        return true;
    }

    // The slot's frame is forgotten; its memory stays for the next store (a Replace does this in one step). `counted` =
    // an eviction (video_gopcache_evictions); a clear is not one.
    void evict(int slot, bool counted = true)
    {
        auto& s = view_[static_cast<size_t>(slot)];
        if (s.frame < 0)
            return;
        if (slotOf(s.frame) == slot)
            index_[static_cast<size_t>(s.frame)] = -1;
        s = Slot{};
        addResident(-1);
        if (counted && stats_ != nullptr)
            ++stats_->gopCacheEvictions;
    }

    // GC6 / trims: the slot's memory goes back to the OS and its bytes to the budget (the last slot moves into its place).
    void freeSlot(int slot, bool counted = true)
    {
        if (slot < 0 || slot >= slots())
            return;
        if (view_[static_cast<size_t>(slot)].frame >= 0)
            evict(slot, counted);
        av_frame_free(&frames_[static_cast<size_t>(slot)]);
        giveBytes(slotBytes_);
        const int last = slots() - 1;
        if (slot != last)
        {
            frames_[static_cast<size_t>(slot)] = frames_[static_cast<size_t>(last)];
            pts_[static_cast<size_t>(slot)] = pts_[static_cast<size_t>(last)];
            view_[static_cast<size_t>(slot)] = view_[static_cast<size_t>(last)];
            const int moved = view_[static_cast<size_t>(slot)].frame;
            if (moved >= 0 && slotOf(moved) == last)
                index_[static_cast<size_t>(moved)] = slot;
        }
        frames_.pop_back();
        pts_.pop_back();
        view_.pop_back();
        noteActive();
    }

    // Every slot freed, every byte given back, the cache inactive. Holes (file properties) stay.
    void clear()
    {
        while (!frames_.empty())
            freeSlot(slots() - 1, false);
        for (auto& i : index_)
            if (i >= 0)
                i = -1;
        addResident(-resident_);
    }

private:
    int fmt_ = -1, w_ = 0, h_ = 0;
    Budget* budget_ = nullptr;
    VideoStats* stats_ = nullptr;
    std::vector<AVFrame*> frames_;
    std::vector<double> pts_;
    std::vector<Slot> view_;
    std::vector<int> index_;
    int resident_ = 0;
    int64_t bytes_ = 0;
    int64_t frameBytes_ = 0;   // the estimate, then one slot's real bytes
    int64_t slotBytes_ = 0;    // one slot's real bytes (0 until the first allocation)
    bool active_ = false;

    int allocate(int64_t floorBytes)
    {
        const int64_t want = slotBytes_ > 0 ? slotBytes_ : frameBytes_;
        if (budget_ != nullptr)
        {
            const auto t = budget_->tryTake(want, bytes_, floorBytes);
            if (t == Budget::Take::Refused)
                return -1;
            if (t == Budget::Take::OkOverBudget && stats_ != nullptr)
                ++stats_->gopCacheOverBudget;
        }
        AVFrame* f = av_frame_alloc();
        if (f != nullptr && !mapBuffer(f))
            av_frame_free(&f);
        if (f == nullptr)
        {
            if (budget_ != nullptr)
                budget_->give(want);
            return -1;
        }
        int64_t real = 0;
        for (auto* b : f->buf)
            real += b != nullptr ? static_cast<int64_t>(b->size) : 0;
        if (slotBytes_ == 0)
        {
            slotBytes_ = real;
            frameBytes_ = real;
            if (budget_ != nullptr && real != want)   // settle the estimate against the real size (GC7: real bytes)
            {
                if (real > want)
                    budget_->bytes.fetch_add(real - want, std::memory_order_relaxed);
                else
                    budget_->give(want - real);
            }
        }
        bytes_ += slotBytes_;
        if (stats_ != nullptr)
            stats_->gopCacheBytes += slotBytes_;
        frames_.push_back(f);
        pts_.push_back(-1.0);
        view_.push_back(Slot{});
        noteActive();
        return slots() - 1;
    }

    // GC10: a slot's planes live in their own anonymous mapping (one buffer, 64-byte aligned lines), unmapped when the frame
    // is freed -- the pages go back to the OS at once (malloc keeps freed multi-MB blocks around: the idle trim of a
    // 900 MB cache moved phys_footprint by 65 MB). Elsewhere: av_frame_get_buffer.
    static void unmap(void* opaque, uint8_t* data)
    {
#if defined(__APPLE__) || defined(__linux__)
        munmap(data, static_cast<size_t>(reinterpret_cast<uintptr_t>(opaque)));
#else
        (void) opaque;
        (void) data;
#endif
    }
    bool mapBuffer(AVFrame* f) const
    {
        f->format = fmt_;
        f->width = w_;
        f->height = h_;
#if defined(__APPLE__) || defined(__linux__)
        const int size = av_image_get_buffer_size(static_cast<AVPixelFormat>(fmt_), w_, h_, 64);
        if (size <= 0)
            return false;
        const size_t page = static_cast<size_t>(sysconf(_SC_PAGESIZE));
        const size_t mapped = (static_cast<size_t>(size) + page - 1) / page * page;
        void* mem = mmap(nullptr, mapped, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
        if (mem == MAP_FAILED)
            return false;
        auto* data = static_cast<uint8_t*>(mem);
        f->buf[0] = av_buffer_create(data, static_cast<size_t>(mapped), &Store::unmap, reinterpret_cast<void*>(mapped), 0);
        if (f->buf[0] == nullptr)
        {
            munmap(mem, mapped);
            return false;
        }
        return av_image_fill_arrays(f->data, f->linesize, data, static_cast<AVPixelFormat>(fmt_), w_, h_, 64) >= 0;
#else
        return av_frame_get_buffer(f, 0) >= 0;
#endif
    }

    void giveBytes(int64_t n)
    {
        bytes_ -= n;
        if (budget_ != nullptr)
            budget_->give(n);
        if (stats_ != nullptr)
            stats_->gopCacheBytes -= n;
    }

    void addResident(int d)
    {
        resident_ += d;
        if (stats_ != nullptr && d != 0)
            stats_->gopCacheFrames += d;
        noteActive();
    }

    // `active` (the budget's cap divisor): this cache holds memory (a resident frame or a kept slot).
    void noteActive()
    {
        const bool now = resident_ > 0 || !frames_.empty();
        if (now == active_)
            return;
        active_ = now;
        if (budget_ != nullptr)
        {
            if (now)
                budget_->activate();
            else
                budget_->deactivate();
        }
        if (stats_ != nullptr)
            stats_->gopCacheActive += now ? 1 : -1;
    }
};
} // namespace GopCache
