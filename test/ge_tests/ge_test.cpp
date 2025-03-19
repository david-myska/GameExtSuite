#include "ge_test.h"

#include "game_enhancer/memory_layout.h"

TEST_F(GE_Tests, Test)
{
    // clang-format off
    auto d2PlayerLayout = GE::LayoutBuilder::MakeAbsolute()->SetTotalSize(0xA0)
        .AddPointerOffsets(0x10, "Quest", 3)
        .AddPointerOffsets(0x9C, "NetClient")
        .Build();
    // clang-format on
}