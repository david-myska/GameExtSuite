#pragma once

#include <memory>

#include "../data.h"
#include "game_enhancer/achis/achievement.h"

namespace D2
{

    template <typename CustomData = GE::None>
    using BLD = GE::AchievementBuilder<std::string, CustomData, Data::SharedData, Data::DataAccess>;

    using D2Achi = std::unique_ptr<GE::Achievement<std::string, Data::SharedData, Data::DataAccess>>;

}