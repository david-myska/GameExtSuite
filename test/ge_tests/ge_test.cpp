#include "ge_test.h"

#include <utility>

#include "d2/data.h"
#include "game_enhancer/achis/achievement.h"
#include "game_enhancer/memory_layout_builder.h"
#include "game_enhancer/memory_processor.h"
#include "pma/logging/console_logger.h"

using namespace D2;

using TestAchiBuilder = GE::AchievementBuilder<std::string, GE::None, Data::SharedData, Data::DataAccess>;
using TestAchievement = decltype(std::declval<TestAchiBuilder>().Build());

void RegisterLayouts(GE::MemoryProcessor& aMemoryProcessor)
{
    auto dynPathLayout = GE::LayoutBuilder::MakeAbsolute()->SetTotalSize(sizeof(Raw::DynamicPath)).Build();
    auto gameLayout = GE::LayoutBuilder::MakeAbsolute()
                          ->SetTotalSize(sizeof(Raw::Game))
                          .AddPointerOffsets(0x1120, "UnitData", 128)
                          .AddPointerOffsets(0x1120 + 1 * 128 * 4, "UnitData", 128)
                          .AddPointerOffsets(0x1120 + 3 * 128 * 4, "UnitData", 128)
                          .Build();
    auto inventoryLayout = GE::LayoutBuilder::MakeAbsolute()->SetTotalSize(sizeof(Raw::Inventory)).Build();
    auto itemLayout = GE::LayoutBuilder::MakeAbsolute()->SetTotalSize(sizeof(Raw::ItemData)).Build();
    auto monsterLayout =
        GE::LayoutBuilder::MakeAbsolute()->SetTotalSize(sizeof(Raw::MonsterData)).AddPointerOffsets(0x2C, 300).Build();
    auto playerDataLayout = GE::LayoutBuilder::MakeAbsolute()->SetTotalSize(sizeof(Raw::PlayerData)).Build();
    auto staticPathLayout = GE::LayoutBuilder::MakeAbsolute()->SetTotalSize(sizeof(Raw::StaticPath)).Build();
    auto statlistLayout = GE::LayoutBuilder::MakeAbsolute()->SetTotalSize(sizeof(Raw::StatList)).Build();
    auto statlistExLayout = GE::LayoutBuilder::MakeAbsolute()
                                ->SetTotalSize(sizeof(Raw::StatListEx))
                                .AddPointerOffsets<Raw::StatListEx>(0x24,
                                                                    [](Raw::StatListEx* aStatList) {
                                                                        return aStatList->m_baseStats.m_count * sizeof(Raw::Stat);
                                                                    })
                                .Build();
    auto unitLayout = GE::LayoutBuilder::MakeAbsolute()
                          ->SetTotalSize(sizeof(Raw::UnitData<>))
                          .AddPointerOffsets<Raw::UnitData<>>(0x14,
                                                              [](Raw::UnitData<>* aUnit) {
                                                                  if (aUnit->m_unitType == 0)
                                                                  {
                                                                      return "PlayerData";
                                                                  }
                                                                  else if (aUnit->m_unitType == 1)
                                                                  {
                                                                      return "MonsterData";
                                                                  }
                                                                  else if (aUnit->m_unitType == 4)
                                                                  {
                                                                      return "ItemData";
                                                                  }
                                                                  throw std::runtime_error("Unknown unit type");
                                                              })
                          .AddPointerOffsets<Raw::UnitData<>>(0x2C,
                                                              [](Raw::UnitData<>* aUnit) {
                                                                  if (aUnit->m_unitType == 0)
                                                                  {
                                                                      return "DynamicPath";
                                                                  }
                                                                  else if (aUnit->m_unitType == 1)
                                                                  {
                                                                      return "DynamicPath";
                                                                  }
                                                                  else if (aUnit->m_unitType == 4)
                                                                  {
                                                                      return "StaticPath";
                                                                  }
                                                                  throw std::runtime_error("Unknown unit type");
                                                              })
                          .AddPointerOffsets(0x5C, "StatListEx")
                          .AddPointerOffsets(0x60, "Inventory")
                          .AddPointerOffsets(0xE4, "UnitData")
                          .Build();

    aMemoryProcessor.RegisterLayout("DynamicPath", dynPathLayout);
    aMemoryProcessor.RegisterLayout("Game", gameLayout);
    aMemoryProcessor.RegisterLayout("Inventory", inventoryLayout);
    aMemoryProcessor.RegisterLayout("ItemData", itemLayout);
    aMemoryProcessor.RegisterLayout("MonsterData", monsterLayout);
    aMemoryProcessor.RegisterLayout("PlayerData", playerDataLayout);
    aMemoryProcessor.RegisterLayout("StaticPath", staticPathLayout);
    aMemoryProcessor.RegisterLayout("StatList", statlistLayout);
    aMemoryProcessor.RegisterLayout("StatListEx", statlistExLayout);
    aMemoryProcessor.RegisterLayout("UnitData", unitLayout);
}

