#pragma once
#include <cstdio>
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
