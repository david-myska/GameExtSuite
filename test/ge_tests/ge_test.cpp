#include "ge_test.h"

#include <utility>

#include "game_enhancer/achis/achievement.h"
#include "game_enhancer/achis/achievement_manager.h"
#include "game_enhancer/backup/backup_engine.h"
#include "game_enhancer/impl/layout/frame_memory_storage.h"
#include "game_enhancer/memory_layout_builder.h"
#include "game_enhancer/memory_processor.h"

struct TestPD : public GE::BaseProgressData
{
    // Activator
    GE::ProgressTrackerInt<> m_intTracker{this, "Int Tracker", 10};
    // Completer
    GE::ProgressTrackerBool m_boolTracker{this, "Bool Tracker", true};
    // Failer
    GE::ProgressTrackerFloat<> m_floatTracker{this, "Float Tracker", 10.0f, 5.0f};
    GE::ProgressTrackerTimer m_countdownTracker{this, 10 * 60, "Countdown Tracker"};
};

using TestAchiBld = GE::AchievementBuilder<std::string, TestPD>;
using TestAchi = decltype(std::declval<TestAchiBld>().Build());
using TestAchiType = std::remove_reference_t<decltype(*std::declval<TestAchi>())>;

TEST_F(GE_Tests, Test)
{
    auto backupEngine = GE::BackupEngine::Create("test_target_path", "test_backup_path", GetConsoleLogger());

    auto achiManager = GE::AchievementManager<TestAchiType>(
        []() {
            std::map<uint32_t, std::unique_ptr<TestAchiType>> achis;
            return achis;
        },
        "test_achievements_storage_path", GetConsoleLogger());

    auto achi1 = TestAchiBld("Test Achievement",
                             [](TestPD& aData,
                                std::unordered_map<GE::ConditionType, std::unordered_set<GE::ProgressTracker*>>& aTrackers) {
                                 aTrackers[GE::ConditionType::Activator].insert(&aData.m_intTracker);
                                 aTrackers[GE::ConditionType::Completer].insert(&aData.m_boolTracker);
                                 aTrackers[GE::ConditionType::Failer].insert(&aData.m_countdownTracker);
                                 aTrackers[GE::ConditionType::Failer].insert(&aData.m_floatTracker);
                             })
                     .Update(GE::Status::Inactive,
                             [](const GE::DataAccessor& aDataAccess, const GE::None&, TestPD& aPD) {
                                 // Modify ProgressTracker assigned to Activator condition
                                 aPD.m_intTracker += 1;
                             })
                     .Update(GE::Status::Active,
                             [](const GE::DataAccessor& aDataAccess, const GE::None&, TestPD& aPD) {
                                 // Modify ProgressTracker assigned to Completer/Failer/Validator conditions
                             })
                     .Build(GetConsoleLogger());
}