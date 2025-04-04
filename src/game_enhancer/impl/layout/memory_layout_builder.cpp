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

    AbsoluteImpl& AbsoluteImpl::AddPointerOffsets(size_t aOffset, const std::string& aStaticType, size_t aCount)
    {
        m_pointerOffsets.push_back({aOffset, aCount, [aStaticType](void*) {
                                        return aStaticType;
                                    }});
        return *this;
    }

    AbsoluteImpl& AbsoluteImpl::AddPointerOffsets(size_t aOffset, const std::function<std::string(void*)>& aDynamicType, size_t aCount)
    {
        m_pointerOffsets.push_back({aOffset, aCount, aDynamicType});
        return *this;
    }

    AbsoluteImpl& AbsoluteImpl::AddPointerOffsets(size_t aOffset, size_t aStaticSize, size_t aCount)
    {
        m_pointerOffsets.push_back({aOffset, aCount, [aStaticSize](void*) {
                                        return aStaticSize;
                                    }});
        return *this;
    }

    AbsoluteImpl& AbsoluteImpl::AddPointerOffsets(size_t aOffset, const std::function<size_t(void*)>& aDynamicSize, size_t aCount)
    {
        m_pointerOffsets.push_back({aOffset, aCount, aDynamicSize});
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
