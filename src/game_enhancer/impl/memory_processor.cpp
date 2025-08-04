#include "game_enhancer/impl/memory_processor.h"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>

#include "game_enhancer/impl/data_accessor.h"
#include "game_enhancer/memory_layout_builder.h"
#include "spdlog/sinks/null_sink.h"

namespace GE
{
    void MemoryProcessorImpl::EnablerImpl::EnsureSubsequent(const LayoutId& aLayout)
    {
        auto& order = m_memProc.m_mainLayoutOrder;
        auto it = std::find(order.begin(), order.end(), aLayout);
        if (it == order.end())
        {
            throw std::runtime_error(std::format("Layout '{}' not registered as MainLayout", aLayout));
        }
        size_t index = std::distance(order.begin(), it);
        if (index <= m_index)
        {
            throw std::runtime_error(std::format("Cannot enable layout '{}' before current layout", aLayout));
        }
    }

    MemoryProcessorImpl::EnablerImpl::EnablerImpl(MemoryProcessorImpl& aMemProc, size_t aIndex)
        : m_memProc(aMemProc)
        , m_index(aIndex)
    {
    }

    void MemoryProcessorImpl::EnablerImpl::Enable(const LayoutId& aLayout, const std::optional<PMA::MemoryAddress>& aData)
    {
        EnsureSubsequent(aLayout);
        m_memProc.m_mainLayouts[aLayout].m_active = true;
        m_memProc.m_mainLayouts[aLayout].m_dataFromEnabler = aData;
    }

    void MemoryProcessorImpl::EnablerImpl::Disable(const LayoutId& aLayout)
    {
        EnsureSubsequent(aLayout);
        auto& l = m_memProc.m_mainLayouts[aLayout];
        if (l.m_active)
        {
            if (l.m_callbacks.m_onDisabled)
            {
                (*l.m_callbacks.m_onDisabled)(*m_memProc.m_dataAccessor);
                l.m_consecutiveFrames = 0;
            }
        }
        l.m_active = false;
    }

    void MemoryProcessorImpl::ReadMainLayouts()
    {
        m_logger->trace("ReadMainLayouts called");
        FrameMemoryStorage& currentFrameStorage = m_storedFrames->emplace_back(FrameMemoryStorage{});
        std::unordered_map<size_t, uint8_t*> pointerMap;
        for (int i = 0; i < m_mainLayoutOrder.size(); ++i)
        {
            const auto& layoutId = m_mainLayoutOrder[i];
            auto& layout = m_mainLayouts[layoutId];
            if (!layout.m_active)
            {
                continue;
            }

            auto baseAddress = layout.m_callbacks.m_baseLocator(m_memoryAccess, layout.m_dataFromEnabler);
            currentFrameStorage.SetLayoutBase(layoutId, ReadLayout(layoutId, baseAddress, pointerMap, currentFrameStorage));
            layout.m_consecutiveFrames++;

            if (layout.m_callbacks.m_enabler)
            {
                EnablerImpl enabler(*this, i);
                (*layout.m_callbacks.m_enabler)(*m_dataAccessor, enabler);
            }
        }
        if (m_storedFrames->size() > m_framesToKeep)
        {
            m_storedFrames->pop_front();
        }
    }

    void MemoryProcessorImpl::Update()
    {
        m_logger->trace("Update called");
        if (m_storedFrames->size() < m_framesToKeep)
        {
            return;
        }

        for (size_t i = 0; i < m_mainLayoutOrder.size(); ++i)
        {
            auto& layout = m_mainLayouts[m_mainLayoutOrder[i]];
            if (layout.m_active && layout.m_callbacks.m_onReady && layout.m_consecutiveFrames == m_framesToKeep)
            {
                (*layout.m_callbacks.m_onReady)(m_dataAccessor);
            }
        }

        try
        {
            m_updateCallback(*m_dataAccessor);
            m_consecutiveFailedUpdates = 0;
        }
        catch (const std::exception& e)
        {
            m_logger->error("Error in Update callback: {}", e.what());
            if (++m_consecutiveFailedUpdates == 10)
            {
                m_logger->error("Too many consecutive errors in Update callback. Stopping MemoryProcessor.");
                RequestStop();
            }
        }
    }

