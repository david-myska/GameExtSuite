#pragma once

#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "pma/memory_core.h"

namespace GE
{
    struct Layout
    {
        using LayoutIdProvider = std::function<std::string(void*)>;
        using DataSizeProvider = std::function<size_t(void*)>;

        struct Ptr
        {
            PMA::MultiLevelPointer m_mlp;
            size_t m_count;
            std::variant<LayoutIdProvider, DataSizeProvider> m_pointeeType;
        };

        [[nodiscard]] virtual bool IsConsecutive() const = 0;

        [[nodiscard]] virtual size_t GetTotalSize() const = 0;

        [[nodiscard]] virtual const std::vector<Ptr>& GetPointerOffsets() const = 0;

        struct Builder
        {
            virtual ~Builder() = default;

            virtual Builder& SetTotalSize(size_t aBytes) = 0;
            virtual Builder& AddPointerOffsets(std::variant<size_t, PMA::MultiLevelPointer> aOffsetOrMlp,
                                               const std::string& aStaticType, size_t aCount = 1) = 0;
            virtual Builder& AddPointerOffsets(std::variant<size_t, PMA::MultiLevelPointer> aOffsetOrMlp,
                                               const std::function<std::string(void*)>& aDynamicType, size_t aCount = 1) = 0;
            virtual Builder& AddPointerOffsets(std::variant<size_t, PMA::MultiLevelPointer> aOffsetOrMlp, size_t aStaticSize,
                                               size_t aCount = 1) = 0;
            virtual Builder& AddPointerOffsets(std::variant<size_t, PMA::MultiLevelPointer> aOffsetOrMlp,
                                               const std::function<size_t(void*)>& aDynamicSize, size_t aCount = 1) = 0;

            template <typename T>
            Builder& AddPointerOffsets(std::variant<size_t, PMA::MultiLevelPointer> aOffsetOrMlp,
                                       const std::function<std::string(T*)>& aDynamicType, size_t aCount = 1,
                                       std::optional<PMA::MultiLevelPointer> aMlp = {})
            {
                return AddPointerOffsets(
                    aOffsetOrMlp,
                    [aDynamicType](void* aPtr) {
                        return aDynamicType(static_cast<T*>(aPtr));
                    },
                    aCount);
            }

            template <typename T>
            Builder& AddPointerOffsets(std::variant<size_t, PMA::MultiLevelPointer> aOffsetOrMlp,
                                       const std::function<size_t(T*)>& aDynamicSize, size_t aCount = 1,
                                       std::optional<PMA::MultiLevelPointer> aMlp = {})
            {
                return AddPointerOffsets(
                    aOffsetOrMlp,
                    [aDynamicSize](void* aPtr) {
                        return aDynamicSize(static_cast<T*>(aPtr));
                    },
                    aCount);
            }

            virtual std::unique_ptr<Layout> Build() = 0;
        };

        /*
         * Consecutive layout is good when the data you want to read is in a consecutive memory block.
         */
        static std::unique_ptr<Builder> MakeConsecutive();

        /*
         * Scattered layout is good when you want to make a custom struct that doesn't correspond to the memory layout of the
         * target process. e.g. Grouping data that doesn't have simple access from some known structure.
         */
        static std::unique_ptr<Builder> MakeScattered();
    };
}
