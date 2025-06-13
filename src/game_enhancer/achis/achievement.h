#pragma once

#include <algorithm>
#include <atomic>
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
        Paused,
        Completed,
        Failed,
        All,
    };

    std::string to_string(Status aStatus);

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

        virtual void Pause(bool aPause) = 0;

        virtual PMA::ScopedTokenPtr OnStatusChanged(const std::function<void(Status, Status)>& aCallback) = 0;

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
            enum class PauseRequest
            {
                None,
                Pause,
                Resume,
            };

            Status m_status = Status::Inactive;
            Status m_prePauseStatus = Status::Inactive;
            std::atomic<PauseRequest> m_pauseRequest = PauseRequest::None;
            const Metadata m_metadata;
            ProgressData m_progressData;
            std::function<void(ProgressData&, std::unordered_map<ConditionType, std::unordered_set<ProgressTracker*>>&)>
                m_conditionsSetup;
            std::unordered_map<ConditionType, std::unordered_set<ProgressTracker*>> m_progressTrackers;

            std::vector<std::function<void(Status, const DataAccess&, const SharedData&, ProgressData&)>> m_updateCallbacks;
            std::vector<std::function<void(Status, const DataAccess&, const SharedData&, ProgressData&)>> m_onEnteringCallbacks;
            std::vector<std::function<void(Status, const DataAccess&, const SharedData&, ProgressData&)>> m_onLeavingCallbacks;

            PMA::Callback<Status, Status> m_onStatusChangedCallback;
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
                SetStatus(Status::Active, aDataAccess, aSharedData, m_progressData);
            }

            void ProcessActive(const DataAccess& aDataAccess, const SharedData& aSharedData)
            {
                if (std::ranges::all_of(m_progressTrackers[ConditionType::Completer], [](const ProgressTracker* tracker) {
                        return tracker->IsCompleted();
                    }))
                {
                    bool validated = std::ranges::all_of(m_progressTrackers[ConditionType::Validator],
                                                         [](const ProgressTracker* tracker) {
                                                             return tracker->IsCompleted();
                                                         });
                    SetStatus(validated ? Status::Completed : Status::Failed, aDataAccess, aSharedData, m_progressData);
                }
                if (m_status == Status::Active &&
                    std::ranges::any_of(m_progressTrackers[ConditionType::Failer], [](const ProgressTracker* tracker) {
                        return tracker->IsCompleted();
                    }))
                {
                    SetStatus(Status::Failed, aDataAccess, aSharedData, m_progressData);
                }
            }

            void ProcessFailed(const DataAccess& aDataAccess, const SharedData& aSharedData)
            {
                if (std::ranges::all_of(m_progressTrackers[ConditionType::Reseter], [](const ProgressTracker* tracker) {
                        return tracker->IsCompleted();
                    }))
                {
                    SetStatus(Status::Inactive, aDataAccess, aSharedData, m_progressData);
                    m_progressData = ProgressData();
                    m_progressTrackers.clear();
                    m_conditionsSetup(m_progressData, m_progressTrackers);
                }
            }

            void RunUpdateForStatus(Status aStatus, const DataAccess& aDataAccess, const SharedData& aSharedData)
            {
                for (const auto& cb : m_updateCallbacks)
                {
                    cb(aStatus, aDataAccess, aSharedData, m_progressData);
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

            auto CreateStatusLogger(Status aStatus, const std::string& aAction)
            {
                return [=](Status status, const DataAccess&, const SharedData&, ProgressData&) {
                    if (aStatus != status)
                    {
                        return;
                    }
                    m_logger->info("Achievement '{}' {} status '{}'", TryGetName(), aAction, to_string(aStatus));
                };
            }

            bool ProcessPause(const DataAccess& aDataAccess, const SharedData& aSharedData)
            {
                auto pauseRequest = m_pauseRequest.exchange(PauseRequest::None);
                if (pauseRequest == PauseRequest::Pause && m_status != Status::Paused)
                {
                    m_prePauseStatus = m_status;
                    SetStatus(Status::Paused, aDataAccess, aSharedData, m_progressData);
                }
                if (pauseRequest == PauseRequest::Resume && m_status == Status::Paused)
                {
                    SetStatus(m_prePauseStatus, aDataAccess, aSharedData, m_progressData);
                }
                return m_status == Status::Paused;
            }

        public:
            AchievementImpl(
                Metadata aMetadata,
                std::function<void(ProgressData&, std::unordered_map<ConditionType, std::unordered_set<ProgressTracker*>>&)>
                    aConditionsSetup,
                std::vector<std::function<void(Status, const DataAccess&, const SharedData&, ProgressData&)>> aUpdateCallbacks,
                std::vector<std::function<void(Status, const DataAccess&, const SharedData&, ProgressData&)>> aEnteringCallbacks,
                std::vector<std::function<void(Status, const DataAccess&, const SharedData&, ProgressData&)>> aLeavingCallbacks,
                std::shared_ptr<spdlog::logger> aLogger)
                : m_metadata(std::move(aMetadata))
                , m_conditionsSetup(std::move(aConditionsSetup))
                , m_updateCallbacks(std::move(aUpdateCallbacks))
                , m_onEnteringCallbacks(std::move(aEnteringCallbacks))
                , m_onLeavingCallbacks(std::move(aLeavingCallbacks))
                , m_logger(std::move(aLogger))
            {
                for (uint32_t i = 0; i < static_cast<uint32_t>(Status::All); ++i)
                {
                    Status status = static_cast<Status>(i);
                    m_onEnteringCallbacks.push_back(CreateStatusLogger(status, "entering"));
                    m_onLeavingCallbacks.push_back(CreateStatusLogger(status, "leaving"));
                }

                for (uint32_t i = 0; i < static_cast<uint32_t>(ConditionType::All); ++i)
                {
                    ConditionType conditionType = static_cast<ConditionType>(i);
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

                if (ProcessPause(aDataAccess, aSharedData))
                {
                    RunUpdateForStatus(Status::Paused, aDataAccess, aSharedData);
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

            void Pause(bool aPause) override
            {
                if (m_status == Status::Disabled || m_status == Status::Completed)
                {
                    return;
                }
                m_pauseRequest = aPause ? PauseRequest::Pause : PauseRequest::Resume;
            }

            Status GetStatus() const override { return m_status; }

            void SetStatus(Status aStatus)
            {
                if (m_status == aStatus)
                {
                    return;
                }
                auto oldStatus = m_status;
                m_status = aStatus;
                m_onStatusChangedCallback(m_status, oldStatus);
            }

            void SetStatus(Status aStatus, const DataAccess& aDataAccess, const SharedData& aSharedData,
                           ProgressData& aProgressData)
            {
                if (m_status == aStatus)
                {
                    return;
                }
                for (const auto& cb : m_onLeavingCallbacks)
                {
                    cb(m_status, aDataAccess, aSharedData, aProgressData);
                }
                for (const auto& cb : m_onEnteringCallbacks)
                {
                    cb(aStatus, aDataAccess, aSharedData, aProgressData);
                }
                SetStatus(aStatus);
            }

            PMA::ScopedTokenPtr OnStatusChanged(const std::function<void(Status, Status)>& aCallback) override
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

        std::vector<std::function<void(Status, const DataAccess&, const SharedData&, ProgressData&)>> m_onEnteringCallbacks;
        std::vector<std::function<void(Status, const DataAccess&, const SharedData&, ProgressData&)>> m_onLeavingCallbacks;
        std::vector<std::function<void(Status, const DataAccess&, const SharedData&, ProgressData&)>> m_updateCallbacks;

        auto StatusWrapper(Status aStatus,
                           const std::function<void(const DataAccess&, const SharedData&, ProgressData&)>& aCallback)
        {
            return [aStatus, aCallback](Status status, const DataAccess& dataAccess, const SharedData& sharedData,
                                        ProgressData& progressData) {
                if (status != aStatus)
                {
                    return;
                }
                aCallback(dataAccess, sharedData, progressData);
            };
        }

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
                std::move(m_metadata), std::move(m_conditionsSetup), std::move(m_updateCallbacks),
                std::move(m_onEnteringCallbacks), std::move(m_onLeavingCallbacks), std::move(aLogger));
        }

        AchievementBuilder& OnEntering(Status aStatus,
                                       const std::function<void(const DataAccess&, const SharedData&, ProgressData&)>& aCallback)
        {
            m_onEnteringCallbacks.push_back(StatusWrapper(aStatus, aCallback));
            return *this;
        }

        AchievementBuilder& OnLeaving(Status aStatus,
                                      const std::function<void(const DataAccess&, const SharedData&, ProgressData&)>& aCallback)
        {
            m_onLeavingCallbacks.push_back(StatusWrapper(aStatus, aCallback));
            return *this;
        }

        AchievementBuilder& Update(Status aStatus,
                                   const std::function<void(const DataAccess&, const SharedData&, ProgressData&)>& aCallback)
        {
            m_updateCallbacks.push_back(StatusWrapper(aStatus, aCallback));
            return *this;
        }
    };
}
