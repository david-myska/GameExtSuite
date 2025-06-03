#pragma once

#include "game_enhancer/achis/conditions.h"

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

    std::string to_string(ConditionType aConditionType)
    {
        switch (aConditionType)
        {
        case ConditionType::Precondition:
            return "Precondition";
        case ConditionType::Activator:
            return "Activator";
        case ConditionType::Completer:
            return "Completer";
        case ConditionType::Failer:
            return "Failer";
        case ConditionType::Validator:
            return "Validator";
        case ConditionType::Reseter:
            return "Reseter";
        default:
            return "Unknown";
        }
    }
}
