#include "game_enhancer/impl/memory_processor.h"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>

#include "game_enhancer/impl/data_accessor.h"
#include "game_enhancer/memory_layout_builder.h"

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

        if (!m_dataAccessor)
        {
            m_dataAccessor = std::make_shared<DataAccessorImpl>(m_storedFrames);
            m_onReadyCallback(m_dataAccessor);
        }

        try
        {
            m_updateCallback(*m_dataAccessor);
        }
        catch (const std::exception& e)
        {
            // TODO log error std::format("Error in MemoryProcessor callback: {}", e.what());
            // and continue
            throw;  // just for now
        }
    }

    uint8_t* MemoryProcessorImpl::ReadData(size_t aBytes, size_t aFromAddress, FrameMemoryStorage& aCurrentFrameStorage)
    {
        uint8_t* storagePtr = aCurrentFrameStorage.Allocate(aBytes);
        GetMetadata(storagePtr)->m_realAddress = aFromAddress;
        m_memoryAccess->Read(aFromAddress, storagePtr, aBytes);
        return storagePtr;
    }

    uint8_t* MemoryProcessorImpl::ReadLayout(const std::string& aLayoutType, size_t aFromAddress,
                                             std::unordered_map<size_t, uint8_t*>& aPointerMap,
                                             FrameMemoryStorage& aCurrentFrameStorage)
    {
        uint8_t* storagePtr = ReadData(m_layouts[aLayoutType].first, aFromAddress, aCurrentFrameStorage);
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
                    if (std::holds_alternative<LayoutBuilder::DynamicLayoutFnc>(ptr.m_pointeeType))
                    {
                        auto& dynamicLayoutFnc = std::get<LayoutBuilder::DynamicLayoutFnc>(ptr.m_pointeeType);
                        aPointerMap[pointerAddress] = ReadLayout(dynamicLayoutFnc(storagePtr), pointerAddress, aPointerMap,
                                                                 aCurrentFrameStorage);
                    }
                    else
                    {
                        auto& dynamicDataFnc = std::get<LayoutBuilder::DynamicDataFnc>(ptr.m_pointeeType);
                        aPointerMap[pointerAddress] = ReadData(dynamicDataFnc(storagePtr), pointerAddress, aCurrentFrameStorage);
                    }
                    *castedPtr = reinterpret_cast<size_t>(aPointerMap[pointerAddress]);
                }
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

    void MemoryProcessorImpl::AddMainLayout(const LayoutId& aLayoutId, const MainLayoutCallbacks& aCallbacks)
    {
        EnsureNotRunning();
        m_mainLayouts[aLayoutId] = aCallbacks;
    }

    void MemoryProcessorImpl::SetUpdateCallback(const std::function<void(const DataAccessor&)>& aCallback, size_t aFramesToKeep,
                                                std::optional<size_t> aRateMs)
    {
        EnsureNotRunning();
        m_updateCallback = aCallback;
        m_framesToKeep = aFramesToKeep;
        m_refreshRateMs = aRateMs.value_or(1000 / aFramesToKeep);
    }

    void MemoryProcessorImpl::RegisterLayout(const LayoutId& aLayoutType, LayoutBuilder::Absolute::Layout aLayout)
    {
        EnsureNotRunning();
        m_layouts[aLayoutType] = std::move(aLayout);
    }

    void MemoryProcessorImpl::EnsureNotRunning() const
    {
        if (m_running)
        {
            throw std::runtime_error("MemoryProcessor is running. Cannot perform this operation.");
        }
    }

    void MemoryProcessorImpl::ResetStoredData() {
        m_dataAccessor.reset();
        m_storedFrames->clear();
    }

    void MemoryProcessorImpl::Start()
    {
        EnsureNotRunning();
        if (!m_updateCallback)
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
                ResetStoredData();
                m_running = false;
            });
        });
        m_autoAttach->Start();
    }

    void MemoryProcessorImpl::Stop()
    {
        RequestStop();
        if (m_updateThread.joinable())
        {
            m_updateThread.join();
        }
    }

    void MemoryProcessorImpl::RequestStop()
    {
        m_onAttachedToken.reset();
        m_updateThread.request_stop();
    }

    void MemoryProcessorImpl::Wait()
    {
        if (m_updateThread.joinable())
        {
            m_updateThread.join();
        }
    }

    MemoryProcessorPtr MemoryProcessor::Create(PMA::TargetProcessPtr aTargetProcess)
    {
        return std::make_unique<MemoryProcessorImpl>(std::move(aTargetProcess));
    }
}
