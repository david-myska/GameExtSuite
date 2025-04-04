#pragma once

#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace GE
{
    namespace LayoutBuilder
    {
        using DynamicLayoutFnc = std::function<std::string(void*)>;
        using DynamicDataFnc = std::function<size_t(void*)>;

        struct Ptr
        {
            size_t m_offset;
            size_t m_count;
            std::variant<DynamicLayoutFnc, DynamicDataFnc> m_pointeeType;
        };

        struct Absolute
        {
            using Layout = std::pair<size_t, std::vector<Ptr>>;
            virtual ~Absolute() = default;

            virtual Absolute& SetTotalSize(size_t aBytes) = 0;
            virtual Absolute& AddPointerOffsets(size_t aOffset, const std::string& aStaticType, size_t aCount = 1) = 0;
            virtual Absolute& AddPointerOffsets(size_t aOffset, const std::function<std::string(void*)>& aDynamicType,
                                                size_t aCount = 1) = 0;
            virtual Absolute& AddPointerOffsets(size_t aOffset, size_t aStaticSize, size_t aCount = 1) = 0;
            virtual Absolute& AddPointerOffsets(size_t aOffset, const std::function<size_t(void*)>& aDynamicSize,
                                                size_t aCount = 1) = 0;

            template <typename T>
            Absolute& AddPointerOffsets(size_t aOffset, const std::function<std::string(T*)>& aDynamicType, size_t aCount = 1)
            {
                return AddPointerOffsets(
                    aOffset,
                    [aDynamicType](void* aPtr) {
                        return aDynamicType(static_cast<T*>(aPtr));
                    },
                    aCount);
            }

            template <typename T>
            Absolute& AddPointerOffsets(size_t aOffset, const std::function<size_t(T*)>& aDynamicSize, size_t aCount = 1)
            {
                return AddPointerOffsets(
                    aOffset,
                    [aDynamicSize](void* aPtr) {
                        return aDynamicSize(static_cast<T*>(aPtr));
                    },
                    aCount);
            }

            virtual Layout Build() = 0;
        };

        std::unique_ptr<Absolute> MakeAbsolute();
        // struct Relative
        //{
        //     virtual ~Relative() = default;

        // virtual Relative& AddData(size_t aBytes) = 0;
        // virtual Relative& AddPointer() = 0;

        // virtual Relative& Repeat(size_t aCount) = 0;
        // virtual MemoryLayoutPtr Build() = 0;
        // };
        // std::unique_ptr<Relative> MakeRelative();

    }
}
