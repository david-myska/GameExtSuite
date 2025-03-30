#include "game_enhancer/impl/memory_processor.h"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>

#include "game_enhancer/impl/frame_accessor.h"

namespace GE
{
    void MemoryProcessorImpl::Update()
    {
        FrameMemoryStorage currentFrameStorage;
        std::unordered_map<size_t, uint8_t*> pointerMap;
        for (const auto& layout : m_starterLayouts)
        {
            currentFrameStorage.SetLayoutBase(
                layout.first, ReadLayout(layout.first, layout.second(m_memoryAccess), pointerMap, currentFrameStorage));
        }

        m_storedFrames->push_back(std::move(currentFrameStorage));
        if (m_storedFrames->size() > m_framesToKeep)
        {
            m_storedFrames->pop_front();
        }
        if (m_storedFrames->size() < m_framesToKeep)
        {
            return;
        }

        try
        {
            m_callback(FrameAccessorImpl(m_storedFrames));
        }
        catch (const std::exception& e)
        {
            // TODO log error std::format("Error in MemoryProcessor callback: {}", e.what());
            // and continue
            throw;  // just for now
        }
    }

    uint8_t* MemoryProcessorImpl::ReadLayout(const std::string& aLayoutType, size_t aFromAddress,
                                             std::unordered_map<size_t, uint8_t*>& aPointerMap,
                                             FrameMemoryStorage& aCurrentFrameStorage)
    {
        uint8_t* storagePtr = aCurrentFrameStorage.Allocate(m_layouts[aLayoutType].first);
        GetMetadata(storagePtr)->m_realAddress = aFromAddress;
        m_memoryAccess->Read(aFromAddress, storagePtr, m_layouts.at(aLayoutType).first);
        for (const auto& ptr : m_layouts[aLayoutType].second)
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
                    aPointerMap[pointerAddress] = ReadLayout(ptr.m_pointeeType, pointerAddress, aPointerMap,
                                                             aCurrentFrameStorage);
                }
                *castedPtr = reinterpret_cast<size_t>(aPointerMap[pointerAddress]);
            }
        }
        return storagePtr;
    }

    MemoryProcessorImpl::MemoryProcessorImpl(PMA::TargetProcessPtr aTargetProcess)
        : m_targetProcess(std::move(aTargetProcess))
        , m_autoAttach(PMA::AutoAttach::Create(m_targetProcess))
        , m_storedFrames(std::make_shared<std::deque<FrameMemoryStorage>>())
    {
    }

    MemoryProcessorImpl::~MemoryProcessorImpl()
    {
        Stop();
    }

    void MemoryProcessorImpl::RegisterLayout(const std::string& aLayoutType, LayoutBuilder::Absolute::Layout aLayout)
    {
        EnsureNotRunning();
        m_layouts[aLayoutType] = std::move(aLayout);
    }

    void MemoryProcessorImpl::SetUpdateCallback(size_t aRateMs, const std::function<void(const FrameAccessor&)>& aCallback)
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
        if (!m_callback)
        {
            throw std::runtime_error("No update callback set!");
        }
        m_onAttachedToken = m_autoAttach->OnAttached([this] {
            m_memoryAccess = m_targetProcess->GetMemoryAccess();
            m_updateThread = std::jthread([this](std::stop_token aStopToken) {
                m_running = true;
                while (!aStopToken.stop_requested())
                {
                    auto frameStartTime = std::chrono::steady_clock::now();
                    try
                    {
                        Update();
                    }
                    catch (const std::exception& e)
                    {
                        std::cout << "Except in Update" << std::endl;
                        if (!m_targetProcess->Exists())
                        {
                            // TODO log info "Target process has exited. Stopping MemoryProcessor."
                            std::cout << "!Exists" << std::endl;
                        }
                        else if (!m_targetProcess->IsAttached())
                        {
                            // TODO log error "Target process has been detached. Stopping MemoryProcessor."
                            std::cout << "!IsAttached" << std::endl;
                        }
                        else
                        {
                            // TODO log error std::format("Error in MemoryProcessor: {}", e.what());
                            std::cout << "Else: " << e.what() << std::endl;
                        }
                        break;
                    }
                    std::this_thread::sleep_until(frameStartTime + std::chrono::milliseconds(m_refreshRateMs));
                }
                m_running = false;
            });
        });
        m_autoAttach->Start();
    }

    void MemoryProcessorImpl::Stop()
    {
        m_onAttachedToken.reset();
        m_updateThread.request_stop();
        if (m_updateThread.joinable())
        {
            m_updateThread.join();
        }
    }

    void MemoryProcessorImpl::AddStarterLayout(const std::string& aType,
                                               const std::function<size_t(PMA::MemoryAccessPtr aMemoryAccess)>& aCallback)
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
        // TODO
    }

    MemoryProcessorPtr MemoryProcessor::Create(PMA::TargetProcessPtr aTargetProcess)
    {
        return std::make_unique<MemoryProcessorImpl>(std::move(aTargetProcess));
    }
}
