#pragma once

#include "data_accessor.h"

#include <string>

namespace GE
{
    std::shared_ptr<std::deque<FrameMemoryStorage>> FrameAccessorImpl::EnsureValid() const
    {
        if (auto frameStorage = m_weakFrameStorage.lock())
        {
            return frameStorage;
        }
        throw std::logic_error("Memory access revoked!");
    }

    FrameAccessorImpl::FrameAccessorImpl(std::weak_ptr<std::deque<FrameMemoryStorage>> aWeakFrameStorage)
        : m_weakFrameStorage(std::move(aWeakFrameStorage))
    {
    }

    const uint8_t* GE::FrameAccessorImpl::GetFrameImpl(const std::string& aLayout, size_t aFrameIdx) const
    {
        return EnsureValid()->at(aFrameIdx).GetLayoutBase(aLayout);
    }
}
