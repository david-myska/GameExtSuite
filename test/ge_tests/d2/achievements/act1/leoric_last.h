#pragma once

#include "../base.h"

namespace D2::Achi::LeoricLast
{
    struct CD
    {
        Data::GUID m_leoricId = 0;
        uint32_t m_killedNearLeoric = 0;
    };

    auto Create()
    {
        return BLD<CD>("He likes to watch")
            .Add(GE::ConditionType::Precondition, "In Cathedral",
                 [](const D2::Data::DataAccess& aDataAccess, const D2::Data::SharedData& aS, CD& aC) {
                     return aDataAccess.GetMisc().GetZone() == Data::Zone::Act1_InnerCloister ||
                            aDataAccess.GetMisc().GetZone() == Data::Zone::Act1_Cathedral;
                 })
            .Add(GE::ConditionType::Activator, "Meet Leoric the Skeleton King",
                 [](const D2::Data::DataAccess& aDataAccess, const D2::Data::SharedData& aS, CD& aC) {
                     return MonsterNearby("LEORIC THE SKELETON KING", aDataAccess, aC.m_leoricId);
                 })
            .Add(GE::ConditionType::Completer, "Kill Leoric the Skeleton King",
                 [](const D2::Data::DataAccess& aDataAccess, const D2::Data::SharedData& aS, CD& aC) {
                     return aS.GetDeadMonsters().contains(aC.m_leoricId);
                 })
            .Add(GE::ConditionType::Validator, "Servants killed in front of Leoric's eyes",
                 [](const D2::Data::DataAccess& aDataAccess, const D2::Data::SharedData& aS, CD& aC) {
                     if (aDataAccess.GetMonsters().GetAlive().contains(aC.m_leoricId))
                     {
                         aC.m_killedNearLeoric += aS.GetDeadMonsters().size();
                     }
                     return aC.m_killedNearLeoric >= 50;
                 })
            .Build();
    }
}
