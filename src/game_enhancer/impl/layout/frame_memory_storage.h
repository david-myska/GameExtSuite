#pragma once

#include <deque>
#include <vector>
#include <unordered_map>

namespace GE
{
    class FrameMemoryStorage
    {
        std::deque<std::vector<uint8_t>> m_storage;
        std::unordered_map<std::string, uint8_t*> m_layoutBase;

    public:
        uint8_t* Allocate(size_t aSize)
        {
            m_storage.push_back(std::vector<uint8_t>(aSize));
            return m_storage.back().data();
        }

        void SetLayoutBase(const std::string& aLayoutType, uint8_t* aBase) { m_layoutBase[aLayoutType] = aBase; }
        uint8_t* GetLayoutBase(const std::string& aLayoutType) { return m_layoutBase[aLayoutType]; }
    };

}
