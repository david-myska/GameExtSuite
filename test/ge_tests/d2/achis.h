#pragma once

#include "data.h"

#include "achievements/act1/andariel_no_hit.h"
#include "achievements/act1/andariel_no_leave.h"
#include "achievements/act1/blood_raven_walk_distance.h"
#include "achievements/act1/countess_gold_steal.h"
#include "achievements/act1/leoric_last.h"
#include "achievements/act1/smith_high_health.h"
#include "achievements/act1/speedrun.h"
#include "achievements/act1/tristram_clear.h"
#include "achievements/base.h"

namespace D2
{
    struct ClearTristramCD
    {
    };

    std::vector<D2Achi> CreateAchievements()
    {
        std::vector<D2Achi> result;

        // Act1
        result.push_back(D2::Achi::TristramClear::Create());

        return result;
    }
}
