#pragma once

#include "../base.h"

namespace D2::Achi::AndarielNoHit
{
    struct TestPersistanceData : GE::PersistentData
    {
        uint32_t killed = 0;

        void Serialize(std::ostream& aOut) const override { aOut << killed; }

        void Deserialize(std::istream& aIn) override { aIn >> killed; }
    };

    auto Create()
    {
        return BLD<TestPersistanceData>("TestPersistance")
            .Add(GE::ConditionType::Activator, "",
                 [](const D2::Data::DataAccess& aDataAccess, const D2::Data::SharedData& aS, TestPersistanceData& aC) {
                     return true;
                 })
            .Add(GE::ConditionType::Completer, "Kill 200 monsters",
                 [](const D2::Data::DataAccess& aDataAccess, const D2::Data::SharedData& aS, TestPersistanceData& aC) {
                     aC.killed += aS.GetDeadMonsters().size();
                     return aC.killed >= 200;
                 })
            .Build();
    }
}
