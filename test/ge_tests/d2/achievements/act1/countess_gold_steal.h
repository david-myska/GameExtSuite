#pragma once

#include "../base.h"

namespace D2::Achi::CountessGoldSteal
{
    struct CD
    {
        std::optional<Data::GUID> m_countessId;
        uint32_t m_initialGold = 0;
    };

    auto Create()
    {
        return BLD<CD>("Fort Boyard")
            .Add(GE::ConditionType::Activator, "Enter Bloodthrone",
                 [](const D2::Data::DataAccess& aDataAccess, const D2::Data::SharedData& aS, CD& aC) {
                     return aDataAccess.GetMisc().GetZone() == Data::Zone::Act1_Bloodthrone;
                 })
            .OnPass(GE::ConditionType::Activator,
                    [](const D2::Data::DataAccess& aDataAccess, const D2::Data::SharedData& aS, CD& aC) {
                        aC.m_initialGold = 0;  // TODO
                    })
            .Add(GE::ConditionType::Completer, "Kill The Countess",
                 [](const D2::Data::DataAccess& aDataAccess, const D2::Data::SharedData& aS, CD& aC) {
                     if (!aC.m_countessId)
                     {
                         Data::GUID countessId = 0;
                         if (MonsterNearby("THE COUNTESS", aDataAccess, countessId))
                         {
                             aC.m_countessId = countessId;
                         }
                         return false;
                     }
                     return aS.GetDeadMonsters().contains(*aC.m_countessId);
                 })
            .Add(GE::ConditionType::Failer, "Stay in Bloodthrone",
                 [](const D2::Data::DataAccess& aDataAccess, const D2::Data::SharedData& aS, CD& aC) {
                     return aDataAccess.GetMisc().GetZone() != Data::Zone::Act1_Bloodthrone;
                 })
            .Add(GE::ConditionType::Validator, "Gold collected",
                 [](const D2::Data::DataAccess& aDataAccess, const D2::Data::SharedData& aS, CD& aC) {
                     int32_t goldCollected = 0;  // aDataAccess.GetPlayers().GetLocal().m_stats... - aC.m_initialGold;
                     return goldCollected >= 1000;
                 })
            .Build();
    }
}
