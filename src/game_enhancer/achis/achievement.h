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

    template <typename Metadata, typename SharedData, typename DataAccess = GE::DataAccessor>
    struct Achievement
    {
        virtual ~Achievement() = default;

        virtual void Update(const DataAccess& aDataAccess, const SharedData& aSharedData) = 0;
        virtual Status GetStatus() const = 0;

        /*
         * Custom data can be used to store name, description, difficulty, reward, ...
         * Just any static data that you might want to display.
         */
        virtual const Metadata& GetMetadata() const = 0;

        virtual const std::vector<bool>& GetConditionResults(ConditionType aConditionType) const = 0;

        virtual const std::vector<std::string>& GetConditionNames(ConditionType aConditionType) const = 0;
    };

    namespace details
    {
        template <typename Metadata, typename CustomData, typename SharedData, typename DataAccess = GE::DataAccessor>
        class AchievementImpl : public Achievement<Metadata, SharedData, DataAccess>
        {
            struct NamedConditions
            {
                std::vector<bool> m_results;
                std::vector<std::string> m_names;
                std::function<void(const DataAccess&, const SharedData&, CustomData&)> m_onPassCallback;
            };

            const size_t m_id = 0;
            Status m_status = Status::Inactive;
            const Metadata m_metadata;
            Conditions<const DataAccess&, const SharedData&, CustomData&> m_conditions;
            CustomData m_customData;

            std::unordered_map<ConditionType, NamedConditions> m_cachedConditions;

            void ResetConditionResults(NamedConditions& aConditions)
            {
                std::vector<bool>(aConditions.m_results.size(), false).swap(aConditions.m_results);
            }

            void ResetCaches()
            {
                if (m_status == Status::Inactive)
                {
                    ResetConditionResults(m_cachedConditions[ConditionType::Precondition]);
                    ResetConditionResults(m_cachedConditions[ConditionType::Activator]);
                }
                else if (m_status == Status::Active)
                {
                    ResetConditionResults(m_cachedConditions[ConditionType::Failer]);
                    ResetConditionResults(m_cachedConditions[ConditionType::Invariant]);
                    ResetConditionResults(m_cachedConditions[ConditionType::Completer]);
                }
                else if (m_status == Status::Failed)
                {
                    ResetConditionResults(m_cachedConditions[ConditionType::Reseter]);
                }
            }

            void ProcessInactive(const DataAccess& aDataAccess, const SharedData& aSharedData)
            {
                auto& preconditionsCached = m_cachedConditions[ConditionType::Precondition];
                preconditionsCached.m_results = m_conditions.Evaluate(ConditionType::Precondition, aDataAccess, aSharedData,
                                                                      m_customData);
                if (!EvaluateAnd(preconditionsCached.m_results))
                {
                    return;
                }
                auto& activatorsCached = m_cachedConditions[ConditionType::Activator];
                activatorsCached.m_results = m_conditions.Evaluate(ConditionType::Activator, aDataAccess, aSharedData,
                                                                   m_customData);
                if (!EvaluateAnd(activatorsCached.m_results))
                {
                    return;
                }
                m_status = Status::Active;
                if (activatorsCached.m_onPassCallback)
                {
                    activatorsCached.m_onPassCallback(aDataAccess, aSharedData, m_customData);
                }
            }

            void ProcessActive(const DataAccess& aDataAccess, const SharedData& aSharedData)
            {
                auto& completersCached = m_cachedConditions[ConditionType::Completer];
                auto& invariantsCached = m_cachedConditions[ConditionType::Invariant];
                auto& failersCached = m_cachedConditions[ConditionType::Failer];
                completersCached.m_results = m_conditions.Evaluate(ConditionType::Completer, aDataAccess, aSharedData,
                                                                   m_customData);
                invariantsCached.m_results = m_conditions.Evaluate(ConditionType::Invariant, aDataAccess, aSharedData,
                                                                   m_customData);
                failersCached.m_results = m_conditions.Evaluate(ConditionType::Failer, aDataAccess, aSharedData, m_customData);
                if (EvaluateAnd(completersCached.m_results))
                {
                    m_status = Status::Completed;
                    if (completersCached.m_onPassCallback)
                    {
                        completersCached.m_onPassCallback(aDataAccess, aSharedData, m_customData);
                    }
                }
                else if (!EvaluateAnd(invariantsCached.m_results) || EvaluateOr(failersCached.m_results))
                {
                    m_status = Status::Failed;
                    if (failersCached.m_onPassCallback)
                    {
                        failersCached.m_onPassCallback(aDataAccess, aSharedData, m_customData);
                    }
                }
            }

            void ProcessFailed(const DataAccess& aDataAccess, const SharedData& aSharedData)
            {
                auto& resetersCached = m_cachedConditions[ConditionType::Reseter];
                resetersCached.m_results = m_conditions.Evaluate(ConditionType::Reseter, aDataAccess, aSharedData, m_customData);
                if (EvaluateAnd(resetersCached.m_results))
                {
                    m_status = Status::Inactive;
                    m_customData = CustomData();
                    if (resetersCached.m_onPassCallback)
                    {
                        resetersCached.m_onPassCallback(aDataAccess, aSharedData, m_customData);
                    }
                }
            }

        public:
            AchievementImpl(
                size_t aId, Metadata aMetadata,
                std::unordered_map<ConditionType, std::vector<Condition<const DataAccess&, const SharedData&, CustomData&>>>
                    aConditions,
                std::unordered_map<ConditionType, std::vector<std::string>> aNames,
                std::unordered_map<ConditionType, std::function<void(const DataAccess&, const SharedData&, CustomData&)>>
                    aOnPassCallbacks = {})
                : m_id(aId)
                , m_metadata(std::move(aMetadata))
                , m_conditions(std::move(aConditions))
            {
                for (uint32_t conditionType = 0; conditionType < static_cast<uint32_t>(ConditionType::All); ++conditionType)
                {
                    m_cachedConditions.try_emplace(static_cast<ConditionType>(conditionType));
                }
                for (auto& [cType, cNames] : aNames)
                {
                    m_cachedConditions[cType].m_names = std::move(cNames);
                    m_cachedConditions[cType].m_results = std::vector<bool>(m_cachedConditions[cType].m_names.size(), false);
                    if (aOnPassCallbacks.contains(cType))
                    {
                        m_cachedConditions[cType].m_onPassCallback = aOnPassCallbacks[cType];
                    }
                }
            }

            void Update(const DataAccess& aDataAccess, const SharedData& aSharedData) override
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
                    return;
                }
            }

            Status GetStatus() const override { return m_status; }

            /*
             * Custom data can be used to store name, description, difficulty, reward, ...
             * Just any static data that you might want to display.
             */
            const Metadata& GetMetadata() const override { return m_metadata; }

            const std::vector<bool>& GetConditionResults(ConditionType aConditionType) const override
            {
                return m_cachedConditions.at(aConditionType).m_results;
            }

            const std::vector<std::string>& GetConditionNames(ConditionType aConditionType) const override
            {
                return m_cachedConditions.at(aConditionType).m_names;
            }
        };

        size_t GetNextId()
        {
            static size_t s_id = 0;
            return ++s_id;
        }
    }

    template <typename Metadata = None, typename CustomData = None, typename SharedData = None,
              typename DataAccess = GE::DataAccessor>
    class AchievementBuilder
    {
        static_assert(std::is_default_constructible_v<CustomData>, "CustomData has to be default constructible");

        Metadata m_metadata;

        std::unordered_map<ConditionType, std::vector<Condition<const DataAccess&, const SharedData&, CustomData&>>> m_conditions;
        std::unordered_map<ConditionType, std::vector<std::string>> m_names;
        std::unordered_map<ConditionType, std::function<void(const DataAccess&, const SharedData&, CustomData&)>>
            m_onPassCallbacks;

    public:
        AchievementBuilder(Metadata aMetadata)
            : m_metadata(std::move(aMetadata))
        {
        }

        std::unique_ptr<Achievement<Metadata, SharedData, DataAccess>> Build()
        {
            if (!m_conditions.contains(ConditionType::Activator) || !m_conditions.contains(ConditionType::Completer))
            {
                throw std::runtime_error("Every achievement needs at least 1 Activator and 1 Completer!");
            }
            return std::make_unique<details::AchievementImpl<Metadata, CustomData, SharedData, DataAccess>>(
                details::GetNextId(), std::move(m_metadata), std::move(m_conditions), std::move(m_names),
                std::move(m_onPassCallbacks));
        }

        AchievementBuilder& Add(ConditionType aConditionType, std::string aDescription,
                                const typename Condition<const DataAccess&, const SharedData&, CustomData&>::Callable& aCallable,
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
                                   const std::function<void(const DataAccess&, const SharedData&, CustomData&)>& aCallback)
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
