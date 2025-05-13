#pragma once

#include "../base.h"

namespace D2::Achi::BloodRavenWalkDistance
{
    struct CD
    {
        Data::GUID m_bloodRavenId = 0;

        uint32_t m_playerSteps = 0;
        uint32_t m_ravenSteps = 0;
    };

    auto Create()
    {
        return BLD<CD>("Blood Raven walk distance")
            .Add(GE::ConditionType::Precondition, "In Burial Grounds",
                 [](const D2::Data::DataAccess& aDataAccess, const D2::Data::SharedData& aS, CD& aC) {
                     return aDataAccess.GetMisc().GetZone() == Data::Zone::Act1_BurialGrounds;
                 })
            .Add(GE::ConditionType::Activator, "Meet Blood Raven",
                 [](const D2::Data::DataAccess& aDataAccess, const D2::Data::SharedData& aS, CD& aC) {
                     return MonsterNearby("BLOOD RAVEN", aDataAccess, aC.m_bloodRavenId);
                 })
            .Add(GE::ConditionType::Completer, "Kill Blood Raven",
                 [](const D2::Data::DataAccess& aDataAccess, const D2::Data::SharedData& aS, CD& aC) {
                     return aS.GetDeadMonsters().contains(aC.m_bloodRavenId);
                 })
            .Add(GE::ConditionType::Validator, "Walk less than Blood Raven",
                 [](const D2::Data::DataAccess& aDataAccess, const D2::Data::SharedData& aS, CD& aC) {
                     {
                         auto previousPos = aDataAccess.GetPlayers(1).GetLocal()->m_pos;
                         auto newPos = aDataAccess.GetPlayers().GetLocal()->m_pos;
                         auto xDist = newPos.x - previousPos.x;
                         auto yDist = newPos.y - previousPos.y;
                         uint32_t playerStepsChange = static_cast<uint32_t>(std::sqrt(xDist * xDist + yDist * yDist));
                         if (playerStepsChange < 10)  // Player walked, not teleported
                         {
                             aC.m_playerSteps += playerStepsChange;
                         }
                     }
                     auto prevRaven = aDataAccess.GetMonsters(1).GetById(aC.m_bloodRavenId);
                     auto newRaven = aDataAccess.GetMonsters().GetById(aC.m_bloodRavenId);
                     if (prevRaven && newRaven)
                     {
                         auto previousPos = prevRaven->m_pos;
                         auto newPos = newRaven->m_pos;
                         auto xDist = newPos.x - previousPos.x;
                         auto yDist = newPos.y - previousPos.y;
                         aC.m_ravenSteps += static_cast<uint32_t>(std::sqrt(xDist * xDist + yDist * yDist));
                     }
                     return aC.m_playerSteps < aC.m_ravenSteps;
                 })
            .Build();
    }
}
