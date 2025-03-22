#include "game_enhancer/impl/memory_processor.h"

#include <memory>
#include <string>
#include <unordered_map>

namespace GE
{
    void MemoryProcessorImpl::Update()
    {
        m_storedFrames.push_back(FrameMemoryStorage());
        std::unordered_map<size_t, size_t> pointerMap;
        for (const auto& layout : m_starterLayouts)
        {
            ReadLayout(layout.first, layout.second(), pointerMap);
        }
        for (const auto& layout : m_starterLayouts)
        {
            // m_callback(layout.first)
            // TODO call the OnRefresh callback
        }
        if (m_storedFrames.size() >= m_framesToKeep)
        {
            m_storedFrames.pop_front();
        }
    }

    uint8_t* MemoryProcessorImpl::ReadLayoutRecursive(const std::string& aLayoutType, size_t aFromAddress,
                                                      std::unordered_map<size_t, size_t>& aPointerMap)
    {
        uint8_t* storagePtr = m_storedFrames.back().Allocate(m_layouts[aLayoutType].first);
        m_layoutReader->ReadLayout(aLayoutType, aFromAddress, storagePtr);
        for (const auto ptr : m_layouts[aLayoutType].second)
        {
            for (size_t i = 0; i < ptr.m_count; ++i)
            {
                auto castedPtr = reinterpret_cast<size_t*>(storagePtr + ptr.m_offset + i * sizeof(size_t));
                size_t pointerAddress = *castedPtr;
                if (pointerAddress == 0)
                {
                    continue;
                }
                if (!pointerMap.contains(pointerAddress))
                {
                    aPointerMap[pointerAddress] = ReadLayoutRecursive(ptr.m_pointeeType, pointerAddress, aPointerMap);
                }
                *castedPtr = aPointerMap[pointerAddress];
            }
        }
        return storagePtr;
    }

    void MemoryProcessorImpl::ReadLayout(const std::string& aLayoutType, size_t aFromAddress,
                                         std::unordered_map<size_t, size_t>& aPointerMap)
    {
        ReadLayoutRecursive(aLayoutType, aFromAddress, pointerMap);
    }

    void MemoryProcessorImpl::RegisterLayout(const std::string& aLayoutType, LayoutBuilder::Absolute::Layout aLayout)
    {
        m_layouts[aLayoutType] = std::move(aLayout);
    }

    void MemoryProcessorImpl::SetRefreshRate(size_t aRateMs)
    {
        m_refreshRateMs = aRateMs;
    }

    void MemoryProcessorImpl::Start() {}

    void MemoryProcessorImpl::Stop() {}

    void MemoryProcessorImpl::AddStarterLayout(const std::string& aType, const std::function<void(const DataStuff&)>& aCallback)
    {
        m_starterLayouts[aType] = aCallback;
    }

    void MemoryProcessorImpl::SetFramesToKeep(size_t aFrames)
    {
        m_framesToKeep = aFrames;
    }

    void MemoryProcessorImpl::Initialize()
    {
        // TODO somewhere the connection to the Process needs to be done
    }
}
