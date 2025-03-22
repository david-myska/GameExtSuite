#pragma once

#include <list>
#include <vector>

namespace GE
{
    class FrameMemoryStorage
    {
        std::list<std::vector<uint8_t>> m_storage;

    public:
        uint8_t* Allocate(size_t aSize)
        {
            m_storage.push_back(std::vector<uint8_t>(aSize));
            return m_storage.back().data();
        }
    };

}
