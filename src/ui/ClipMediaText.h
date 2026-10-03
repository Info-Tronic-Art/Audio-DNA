#pragma once
// ClipMediaText.h -- what a clip's file is and where it lives, in words (BF3 "Codec display for each video and easy
// access to that video in finder"; s-rta-1002b ui U2.1, plan-ui.md + ruling-ui.md AM9 / AM12).
//
// Pure: juce_core + Clip + VideoInfo, no widget, no I/O (no stat, no probe, no decode). The cell tooltip, the cell's
// right-click menu, the Clip inspector's info rows and the TEST-ONLY /api/debug/clip_media all read these, so they can
// never disagree. A video's codec / size / rate come from the player that is open for the clip (VideoInfoSource, wired
// by MainComponent::videoInfoFor); "missing" is the clip's own presence flag (Clip::mediaMissing).
#include "model/Clip.h"
#include "media/VideoInfo.h"
#include <functional>
#include <optional>
#include <vector>

// The open player's info for a Video clip, or nothing (no player yet, another file, unknown codec). Message thread.
using VideoInfoSource = std::function<std::optional<VideoInfo>(const Clip&)>;

struct Described
{
    juce::String line;                   // one line: tooltip / REST ("H.264 High, 1920 x 1080, 29.97 frames per second")
    std::vector<juce::String> lines;     // the inspector's rows (AM9: a known video = {codec, size + rate})
    juce::String pathTip;                // the reveal target's full path
    juce::File revealTarget;             // what Show in Finder selects
    bool fileBacked = false;             // a video, picture or sequence with a file: the row, the button, the menu
    bool missing = false;                // Clip::mediaMissing (Video / Image only)
};

namespace clipmedia
{
// Video / Image: the clip's file; ImageSequence: its first image; anything else (or no file): File().
inline juce::File revealTarget(const Clip& c)
{
    switch (c.mediaType)
    {
        case Clip::MediaType::Video:
        case Clip::MediaType::Image:         return c.mediaFile;
        case Clip::MediaType::ImageSequence: return c.sequenceFiles.empty() ? juce::File() : c.sequenceFiles.front();
        default:                             return {};
    }
}

// A picture's kind from its extension, no decode: "PNG image", "JPEG image" (JPG too), "TIFF image" (TIF too).
inline juce::String imageKind(const juce::File& f)
{
    auto ext = f.getFileExtension().fromFirstOccurrenceOf(".", false, false).toUpperCase();
    if (ext == "JPG") ext = "JPEG";
    if (ext == "TIF") ext = "TIFF";
    return ext.isEmpty() ? juce::String("Image") : ext + " image";
}

inline Described describe(const Clip& c, const std::optional<VideoInfo>& video)
{
    Described d;
    d.revealTarget = revealTarget(c);
    d.fileBacked = d.revealTarget != juce::File();
    if (!d.fileBacked)
        return d;
    d.pathTip = d.revealTarget.getFullPathName();
    switch (c.mediaType)
    {
        case Clip::MediaType::Video:
            d.missing = c.mediaMissing;
            if (d.missing)
                d.lines = { "File missing" };
            else if (video.has_value() && video->known())
                d.lines = { juce::String(videoinfo::codecLine(*video)), juce::String(videoinfo::frameLine(*video)) };
            else
                d.lines = { "Video file not loaded" };
            break;
        case Clip::MediaType::Image:
            d.missing = c.mediaMissing;
            d.lines = { d.missing ? juce::String("File missing") : imageKind(c.mediaFile) };
            break;
        case Clip::MediaType::ImageSequence:
            d.lines = { "Image sequence, " + juce::String(static_cast<int>(c.sequenceFiles.size())) + " images" };
            break;
        default:
            break;
    }
    d.line = juce::StringArray(d.lines.data(), static_cast<int>(d.lines.size())).joinIntoString(", ");
    return d;
}

// The cell's hover text. Video / picture: "<file name>\n<the one-line info>\nRight-click: Show in Finder". Sequence:
// "Image sequence -- N images at X.X images per second" (the em dash; AM12's whole words) + the menu line when it has an
// image. Anything else: "" (no tooltip, as before).
inline juce::String cellTooltip(const Clip& c, const std::optional<VideoInfo>& video)
{
    static const juce::String kMenuLine = "\nRight-click: Show in Finder";
    if (c.mediaType == Clip::MediaType::ImageSequence)
    {
        const juce::String first = "Image sequence " + juce::String(juce::CharPointer_UTF8("\xe2\x80\x94")) + " "
                                 + juce::String(static_cast<int>(c.sequenceFiles.size())) + " images at "
                                 + juce::String(c.sequenceFps, 1) + " images per second";
        return revealTarget(c) != juce::File() ? first + kMenuLine : first;
    }
    if (c.mediaType != Clip::MediaType::Video && c.mediaType != Clip::MediaType::Image)
        return {};
    const auto d = describe(c, video);
    if (!d.fileBacked)
        return {};
    return d.revealTarget.getFileName() + "\n" + d.line + kMenuLine;
}

// The cell's right-click menu: {"Show in Finder"} when the clip has a file to show, else {} (no menu).
inline juce::StringArray menuItems(const Clip* c)
{
    if (c == nullptr || revealTarget(*c) == juce::File())
        return {};
    return { "Show in Finder" };
}
} // namespace clipmedia
