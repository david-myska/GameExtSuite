#pragma once

#include <algorithm>
#include <iostream>
#include <ranges>
#include <unordered_set>
#include <variant>

#include "game_enhancer/achis/conditions.h"
#include "game_enhancer/data_accessor.h"
#include "pma/impl/callback/callback.h"
#include "spdlog/spdlog.h"

namespace GE
{
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
    std::string UnboundDynamicMessage(T aCurrent, T aTarget)
    {
        return std::format("{} : {}", aCurrent, aTarget);
    }

    template <typename T, typename Compare = std::greater_equal<T>>
    std::string BoundDynamicMessage(T aCurrent, T aTarget)
    {
        return std::format("{} / {}", Compare{}(aCurrent, aTarget) ? aTarget : aCurrent, aTarget);
    }

    template <typename T, typename Compare>
    class ProgressTrackerT : public ProgressTracker
    {
        T m_target;
        T m_current;
        std::function<std::string(T, T)> m_dynamicMessage;
        Compare m_compare;

    public:
        ProgressTrackerT(BaseProgressData* aOwner, const std::string& aStaticMessage, T aTarget, T aCurrent = {},
                         std::function<std::string(T, T)> aDynamicMessage = &BoundDynamicMessage<T, Compare>)
            : ProgressTracker(aOwner, aStaticMessage)
            , m_target(aTarget)
            , m_current(aCurrent)
            , m_dynamicMessage(aDynamicMessage)
        {
        }

        bool IsCompleted() const override { return m_compare(m_current, m_target); }

        std::string GetMessage() const override
        {
            return std::format("{}: {}", m_staticMessage, m_dynamicMessage(m_current, m_target));
        }

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

    class ProgressTrackerBool : public ProgressTrackerT<bool, std::equal_to<bool>>, public AssignOps<bool, ProgressTrackerBool>
    {
    public:
        ProgressTrackerBool(BaseProgressData* aOwner, const std::string& aStaticMessage, bool aTarget = true)
            : ProgressTrackerT(aOwner, aStaticMessage, aTarget, !aTarget)
        {
        }

        using AssignOps<bool, ProgressTrackerBool>::operator=;

        std::string GetMessage() const override { return m_staticMessage; }
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

    template <class Compare = std::greater_equal<int>>
    class ProgressTrackerInt : public ProgressTrackerT<int, Compare>,
                               public ArithmeticOps<int, ProgressTrackerInt<Compare>>,
                               public AssignOps<int, ProgressTrackerInt<Compare>>
    {
    public:
        using ProgressTrackerT<int, Compare>::ProgressTrackerT;
        using AssignOps<int, ProgressTrackerInt<Compare>>::operator=;
        using ProgressTrackerT<int, Compare>::GetMessage;
    };

    template <class Compare = std::greater_equal<float>>
    class ProgressTrackerFloat : public ProgressTrackerT<float, Compare>,
                                 public ArithmeticOps<float, ProgressTrackerFloat<Compare>>,
                                 public AssignOps<float, ProgressTrackerFloat<Compare>>
    {
    public:
        using ProgressTrackerT<float, Compare>::ProgressTrackerT;
        using AssignOps<float, ProgressTrackerFloat<Compare>>::operator=;
        using ProgressTrackerT<float, Compare>::GetMessage;
    };

    template <std::integral T>
    std::string MinSecDynamicMessage(T aCurrent, T aTarget)
    {
        auto remaining = aTarget - aCurrent;
        return std::format("{}:{:02}", remaining / 60, remaining % 60);
    }

    template <std::integral T>
    std::string HourMinSecDynamicMessage(T aCurrent, T aTarget)
    {
        auto remaining = aTarget - aCurrent;
        return std::format("{}:{:02}:{:02}", remaining / 3600, (remaining % 3600) / 60, remaining % 60);
    }

    class ProgressTrackerTimer : public ProgressTrackerT<int, std::equal_to<int>>, public AssignOps<int, ProgressTrackerTimer>
    {
        bool m_running = false;
        bool m_paused = false;
        std::chrono::steady_clock::time_point m_startTime;
        std::chrono::steady_clock::time_point m_pauseStarted;
        std::chrono::steady_clock::duration m_pauseDuration = std::chrono::steady_clock::duration::zero();

    public:
        ProgressTrackerTimer(BaseProgressData* aOwner, int aTarget, const std::string& aStaticMessage = "Time remaining")
            : ProgressTrackerT(aOwner, aStaticMessage, aTarget, 0, &MinSecDynamicMessage<int>)
        {
        }

        using AssignOps<int, ProgressTrackerTimer>::operator=;

        void Start()
        {
            if (m_running)
            {
                return;
            }
            m_startTime = std::chrono::steady_clock::now();
            m_running = true;
            m_paused = false;
        }

        void Stop()
        {
            if (!m_running)
            {
                return;
            }
            m_running = false;
            m_paused = false;
        }

        void Reset()
        {
            m_running = false;
            m_paused = false;
            m_pauseDuration = std::chrono::steady_clock::duration::zero();
            SetCurrent(0);
        }

        void Pause(bool aPause = true)
        {
            if (!m_running || m_paused == aPause)
            {
                return;
            }
            m_paused = aPause;
            if (m_paused)
            {
                m_pauseStarted = std::chrono::steady_clock::now();
            }
            else
            {
                m_pauseDuration += std::chrono::steady_clock::now() - m_pauseStarted;
            }
        }

        void Update()
        {
            if (!m_running || m_paused)
            {
                return;
            }
            auto elapsed = (std::chrono::steady_clock::now() - m_pauseDuration) - m_startTime;
            SetCurrent(static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(elapsed).count()));
            if (IsCompleted())
            {
                Stop();
            }
        }

        bool IsRunning() const { return m_running; }

        bool IsPaused() const { return m_paused; }
    };
}
