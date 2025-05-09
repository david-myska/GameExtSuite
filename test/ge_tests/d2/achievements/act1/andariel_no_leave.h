#pragma once

#include "../base.h"

namespace D2::Achi::AndarielNoLeave
{
    struct CD
    {
        Data::GUID m_andy = 0;
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
                     auto andys = aDataAccess.GetMonsters().GetByName("ANDARIEL");
                     if (andys.empty())
                     {
                         return false;
                     }
                     aC.m_andy = andys.begin()->first;
                     return true;
                 })
            .Add(GE::ConditionType::Activator, "Enter Andariel's room",
                 [](const D2::Data::DataAccess& aDataAccess, const D2::Data::SharedData& aS, CD& aC) {
                     auto currentPos = aDataAccess.GetPlayers().GetLocal()->m_pos;
                     // return IsIn(currentPos, Rectangle(...));
                     return false;
                    })
            .Add(GE::ConditionType::Completer, "Kill Andariel",
                 [](const D2::Data::DataAccess& aDataAccess, const D2::Data::SharedData& aS, CD& aC) {
                     return aS.GetDeadMonsters().contains(aC.m_andy);
                 })
            .Add(GE::ConditionType::Failer, "Stay in the room",
                 [](const D2::Data::DataAccess& aDataAccess, const D2::Data::SharedData& aS, CD& aC) {
                     auto currentPos = aDataAccess.GetPlayers().GetLocal()->m_pos;
                     // return IsIn(currentPos, Rectangle(...));
                     return false;
                 })
            .Build();
    }
}