    uint8_t* MemoryProcessorImpl::Allocate(size_t aBytes, size_t aFromAddress, FrameMemoryStorage& aCurrentFrameStorage)
    {
        uint8_t* storagePtr = aCurrentFrameStorage.Allocate(aBytes);
        GetMetadata(storagePtr)->m_realAddress = aFromAddress;
        return storagePtr;
    }

    uint8_t* MemoryProcessorImpl::ReadData(size_t aBytes, size_t aFromAddress, FrameMemoryStorage& aCurrentFrameStorage)
    {
        uint8_t* storagePtr = Allocate(aBytes, aFromAddress, aCurrentFrameStorage);
        GetMetadata(storagePtr)->m_bytesRead = m_memoryAccess->Read(aFromAddress, storagePtr, aBytes);
        return storagePtr;
    }

    // TODO refactor this + Layout logic
    uint8_t* MemoryProcessorImpl::ReadLayout(const LayoutId& aLayoutId, size_t aFromAddress,
                                             std::unordered_map<size_t, uint8_t*>& aPointerMap,
                                             FrameMemoryStorage& aCurrentFrameStorage)
    {
        auto& layout = m_layouts[aLayoutId];
        uint8_t* storagePtr = nullptr;
        if (layout->IsConsecutive())
        {
            storagePtr = ReadData(layout->GetTotalSize(), aFromAddress, aCurrentFrameStorage);
        }
        else
        {
            storagePtr = Allocate(layout->GetTotalSize(), aFromAddress, aCurrentFrameStorage);
        }
        size_t localOffset = 0;
        for (const auto& ptr : layout->GetPointerOffsets())
        {
            for (size_t i = 0; i < ptr.m_count; ++i)
            {
                size_t* castedPtr = nullptr;
                if (layout->IsConsecutive())
                {
                    localOffset = ptr.m_mlp.front() + i * sizeof(size_t);
                    castedPtr = reinterpret_cast<size_t*>(storagePtr + localOffset);
                    if (*castedPtr == 0)
                    {
                        continue;
                    }
                }
                else
                {
                    castedPtr = reinterpret_cast<size_t*>(storagePtr + localOffset);
                    localOffset += sizeof(size_t);
                }
                // TODO here I read again already read address, but it should work for now
                auto finalAddress = m_memoryAccess->Dereference(aFromAddress + i * sizeof(size_t), ptr.m_mlp);
                if (layout->IsConsecutive())
                {
                    m_memoryAccess->Read(finalAddress, PMA::mem_cast(finalAddress), sizeof(finalAddress));
                }
                if (!aPointerMap.contains(finalAddress))
                {
                    if (std::holds_alternative<Layout::LayoutIdProvider>(ptr.m_pointeeType))
                    {
                        auto& layoutIdProvider = std::get<Layout::LayoutIdProvider>(ptr.m_pointeeType);
                        aPointerMap[finalAddress] = ReadLayout(layoutIdProvider(storagePtr), finalAddress, aPointerMap,
                                                               aCurrentFrameStorage);
                    }
                    else
                    {
                        auto& dataSizeProvider = std::get<Layout::DataSizeProvider>(ptr.m_pointeeType);
                        aPointerMap[finalAddress] = ReadData(dataSizeProvider(storagePtr), finalAddress, aCurrentFrameStorage);
                    }
                }
                *castedPtr = reinterpret_cast<size_t>(aPointerMap[finalAddress]);
            }
        }
        return storagePtr;
    }

    MemoryProcessorImpl::MemoryProcessorImpl(std::shared_ptr<spdlog::logger> aLogger)
        : m_storedFrames(std::make_shared<std::deque<FrameMemoryStorage>>())
        , m_logger(std::move(aLogger))
    {
        m_logger->info("MemoryProcessor created");
    }

    MemoryProcessorImpl::~MemoryProcessorImpl()
    {
        Stop();
        m_logger->info("MemoryProcessor destroyed");
    }

    void MemoryProcessorImpl::AddMainLayout(const LayoutId& aLayoutId, const MainLayoutCallbacks& aCallbacks)
    {
        EnsureNotRunning();
        m_logger->info("Adding main layout: {}", aLayoutId);
        if (m_mainLayouts.contains(aLayoutId))
        {
            throw std::runtime_error(std::format("Layout '{}' already defined as MainLayout", aLayoutId));
        }
        m_mainLayoutOrder.push_back(aLayoutId);
        m_mainLayouts[aLayoutId] = {aCallbacks, m_mainLayouts.empty()};
    }

