#pragma once

#include "../base.h"

namespace D2::Achi::Act1Speedrun
{
    struct CD
    {
        Data::GUID m_bloodRavenId = 0;
        Data::GUID m_griswoldId = 0;
        Data::GUID m_countessId = 0;
        Data::GUID m_smithId = 0;
        Data::GUID m_leoricId = 0;
        Data::GUID m_andarielId = 0;

        int timer = 0;
    };

    auto Create()
    {
        return BLD<CD>("Speedrun Act 1")
            .Add(GE::ConditionType::Activator, "Leave town on level 1",
                 [](const D2::Data::DataAccess& aDataAccess, const D2::Data::SharedData& aS, CD& aC) {
                     return aDataAccess.GetMisc().GetZone() == Data::Zone::Act1_BloodMoor &&
                            aDataAccess.GetMisc(1).GetZone() == Data::Zone::Act1_RogueEncampment &&
                            *aDataAccess.GetPlayers().GetLocal()->m_stats.GetValue(Data::StatType::CharLevel);
                 })
            .OnPass(GE::ConditionType::Activator,
                    [](const D2::Data::DataAccess& aDataAccess, const D2::Data::SharedData& aS, CD& aC) {
                        aC.timer = 0;  // TODO
                    })
            .Add(
                GE::ConditionType::Completer, "Kill Blood Raven",
                [](const D2::Data::DataAccess& aDataAccess, const D2::Data::SharedData& aS, CD& aC) {
                    if (aC.m_bloodRavenId == 0)
                    {
                        if (!MonsterNearby("BLOOD RAVEN", aDataAccess, aC.m_bloodRavenId))
                        {
                            return false;
                        }
                    }
                    return aS.GetDeadMonsters().contains(aC.m_bloodRavenId);
                },
                true)
            .Add(
                GE::ConditionType::Completer, "Kill Griswold",
                [](const D2::Data::DataAccess& aDataAccess, const D2::Data::SharedData& aS, CD& aC) {
                    if (aC.m_griswoldId == 0)
                    {
                        if (!MonsterNearby("GRISWOLD", aDataAccess, aC.m_griswoldId))
                        {
                            return false;
                        }
                    }
                    return aS.GetDeadMonsters().contains(aC.m_griswoldId);
                },
                true)
            .Add(
                GE::ConditionType::Completer, "Kill The Countess",
                [](const D2::Data::DataAccess& aDataAccess, const D2::Data::SharedData& aS, CD& aC) {
                    if (aC.m_countessId == 0)
                    {
                        if (!MonsterNearby("THE COUNTESS", aDataAccess, aC.m_countessId))
                        {
                            return false;
                        }
                    }
                    return aS.GetDeadMonsters().contains(aC.m_countessId);
                },
                true)
            .Add(
                GE::ConditionType::Completer, "Kill The Smith",
                [](const D2::Data::DataAccess& aDataAccess, const D2::Data::SharedData& aS, CD& aC) {
                    if (aC.m_smithId == 0)
                    {
                        if (!MonsterNearby("THE SMITH", aDataAccess, aC.m_smithId))
                        {
                            return false;
                        }
                    }
                    return aS.GetDeadMonsters().contains(aC.m_smithId);
                },
                true)
            .Add(
                GE::ConditionType::Completer, "Kill Leoric the Skeleton King",
                [](const D2::Data::DataAccess& aDataAccess, const D2::Data::SharedData& aS, CD& aC) {
                    if (aC.m_leoricId == 0)
                    {
                        if (!MonsterNearby("LEORIC THE SKELETON KING", aDataAccess, aC.m_leoricId))
                        {
                            return false;
                        }
                    }
                    return aS.GetDeadMonsters().contains(aC.m_leoricId);
                },
                true)
            .Add(
                GE::ConditionType::Completer, "Kill Andariel",
                [](const D2::Data::DataAccess& aDataAccess, const D2::Data::SharedData& aS, CD& aC) {
                    if (aC.m_andarielId == 0)
                    {
                        if (!MonsterNearby("ANDARIEL", aDataAccess, aC.m_andarielId))
                        {
                            return false;
                        }
                    }
                    return aS.GetDeadMonsters().contains(aC.m_andarielId);
                },
                true)
            .Add(GE::ConditionType::Failer, "Timed",
                 [](const D2::Data::DataAccess& aDataAccess, const D2::Data::SharedData& aS, CD& aC) {
                     return false;  // TODO
                 })
            .Build();
    }
}
