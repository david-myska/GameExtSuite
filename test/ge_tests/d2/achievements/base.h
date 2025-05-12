#pragma once

#include <memory>

#include "../data.h"
#include "game_enhancer/achis/achievement.h"

namespace D2
{

    template <typename CustomData = GE::None>
    using BLD = GE::AchievementBuilder<std::string, CustomData, Data::SharedData, Data::DataAccess>;

    using D2Achi = std::unique_ptr<GE::Achievement<std::string, Data::SharedData, Data::DataAccess>>;

    bool MonsterNearby(const std::string& aName, const D2::Data::DataAccess& aDataAccess, Data::GUID& aGuid) {
        auto mons = aDataAccess.GetMonsters().GetByName(aName);
        if (mons.empty())
        {
            return false;
        }
        aGuid = mons.begin()->first;
        return true;
    }
}