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
        using Callable = std::function<bool(const DataAccessor&, CallableArgs&...)>;

        constexpr Condition(const Callable& aCallable, bool aOneTimeSuffice = false)
            : m_callable(aCallable)
            , m_oneTimeSuffice(oneTimeSuffice)
        {
        }

        bool ForceEvaluate(const DataAccessor& aDataAccess, CallableArgs&&... aCallableArgs) const
        {
            return m_evaluatedToTrue = m_func(aDataAccess, std::forward<CallableArgs>(aCallableArgs)...);
        }

        bool Evaluate(const DataAccessor& aDataAccess, CallableArgs&&... aCallableArgs) const
        {
            if (CanSkipEvaluation())
            {
                return true;
            }
            return ForceEvaluate(aDataAccess, std::forward<CallableArgs>(aCallableArgs)...);
        }

        bool IsOneTimeSuffice() const { return m_oneTimeSuffice; }

        bool CanSkipEvaluation() const { return m_oneTimeSuffice && m_evaluatedToTrue; }

        void Reset() { m_evaluatedToTrue = false; }

    private:
        Callable m_callable;

        bool m_oneTimeSuffice = false;
        mutable bool m_evaluatedToTrue = false;
    };

    template <typename AchiData>
    struct AchiCondition
    {
        std::string m_description;
        Condition<AchiData> m_condition;
    };

    template <typename AchiData>
    std::vector<bool> EvaluateEach(const std::vector<AchiCondition<AchiData>>& aConditions, const DataAccessor& aDataAccess,
                                   AchiData& aAchiData)
    {
        std::vector<bool> result;
        result.reserve(aConditions.size());
        for (const auto& c : aConditions)
        {
            result.push_back(c.m_condition.Evaluate(aDataAccess, aAchiData));
        }
        return result;
    }

    template <typename AchiData>
    bool EvaluateAnd(const std::vector<AchiCondition<AchiData>>& aConditions, const DataAccessor& aDataAccess,
                     AchiData& aAchiData)
    {
        auto boolVector = EvaluateEach(aConditions, aDataAccess, aAchiData);
        return std::accumulate(boolVector.begin(), boolVector.end(), true, std::logical_and<bool>());
    }

    template <typename AchiData>
    bool EvaluateOr(const std::vector<AchiCondition<AchiData>>& aConditions, const DataAccessor& aDataAccess, AchiData& aAchiData)
    {
        auto boolVector = EvaluateEach(aConditions, aDataAccess, aAchiData);
        return std::accumulate(boolVector.begin(), boolVector.end(), false, std::logical_or<bool>());
    }

    template <typename AchiData>
    class Conditions
    {
        /*
         * All preconditions need to be met before activators start trying.
         * Commonly used to filter achievements according to very general requirements.
         * Level, location, story progress...
         */
        std::vector<AchiCondition<AchiData>> m_preconditions;
        /*
         * All activators need to evaluate to true at the same time to activate the achievement.
         */
        std::vector<AchiCondition<AchiData>> m_activators;
        /*
         * All invariants need to evaluate to true on each update.
         * If not, the achievement is marked as Failed.
         * Invariants are just a special case of failers.
         */
        std::vector<AchiCondition<AchiData>> m_invariants;

        /*
         * When all completers evaluate to true at the same time, the achievemnt is marked as Completed.
         */
        std::vector<AchiCondition<AchiData>> m_completers;
        /*
         * Any failer evaluating to true causes the achievement to be marked as Failed.
         */
        std::vector<AchiCondition<AchiData>> m_failers;

        /*
         * When all reseters evaluate to true, achievement can change status from Failed to Inactive.
         */
        std::vector<AchiCondition<AchiData>> m_reseters;

    public:
        Conditions& AddPrecondition(std::string aDescription, const Condition<AchiData>::Callable& aCallable,
                                    bool aOneTimeSuffice = false)
        {
            m_preconditions.push_back({std::move(aDescription), Condition<AchiData>(aCallable, aOneTimeSuffice)});
            return *this;
        }

        Conditions& AddActivator(std::string aDescription, const Condition<AchiData>::Callable& aCallable,
                                 bool aOneTimeSuffice = false)
        {
            m_activators.push_back({std::move(aDescription), Condition<AchiData>(aCallable, aOneTimeSuffice)});
            return *this;
        }

        Conditions& AddInvariant(std::string aDescription, const Condition<AchiData>::Callable& aCallable,
                                 bool aOneTimeSuffice = false)
        {
            m_invariants.push_back({std::move(aDescription), Condition<AchiData>(aCallable, aOneTimeSuffice)});
            return *this;
        }

        Conditions& AddCompleter(std::string aDescription, const Condition<AchiData>::Callable& aCallable,
                                 bool aOneTimeSuffice = false)
        {
            m_completers.push_back({std::move(aDescription), Condition<AchiData>(aCallable, aOneTimeSuffice)});
            return *this;
        }

        Conditions& AddFailer(std::string aDescription, const Condition<AchiData>::Callable& aCallable,
                              bool aOneTimeSuffice = false)
        {
            m_failers.push_back({std::move(aDescription), Condition<AchiData>(aCallable, aOneTimeSuffice)});
            return *this;
        }

        Conditions& AddReseter(std::string aDescription, const Condition<AchiData>::Callable& aCallable,
                               bool aOneTimeSuffice = false)
        {
            m_reseters.push_back({std::move(aDescription), Condition<AchiData>(aCallable, aOneTimeSuffice)});
            return *this;
        }

        bool EvaluatePreconditions(const DataAccessor& aDataAccess, AchiData& aAchiData) const
        {
            return EvaluateAnd(m_preconditions, aDataAccess, aAchiData);
        }

        bool EvaluateActivators(const DataAccessor& aDataAccess, AchiData& aAchiData) const
        {
            return EvaluateAnd(m_activators, aDataAccess, aAchiData);
        }

        bool EvaluateInvariants(const DataAccessor& aDataAccess, AchiData& aAchiData) const
        {
            return EvaluateAnd(m_invariants, aDataAccess, aAchiData);
        }

        bool EvaluateCompleters(const DataAccessor& aDataAccess, AchiData& aAchiData) const
        {
            return EvaluateAnd(m_completers, aDataAccess, aAchiData);
        }

        bool EvaluateFailers(const DataAccessor& aDataAccess, AchiData& aAchiData) const
        {
            return EvaluateOr(m_failers, aDataAccess, aAchiData);
        }

        bool EvaluateReseters(const DataAccessor& aDataAccess, AchiData& aAchiData) const
        {
            return EvaluateAnd(m_reseters, aDataAccess, aAchiData);
        }
    };
}
