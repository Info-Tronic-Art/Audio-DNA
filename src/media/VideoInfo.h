#pragma once
// VideoInfo.h -- what a video file is (codec, size, frame rate), in words a performer reads (BF3, s-rta-1002b ui U1).
//
// Pure: std only, no JUCE, no FFmpeg. VideoPlayer::open() fills one VideoInfo from the stream it already has open
// (no extra I/O, never a decode) and keeps it read-only after; the message thread reads it through the player that is
// open for a clip (plan-ui.md F-U1). The strings follow the UI Text Rules: whole words, no abbreviations
// ("frames per second", never "fps").
#include <cstdint>
#include <string>
#include <string_view>

struct VideoInfo
{
    std::string codec;          // codecLabel(...), e.g. "H.264 High"; empty = unknown
    int width = 0, height = 0;
    double fps = 0.0;           // 0 = the stream has no rate (never the player's made-up 30.0 fallback)
    bool known() const { return !codec.empty(); }
};

namespace videoinfo
{
// A FourCC as FFmpeg stores AVCodecParameters::codec_tag (MKTAG order: the first character in the low byte).
constexpr uint32_t fourcc(char a, char b, char c, char d)
{
    return static_cast<uint32_t>(static_cast<unsigned char>(a))
         | (static_cast<uint32_t>(static_cast<unsigned char>(b)) << 8)
         | (static_cast<uint32_t>(static_cast<unsigned char>(c)) << 16)
         | (static_cast<uint32_t>(static_cast<unsigned char>(d)) << 24);
}

// The codec's name for a performer. ffName = avcodec_get_name(codec_id); fourcc = codecpar->codec_tag;
// profile = avcodec_profile_name(...) or "" (unknown).
inline std::string codecLabel(std::string_view ffName, uint32_t fourccTag, std::string_view profile)
{
    if (ffName.empty())
        return {};
    const auto withProfile = [&](const char* name) {
        return profile.empty() ? std::string(name) : std::string(name) + " " + std::string(profile);
    };
    if (ffName == "h264") return withProfile("H.264");
    if (ffName == "hevc") return withProfile("HEVC");
    if (ffName == "prores")
    {
        struct Row { const char* profile; uint32_t tag; const char* name; };
        static constexpr Row rows[] = {
            { "Proxy",    fourcc('a', 'p', 'c', 'o'), "ProRes 422 Proxy" },
            { "LT",       fourcc('a', 'p', 'c', 's'), "ProRes 422 LT" },
            { "Standard", fourcc('a', 'p', 'c', 'n'), "ProRes 422" },
            { "HQ",       fourcc('a', 'p', 'c', 'h'), "ProRes 422 HQ" },
            { "4444",     fourcc('a', 'p', '4', 'h'), "ProRes 4444" },
            { "XQ",       fourcc('a', 'p', '4', 'x'), "ProRes 4444 XQ" },
        };
        for (const auto& r : rows)
            if (profile == r.profile) return r.name;
        for (const auto& r : rows)
            if (fourccTag == r.tag) return r.name;
        return "ProRes";
    }
    if (ffName == "hap")
    {
        if (fourccTag == fourcc('H', 'a', 'p', '1')) return "HAP";
        if (fourccTag == fourcc('H', 'a', 'p', '5')) return "HAP Alpha";
        if (fourccTag == fourcc('H', 'a', 'p', 'Y')) return "HAP Q";
        if (fourccTag == fourcc('H', 'a', 'p', 'M')) return "HAP Q Alpha";
        if (fourccTag == fourcc('H', 'a', 'p', 'A')) return "HAP Alpha Only";
        return "HAP";
    }
    if (ffName == "dnxhd")
    {
        // FFmpeg's DNxHR profiles are "DNXHR LB" / "SQ" / "HQ" / "HQX" / "444": keep the grade, case the family name.
        if (profile.substr(0, 5) == "DNXHR")
            return "DNxHR" + std::string(profile.substr(5));
        return "DNxHD";
    }
    struct Named { const char* ff; const char* name; };
    static constexpr Named named[] = {
        { "dxv", "DXV" },           { "mjpeg", "Motion JPEG" },        { "mpeg4", "MPEG-4 Part 2" },
        { "mpeg2video", "MPEG-2" }, { "qtrle", "QuickTime Animation" }, { "png", "PNG" },
        { "rawvideo", "Uncompressed" }, { "vp8", "VP8" }, { "vp9", "VP9" }, { "av1", "AV1" }, { "cfhd", "CineForm" },
    };
    for (const auto& n : named)
        if (ffName == n.ff) return n.name;
    std::string upper(ffName);
    for (auto& ch : upper)
        if (ch >= 'a' && ch <= 'z') ch = static_cast<char>(ch - 'a' + 'A');
    return upper;
}

// A frame rate rounded to 3 decimals with the trailing zeros stripped: 29.97 / 23.976 / 59.94 / 30 / 25.
// 0 (or anything not a positive finite number) -> "" (the stream has no rate). Integer arithmetic: no locale.
inline std::string fpsText(double fps)
{
    if (!(fps > 0.0) || fps > 1.0e9)
        return {};
    const auto milli = static_cast<long long>(fps * 1000.0 + 0.5);
    std::string text = std::to_string(milli / 1000);
    int frac = static_cast<int>(milli % 1000);
    if (frac != 0)
    {
        std::string digits = { static_cast<char>('0' + frac / 100), static_cast<char>('0' + frac / 10 % 10),
                               static_cast<char>('0' + frac % 10) };
        while (!digits.empty() && digits.back() == '0')
            digits.pop_back();
        text += "." + digits;
    }
    return text;
}

// The codec line: "H.264 High".
inline std::string codecLine(const VideoInfo& info)
{
    return info.codec;
}

// The frame line: "1920 x 1080, 29.97 frames per second"; with no rate (fps 0) -> "1920 x 1080".
inline std::string frameLine(const VideoInfo& info)
{
    std::string line = std::to_string(info.width) + " x " + std::to_string(info.height);
    const auto rate = fpsText(info.fps);
    if (!rate.empty())
        line += ", " + rate + " frames per second";
    return line;
}

// The one-line form: codecLine + ", " + frameLine -> "H.264 High, 1920 x 1080, 29.97 frames per second".
inline std::string describe(const VideoInfo& info)
{
    return codecLine(info) + ", " + frameLine(info);
}
} // namespace videoinfo
