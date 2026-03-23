#include "sources/ProjectMPresetManager.h"
#include <juce_core/juce_core.h>
#include <algorithm>
#include <random>

void ProjectMPresetManager::scanDirectory(const std::string& dirPath)
{
    juce::File dir(dirPath);
    if (!dir.isDirectory())
        return;

    auto files = dir.findChildFiles(juce::File::findFiles, true, "*.milk;*.prjm");
    for (const auto& file : files)
    {
        // Skip duplicates
        bool exists = false;
        for (const auto& p : presets_)
        {
            if (p.path == file.getFullPathName().toStdString())
            {
                exists = true;
                break;
            }
        }
        if (exists) continue;

        PresetInfo info;
        info.name = file.getFileNameWithoutExtension().toStdString();
        info.path = file.getFullPathName().toStdString();
        info.mood = guessMood(info.name);
        info.energy = moodToEnergy(info.mood);
        presets_.push_back(std::move(info));
    }

    // Sort by name
    std::sort(presets_.begin(), presets_.end(),
              [](const PresetInfo& a, const PresetInfo& b) { return a.name < b.name; });
}

void ProjectMPresetManager::setPresetDirectories(const std::vector<std::string>& dirs)
{
    presetDirs_ = dirs;
}

void ProjectMPresetManager::rescan()
{
    presets_.clear();
    for (const auto& dir : presetDirs_)
        scanDirectory(dir);
}

void ProjectMPresetManager::loadManifest(const std::string& jsonPath)
{
    juce::File file(jsonPath);
    if (!file.existsAsFile())
        return;

    auto json = juce::JSON::parse(file.loadFileAsString());
    if (auto* obj = json.getDynamicObject())
    {
        for (auto& prop : obj->getProperties())
        {
            auto presetName = prop.name.toString().toStdString();
            if (auto* entry = prop.value.getDynamicObject())
            {
                // Find matching preset by name
                for (auto& p : presets_)
                {
                    if (p.name == presetName)
                    {
                        if (entry->hasProperty("mood"))
                            p.mood = entry->getProperty("mood").toString().toStdString();
                        if (entry->hasProperty("energy"))
                            p.energy = static_cast<float>(entry->getProperty("energy"));
                        if (entry->hasProperty("style"))
                            p.style = entry->getProperty("style").toString().toStdString();
                        break;
                    }
                }
            }
        }
    }
}

void ProjectMPresetManager::saveUserData(const std::string& jsonPath) const
{
    auto obj = std::make_unique<juce::DynamicObject>();

    // Save favorites
    juce::Array<juce::var> favArray;
    for (const auto& p : presets_)
    {
        if (p.favorite)
            favArray.add(juce::String(p.name));
    }
    obj->setProperty("favorites", favArray);

    // Save user presets
    juce::Array<juce::var> userArray;
    for (const auto& p : presets_)
    {
        if (p.userPreset)
            userArray.add(juce::String(p.path));
    }
    obj->setProperty("userPresets", userArray);

    juce::File file(jsonPath);
    file.replaceWithText(juce::JSON::toString(juce::var(obj.release())));
}

void ProjectMPresetManager::loadUserData(const std::string& jsonPath)
{
    juce::File file(jsonPath);
    if (!file.existsAsFile())
        return;

    auto json = juce::JSON::parse(file.loadFileAsString());
    if (auto* obj = json.getDynamicObject())
    {
        // Load favorites
        if (auto* favs = obj->getProperty("favorites").getArray())
        {
            for (const auto& fav : *favs)
            {
                auto name = fav.toString().toStdString();
                for (auto& p : presets_)
                {
                    if (p.name == name)
                    {
                        p.favorite = true;
                        break;
                    }
                }
            }
        }

        // Load user presets
        if (auto* ups = obj->getProperty("userPresets").getArray())
        {
            for (const auto& up : *ups)
            {
                auto path = up.toString().toStdString();
                for (auto& p : presets_)
                {
                    if (p.path == path)
                    {
                        p.userPreset = true;
                        break;
                    }
                }
            }
        }
    }
}

const ProjectMPresetManager::PresetInfo* ProjectMPresetManager::getPreset(int index) const
{
    if (index >= 0 && index < static_cast<int>(presets_.size()))
        return &presets_[static_cast<size_t>(index)];
    return nullptr;
}

std::vector<const ProjectMPresetManager::PresetInfo*>
ProjectMPresetManager::getPresetsByMood(const std::string& mood) const
{
    std::vector<const PresetInfo*> result;
    for (const auto& p : presets_)
    {
        if (p.mood == mood)
            result.push_back(&p);
    }
    return result;
}

std::vector<const ProjectMPresetManager::PresetInfo*>
ProjectMPresetManager::getFavorites() const
{
    std::vector<const PresetInfo*> result;
    for (const auto& p : presets_)
    {
        if (p.favorite)
            result.push_back(&p);
    }
    return result;
}

std::vector<const ProjectMPresetManager::PresetInfo*>
ProjectMPresetManager::getUserPresets() const
{
    std::vector<const PresetInfo*> result;
    for (const auto& p : presets_)
    {
        if (p.userPreset)
            result.push_back(&p);
    }
    return result;
}

std::vector<const ProjectMPresetManager::PresetInfo*>
ProjectMPresetManager::search(const std::string& query) const
{
    std::vector<const PresetInfo*> result;
    if (query.empty())
    {
        for (const auto& p : presets_)
            result.push_back(&p);
        return result;
    }

    // Case-insensitive substring match
    std::string lowerQuery = query;
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);

    for (const auto& p : presets_)
    {
        std::string lowerName = p.name;
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
        if (lowerName.find(lowerQuery) != std::string::npos)
            result.push_back(&p);
    }
    return result;
}

