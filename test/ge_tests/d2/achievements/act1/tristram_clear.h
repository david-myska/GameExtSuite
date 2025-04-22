#pragma once

#include "../base.h"

namespace D2::Achi::TristramClear
{
    struct CD
    {
        std::map<Data::GUID, const Data::Monster*> m_monsters;
        uint32_t m_deadMonsters = 0;
        Data::Position m_initialPos = {0, 0};
        bool m_riverVisited = false;
        bool m_wirtVisited = false;
        bool m_tombVisited = false;
    };

    auto Create()
    {
        return BLD<CD>("ClearTristram")
            .Add(GE::ConditionType::Precondition, "In Tristram",
                 [](const Data::DataAccess& aDataAccess, const Data::SharedData& aShared, CD& aCustom) {
                     return aDataAccess.GetMisc().GetZone() == Data::Zone::Act1_Tristram;
                 })
            .Add(GE::ConditionType::Activator, "Enter Tristram",
                 [](const Data::DataAccess& aDataAccess, const Data::SharedData& aShared, CD& aCustom) {
                     return true;
                 })
            .OnPass(GE::ConditionType::Activator,
                    [](const Data::DataAccess& aDataAccess, const Data::SharedData&, CD& aCustom) {
                        aCustom.m_initialPos = aDataAccess.GetPlayers().GetLocal()->m_pos;
                    })
            .Add(GE::ConditionType::Completer, "Kill all monsters in Tristram",
                 [](const Data::DataAccess& aDataAccess, const Data::SharedData& aShared, CD& aCustom) {
                     aCustom.m_monsters = aCustom.m_monsters + aShared.GetNewMonsters();
                     aCustom.m_deadMonsters += aShared.GetDeadMonsters().size();
                     auto pos = aDataAccess.GetPlayers().GetLocal()->m_pos;
                     aCustom.m_riverVisited = aCustom.m_riverVisited || pos.x < aCustom.m_initialPos.x - 80;
                     aCustom.m_tombVisited = aCustom.m_tombVisited || pos.y > aCustom.m_initialPos.y + 40;
                     aCustom.m_wirtVisited = aCustom.m_wirtVisited ||
                                             (pos.x < aCustom.m_initialPos.x - 80 && pos.y > aCustom.m_initialPos.y + 40);
                     return aCustom.m_riverVisited && aCustom.m_tombVisited && aCustom.m_wirtVisited &&
                            aCustom.m_monsters.size() == aCustom.m_deadMonsters + 1;  // 1 is always missing
                 })
            .Build();
    }
}
