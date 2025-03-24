#pragma once

#include <vector>

#include "game_enhancer/memory_layout_builder.h"

namespace GE
{
    class AbsoluteImpl : public LayoutBuilder::Absolute
    {
        size_t m_totalSize = 0;
        std::vector<LayoutBuilder::Ptr> m_pointerOffsets;

    public:
        AbsoluteImpl& SetTotalSize(size_t aBytes) override;
        AbsoluteImpl& AddPointerOffsets(size_t aOffset, const std::string& aPointeeType, size_t aCount = 1) override;

        LayoutBuilder::Absolute::Layout Build() override;
    };
}  // namespace GE
