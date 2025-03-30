#pragma once

#include <deque>
#include <vector>
#include <unordered_map>

namespace GE
{
    struct Metadata {
        PMA::MemoryAddress m_realAddress = 0;
        bool m_dirty = false;
    };

    Metadata* GetMetadata(uint8_t* fromData)
    {
        return reinterpret_cast<Metadata*>(fromData - sizeof(Metadata));
    }


    class FrameMemoryStorage
    {
        std::deque<std::vector<uint8_t>> m_storage;
        std::unordered_map<std::string, uint8_t*> m_layoutBase;

    public:
        uint8_t* Allocate(size_t aSize)
        {
            auto dataPtr = m_storage.emplace_back(std::vector<uint8_t>(sizeof(Metadata) + aSize)).data() + sizeof(Metadata);
            *GetMetadata(dataPtr) = {};
            return dataPtr;
        }

        void SetLayoutBase(const std::string& aLayoutType, uint8_t* aBase) { m_layoutBase[aLayoutType] = aBase; }
        uint8_t* GetLayoutBase(const std::string& aLayoutType) { return m_layoutBase[aLayoutType]; }
    };

}
