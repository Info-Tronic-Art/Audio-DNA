#pragma once
#include <algorithm>
#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <string>

// logLine: the stderr log line for code that can run off the message thread (lane tsan, s-rta-1002; Pitfall 63).
// Two threads streaming into std::cerr race on its shared ios_base state (ThreadSanitizer family A: AnalysisThread
// vs the ApiServer thread, VideoPlayer's "Opened" line on two MediaOpen threads). logLine formats into a LOCAL
// std::ostringstream (no shared stream state) and writes the whole line with ONE std::fwrite to stderr, which the
// stdio FILE lock covers, so two threads' lines never interleave either. Text is what `std::cerr << a << b ... <<
// '\n'` would print. Manipulators that are objects (std::setw(n), std::fixed) pass as arguments; std::endl does not
// (an overload set): end the line by calling logLine, it appends the '\n'.
// Rule: a file that can run off the message thread uses logLine, never std::cerr (test_log_line_lint pins the list).
template <class... A>
inline void logLine(const A&... a)
{
    std::ostringstream os;
    (os << ... << a);
    os << '\n';
    const std::string s = os.str();
    std::fwrite(s.data(), 1, s.size(), stderr);
}

// logLinef: the ZERO-HEAP twin of logLine (s-rta-1002 fix round, ruling F3) for a thread that must not allocate in
// its steady state -- CLAUDE.md Sacred Rule 3: the analysis thread (its rate-change line and its periodic profile),
// and the GL thread's periodic Render Profile. printf-style: vsnprintf into a STACK buffer of kLogLineMax bytes, then
// ONE std::fwrite of the whole line (the '\n' appended) to stderr -- no std::string, no stream, no heap (measured on
// Apple libc for %d / %s / %g). A line longer than the buffer is cut and ends with kLogLineTruncated. A bad format
// writes kLogLineBadFormat. formatLogLinef / vformatLogLinef format into a caller buffer (the unit test pins them);
// logLinefTo writes to any FILE*.
#if defined(__clang__) || defined(__GNUC__)
#define ADNA_LOGLINE_PRINTF(f, a) __attribute__((format(printf, f, a)))
#else
#define ADNA_LOGLINE_PRINTF(f, a)
#endif

inline constexpr std::size_t kLogLineMax = 512;
inline constexpr char kLogLineTruncated[] = "...[truncated]";
inline constexpr char kLogLineBadFormat[] = "[logLinef: bad format]";

// Formats one line into out[0 .. cap): the text, then '\n', then a NUL. Returns the line length including the '\n'
// (what logLinef writes), 0 when cap is too small to hold even the truncation marker.
ADNA_LOGLINE_PRINTF(3, 0)
inline std::size_t vformatLogLinef(char* out, std::size_t cap, const char* fmt, std::va_list ap) noexcept
{
    constexpr std::size_t markLen = sizeof(kLogLineTruncated) - 1;
    if (out == nullptr || cap < markLen + 2)
        return 0;
    const int n = std::vsnprintf(out, cap - 1, fmt, ap);   // leave one byte for the '\n'
    std::size_t len;
    if (n < 0)
    {
        len = std::min(sizeof(kLogLineBadFormat) - 1, cap - 2);
        std::memcpy(out, kLogLineBadFormat, len);
    }
    else if (static_cast<std::size_t>(n) < cap - 1)
    {
        len = static_cast<std::size_t>(n);
    }
    else
    {
        len = cap - 2;   // vsnprintf kept cap - 2 characters: mark the cut at their end
        std::memcpy(out + len - markLen, kLogLineTruncated, markLen);
    }
    out[len] = '\n';
    out[len + 1] = '\0';
    return len + 1;
}

ADNA_LOGLINE_PRINTF(3, 4)
inline std::size_t formatLogLinef(char* out, std::size_t cap, const char* fmt, ...) noexcept
{
    std::va_list ap;
    va_start(ap, fmt);
    const std::size_t len = vformatLogLinef(out, cap, fmt, ap);
    va_end(ap);
    return len;
}

ADNA_LOGLINE_PRINTF(2, 3)
inline void logLinefTo(std::FILE* f, const char* fmt, ...) noexcept
{
    char buf[kLogLineMax];
    std::va_list ap;
    va_start(ap, fmt);
    const std::size_t len = vformatLogLinef(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    std::fwrite(buf, 1, len, f);
}

ADNA_LOGLINE_PRINTF(1, 2)
inline void logLinef(const char* fmt, ...) noexcept
{
    char buf[kLogLineMax];
    std::va_list ap;
    va_start(ap, fmt);
    const std::size_t len = vformatLogLinef(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    std::fwrite(buf, 1, len, stderr);
}