void ProjectMPresetManager::toggleFavorite(int index)
{
    if (index >= 0 && index < static_cast<int>(presets_.size()))
        presets_[static_cast<size_t>(index)].favorite = !presets_[static_cast<size_t>(index)].favorite;
}

std::map<std::string, int> ProjectMPresetManager::getMoodCounts() const
{
    std::map<std::string, int> counts;
    for (const auto& p : presets_)
    {
        if (!p.mood.empty())
            counts[p.mood]++;
    }
    return counts;
}

const ProjectMPresetManager::PresetInfo* ProjectMPresetManager::getCurrentPreset() const
{
    return getPreset(currentIndex_);
}

const ProjectMPresetManager::PresetInfo* ProjectMPresetManager::nextPreset()
{
    if (presets_.empty()) return nullptr;
    currentIndex_ = (currentIndex_ + 1) % static_cast<int>(presets_.size());
    auto* p = &presets_[static_cast<size_t>(currentIndex_)];
    if (onPresetChanged) onPresetChanged(*p);
    return p;
}

const ProjectMPresetManager::PresetInfo* ProjectMPresetManager::prevPreset()
{
    if (presets_.empty()) return nullptr;
    currentIndex_ = (currentIndex_ - 1 + static_cast<int>(presets_.size())) % static_cast<int>(presets_.size());
    auto* p = &presets_[static_cast<size_t>(currentIndex_)];
    if (onPresetChanged) onPresetChanged(*p);
    return p;
}

const ProjectMPresetManager::PresetInfo* ProjectMPresetManager::randomPreset()
{
    if (presets_.empty()) return nullptr;

    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, static_cast<int>(presets_.size()) - 1);
    currentIndex_ = dist(rng);
    auto* p = &presets_[static_cast<size_t>(currentIndex_)];
    if (onPresetChanged) onPresetChanged(*p);
    return p;
}

const ProjectMPresetManager::PresetInfo*
ProjectMPresetManager::randomPresetInMood(const std::string& mood)
{
    auto filtered = getPresetsByMood(mood);
    if (filtered.empty())
        return randomPreset(); // Fallback to any

    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, static_cast<int>(filtered.size()) - 1);
    auto* selected = filtered[static_cast<size_t>(dist(rng))];

    // Update current index to match
    for (int i = 0; i < static_cast<int>(presets_.size()); ++i)
    {
        if (presets_[static_cast<size_t>(i)].path == selected->path)
        {
            currentIndex_ = i;
            break;
        }
    }

    if (onPresetChanged) onPresetChanged(*selected);
    return selected;
}

std::string ProjectMPresetManager::guessMood(const std::string& name)
{
    std::string lower = name;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    // Energetic keywords
    if (lower.find("energy") != std::string::npos ||
        lower.find("fire") != std::string::npos ||
        lower.find("blast") != std::string::npos ||
        lower.find("explod") != std::string::npos ||
        lower.find("rave") != std::string::npos ||
        lower.find("strobe") != std::string::npos ||
        lower.find("flash") != std::string::npos ||
        lower.find("chaos") != std::string::npos)
        return "Energetic";

    // Psychedelic keywords
    if (lower.find("psyche") != std::string::npos ||
        lower.find("trip") != std::string::npos ||
        lower.find("acid") != std::string::npos ||
        lower.find("warp") != std::string::npos ||
        lower.find("morph") != std::string::npos ||
        lower.find("halluc") != std::string::npos ||
        lower.find("fractal") != std::string::npos ||
        lower.find("kaleid") != std::string::npos)
        return "Psychedelic";

    // Geometric keywords
    if (lower.find("geom") != std::string::npos ||
        lower.find("grid") != std::string::npos ||
        lower.find("line") != std::string::npos ||
        lower.find("cube") != std::string::npos ||
        lower.find("sphere") != std::string::npos ||
        lower.find("tunnel") != std::string::npos ||
        lower.find("spiral") != std::string::npos)
        return "Geometric";

    // Dark keywords
    if (lower.find("dark") != std::string::npos ||
        lower.find("shadow") != std::string::npos ||
        lower.find("void") != std::string::npos ||
        lower.find("black") != std::string::npos ||
        lower.find("night") != std::string::npos)
        return "Dark";

    // Minimal keywords
    if (lower.find("minimal") != std::string::npos ||
        lower.find("simple") != std::string::npos ||
        lower.find("clean") != std::string::npos ||
        lower.find("subtle") != std::string::npos)
        return "Minimal";

    // Calm keywords
    if (lower.find("calm") != std::string::npos ||
        lower.find("soft") != std::string::npos ||
        lower.find("gentle") != std::string::npos ||
        lower.find("dream") != std::string::npos ||
        lower.find("flow") != std::string::npos ||
        lower.find("water") != std::string::npos ||
        lower.find("ocean") != std::string::npos ||
        lower.find("cloud") != std::string::npos ||
        lower.find("float") != std::string::npos)
        return "Calm";

    // Default: classify by first letter range for variety
    if (!lower.empty())
    {
        char c = lower[0];
        if (c < 'g') return "Energetic";
        if (c < 'm') return "Geometric";
        if (c < 's') return "Psychedelic";
        return "Calm";
    }

    return "Calm";
}

float ProjectMPresetManager::moodToEnergy(const std::string& mood)
{
    if (mood == "Energetic") return 0.9f;
    if (mood == "Psychedelic") return 0.7f;
    if (mood == "Geometric") return 0.5f;
    if (mood == "Dark") return 0.4f;
    if (mood == "Minimal") return 0.3f;
    if (mood == "Calm") return 0.2f;
    return 0.5f;
}
