#pragma once

#include <deque>
#include <string>
#include <unordered_map>
#include <vector>

namespace GE
{
    struct Metadata
    {
        size_t m_realAddress = 0;
        bool m_dirty = false;
    };

    Metadata* GetMetadata(uint8_t* fromData);

    class FrameMemoryStorage
    {
        std::deque<std::vector<uint8_t>> m_storage;
        std::unordered_map<std::string, uint8_t*> m_layoutBase;

    public:
        uint8_t* Allocate(size_t aSize);

        void SetLayoutBase(const std::string& aLayoutType, uint8_t* aBase);
        uint8_t* GetLayoutBase(const std::string& aLayoutType);
    };

}
