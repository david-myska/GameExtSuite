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
    public:
        using PassCallback = std::function<void(const DataAccessor&, CustomData&, const SharedData&)>;

    private:
        struct NamedConditions
        {
            std::optional<std::vector<bool>> m_results;
            std::vector<std::string> m_names;
            PassCallback m_onPassCallback;
        };

        const size_t m_id = 0;
        Status m_status = Status::Inactive;
        const Metadata m_metadata;
        Conditions<CustomData, SharedData> m_conditions;
        CustomData m_customData;

        std::unordered_map<ConditionType, NamedConditions> m_cachedConditions;

        void ResetCaches()
        {
            for (auto& [type, conditions] : m_cachedConditions)
            {
                conditions.m_results.reset();
            }
        }

    public:
        Achievement(size_t aId, Metadata aMetadata,
                    std::unordered_map<ConditionType, std::vector<Condition<CustomData, SharedData>>> aConditions,
                    std::unordered_map<ConditionType, std::vector<std::string>> aNames,
                    std::unordered_map<ConditionType, PassCallback> aCallbacks = {})
            : m_id(aId)
            , m_metadata(std::move(aMetadata))
            , m_conditions(std::move(aConditions))
        {
            for (auto& [cType, cNames] : aNames)
            {
                m_cachedConditions[cType].m_names = std::move(cNames);
                if (aCallbacks.contains(cType))
                {
                    m_cachedConditions[cType].m_onPassCallback = aCallbacks[cType];
                }
            }
        }

        void Update(const DataAccessor& aDataAccess, const SharedData& aSharedData)
        {
            ResetCaches();

            switch (m_status)
            {
            case Status::Inactive:
                ProcessInactive(aDataAccess, aSharedData);
                break;
            case Status::Active:
                ProcessActive(aDataAccess, aSharedData);
                break;
            case Status::Failed:
                ProcessFailed(aDataAccess, aSharedData);
                break;
            default:
                // TODO log issue
                [[fallthrough]];
            }
        }

        void ProcessInactive(const DataAccessor& aDataAccess, const SharedData& aSharedData)
        {
            auto& preconditionsCached = m_cachedConditions[ConditionType::Precondition];
            preconditionsCached.m_results = m_conditions.Evaluate(ConditionType::Precondition, aDataAccess, aSharedData);
            if (!EvaluateAnd(*preconditionsCached.m_results))
            {
                return;
            }
            auto& activatorsCached = m_cachedConditions[ConditionType::Activator];
            activatorsCached.m_results = m_conditions.Evaluate(ConditionType::Activator, aDataAccess, aSharedData);
            if (!EvaluateAnd(*activatorsCached.m_results))
            {
                return;
            }
            m_status = Status::Active;
            if (activatorsCached.m_onPassCallback)
            {
                activatorsCached.m_onPassCallback(aDataAccess, m_customData, aSharedData);
            }
        }

        void ProcessActive(const DataAccessor& aDataAccess, const SharedData& aSharedData)
        {
            auto& completersCached = m_cachedConditions[ConditionType::Completer];
            auto& invariantsCached = m_cachedConditions[ConditionType::Invariant];
            auto& failersCached = m_cachedConditions[ConditionType::Failer];
            completersCached.m_results = m_conditions.Evaluate(ConditionType::Completer, aDataAccess, aSharedData);
            invariantsCached.m_results = m_conditions.Evaluate(ConditionType::Invariant, aDataAccess, aSharedData);
            failersCached.m_results = m_conditions.Evaluate(ConditionType::Failer, aDataAccess, aSharedData);
            if (EvaluateAnd(*completersCached.m_results))
            {
                m_status = Status::Completed;
                if (completersCached.m_onPassCallback)
                {
                    completersCached.m_onPassCallback(aDataAccess, m_customData, aSharedData);
                }
            }
            else if (!EvaluateAnd(*invariantsCached.m_results) || EvaluateOr(*failersCached.m_results))
            {
                m_status = Status::Failed;
                if (failersCached.m_onPassCallback)
                {
                    failersCached.m_onPassCallback(aDataAccess, m_customData, aSharedData);
                }
            }
        }

        void ProcessFailed(const DataAccessor& aDataAccess, const SharedData& aSharedData)
        {
            auto& resetersCached = m_cachedConditions[ConditionType::Reseter];
            resetersCached.m_results = m_conditions.Evaluate(ConditionType::Reseter, aDataAccess);
            if (EvaluateAnd(*resetersCached.m_results))
            {
                m_status = Status::Inactive;
                m_customData = CustomData();
                if (resetersCached.m_onPassCallback)
                {
                    resetersCached.m_onPassCallback(aDataAccess, m_customData, aSharedData);
                }
            }
        }

        Status GetStatus() const { return m_status; }

        /*
         * Custom data can be used to store name, description, difficulty, reward, ...
         * Just any static data that you might want to display.
         */
        const Metadata& GetMetadata() const { return m_metadata; }

        const std::vector<bool>& GetConditionResults(ConditionType aConditionType) const
        {
            auto& cached = m_cachedConditions.at(aConditionType).m_results;
            if (!cached)
            {
                throw std::runtime_error("Conditions not evaluated");
            }
            return *cached;
        }

        const std::vector<std::string>& GetConditionNames(ConditionType aConditionType) const
        {
            return m_cachedConditions.at(aConditionType).m_names;
        }
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

        std::unordered_map<ConditionType, std::vector<Condition<CustomData, SharedData>>> m_conditions;
        std::unordered_map<ConditionType, std::vector<std::string>> m_names;
        std::unordered_map<ConditionType, typename Achievement<Metadata, CustomData, SharedData>::PassCallback> m_onPassCallbacks;

    public:
        AchievementBuilder(Metadata aMetadata)
            : m_metadata(std::move(aMetadata))
        {
        }

        Achievement<Metadata, CustomData, SharedData> Build()
        {
            return Achievement<Metadata, CustomData, SharedData>(details::GetNextId(), std::move(m_metadata),
                                                                 std::move(m_conditions), std::move(m_names),
                                                                 std::move(m_onPassCallbacks));
        }

        AchievementBuilder& Add(ConditionType aConditionType, std::string aDescription,
                                const typename Condition<CustomData, SharedData>::Callable& aCallable,
                                bool aOneTimeSuffice = false)
        {
            m_names[aConditionType].emplace_back(std::move(aDescription));
            m_conditions[aConditionType].emplace_back(std::move(aCallable), aOneTimeSuffice);
            return *this;
        }

        /*
         * Callback will be triggered when the corresponding conditions pass and achievement status changes.
         * Activator callback: when Inactive -> Active
         * Completer callback: when Active -> Completed
         * Failer callback: when Active -> Failed
         * Reseter callback: when Failed -> Inactive
         */
        AchievementBuilder& OnPass(ConditionType aConditionType,
                                   const typename Achievement<Metadata, CustomData, SharedData>::PassCallback& aCallback)
        {
            if (aConditionType == ConditionType::Precondition || aConditionType == ConditionType::Invariant)
            {
                throw std::runtime_error("Precondition and Invariant cannot have a pass callback");
            }
            m_onPassCallbacks[aConditionType] = aCallback;
            return *this;
        }
    };

}
