#pragma once

#include <functional>
#include <numeric>
#include <vector>

#include "game_enhancer/data_accessor.h"

namespace GE
{

    template <typename... CallableArgs>
    struct Condition
    {
        using Callable = std::function<bool(CallableArgs&&...)>;

        constexpr Condition(const Callable& aCallable, bool aOneTimeSuffice = false)
            : m_callable(aCallable)
            , m_oneTimeSuffice(aOneTimeSuffice)
        {
        }

        bool ForceEvaluate(CallableArgs&&... aCallableArgs) const
        {
            return m_evaluatedToTrue = m_callable(std::forward<CallableArgs>(aCallableArgs)...);
        }

        bool Evaluate(CallableArgs&&... aCallableArgs) const
        {
            if (CanSkipEvaluation())
            {
                return true;
            }
            return ForceEvaluate(std::forward<CallableArgs>(aCallableArgs)...);
        }

        bool IsOneTimeSuffice() const { return m_oneTimeSuffice; }

        bool CanSkipEvaluation() const { return m_oneTimeSuffice && m_evaluatedToTrue; }

        void Reset() { m_evaluatedToTrue = false; }

    private:
        Callable m_callable;

        bool m_oneTimeSuffice = false;
        mutable bool m_evaluatedToTrue = false;
    };

    enum class ConditionType
    {
        /*
         * All preconditions need to be met before activators start trying.
         * Commonly used to filter achievements according to very general requirements.
         * Level, location, story progress...
         */
        Precondition,
        /*
         * All activators need to evaluate to true at the same time to activate the achievement.
         */
        Activator,
        /*
         * All invariants need to evaluate to true on each update.
         * If not, the achievement is marked as Failed.
         * Invariants are just a special case of failers.
         */
        Invariant,
        /*
         * When all completers evaluate to true at the same time, the achievemnt is marked as Completed.
         */
        Completer,
        /*
         * Any failer evaluating to true causes the achievement to be marked as Failed.
         */
        Failer,
        /*
         * When all reseters evaluate to true, achievement can change status from Failed to Inactive.
         */
        Reseter,
        /*
         * Helper value that allows to iterate over the enum.
         */
        All
    };

    template <typename... CallableArgs>
    std::vector<bool> EvaluateEach(const std::vector<Condition<CallableArgs...>>& aConditions,
                                   CallableArgs&&... aArgs)
    {
        std::vector<bool> result;
        result.reserve(aConditions.size());
        for (const auto& c : aConditions)
        {
            result.push_back(c.Evaluate(std::forward<CallableArgs>(aArgs)...));
        }
        return result;
    }

    bool EvaluateAnd(const std::vector<bool>& aBoolVector)
    {
        return std::accumulate(aBoolVector.begin(), aBoolVector.end(), true, std::logical_and<bool>());
    }

    bool EvaluateOr(const std::vector<bool>& aBoolVector)
    {
        return std::accumulate(aBoolVector.begin(), aBoolVector.end(), false, std::logical_or<bool>());
    }

    template <typename... CallableArgs>
    class Conditions
    {
        std::unordered_map<ConditionType, std::vector<Condition<CallableArgs...>>> m_conditionsMap;

    public:
        Conditions(std::vector<Condition<CallableArgs...>> preconditions, std::vector<Condition<CallableArgs...>> activators,
                   std::vector<Condition<CallableArgs...>> invariants, std::vector<Condition<CallableArgs...>> completers,
                   std::vector<Condition<CallableArgs...>> failers, std::vector<Condition<CallableArgs...>> reseters)
            : m_conditionsMap({
                  {ConditionType::Precondition, std::move(preconditions)},
                  {ConditionType::Activator,    std::move(activators)   },
                  {ConditionType::Invariant,    std::move(invariants)   },
                  {ConditionType::Completer,    std::move(completers)   },
                  {ConditionType::Failer,       std::move(failers)      },
                  {ConditionType::Reseter,      std::move(reseters)     },
        })
        {
        }

        Conditions(std::unordered_map<ConditionType, std::vector<Condition<CallableArgs...>>> aConditionsMap)
            : m_conditionsMap(std::move(aConditionsMap))
        {
        }

        std::vector<bool> Evaluate(ConditionType aConditionType, CallableArgs&&... aArgs) const
        {
            return EvaluateEach(m_conditionsMap.at(aConditionType), std::forward<CallableArgs>(aArgs)...);
        }
    };
}
