#pragma once

#include "data.h"

#include "game_enhancer/achis/achievement.h"

namespace D2
{
    template <typename CustomData = GE::None>
    using BLD = GE::AchievementBuilder<std::string, CustomData, Data::SharedData, Data::DataAccess>;

    using D2Achi = std::unique_ptr<GE::Achievement<std::string, Data::SharedData, Data::DataAccess>>;

    std::vector<D2Achi> CreateAchievements()
    {
        std::vector<D2Achi> achis;
        achis.push_back(
            BLD<TestAchiCD>("ClearTristram")
                .Add(GE::ConditionType::Precondition, "In Tristram",
                     [](const D2::Data::DataAccess& aDataAccess, const D2::Data::SharedData& aShared, TestAchiCD& aCustom) {
                         return aDataAccess.GetMisc().GetZone() == Data::Zone::Act1_Tristram;
                     })
                .Add(GE::ConditionType::Activator, "Enter Tristram",
                     [](const D2::Data::DataAccess& aDataAccess, const D2::Data::SharedData& aShared, TestAchiCD& aCustom) {
                         return true;
                     })
                .Add(GE::ConditionType::Completer, "Kill all monsters in Tristram",
                     [](const D2::Data::DataAccess& aDataAccess, const D2::Data::SharedData& aShared, TestAchiCD& aCustom) {
                        // TODO store seen monsters in custom data, when all dead, check that we have at least 100
                         return aDataAccess.GetCurrentGameFrame() > aCustom.m_activationFrame + 25 * 10;
                     })
                .Build());
        achis.push_back(
            BLD<uint32_t>("ALPHA_2")
                .Add(GE::ConditionType::Precondition, "Pre_1",
                     [](const D2::Data::DataAccess& aDataAccess, const D2::Data::SharedData& aShared, uint32_t& aCustom) {
                         return true;
                     })
                .Add(GE::ConditionType::Activator, "Activ_1",
                     [](const D2::Data::DataAccess& aDataAccess, const D2::Data::SharedData& aShared, uint32_t& aCustom) {
                         return true;
                     })
                .OnPass(GE::ConditionType::Activator,
                        [](const D2::Data::DataAccess& aDataAccess, const D2::Data::SharedData& aShared, uint32_t& aCustom) {
                            aCustom = aDataAccess.GetCurrentGameFrame();
                        })
                .Add(GE::ConditionType::Completer, "Comp_1",
                     [](const D2::Data::DataAccess& aDataAccess, const D2::Data::SharedData& aShared, uint32_t& aCustom) {
                         return false;
                     })
                .Add(GE::ConditionType::Failer, "Fail_1",
                     [](const D2::Data::DataAccess& aDataAccess, const D2::Data::SharedData& aShared, uint32_t& aCustom) {
                         return aDataAccess.GetCurrentGameFrame() > aCustom + 25 * 5;
                     })
                .Add(GE::ConditionType::Reseter, "Reset_1",
                     [](const D2::Data::DataAccess& aDataAccess, const D2::Data::SharedData& aShared, uint32_t& aCustom) {
                         return aDataAccess.GetCurrentGameFrame() > aCustom + 25 * 10;
                     })
                .Build());
        return achis;
    }
}
