#pragma once

#include <iostream>
#include <unordered_set>
#include <variant>

#include "game_enhancer/achis/conditions.h"
#include "game_enhancer/data_accessor.h"
#include "pma/impl/callback/callback.h"

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

    struct ProgressTracker;

    class BaseProgressData
    {
        std::unordered_set<ProgressTracker*> m_modifiedTrackers;

    public:
        virtual ~BaseProgressData() = default;

        void AddModifiedTracker(ProgressTracker* aTracker) { m_modifiedTrackers.insert(aTracker); }

        std::unordered_set<ProgressTracker*> ExtractModifiedTrackers()
        {
            decltype(m_modifiedTrackers) result;
            std::swap(m_modifiedTrackers, result);
            return result;
        }
    };

    struct ProgressTracker
    {
        virtual ~ProgressTracker() = default;

        virtual bool IsCompleted() const = 0;

        virtual std::string GetMessage() const { return m_staticMessage; }

        uint32_t GetId() const { return m_id; }

    protected:
        static uint32_t GetUniqueId()
        {
            static std::atomic_uint32_t id = 0;
            return ++id;
        }

        ProgressTracker(BaseProgressData* aOwner, const std::string& aStaticMessage)
            : m_owner(aOwner)
            , m_staticMessage(aStaticMessage)
        {
            if (!m_owner)
            {
                throw std::runtime_error("ProgressTracker owner cannot be null");
            }
        }

        BaseProgressData* m_owner;
        std::string m_staticMessage;
        uint32_t m_id = GetUniqueId();
    };

    template <typename T>
    class ProgressTrackerT : public ProgressTracker
    {
        T m_target;
        T m_current;

    public:
        ProgressTrackerT(BaseProgressData* aOwner, const std::string& aStaticMessage, T aTarget, T aCurrent = {})
            : ProgressTracker(aOwner, aStaticMessage)
            , m_target(aTarget)
            , m_current(aCurrent)
        {
        }

        bool IsCompleted() const override { return m_current == m_target; }

        T GetTarget() const { return m_target; }

        void SetTarget(T aTarget)
        {
            if (aTarget == m_target)
            {
                return;
            }
            m_target = aTarget;
            m_owner->AddModifiedTracker(this);
        }

        T GetCurrent() const { return m_current; }

        void SetCurrent(T aCurrent)
        {
            if (aCurrent == m_current)
            {
                return;
            }
            m_current = aCurrent;
            m_owner->AddModifiedTracker(this);
        }

        // TODO add clamp between min and max (original and target)
    };

    template <typename T, typename Derived>
    class AssignOps
    {
    public:
        Derived& operator=(T v)
        {
            derived().SetCurrent(v);
            return derived();
        }

    private:
        Derived& derived() { return static_cast<Derived&>(*this); }
    };

    class ProgressTrackerBool : public ProgressTrackerT<bool>, public AssignOps<bool, ProgressTrackerBool>
    {
    public:
        ProgressTrackerBool(BaseProgressData* aOwner, const std::string& aStaticMessage, bool aTarget)
            : ProgressTrackerT(aOwner, aStaticMessage, aTarget, !aTarget)
        {
        }
    };

    template <typename T, typename Derived>
    class ArithmeticOps
    {
    public:
        Derived& operator+=(T v)
        {
            static_assert(std::is_arithmetic_v<T>);
            derived().SetCurrent(derived().GetCurrent() + v);
            return derived();
        }

        Derived& operator-=(T v)
        {
            static_assert(std::is_arithmetic_v<T>);
            derived().SetCurrent(derived().GetCurrent() - v);
            return derived();
        }

    private:
        Derived& derived() { return static_cast<Derived&>(*this); }
    };

    class ProgressTrackerInt : public ProgressTrackerT<int>,
                               public ArithmeticOps<int, ProgressTrackerInt>,
                               public AssignOps<int, ProgressTrackerInt>
    {
    public:
        using ProgressTrackerT::ProgressTrackerT;
    };

    class ProgressTrackerFloat : public ProgressTrackerT<float>,
                                 public ArithmeticOps<float, ProgressTrackerFloat>,
                                 public AssignOps<float, ProgressTrackerFloat>
    {
    public:
        using ProgressTrackerT::ProgressTrackerT;
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
            std::unordered_map<ConditionType, std::function<void(const DataAccess&, const SharedData&, ProgressData&)>>
                m_onPassCallbacks;

            PMA::Callback<Status> m_onStatusChangedCallback;
            PMA::Callback<const std::unordered_set<ProgressTracker*>&> m_onProgressMadeCallback;

            void ProcessInactive(const DataAccess& aDataAccess, const SharedData& aSharedData)
            {
                if (std::ranges::any_of(m_progressTrackers[ConditionType::Activator], [](const ProgressTracker* tracker) {
                        return !tracker->IsCompleted();
                    }))
                {
                    return;
                }
                SetStatus(Status::Active);
                m_onPassCallbacks[ConditionType::Activator](aDataAccess, aSharedData, m_progressData);
            }

            void ProcessActive(const DataAccess& aDataAccess, const SharedData& aSharedData)
            {
                std::function<void(const DataAccess&, const SharedData&, ProgressData&)>* passCallback = nullptr;

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
                        passCallback = &m_onPassCallbacks[ConditionType::Completer];
                    }
                }
                if (m_status == Status::Failed ||
                    std::ranges::any_of(m_progressTrackers[ConditionType::Failer], [](const ProgressTracker* tracker) {
                        return tracker->IsCompleted();
                    }))
                {
                    SetStatus(Status::Failed);
                    passCallback = &m_onPassCallbacks[ConditionType::Failer];
                }

                if (passCallback)
                {
                    (*passCallback)(aDataAccess, aSharedData, m_progressData);
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
                    m_onPassCallbacks[ConditionType::Reseter](aDataAccess, aSharedData, m_progressData);
                }
            }

            void RunUpdateForStatus(Status aStatus, const DataAccess& aDataAccess, const SharedData& aSharedData)
            {
                for (const auto& cb : m_updateCallbacks)
                {
                    cb(aStatus, aDataAccess, aSharedData, m_progressData);
                }
            }

        public:
            AchievementImpl(
                Metadata aMetadata,
                std::function<void(ProgressData&, std::unordered_map<ConditionType, std::unordered_set<ProgressTracker*>>&)>
                    aConditionsSetup,
                std::vector<std::function<void(Status, const DataAccess&, const SharedData&, ProgressData&)>> aUpdateCallbacks,
                std::unordered_map<ConditionType, std::function<void(const DataAccess&, const SharedData&, ProgressData&)>>
                    aOnPassCallbacks = {})
                : m_metadata(std::move(aMetadata))
                , m_conditionsSetup(std::move(aConditionsSetup))
                , m_updateCallbacks(std::move(aUpdateCallbacks))
                , m_onPassCallbacks(std::move(aOnPassCallbacks))
            {
                for (uint32_t i = 0; i < static_cast<uint32_t>(ConditionType::All); ++i)
                {
                    ConditionType conditionType = static_cast<ConditionType>(i);
                    m_onPassCallbacks[conditionType] = m_onPassCallbacks.contains(conditionType) ?
                                                           m_onPassCallbacks[conditionType] :
                                                           [](const DataAccess&, const SharedData&, ProgressData&) {};
                    m_progressTrackers[conditionType] = {};
                }
                m_conditionsSetup(m_progressData, m_progressTrackers);
            }

            void Update(const DataAccess& aDataAccess, const SharedData& aSharedData) override
            {
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

        std::unordered_map<ConditionType, std::function<void(const DataAccess&, const SharedData&, ProgressData&)>>
            m_onPassCallbacks;
        std::vector<std::function<void(Status, const DataAccess&, const SharedData&, ProgressData&)>> m_updateCallbacks;

    public:
        using ProgressTrackerAccessor = std::variant<ProgressTrackerBool ProgressData::*, ProgressTrackerInt ProgressData::*,
                                                     ProgressTrackerFloat ProgressData::*>;

        AchievementBuilder(Metadata aMetadata, std::unordered_map<ConditionType, std::vector<ProgressTrackerAccessor>> aMapping)
            : AchievementBuilder(std::move(aMetadata),
                                 [aMapping = std::move(aMapping)](
                                     ProgressData& aProgressData,
                                     std::unordered_map<ConditionType, std::unordered_set<ProgressTracker*>>& aTrackers) {
                                     for (const auto& [conditionType, accessors] : aMapping)
                                     {
                                         for (const auto& accessorVariant : accessors)
                                         {
                                             std::visit(
                                                 [&](auto accessor) {
                                                     aTrackers[conditionType].insert(&(aProgressData.*accessor));
                                                 },
                                                 accessorVariant);
                                         }
                                     }
                                 })
        {
        }

        AchievementBuilder(
            Metadata aMetadata,
            std::function<void(ProgressData&, std::unordered_map<ConditionType, std::unordered_set<ProgressTracker*>>&)>
                aConditionsSetup)
            : m_metadata(std::move(aMetadata))
            , m_conditionsSetup(std::move(aConditionsSetup))
        {
        }

        std::unique_ptr<Achievement<Metadata, SharedData, DataAccess>> Build()
        {
            return std::make_unique<details::AchievementImpl<Metadata, ProgressData, SharedData, DataAccess>>(
                std::move(m_metadata), std::move(m_conditionsSetup), std::move(m_updateCallbacks), std::move(m_onPassCallbacks));
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
            m_onPassCallbacks[aConditionType] = aCallback;
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