    void MemoryProcessorImpl::SetUpdateCallback(const std::function<void(const DataAccessor&)>& aCallback, size_t aFramesToKeep,
                                                std::optional<size_t> aRateMs)
    {
        EnsureNotRunning();
        m_updateCallback = aCallback;
        m_framesToKeep = aFramesToKeep;
        m_refreshRateMs = aRateMs.value_or(1000 / aFramesToKeep);
    }

    void MemoryProcessorImpl::RegisterLayout(const LayoutId& aLayoutId, std::unique_ptr<Layout> aLayout)
    {
        EnsureNotRunning();
        m_logger->info("Adding layout: {}", aLayoutId);
        m_layouts[aLayoutId] = std::move(aLayout);
    }

    void MemoryProcessorImpl::EnsureNotRunning() const
    {
        if (m_running)
        {
            throw std::runtime_error("MemoryProcessor is running. Cannot perform this operation.");
        }
    }

    void MemoryProcessorImpl::ResetStoredData()
    {
        m_dataAccessor.reset();
        m_storedFrames->clear();
    }

    void MemoryProcessorImpl::Start(PMA::MemoryAccessPtr aMemoryAccess)
    {
        RequestStart(std::move(aMemoryAccess));
        while (!m_running)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        m_logger->info("MemoryProcessor started");
    }

    void MemoryProcessorImpl::RequestStart(PMA::MemoryAccessPtr aMemoryAccess)
    {
        EnsureNotRunning();
        if (!m_updateCallback)
        {
            throw std::runtime_error("No update callback set!");
        }
        m_logger->info("Requesting start");
        m_memoryAccess = std::move(aMemoryAccess);
        m_updateThread = std::jthread([this](std::stop_token aStopToken) {
            try
            {
                m_logger->info("Update thread started");
                m_running = true;
                m_onRunningChangedCallback(true);
                m_dataAccessor = std::make_shared<DataAccessorImpl>(m_storedFrames);
                while (!aStopToken.stop_requested())
                {
                    m_logger->trace("Next frame iteration");
                    auto frameStartTime = std::chrono::steady_clock::now();
                    try
                    {
                        ReadMainLayouts();
                        Update();
                    }
                    catch (const std::exception& e)
                    {
                        if (!m_memoryAccess->IsValid())
                        {
                            m_logger->warn("Stopping MemoryProcessor: MemoryAccess is no longer valid - {}", e.what());
                        }
                        else
                        {
                            m_logger->error("Stopping MemoryProcessor: Unrecoverable error - {}", e.what());
                        }
                        break;
                    }
                    std::this_thread::sleep_until(frameStartTime + std::chrono::milliseconds(m_refreshRateMs));
                }
                ResetStoredData();
                m_running = false;
                m_memoryAccess.reset();
                m_onRunningChangedCallback(false);
                m_logger->info("Update thread stopped");
            }
            catch (const std::exception& e)
            {
                m_logger->error("Unhandled exception in update thread: {}", e.what());
                m_running = false;
                m_onRunningChangedCallback(false);
                ResetStoredData();
            }
            catch (...)
            {
                m_logger->error("Super unexpected error in update thread");
            }
        });
    }

    void MemoryProcessorImpl::Stop()
    {
        RequestStop();
        Wait();
    }

    void MemoryProcessorImpl::RequestStop()
    {
        m_logger->info("Requesting stop");
        m_updateThread.request_stop();
    }

    void MemoryProcessorImpl::Wait()
    {
        m_logger->info("Waiting for main loop to finish");
        if (m_updateThread.joinable())
        {
            m_updateThread.join();
            m_logger->info("Main loop finished");
        }
    }

    bool MemoryProcessorImpl::IsRunning() const
    {
        return m_running;
    }

    PMA::ScopedTokenPtr MemoryProcessorImpl::OnRunningChanged(const std::function<void(bool)>& aCallback)
    {
        return m_onRunningChangedCallback.Add(aCallback);
    }

    MemoryProcessorPtr MemoryProcessor::Create(std::shared_ptr<spdlog::logger> aLogger)
    {
        if (!aLogger)
        {
            aLogger = std::make_shared<spdlog::logger>("nolog");
        }
        return std::make_unique<MemoryProcessorImpl>(std::move(aLogger));
    }
}
