#pragma once

#include "../base.h"

namespace D2::Achi::AndarielNoHit
{
    struct CD
    {
        Data::GUID m_andarielId = 0;
    };

    auto Create()
    {
        return BLD<CD>("Andariel no hit")
            .Add(GE::ConditionType::Precondition, "In Catacombs Level 4",
                 [](const D2::Data::DataAccess& aDataAccess, const D2::Data::SharedData& aS, CD& aC) {
                     return aDataAccess.GetMisc().GetZone() == Data::Zone::Act1_CatacombsLevel4;
                 })
            .Add(GE::ConditionType::Activator, "Meet Andariel",
                 [](const D2::Data::DataAccess& aDataAccess, const D2::Data::SharedData& aS, CD& aC) {
                     return MonsterNearby("ANDARIEL", aDataAccess, aC.m_andarielId);
                 })
            .Add(GE::ConditionType::Completer, "Kill Andariel",
                 [](const D2::Data::DataAccess& aDataAccess, const D2::Data::SharedData& aS, CD& aC) {
                     return aS.GetDeadMonsters().contains(aC.m_andarielId);
                 })
            .Add(GE::ConditionType::Failer, "Don't get hit",
                 [](const D2::Data::DataAccess& aDataAccess, const D2::Data::SharedData& aS, CD& aC) {
                     auto currentLife = aDataAccess.GetPlayers().GetLocal()->m_stats.GetValue(Data::StatType::Life);
                     auto previousLife = aDataAccess.GetPlayers(1).GetLocal()->m_stats.GetValue(Data::StatType::Life);
                     return currentLife < previousLife;
                 })
            .Build();
    }
}
