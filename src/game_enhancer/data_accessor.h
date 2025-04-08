#pragma once

#include <string>

namespace GE
{
    struct DataAccessor
    {
        virtual ~DataAccessor() = default;

        virtual const uint8_t* GetRaw(const std::string& aLayout, size_t aFrameIdx = 0) const = 0;

        virtual size_t GetNumberOfFrames() const = 0;
        /*
         * aFrameIdx 0 is the most recent frame, 1 is the frame before that, etc.
        */
        template <typename T>
        auto Get(const std::string& aLayout, size_t aFrameIdx = 0) const
        {
            return reinterpret_cast<const T*>(GetRaw(aLayout, aFrameIdx));
        }
    };
}
