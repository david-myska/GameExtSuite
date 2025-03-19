#pragma once

#include <memory>
#include <string>
#include <vector>

namespace GE
{
    namespace LayoutBuilder
    {
        struct Ptr
        {
            size_t m_offset;
            size_t m_count;
            std::string m_pointeeType;
        };

        struct Absolute
        {
            using Layout = std::pair<size_t, std::vector<Ptr>>;
            virtual ~Absolute() = default;

            virtual Absolute& SetTotalSize(size_t aBytes) = 0;
            virtual Absolute& AddPointerOffsets(size_t aOffset, const std::string& aPointeeType, size_t aCount = 1) = 0;

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
