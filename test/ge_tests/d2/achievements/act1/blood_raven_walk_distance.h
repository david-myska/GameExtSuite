#pragma once

#include "../base.h"

namespace D2::Achi::BloodRavenWalkDistance
{
    struct CD
    {
        Data::GUID m_bloodRavenId = 0;
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
            .Add(GE::ConditionType::Completer, "Walk less than Blood Raven",
                 [](const D2::Data::DataAccess& aDataAccess, const D2::Data::SharedData& aS, CD& aC) {
                     return false;
                 })
            .Add(GE::ConditionType::Completer, "Kill Blood Raven",
                 [](const D2::Data::DataAccess& aDataAccess, const D2::Data::SharedData& aS, CD& aC) {
                     return aS.GetDeadMonsters().contains(aC.m_bloodRavenId);
                 })
            // Maybe something like ConditionType::Decider that gets run after completers to
            // decide if completed successfully (here it would be the Walk less than Blood Raven)
            .Add(GE::ConditionType::Failer, "This is weird",
                 [](const D2::Data::DataAccess& aDataAccess, const D2::Data::SharedData& aS, CD& aC) {
                     // this works only because Failers are evaluated after Completers
                     return aS.GetDeadMonsters().contains(aC.m_bloodRavenId);
                 })
            .Build();
    }
}
