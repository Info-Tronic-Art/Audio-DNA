#pragma once
#include <algorithm>
#include <vector>

// TabBarLayout -- s-rta-0925 step-4 polish D2. Widths for a row of tab buttons across `totalWidth`: every tab gets
// at least its label width plus `padding` on each side (whole words, never clipped -- CLAUDE.md UI Text Rules),
// and the width left over is shared equally, one extra pixel to the leftmost tabs so the widths sum to
// `totalWidth` exactly (the last tab ends flush with the bar, as before). If even the minimums do not fit, every
// minimum is scaled by the same factor (the longest label keeps the largest share): a narrow bar clips every label
// a little rather than one label a lot. Pure -- the caller measures the labels; pinned by tests/test_tab_bar_layout.cpp.
inline std::vector<int> tabWidthsFor(const std::vector<int>& labelWidths, int padding, int totalWidth)
{
    const int n = static_cast<int>(labelWidths.size());
    std::vector<int> widths(static_cast<size_t>(n), 0);
    if (n == 0 || totalWidth <= 0)
        return widths;

    long long minSum = 0;
    for (int i = 0; i < n; ++i)
    {
        widths[static_cast<size_t>(i)] = std::max(0, labelWidths[static_cast<size_t>(i)]) + 2 * std::max(0, padding);
        minSum += widths[static_cast<size_t>(i)];
    }

    if (minSum <= totalWidth)
    {
        const int extra = totalWidth - static_cast<int>(minSum);
        for (int i = 0; i < n; ++i)
            widths[static_cast<size_t>(i)] += extra / n + (i < extra % n ? 1 : 0);
        return widths;
    }

    // Too narrow: floor(min_i * total / minSum), then the (< n) missing pixels go one each to the leftmost tabs.
    long long sum = 0;
    for (int i = 0; i < n; ++i)
    {
        widths[static_cast<size_t>(i)] = static_cast<int>((static_cast<long long>(widths[static_cast<size_t>(i)]) * totalWidth) / minSum);
        sum += widths[static_cast<size_t>(i)];
    }
    for (int i = 0; sum < totalWidth; ++i, ++sum)
        ++widths[static_cast<size_t>(i % n)];
    return widths;
}
