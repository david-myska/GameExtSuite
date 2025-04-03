#pragma once

#include "game_enhancer/achis/conditions.h"
#include "game_enhancer/data_accessor.h"

namespace GE
{
    enum class Status
    {
        Disabled,
        Inactive,
        Active,
        Completed,
        Failed,
    };

    struct None
    {
    };

    template <typename Metadata, typename CustomData, typename SharedData>
    class Achievement
    {
        const size_t m_id = 0;
        Status m_status = Status::Inactive;
        const Metadata m_metadata;
        Conditions<CustomData, SharedData> m_conditions;
        CustomData m_customData;

    public:
        Achievement(size_t aId, Metadata aMetadata, Conditions<CustomData, SharedData> aConditions)
            : m_id(aId)
            , m_metadata(std::move(aMetadata))
            , m_conditions(std::move(aConditions))
        {
        }

        void Update(const DataAccessor& aDataAccess)
        {
            switch (m_status)
            {
            case Status::Inactive:
                ProcessInactive(aDataAccess);
                break;
            case Status::Active:
                ProcessActive(aDataAccess);
                break;
            case Status::Failed:
                ProcessFailed(aDataAccess);
                break;
            default:
                // TODO log issue
                [[fallthrough]];
            }
        }

        void ProcessInactive(const DataAccessor& aDataAccess)
        {
            if (!m_conditions.EvaluatePreconditions(aDataAccess))
            {
                return;
            }
            if (!m_conditions.EvaluateActivators(aDataAccess))
            {
                return;
            }
            m_status = Status::Active;
            m_customData = CustomData();
        }

        void ProcessActive(const DataAccessor& aDataAccess)
        {
            if (m_conditions.EvaluateCompleters(aDataAccess))
            {
                m_status = Status::Completed;
            }
            else if (!m_conditions.EvaluateInvariants(aDataAccess) || m_conditions.EvaluateFailers(aDataAccess))
            {
                m_status = Status::Failed;
            }
        }

        void ProcessFailed(const DataAccessor& aDataAccess)
        {
            if (m_conditions.EvaluateReseters(aDataAccess))
            {
                m_status = Status::Inactive;
            }
        }

        Status GetStatus() const { return m_status; }

        /*
         * Custom data can be used to store name, description, difficulty, reward, ...
         * Just any static data that you might want to display.
         */
        const Metadata& GetMetadata() const { return m_metadata; }
    };

    namespace details
    {
        size_t GetNextId()
        {
            static size_t s_id = 0;
            return ++s_id;
        }
    }

    template <typename Metadata = None, typename CustomData = None, typename SharedData = None>
    class AchievementBuilder
    {
        static_assert(std::is_default_constructible_v<CustomData>, "CustomData has to be default constructible");

        Metadata m_metadata;
        std::vector<AchiCondition<CustomData, SharedData>> m_preconditions;
        std::vector<AchiCondition<CustomData, SharedData>> m_activators;
        std::vector<AchiCondition<CustomData, SharedData>> m_invariants;
        std::vector<AchiCondition<CustomData, SharedData>> m_completers;
        std::vector<AchiCondition<CustomData, SharedData>> m_failers;
        std::vector<AchiCondition<CustomData, SharedData>> m_reseters;

    public:
        AchievementBuilder(Metadata aMetadata)
            : m_metadata(std::move(aMetadata))
        {
        }

        Achievement<Metadata, CustomData, SharedData> Build()
        {
            return Achievement<Metadata, CustomData, SharedData>(
                details::GetNextId(), std::move(m_metadata),
                Conditions<CustomData, SharedData>(std::move(m_preconditions), std::move(m_activators), std::move(m_invariants),
                                                   std::move(m_completers), std::move(m_failers), std::move(m_reseters)));
        }

        AchievementBuilder& AddPrecondition(std::string aDescription,
                                            const typename Condition<CustomData, SharedData>::Callable& aCallable,
                                            bool aOneTimeSuffice = false)
        {
            m_preconditions.emplace_back(std::move(aDescription), std::move(aCallable), aOneTimeSuffice);
            return *this;
        }

        AchievementBuilder& AddActivator(std::string aDescription,
                                         const typename Condition<CustomData, SharedData>::Callable& aCallable,
                                         bool aOneTimeSuffice = false)
        {
            m_activators.emplace_back(std::move(aDescription), std::move(aCallable), aOneTimeSuffice);
            return *this;
        }

        AchievementBuilder& AddInvariant(std::string aDescription,
                                         const typename Condition<CustomData, SharedData>::Callable& aCallable,
                                         bool aOneTimeSuffice = false)
        {
            m_invariants.emplace_back(std::move(aDescription), std::move(aCallable), aOneTimeSuffice);
            return *this;
        }

        AchievementBuilder& AddCompleter(std::string aDescription,
                                         const typename Condition<CustomData, SharedData>::Callable& aCallable,
                                         bool aOneTimeSuffice = false)
        {
            m_completers.emplace_back(std::move(aDescription), std::move(aCallable), aOneTimeSuffice);
            return *this;
        }

        AchievementBuilder& AddFailer(std::string aDescription,
                                      const typename Condition<CustomData, SharedData>::Callable& aCallable,
                                      bool aOneTimeSuffice = false)
        {
            m_failers.emplace_back(std::move(aDescription), std::move(aCallable), aOneTimeSuffice);
            return *this;
        }

        AchievementBuilder& AddReseter(std::string aDescription,
                                       const typename Condition<CustomData, SharedData>::Callable& aCallable,
                                       bool aOneTimeSuffice = false)
        {
            m_reseters.emplace_back(std::move(aDescription), std::move(aCallable), aOneTimeSuffice);
            return *this;
        }
    };

}
