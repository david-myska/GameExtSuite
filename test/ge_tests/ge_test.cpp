#include "ge_test.h"

#include <utility>

#include "d2/data.h"
#include "game_enhancer/achis/achievement.h"
#include "game_enhancer/impl/layout/frame_memory_storage.h"
#include "game_enhancer/memory_layout_builder.h"
#include "game_enhancer/memory_processor.h"
#include "pma/logging/console_logger.h"

using namespace D2;

using TestAchiBuilder = GE::AchievementBuilder<std::string, GE::None, Data::SharedData, Data::DataAccess>;
using TestAchievement = decltype(std::declval<TestAchiBuilder>().Build());

struct ScatteredLayout
{
    uint16_t* m_inGame;
};

void RegisterLayouts(GE::MemoryProcessor& aMemoryProcessor)
{
    auto baseLayout =
        GE::Layout::MakeScattered()->SetTotalSize(sizeof(ScatteredLayout)).AddPointerOffsets(0x113AB4u, sizeof(uint16_t)).Build();
    auto dynPathLayout = GE::Layout::MakeConsecutive()->SetTotalSize(sizeof(Raw::DynamicPath)).Build();
    auto gameLayout = GE::Layout::MakeConsecutive()
                          ->SetTotalSize(sizeof(Raw::Game))
                          .AddPointerOffsets(0x1120u, "UnitData", 128)
                          .AddPointerOffsets(0x1120u + 1 * 128 * 4, "UnitData", 128)
                          .AddPointerOffsets(0x1120u + 3 * 128 * 4, "UnitData", 128)
                          .Build();
    auto inventoryLayout = GE::Layout::MakeConsecutive()->SetTotalSize(sizeof(Raw::Inventory)).Build();
    auto itemLayout = GE::Layout::MakeConsecutive()->SetTotalSize(sizeof(Raw::ItemData)).Build();
    auto monsterLayout =
        GE::Layout::MakeConsecutive()->SetTotalSize(sizeof(Raw::MonsterData)).AddPointerOffsets(0x2Cu, 300).Build();
    auto playerDataLayout = GE::Layout::MakeConsecutive()->SetTotalSize(sizeof(Raw::PlayerData)).Build();
    auto staticPathLayout = GE::Layout::MakeConsecutive()->SetTotalSize(sizeof(Raw::StaticPath)).Build();
    auto statlistLayout = GE::Layout::MakeConsecutive()->SetTotalSize(sizeof(Raw::StatList)).Build();
    auto statlistExLayout = GE::Layout::MakeConsecutive()
                                ->SetTotalSize(sizeof(Raw::StatListEx))
                                .AddPointerOffsets<Raw::StatListEx>(0x48u,
                                                                    [](Raw::StatListEx* aStatList) {
                                                                        return aStatList->m_baseStats.m_count * sizeof(Raw::Stat);
                                                                    })
                                .Build();
    auto unitLayout = GE::Layout::MakeConsecutive()
                          ->SetTotalSize(sizeof(Raw::UnitData<>))
                          .AddPointerOffsets<Raw::UnitData<>>(0x14u,
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
                                                                  throw std::runtime_error(
                                                                      std::format("Unknown unit type: {}", aUnit->m_unitType));
                                                              })
                          .AddPointerOffsets<Raw::UnitData<>>(0x2Cu,
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
                                                                  throw std::runtime_error(
                                                                      std::format("Unknown unit type: {}", aUnit->m_unitType));
                                                              })
                          .AddPointerOffsets(0x5Cu, "StatListEx")
                          .AddPointerOffsets(0x60u, "Inventory")
                          .AddPointerOffsets(0xE4u, "UnitData")
                          .Build();

    aMemoryProcessor.RegisterLayout("Base", std::move(baseLayout));
    aMemoryProcessor.RegisterLayout("DynamicPath", std::move(dynPathLayout));
    aMemoryProcessor.RegisterLayout("Game", std::move(gameLayout));
    aMemoryProcessor.RegisterLayout("Inventory", std::move(inventoryLayout));
    aMemoryProcessor.RegisterLayout("ItemData", std::move(itemLayout));
    aMemoryProcessor.RegisterLayout("MonsterData", std::move(monsterLayout));
    aMemoryProcessor.RegisterLayout("PlayerData", std::move(playerDataLayout));
    aMemoryProcessor.RegisterLayout("StaticPath", std::move(staticPathLayout));
    aMemoryProcessor.RegisterLayout("StatList", std::move(statlistLayout));
    aMemoryProcessor.RegisterLayout("StatListEx", std::move(statlistExLayout));
    aMemoryProcessor.RegisterLayout("UnitData", std::move(unitLayout));
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
    PMA::Setup::InjectLogger(std::make_unique<PMA::ConsoleLogger>(), PMA::LogLevel::Warning);
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

    GE::MainLayoutCallbacks baseCallbacks;
    baseCallbacks.m_baseLocator = [](PMA::MemoryAccessPtr aMemoryAccess, const std::optional<PMA::MemoryAddress>&) {
        return aMemoryAccess->GetBaseAddress("D2Client.dll");
    };
    baseCallbacks.m_enabler = [](const GE::DataAccessor& aDataAccess, GE::Enabler& aEnabler) {
        auto baseLayout = aDataAccess.Get<ScatteredLayout>("Base");
        if (*baseLayout->m_inGame)
        {
            aEnabler.Enable("Game");
        }
        else
        {
            aEnabler.Disable("Game");
        }
    };

    GE::MainLayoutCallbacks inGameCallbacks;
    inGameCallbacks.m_baseLocator = [](PMA::MemoryAccessPtr aMemoryAccess, const std::optional<PMA::MemoryAddress>&) {
        size_t address = 0;
        EXPECT_NO_THROW(aMemoryAccess->Read("D2Client.dll", 0x12236C, PMA::mem_cast(address), sizeof(size_t)));
        return address;
    };
    inGameCallbacks.m_onReady = [&](std::shared_ptr<GE::DataAccessor> aDataAccess) {
        dataAccess = std::make_shared<Data::DataAccess>(aDataAccess);
        sharedData = std::make_shared<Data::SharedData>(dataAccess);
    };
    inGameCallbacks.m_onDisabled = [&](const GE::DataAccessor&) {
        dataAccess.reset();
        sharedData.reset();
    };

    memoryProcessor->AddMainLayout("Base", baseCallbacks);
    memoryProcessor->AddMainLayout("Game", inGameCallbacks);

    std::cout << "Creating achievements" << std::endl;
    auto achis = GetAchievements();

    memoryProcessor->SetUpdateCallback([&](const GE::DataAccessor& aTmp) {
        if (!dataAccess || !sharedData)
        {
            std::cout << "Update - No Game" << std::endl;
            return;
        }
        std::cout << "Update - Game" << std::endl;
        // dataAccess->AdvanceFrame();
        // sharedData->Update();
        // for (auto& a : achis)
        // {
        //     a->Update(*dataAccess, *sharedData);
        // }
        // PrintAchievements(achis);
        // if (std::all_of(achis.begin(), achis.end(), [&memoryProcessor](const auto& a) {
        //         return a->GetStatus() == GE::Status::Completed || a->GetStatus() == GE::Status::Failed;
        //     }))
        // {
        //     std::cout << "All achievements completed" << std::endl;
        //     memoryProcessor->RequestStop();
        // }
    });

    std::cout << "Starting memoryProcessor" << std::endl;
    memoryProcessor->Start();
    std::cout << "Waiting" << std::endl;
    memoryProcessor->Wait();
}