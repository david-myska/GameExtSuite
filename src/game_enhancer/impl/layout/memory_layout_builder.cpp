#pragma once

#include "game_enhancer/impl/layout/memory_layout_builder.h"

#include <memory>
#include <string>

namespace GE
{
    namespace
    {
        PMA::MultiLevelPointer GetMlp(std::variant<size_t, PMA::MultiLevelPointer>& aOffsetOrMlp)
        {
            if (std::holds_alternative<size_t>(aOffsetOrMlp))
            {
                return {std::get<size_t>(aOffsetOrMlp)};
            }
            else
            {
                return std::get<PMA::MultiLevelPointer>(aOffsetOrMlp);
            }
        }
    }

    LayoutImpl::LayoutImpl(bool aConsecutive, size_t aTotalSize, std::vector<Layout::Ptr> aPointerOffsets)
        : m_consecutive(aConsecutive)
        , m_totalSize(aTotalSize)
        , m_pointerOffsets(std::move(aPointerOffsets))
    {
    }

    bool LayoutImpl::IsConsecutive() const
    {
        return m_consecutive;
    }

    size_t LayoutImpl::GetTotalSize() const
    {
        return m_totalSize;
    }

    const std::vector<Layout::Ptr>& LayoutImpl::GetPointerOffsets() const
    {
        return m_pointerOffsets;
    }

    BuilderImpl::BuilderImpl(bool aConsecutive)
        : m_consecutive(aConsecutive)
    {
    }

    Layout::Builder& BuilderImpl::SetTotalSize(size_t aBytes)
    {
        m_totalSize = aBytes;
        return *this;
    }

    Layout::Builder& BuilderImpl::AddPointerOffsets(std::variant<size_t, PMA::MultiLevelPointer> aOffsetOrMlp,
                                                    const std::string& aStaticType, size_t aCount)
    {
        m_pointerOffsets.push_back({GetMlp(aOffsetOrMlp), aCount, [aStaticType](void*) {
                                        return aStaticType;
                                    }});
        return *this;
    }

    Layout::Builder& BuilderImpl::AddPointerOffsets(std::variant<size_t, PMA::MultiLevelPointer> aOffsetOrMlp,
                                                    const std::function<std::string(void*)>& aDynamicType, size_t aCount)
    {
        m_pointerOffsets.push_back({GetMlp(aOffsetOrMlp), aCount, aDynamicType});
        return *this;
    }

    Layout::Builder& BuilderImpl::AddPointerOffsets(std::variant<size_t, PMA::MultiLevelPointer> aOffsetOrMlp, size_t aStaticSize,
                                                    size_t aCount)
    {
        m_pointerOffsets.push_back({GetMlp(aOffsetOrMlp), aCount, [aStaticSize](void*) {
                                        return aStaticSize;
                                    }});
        return *this;
    }

    Layout::Builder& BuilderImpl::AddPointerOffsets(std::variant<size_t, PMA::MultiLevelPointer> aOffsetOrMlp,
                                                    const std::function<size_t(void*)>& aDynamicSize, size_t aCount)
    {
        m_pointerOffsets.push_back({GetMlp(aOffsetOrMlp), aCount, aDynamicSize});
        return *this;
    }

    std::unique_ptr<Layout> BuilderImpl::Build()
    {
        return std::make_unique<LayoutImpl>(m_consecutive, m_totalSize, std::move(m_pointerOffsets));
    }

    std::unique_ptr<Layout::Builder> Layout::MakeConsecutive()
    {
        return std::make_unique<BuilderImpl>(true);
    }

    std::unique_ptr<Layout::Builder> Layout::MakeScattered()
    {
        return std::make_unique<BuilderImpl>(false);
    }

}  // namespace GE
