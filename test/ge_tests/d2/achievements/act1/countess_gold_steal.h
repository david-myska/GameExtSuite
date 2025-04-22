#pragma once

#include "../base.h"

namespace D2::Achi::CountessGoldSteal
{
    struct CD
    {
    };

    auto Create()
    {
        return BLD<CD>("ClearTristram")
            .Add(GE::ConditionType::Precondition, "In Tristram",
                 [](const D2::Data::DataAccess& aDataAccess, const D2::Data::SharedData& aS, CD& aC) {
                     return aDataAccess.GetMisc().GetZone() == Data::Zone::Act1_Tristram;
                 })
            .Add(GE::ConditionType::Activator, "Enter Tristram",
                 [](const D2::Data::DataAccess& aDataAccess, const D2::Data::SharedData& aS, CD& aC) {
                     return true;
                 })
            .Add(GE::ConditionType::Completer, "Kill all monsters in Tristram",
                 [](const D2::Data::DataAccess& aDataAccess, const D2::Data::SharedData& aS, CD& aC) {
                     return false;
                 })
            .Build();
    }
}