std::string ToString(const GE::ConditionType aType)
{
    switch (aType)
    {
    case GE::ConditionType::Precondition:
        return "Precondition";
    case GE::ConditionType::Activator:
        return "Activator";
    case GE::ConditionType::Invariant:
        return "Invariant";
    case GE::ConditionType::Completer:
        return "Completer";
    case GE::ConditionType::Failer:
        return "Failer";
    case GE::ConditionType::Reseter:
        return "Reseter";
    default:
        return "Unknown";
    }
    return "Unknown";
}

void PrintAchievement(const TestAchievement& aAchievement)
{
    std::cout << "--- " << aAchievement->GetMetadata() << " ---" << std::endl;
    for (uint32_t i = 0; i < static_cast<uint32_t>(GE::ConditionType::All); ++i)
    {
        auto cType = static_cast<GE::ConditionType>(i);
        std::cout << ToString(cType) << ": ";
        const auto& names = aAchievement->GetConditionNames(cType);
        const auto& results = aAchievement->GetConditionResults(cType);
        for (uint32_t j = 0; j < names.size(); ++j)
        {
            std::cout << names[j] << "[" << results[j] << "], ";
        }
        std::cout << std::endl;
    }
}

void PrintAchievements(const std::vector<TestAchievement>& aAchievements)
{
    for (const auto& a : aAchievements)
    {
        PrintAchievement(a);
        std::cout << "\n-----\n" << std::endl;
    }
}

std::vector<TestAchievement> GetAchievements()
{
    std::vector<TestAchievement> achis;
    achis.push_back(TestAchiBuilder("Name").Build());
    return achis;
}

TEST_F(GE_Tests, Test)
{
    // PMA::Setup::InjectLogger(std::make_unique<PMA::ConsoleLogger>(), PMA::LogLevel::Verbose);
    auto config = PMA::TargetProcess::Config{
        .windowInfo = PMA::TargetProcess::Config::WindowInfo{.windowTitle = "Diablo II"},
        .modules = {"D2Client.dll", "D2Common.dll", "D2Win.dll", "D2Lang.dll", "D2Sigma.dll", "D2Game.dll"},
        .pathPrefix = "C:\\games\\median-xl\\"
    };
    auto targetProcess = PMA::TargetProcess::Create(config);
    auto memoryProcessor = GE::MemoryProcessor::Create(std::move(targetProcess));

    RegisterLayouts(*memoryProcessor);

    std::shared_ptr<Data::DataAccess> dataAccess;
    std::shared_ptr<Data::SharedData> sharedData;

    GE::MainLayoutCallbacks mainLayoutCallbacks;
    mainLayoutCallbacks.m_baseLocator = [](PMA::MemoryAccessPtr aMemoryAccess, const std::optional<PMA::MemoryAddress>&) {
        size_t address = 0;
        EXPECT_NO_THROW(aMemoryAccess->Read("D2Client.dll", 0x12236C, reinterpret_cast<uint8_t*>(&address), sizeof(size_t)));
        return address;
    };
    mainLayoutCallbacks.m_onReady = [&](std::shared_ptr<GE::DataAccessor> aDataAccess) {
        dataAccess = std::make_shared<Data::DataAccess>(aDataAccess);
        sharedData = std::make_shared<Data::SharedData>(dataAccess);
    };
    memoryProcessor->AddMainLayout("Game", mainLayoutCallbacks);

    std::cout << "Creating achievements" << std::endl;
    auto achis = GetAchievements();

    memoryProcessor->SetUpdateCallback([&](const GE::DataAccessor&) {
        dataAccess->AdvanceFrame();
        sharedData->Update();
        for (auto& a : achis)
        {
            a->Update(*dataAccess, *sharedData);
        }
        PrintAchievements(achis);
        if (std::all_of(achis.begin(), achis.end(), [&memoryProcessor](const auto& a) {
                return a->GetStatus() == GE::Status::Completed || a->GetStatus() == GE::Status::Failed;
            }))
        {
            std::cout << "All achievements completed" << std::endl;
            memoryProcessor->RequestStop();
        }
    });

    std::cout << "Starting memoryProcessor" << std::endl;
    memoryProcessor->Start();
    memoryProcessor->Wait();
}