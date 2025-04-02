#pragma once

#include <list>
#include <memory>
#include <stdexcept>

#include "game_enhancer/data_accessor.h"
#include "game_enhancer/impl/layout/frame_memory_storage.h"

namespace GE
{
    class DataAccessorImpl : public DataAccessor
    {
        std::weak_ptr<std::deque<FrameMemoryStorage>> m_weakFrameStorage;

        std::shared_ptr<std::deque<FrameMemoryStorage>> EnsureValid() const;

    public:
        DataAccessorImpl(std::weak_ptr<std::deque<FrameMemoryStorage>> aWeakFrameStorage);
        const uint8_t* GetFrameImpl(const std::string& aLayout, size_t aFrameIdx) const override;
    };
}
