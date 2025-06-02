#pragma once

#include <functional>
#include <numeric>
#include <vector>


namespace GE
{
    bool EvaluateAnd(const std::vector<bool>& aBoolVector)
    {
        return std::accumulate(aBoolVector.begin(), aBoolVector.end(), true, std::logical_and<bool>());
    }

    bool EvaluateOr(const std::vector<bool>& aBoolVector)
    {
        return std::accumulate(aBoolVector.begin(), aBoolVector.end(), false, std::logical_or<bool>());
    }
}
