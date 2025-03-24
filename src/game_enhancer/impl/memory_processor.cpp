#include "game_enhancer/impl/memory_processor.h"

#include <stdexcept>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>

namespace GE
{
    void MemoryProcessorImpl::Update()
    {
        m_storedFrames.push_back(FrameMemoryStorage());
        std::unordered_map<size_t, uint8_t*> pointerMap;
        for (const auto& layout : m_starterLayouts)
        {
            ReadLayout(layout.first, layout.second(), pointerMap);
        }

        m_callback({} /*TODO need to create a correct object*/);

        if (m_storedFrames.size() >= m_framesToKeep)
        {
            m_storedFrames.pop_front();
        }
    }

    uint8_t* MemoryProcessorImpl::ReadLayout(const std::string& aLayoutType, size_t aFromAddress,
                                             std::unordered_map<size_t, uint8_t*>& aPointerMap)
    {
        uint8_t* storagePtr = m_storedFrames.back().Allocate(m_layouts[aLayoutType].first);
        //m_layoutReader->ReadLayout(aLayoutType, aFromAddress, storagePtr);
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
                if (!aPointerMap.contains(pointerAddress))
                {
                    aPointerMap[pointerAddress] = ReadLayout(ptr.m_pointeeType, pointerAddress, aPointerMap);
                }
                *castedPtr = reinterpret_cast<size_t>(aPointerMap[pointerAddress]);
            }
        }
        return storagePtr;
    }

    void MemoryProcessorImpl::RegisterLayout(const std::string& aLayoutType, LayoutBuilder::Absolute::Layout aLayout)
    {
        EnsureNotRunning();
        m_layouts[aLayoutType] = std::move(aLayout);
    }

    void MemoryProcessorImpl::SetUpdateCallback(size_t aRateMs, const std::function<void(const DataStuff&)>& aCallback)
    {
        EnsureNotRunning();
        m_refreshRateMs = aRateMs;
        m_callback = aCallback;
    }

    void MemoryProcessorImpl::EnsureNotRunning() const
    {
        if (m_running)
        {
            throw std::runtime_error("MemoryProcessor is running. Cannot perform this operation.");
        }
    }

    void MemoryProcessorImpl::Start()
    {
        EnsureNotRunning();
        m_updateThread = std::jthread([this](std::stop_token aStopToken) {
            m_running = true;
            while (!aStopToken.stop_requested())
            {
                auto frameStartTime = std::chrono::steady_clock::now();
                // TODO error handling from Update method
                Update();
                std::this_thread::sleep_until(frameStartTime + std::chrono::milliseconds(m_refreshRateMs));
            }
            m_running = false;
        });
    }

    void MemoryProcessorImpl::Stop()
    {
        m_updateThread.request_stop();
    }

    void MemoryProcessorImpl::AddStarterLayout(const std::string& aType, const std::function<size_t()>& aCallback)
    {
        EnsureNotRunning();
        m_starterLayouts[aType] = aCallback;
    }

    void MemoryProcessorImpl::SetFramesToKeep(size_t aFrames)
    {
        EnsureNotRunning();
        m_framesToKeep = aFrames;
    }

    void MemoryProcessorImpl::Initialize()
    {
        EnsureNotRunning();
        // TODO somewhere the connection to the Process needs to be done
    }
}
