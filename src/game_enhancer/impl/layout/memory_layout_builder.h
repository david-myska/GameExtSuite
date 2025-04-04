#pragma once

#include <functional>
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
        AbsoluteImpl& AddPointerOffsets(size_t aOffset, const std::string& aStaticType, size_t aCount = 1) override;
        AbsoluteImpl& AddPointerOffsets(size_t aOffset, const std::function<std::string(void*)>& aDynamicType,
                                        size_t aCount = 1) override;
        AbsoluteImpl& AddPointerOffsets(size_t aOffset, size_t aStaticSize, size_t aCount = 1) override;
        AbsoluteImpl& AddPointerOffsets(size_t aOffset, const std::function<size_t(void*)>& aDynamicSize,
                                        size_t aCount = 1) override;

        LayoutBuilder::Absolute::Layout Build() override;
    };
}  // namespace GE
