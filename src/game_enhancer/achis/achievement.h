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

    template <typename AchiData, typename CustomData>
    class Achievement
    {
        const size_t m_id = 0;
        Status m_status = Status::Disabled;
        CustomData m_customData;
        Conditions<AchiData> m_conditions;
        AchiData m_achiData;

    public:
        Achievement(size_t aId, CustomData aCustomData, Conditions<AchiData> aConditions)
            : m_id(aId)
            , m_customData(std::move(aCustomData))
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
            m_achiData = AchiData();
        }

        void ProcessActive(const DataAccessor& aDataAccess)
        {
            if (m_condition.EvaluateCompleters(aDataAccess))
            {
                m_status = Status::Completed;
            }
            else if (!m_condition.EvaluateInvariants(aDataAccess) || m_conditions.EvaluateFailers(aDataAccess))
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
        const CustomData& GetCustomData() const { return m_customData; }
    };
}
