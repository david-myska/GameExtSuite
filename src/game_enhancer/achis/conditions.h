#pragma once

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
         * Preconditions must evaluate to true **every time** before any other conditions are processed.
         * If a Precondition fails, no other conditions will be evaluated.
         * Commonly used to filter achievements based on broad requirements such as
         * level, location, story progress, etc.
         */
        Precondition,

        /*
         * All Activators must evaluate to true **simultaneously** for the achievement to activate.
         */
        Activator,

        /*
         * All Completers must evaluate to true **simultaneously**.
         * Once all Completers pass, Validators are triggered.
         */
        Completer,

        /*
         * If any Failer evaluates to true, the achievement is immediately marked as Failed.
         */
        Failer,

        /*
         * Validators are evaluated **only once** after all Completers have successfully passed.
         * If any Validator fails, the achievement is marked as **Failed**.
         * If all Validators succeed, the achievement is marked as **Completed**.
         * Validators provide a final check to ensure specific success conditions are met.
         *
         * (Validators are run every frame, the same as Completers and Failers.
         *  However; their outcome is ignored until Completers pass.)
         */
        Validator,

        /*
         * All Reseters must evaluate to true **simultaneously** for a previously Failed achievement to revert to Inactive.
         */
        Reseter,

        /*
         * A helper value allowing iteration over all condition types.
         */
        All
    };

    template <typename... CallableArgs>
    std::vector<bool> EvaluateEach(const std::vector<Condition<CallableArgs...>>& aConditions, CallableArgs&&... aArgs)
    {
        std::vector<bool> result;
        result.reserve(aConditions.size());
        for (const auto& c : aConditions)
        {
            result.push_back(c.Evaluate(std::forward<CallableArgs>(aArgs)...));
        }
        return result;
    }

    bool EvaluateAnd(const std::vector<bool>& aBoolVector);

    bool EvaluateOr(const std::vector<bool>& aBoolVector);

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
            for (uint32_t conditionType = 0; conditionType < static_cast<uint32_t>(ConditionType::All); ++conditionType)
            {
                m_conditionsMap.try_emplace(static_cast<ConditionType>(conditionType));
            }
        }

        std::vector<bool> Evaluate(ConditionType aConditionType, CallableArgs&&... aArgs) const
        {
            return EvaluateEach(m_conditionsMap.at(aConditionType), std::forward<CallableArgs>(aArgs)...);
        }
    };
}
