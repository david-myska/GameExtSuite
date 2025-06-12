#pragma once

#include <algorithm>
#include <iostream>
#include <ranges>
#include <unordered_set>
#include <variant>

#include "game_enhancer/achis/conditions.h"
#include "game_enhancer/achis/progress_tracker.h"
#include "game_enhancer/data_accessor.h"
#include "pma/impl/callback/callback.h"
#include "spdlog/spdlog.h"

namespace GE
{
    enum class Status
    {
        Disabled,
        Inactive,
        Active,
        Completed,
        Failed,
        All,
    };

    struct PersistentData
    {
        virtual void Serialize(std::ostream& aOut) const = 0;
        virtual void Deserialize(std::istream& aIn) = 0;
    };

    struct None
    {
    };

    template <typename Metadata, typename SharedData, typename DataAccess = GE::DataAccessor>
    struct Achievement
    {
        using _Metadata = Metadata;
        using _SharedData = SharedData;
        using _DataAccess = DataAccess;

        virtual ~Achievement() = default;

        virtual Status GetStatus() const = 0;

        virtual void Update(const DataAccess& aDataAccess, const SharedData& aSharedData) = 0;

        virtual PMA::ScopedTokenPtr OnStatusChanged(const std::function<void(Status)>& aCallback) = 0;

        virtual PMA::ScopedTokenPtr OnProgressMade(
            const std::function<void(const std::unordered_set<ProgressTracker*>&)>& aCallback) = 0;

        /*
         * Metadata can be used to store name, description, difficulty, reward, ...
         * Just any static data that you might want to display.
         */
        virtual const Metadata& GetMetadata() const = 0;

        virtual const std::unordered_set<ProgressTracker*>& GetProgress(ConditionType aConditionType) const = 0;

        virtual void Serialize(std::ostream& aOut) const = 0;
        virtual void Deserialize(std::istream& aIn) = 0;
    };

    namespace details
    {
        template <typename Metadata, typename ProgressData, typename SharedData, typename DataAccess = GE::DataAccessor>
        class AchievementImpl : public Achievement<Metadata, SharedData, DataAccess>
        {
            Status m_status = Status::Inactive;
            const Metadata m_metadata;
            ProgressData m_progressData;
            std::function<void(ProgressData&, std::unordered_map<ConditionType, std::unordered_set<ProgressTracker*>>&)>
                m_conditionsSetup;
            std::unordered_map<ConditionType, std::unordered_set<ProgressTracker*>> m_progressTrackers;

            std::vector<std::function<void(Status, const DataAccess&, const SharedData&, ProgressData&)>> m_updateCallbacks;
            std::unordered_map<ConditionType,
                               std::vector<std::function<void(const DataAccess&, const SharedData&, ProgressData&)>>>
                m_onPassCallbacks;

            PMA::Callback<Status> m_onStatusChangedCallback;
            PMA::Callback<const std::unordered_set<ProgressTracker*>&> m_onProgressMadeCallback;

            std::shared_ptr<spdlog::logger> m_logger;

            void ProcessInactive(const DataAccess& aDataAccess, const SharedData& aSharedData)
            {
                if (std::ranges::any_of(m_progressTrackers[ConditionType::Activator], [](const ProgressTracker* tracker) {
                        return !tracker->IsCompleted();
                    }))
                {
                    return;
                }
                SetStatus(Status::Active);
                RunOnPassCallbacks(ConditionType::Activator, aDataAccess, aSharedData);
            }

            void ProcessActive(const DataAccess& aDataAccess, const SharedData& aSharedData)
            {
                ConditionType passConditionType = ConditionType::All;

                if (std::ranges::all_of(m_progressTrackers[ConditionType::Completer], [](const ProgressTracker* tracker) {
                        return tracker->IsCompleted();
                    }))
                {
                    bool validated = std::ranges::all_of(m_progressTrackers[ConditionType::Validator],
                                                         [](const ProgressTracker* tracker) {
                                                             return tracker->IsCompleted();
                                                         });
                    SetStatus(validated ? Status::Completed : Status::Failed);
                    if (m_status == Status::Completed)
                    {
                        passConditionType = ConditionType::Completer;
                    }
                }
                if (m_status == Status::Failed ||
                    std::ranges::any_of(m_progressTrackers[ConditionType::Failer], [](const ProgressTracker* tracker) {
                        return tracker->IsCompleted();
                    }))
                {
                    SetStatus(Status::Failed);
                    passConditionType = ConditionType::Failer;
                }

                if (passConditionType != ConditionType::All)
                {
                    RunOnPassCallbacks(passConditionType, aDataAccess, aSharedData);
                }
            }

