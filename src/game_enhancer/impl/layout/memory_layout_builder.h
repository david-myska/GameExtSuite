#pragma once

#include <functional>
#include <vector>

#include "game_enhancer/memory_layout_builder.h"
#include "pma/memory_core.h"

namespace GE
{
    class LayoutImpl : public Layout
    {
        const bool m_consecutive;
        size_t m_totalSize = 0;
        std::vector<Layout::Ptr> m_pointerOffsets;

    public:
        LayoutImpl(bool aConsecutive, size_t aTotalSize, std::vector<Layout::Ptr> aPointerOffsets);

        bool IsConsecutive() const override;

        size_t GetTotalSize() const override;

        const std::vector<Ptr>& GetPointerOffsets() const override;
    };

    class BuilderImpl : public Layout::Builder
    {
        const bool m_consecutive;
        size_t m_totalSize = 0;
        std::vector<Layout::Ptr> m_pointerOffsets;

    public:
        BuilderImpl(bool aConsecutive = true);

        Layout::Builder& SetTotalSize(size_t aBytes) override;

        Layout::Builder& AddPointerOffsets(std::variant<size_t, PMA::MultiLevelPointer> aOffsetOrMlp,
                                           const std::string& aStaticType, size_t aCount) override;
        Layout::Builder& AddPointerOffsets(std::variant<size_t, PMA::MultiLevelPointer> aOffsetOrMlp,
                                           const std::function<std::string(void*)>& aDynamicType, size_t aCount) override;
        Layout::Builder& AddPointerOffsets(std::variant<size_t, PMA::MultiLevelPointer> aOffsetOrMlp, size_t aStaticSize,
                                           size_t aCount) override;
        Layout::Builder& AddPointerOffsets(std::variant<size_t, PMA::MultiLevelPointer> aOffsetOrMlp,
                                           const std::function<size_t(void*)>& aDynamicSize, size_t aCount) override;

        std::unique_ptr<Layout> Build() override;
    };

}  // namespace GE
