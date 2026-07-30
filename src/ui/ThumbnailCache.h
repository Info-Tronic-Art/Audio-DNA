#pragma once
#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include <list>
#include <unordered_map>

// ThumbnailCache: bounded LRU cache of decoded/rescaled thumbnail images,
// keyed by (full path + file modification time). A file edited in place gets
// a new mtime and is therefore treated as a fresh key (cache miss), while an
// unchanged file always hits the cache on folder revisit or re-filter — no
// stale image is ever served.
//
// Header-only, dependency-free (juce_core + juce_graphics only) so it can be
// unit-tested headlessly without pulling in juce_gui_basics.
class ThumbnailCache
{
public:
    // Bounds memory: 64x64 ARGB thumbnails are ~16KB each uncompressed, so
    // the default cap keeps total cache size around ~8MB even after
    // browsing many folders in one session.
    static constexpr int kDefaultMaxEntries = 500;

    explicit ThumbnailCache(int maxEntries = kDefaultMaxEntries) : maxEntries_(maxEntries) {}

    // Returns the cached thumbnail for this file at its current mtime, or an
    // invalid Image on a cache miss (never decoded, or decoded at a
    // different mtime). A hit promotes the entry to most-recently-used.
    juce::Image get(const juce::File& file)
    {
        auto it = index_.find(makeKey(file));
        if (it == index_.end())
            return {};

        order_.splice(order_.begin(), order_, it->second);
        return it->second->image;
    }

    // Stores (or refreshes) the thumbnail for this file at its current
    // mtime, evicting the least-recently-used entry if over capacity.
    void put(const juce::File& file, juce::Image thumbnail)
    {
        auto key = makeKey(file);
        auto it = index_.find(key);
        if (it != index_.end())
        {
            it->second->image = std::move(thumbnail);
            order_.splice(order_.begin(), order_, it->second);
            return;
        }

        order_.push_front({key, std::move(thumbnail)});
        index_[key] = order_.begin();

        while (static_cast<int>(order_.size()) > maxEntries_)
        {
            index_.erase(order_.back().key);
            order_.pop_back();
        }
    }

    int size() const { return static_cast<int>(order_.size()); }

private:
    struct Key
    {
        juce::String path;
        juce::int64 mtimeMs;

        bool operator==(const Key& other) const
        {
            return mtimeMs == other.mtimeMs && path == other.path;
        }
    };

    struct KeyHash
    {
        size_t operator()(const Key& k) const
        {
            return std::hash<juce::String>{}(k.path) ^ (std::hash<juce::int64>{}(k.mtimeMs) << 1);
        }
    };

    struct Entry
    {
        Key key;
        juce::Image image;
    };

    static Key makeKey(const juce::File& file)
    {
        return {file.getFullPathName(), file.getLastModificationTime().toMilliseconds()};
    }

    std::list<Entry> order_;  // front = most recently used
    std::unordered_map<Key, std::list<Entry>::iterator, KeyHash> index_;
    int maxEntries_;
};