            void ProcessFailed(const DataAccess& aDataAccess, const SharedData& aSharedData)
            {
                if (std::ranges::all_of(m_progressTrackers[ConditionType::Reseter], [](const ProgressTracker* tracker) {
                        return tracker->IsCompleted();
                    }))
                {
                    SetStatus(Status::Inactive);
                    m_progressData = ProgressData();
                    m_progressTrackers.clear();
                    m_conditionsSetup(m_progressData, m_progressTrackers);
                    RunOnPassCallbacks(ConditionType::Reseter, aDataAccess, aSharedData);
                }
            }

            void RunUpdateForStatus(Status aStatus, const DataAccess& aDataAccess, const SharedData& aSharedData)
            {
                for (const auto& cb : m_updateCallbacks)
                {
                    cb(aStatus, aDataAccess, aSharedData, m_progressData);
                }
            }

            void RunOnPassCallbacks(ConditionType aConditionType, const DataAccess& aDataAccess, const SharedData& aSharedData)
            {
                for (const auto& cb : m_onPassCallbacks[aConditionType])
                {
                    cb(aDataAccess, aSharedData, m_progressData);
                }
            }

            // clang-format off
            std::string TryGetName() {
                if constexpr (requires { { m_metadata.GetName() } -> std::same_as<std::string>; }) {
                    return m_metadata.GetName();
                }
                else if constexpr (requires { { m_metadata.Name() } -> std::same_as<std::string>; }) {
                    return m_metadata.Name();
                }
                else if constexpr (requires { { m_metadata.name } -> std::same_as<std::string>; }) {
                    return m_metadata.name;
                }
                else if constexpr (requires { { m_metadata.m_name } -> std::same_as<std::string>; }) {
                    return m_metadata.m_name;
                }
                else if constexpr (std::is_same_v<Metadata, std::string>) {
                    return m_metadata;
                }
                return {};
            }

            // clang-format on

            auto CreateOnPassLogger(ConditionType aConditionType)
            {
                return [=](const DataAccess&, const SharedData&, ProgressData&) {
                    m_logger->info("Achievement '{}' passed condition '{}'", TryGetName(), to_string(aConditionType));
                };
            }

        public:
            AchievementImpl(
                Metadata aMetadata,
                std::function<void(ProgressData&, std::unordered_map<ConditionType, std::unordered_set<ProgressTracker*>>&)>
                    aConditionsSetup,
                std::vector<std::function<void(Status, const DataAccess&, const SharedData&, ProgressData&)>> aUpdateCallbacks,
                std::unordered_map<ConditionType,
                                   std::vector<std::function<void(const DataAccess&, const SharedData&, ProgressData&)>>>
                    aOnPassCallbacks,
                std::shared_ptr<spdlog::logger> aLogger)
                : m_metadata(std::move(aMetadata))
                , m_conditionsSetup(std::move(aConditionsSetup))
                , m_updateCallbacks(std::move(aUpdateCallbacks))
                , m_onPassCallbacks(std::move(aOnPassCallbacks))
                , m_logger(std::move(aLogger))
            {
                for (uint32_t i = 0; i < static_cast<uint32_t>(ConditionType::All); ++i)
                {
                    ConditionType conditionType = static_cast<ConditionType>(i);
                    if (!m_onPassCallbacks.contains(conditionType))
                    {
                        m_onPassCallbacks[conditionType] = {};
                    }
                    m_onPassCallbacks[conditionType].insert(m_onPassCallbacks[conditionType].begin(),
                                                            CreateOnPassLogger(conditionType));
                    m_progressTrackers[conditionType] = {};
                }
                m_conditionsSetup(m_progressData, m_progressTrackers);
            }

