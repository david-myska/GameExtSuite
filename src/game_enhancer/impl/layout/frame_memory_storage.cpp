#pragma once

#include "game_enhancer/impl/layout/frame_memory_storage.h"

namespace GE
{
    Metadata* GetMetadata(uint8_t* fromData)
    {
        return reinterpret_cast<Metadata*>(fromData - sizeof(Metadata));
    }

    uint8_t* FrameMemoryStorage::Allocate(size_t aSize)
    {
        auto dataPtr = m_storage.emplace_back(std::vector<uint8_t>(sizeof(Metadata) + aSize)).data() + sizeof(Metadata);
        *GetMetadata(dataPtr) = {};
        return dataPtr;
    }

    void FrameMemoryStorage::SetLayoutBase(const std::string& aLayoutType, uint8_t* aBase)
    {
        m_layoutBase[aLayoutType] = aBase;
    }

    uint8_t* FrameMemoryStorage::GetLayoutBase(const std::string& aLayoutType)
    {
        return m_layoutBase[aLayoutType];
    }
}
