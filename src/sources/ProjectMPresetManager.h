#pragma once
#include <string>
#include <vector>
#include <set>
#include <map>
#include <functional>

// Manages MilkDrop .milk preset files — scanning, loading, categorization.
class ProjectMPresetManager
{
public:
    struct PresetInfo
    {
        std::string name;           // Display name (e.g., "Geiss - Soft Flower")
        std::string path;           // Full file path
        std::string mood;           // Mood tag: Calm, Energetic, Psychedelic, Geometric, Dark, Minimal
        std::string style;          // Optional style tag
        float energy = 0.5f;        // Energy level [0,1] for auto-DJ matching
        bool favorite = false;      // User-starred
        bool userPreset = false;    // Created/customized by user
    };

    ProjectMPresetManager() = default;

    // Scan a directory (and subdirs) for .milk files
    void scanDirectory(const std::string& dirPath);

    // Add preset directories from preferences
    void setPresetDirectories(const std::vector<std::string>& dirs);
    const std::vector<std::string>& getPresetDirectories() const { return presetDirs_; }

    // Rescan all configured directories
    void rescan();

    // Load mood/energy metadata from a JSON manifest (presets.json)
    void loadManifest(const std::string& jsonPath);

    // Save user preferences (favorites, user presets)
    void saveUserData(const std::string& jsonPath) const;
    void loadUserData(const std::string& jsonPath);

    // Get all presets
    const std::vector<PresetInfo>& getAllPresets() const { return presets_; }
    int getPresetCount() const { return static_cast<int>(presets_.size()); }

    // Get preset by index
    const PresetInfo* getPreset(int index) const;

    // Get presets filtered by mood
    std::vector<const PresetInfo*> getPresetsByMood(const std::string& mood) const;

    // Get favorites
    std::vector<const PresetInfo*> getFavorites() const;

    // Get user presets
    std::vector<const PresetInfo*> getUserPresets() const;

    // Search by name (case-insensitive substring match)
    std::vector<const PresetInfo*> search(const std::string& query) const;

    // Toggle favorite status
    void toggleFavorite(int index);

    // Get mood categories with counts
    std::map<std::string, int> getMoodCounts() const;

    // Navigate presets
    int getCurrentIndex() const { return currentIndex_; }
    void setCurrentIndex(int index) { currentIndex_ = index; }
    const PresetInfo* getCurrentPreset() const;
    const PresetInfo* nextPreset();      // Returns next preset (wraps around)
    const PresetInfo* prevPreset();      // Returns previous preset (wraps around)
    const PresetInfo* randomPreset();    // Returns random preset

    // Random preset within a mood filter
    const PresetInfo* randomPresetInMood(const std::string& mood);

    // Callback when preset changes
    std::function<void(const PresetInfo&)> onPresetChanged;

private:
    std::vector<PresetInfo> presets_;
    std::vector<std::string> presetDirs_;
    int currentIndex_ = 0;

    // Derive mood from preset name heuristics if no manifest entry
    static std::string guessMood(const std::string& name);

    // Derive energy from mood
    static float moodToEnergy(const std::string& mood);
};
