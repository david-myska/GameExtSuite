#pragma once

#include "game_enhancer/impl/layout/memory_layout_builder.h"

#include <memory>
#include <string>

namespace GE
{
    AbsoluteImpl& AbsoluteImpl::SetTotalSize(size_t aBytes)
    {
        m_totalSize = aBytes;
        return *this;
    }

    AbsoluteImpl& AbsoluteImpl::AddPointerOffsets(size_t aOffset, const std::string& aPointeeType, size_t aCount)
    {
        m_pointerOffsets.push_back({aOffset, aCount, aPointeeType});
        return *this;
    }

    LayoutBuilder::Absolute::Layout AbsoluteImpl::Build()
    {
        return {m_totalSize, std::move(m_pointerOffsets)};
    }

    std::unique_ptr<LayoutBuilder::Absolute> LayoutBuilder::MakeAbsolute()
    {
        return std::make_unique<AbsoluteImpl>();
    }

}  // namespace GE