            void Update(const DataAccess& aDataAccess, const SharedData& aSharedData) override
            {
                if (m_status == Status::Disabled || m_status == Status::Completed)
                {
                    return;
                }
                RunUpdateForStatus(Status::All, aDataAccess, aSharedData);
                if (std::ranges::any_of(m_progressTrackers[ConditionType::Precondition], [](const ProgressTracker* tracker) {
                        return !tracker->IsCompleted();
                    }))
                {
                    return;
                }

                RunUpdateForStatus(m_status, aDataAccess, aSharedData);

                auto modifiedTrackers = m_progressData.ExtractModifiedTrackers();
                if (modifiedTrackers.empty())
                {
                    return;  // no progress made
                }

                m_onProgressMadeCallback(modifiedTrackers);

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

            void SetStatus(Status aStatus)
            {
                if (m_status == aStatus)
                {
                    return;
                }
                m_status = aStatus;
                m_onStatusChangedCallback(m_status);
            }

            PMA::ScopedTokenPtr OnStatusChanged(const std::function<void(Status)>& aCallback) override
            {
                return m_onStatusChangedCallback.Add(aCallback);
            }

            PMA::ScopedTokenPtr OnProgressMade(
                const std::function<void(const std::unordered_set<ProgressTracker*>&)>& aCallback) override
            {
                return m_onProgressMadeCallback.Add(aCallback);
            }

            /*
             * Custom data can be used to store name, description, difficulty, reward, ...
             * Just any static data that you might want to display.
             */
            const Metadata& GetMetadata() const override { return m_metadata; }

            const std::unordered_set<ProgressTracker*>& GetProgress(ConditionType aConditionType) const override
            {
                return m_progressTrackers.at(aConditionType);
            }

            void Serialize(std::ostream& aOut) const override
            {
                aOut << (m_status == Status::Completed);
                if constexpr (std::is_base_of_v<PersistentData, ProgressData>)
                {
                    m_progressData.Serialize(aOut);
                }
            }

            void Deserialize(std::istream& aIn) override
            {
                bool completed = false;
                aIn >> completed;
                SetStatus(completed ? Status::Completed : Status::Inactive);
                if constexpr (std::is_base_of_v<PersistentData, ProgressData>)
                {
                    m_progressData.Deserialize(aIn);
                }
            }
        };
    }

    template <typename Metadata = None, typename ProgressData = None, typename SharedData = None,
              typename DataAccess = GE::DataAccessor>
    class AchievementBuilder
    {
        static_assert(std::is_default_constructible_v<ProgressData>, "ProgressData has to be default constructible");

        Metadata m_metadata;
        std::function<void(ProgressData&, std::unordered_map<ConditionType, std::unordered_set<ProgressTracker*>>&)>
            m_conditionsSetup;

        std::unordered_map<ConditionType, std::vector<std::function<void(const DataAccess&, const SharedData&, ProgressData&)>>>
            m_onPassCallbacks;
        std::vector<std::function<void(Status, const DataAccess&, const SharedData&, ProgressData&)>> m_updateCallbacks;

    public:
        AchievementBuilder(
            Metadata aMetadata,
            std::function<void(ProgressData&, std::unordered_map<ConditionType, std::unordered_set<ProgressTracker*>>&)>
                aConditionsSetup)
            : m_metadata(std::move(aMetadata))
            , m_conditionsSetup(std::move(aConditionsSetup))
        {
        }

        std::unique_ptr<Achievement<Metadata, SharedData, DataAccess>> Build(std::shared_ptr<spdlog::logger> aLogger = {})
        {
            if (!aLogger)
            {
                aLogger = std::make_shared<spdlog::logger>("empty");
            }
            return std::make_unique<details::AchievementImpl<Metadata, ProgressData, SharedData, DataAccess>>(
                std::move(m_metadata), std::move(m_conditionsSetup), std::move(m_updateCallbacks), std::move(m_onPassCallbacks),
                std::move(aLogger));
        }

        // TODO do it according to state and not conditiontypes
        /*
         * Callback will be triggered when the corresponding conditions pass and achievement status changes.
         * Activator callback: when Inactive -> Active
         * Completer callback: when Active -> Completed
         * Failer callback: when Active -> Failed
         * Reseter callback: when Failed -> Inactive
         */
        AchievementBuilder& OnPass(ConditionType aConditionType,
                                   const std::function<void(const DataAccess&, const SharedData&, ProgressData&)>& aCallback)
        {
            if (aConditionType == ConditionType::Precondition || aConditionType == ConditionType::Validator)
            {
                throw std::runtime_error("Precondition and Validator cannot have a pass callback");
            }
            if (!m_onPassCallbacks.contains(aConditionType))
            {
                m_onPassCallbacks[aConditionType] = {};
            }
            m_onPassCallbacks[aConditionType].push_back(aCallback);
            return *this;
        }

        AchievementBuilder& Update(Status aUpdateStatus,
                                   const std::function<void(const DataAccess&, const SharedData&, ProgressData&)>& aCallback)
        {
            m_updateCallbacks.push_back([aUpdateStatus, aCallback](Status aStatus, const DataAccess& aDataAccess,
                                                                   const SharedData& aSharedData, ProgressData& aProgressData) {
                if (aStatus != aUpdateStatus)
                {
                    return;
                }
                aCallback(aDataAccess, aSharedData, aProgressData);
            });
            return *this;
        }
    };
}
