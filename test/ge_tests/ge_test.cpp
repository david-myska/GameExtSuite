#include "ge_test.h"

#include "game_enhancer/memory_layout_builder.h"
#include "game_enhancer/memory_processor.h"

TEST_F(GE_Tests, Test)
{
    struct Player
    {
        uint32_t m_superData = 1;
    };
    // clang-format off
    auto d2PlayerLayout = GE::LayoutBuilder::MakeAbsolute()->SetTotalSize(0xA0)
        .AddPointerOffsets(0x10, "Quest", 3)
        .AddPointerOffsets(0x9C, "NetClient")
        .Build();
    // clang-format on
    auto targetProcess = PMA::TargetProcess::Create({});
    auto memoryProcessor = GE::MemoryProcessor::Create(targetProcess);
    memoryProcessor->SetFramesToKeep(6);
    memoryProcessor->RegisterLayout("player", d2PlayerLayout);
    memoryProcessor->AddStarterLayout("player", [](PMA::MemoryAccessPtr aMemoryAccess) {
        return 0;
    });
    memoryProcessor->SetUpdateCallback(1000 / 10, [](const GE::FrameAccessor& aFrameAccessor) {
        auto player2Frames = aFrameAccessor.Get2Frames<Player>("player");
        auto playerCustomFrames = aFrameAccessor.GetFrames<Player, 0, 1, 3, 5>("player");
    });
    memoryProcessor->Initialize();
    memoryProcessor->Start();
    memoryProcessor->Stop();
}