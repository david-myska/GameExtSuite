#pragma once

#include "data.h"

#include "achievements/act1/andariel_no_hit.h"
#include "achievements/act1/test_persistance.h"
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
    std::map<uint32_t, D2Achi> CreateAchievements()
    {
        std::map<uint32_t, D2Achi> result;

        result.emplace(100, D2::Achi::TestPersistance::Create());// just testing
        // Act1
        result.emplace(1, D2::Achi::TristramClear::Create());
        result.emplace(2, D2::Achi::AndarielNoHit::Create());
        result.emplace(3, D2::Achi::AndarielNoLeave::Create());
        result.emplace(4, D2::Achi::BloodRavenWalkDistance::Create());

        return result;
    }
}
