#pragma once

#include <string>

namespace GE
{
    struct FrameAccessor
    {
        virtual ~FrameAccessor() = default;

        /*
         * aFrameIdx 0 is the most recent frame, 1 is the frame before that, etc.
        */
        virtual const uint8_t* GetFrameImpl(const std::string& aLayout, size_t aFrameIdx) const = 0;

        template <typename T, size_t... Frames>
        auto GetFrames(const std::string& aLayout) const
        {
            return std::make_tuple(reinterpret_cast<const T*>(GetFrameImpl(aLayout, Frames))...);
        }

        template <typename T>
        auto Get2Frames(const std::string& aLayout) const
        {
            return GetFrames<T, 0, 1>(aLayout);
        }
    };
}
