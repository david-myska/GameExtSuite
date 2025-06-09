#pragma once

#include <atomic>
#include <deque>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>

#include "game_enhancer/impl/layout/frame_memory_storage.h"
#include "game_enhancer/memory_processor.h"
#include "pma/target_process.h"
#include "pma/impl/callback/callback.h"
#include "pma/utility/auto_attach.h"

namespace GE
{
    struct MainLayout
    {
        MainLayoutCallbacks m_callbacks;
        bool m_active = false;
        size_t m_index = 0;
        size_t m_consecutiveFrames = 0;
        std::optional<PMA::MemoryAddress> m_dataFromEnabler;
    };

    class MemoryProcessorImpl : public MemoryProcessor
    {
        class EnablerImpl : public Enabler
        {
            MemoryProcessorImpl& m_memProc;
            const size_t m_index;

            void EnsureSubsequent(const LayoutId& aLayout);

        public:
            EnablerImpl(MemoryProcessorImpl& aMemProc, size_t aIndex);

            void Enable(const LayoutId& aLayout, const std::optional<PMA::MemoryAddress>& aData = {}) override;

            void Disable(const LayoutId& aLayout) override;
        };

        std::vector<LayoutId> m_mainLayoutOrder;
        std::unordered_map<LayoutId, MainLayout> m_mainLayouts;
        std::unordered_map<LayoutId, std::unique_ptr<Layout>> m_layouts;

        PMA::TargetProcessPtr m_targetProcess;
        PMA::AutoAttachPtr m_autoAttach;
        PMA::ScopedTokenPtr m_onAttachedToken;
        PMA::MemoryAccessPtr m_memoryAccess;

        PMA::Callback<bool> m_onRunningChangedCallback;

        std::shared_ptr<std::deque<FrameMemoryStorage>> m_storedFrames;
        size_t m_framesToKeep = 2;

        size_t m_refreshRateMs = 100;
        std::jthread m_updateThread;
        std::function<void(const DataAccessor&)> m_updateCallback;
        size_t m_consecutiveFailedUpdates = 0;
        std::atomic<bool> m_running = false;

        std::shared_ptr<DataAccessor> m_dataAccessor;

        std::shared_ptr<spdlog::logger> m_logger;

        void ReadMainLayouts();
        void Update();
        uint8_t* Allocate(size_t aBytes, size_t aFromAddress, FrameMemoryStorage& aCurrentFrameStorage);
        uint8_t* ReadData(size_t aBytes, size_t aFromAddress, FrameMemoryStorage& aCurrentFrameStorage);
        uint8_t* ReadLayout(const LayoutId& aLayoutId, size_t aFromAddress,
                            std::unordered_map<size_t, uint8_t*>& aPointerMap, FrameMemoryStorage& aCurrentFrameStorage);
        void EnsureNotRunning() const;

        void ResetStoredData();

    public:
        MemoryProcessorImpl(PMA::TargetProcessPtr aTargetProcess, std::shared_ptr<spdlog::logger> aLogger);
        ~MemoryProcessorImpl();

        void RegisterLayout(const LayoutId& aLayoutId, std::unique_ptr<Layout> aLayout) override;
        void AddMainLayout(const LayoutId& aLayoutId, const MainLayoutCallbacks& aCallbacks) override;
        void SetUpdateCallback(const std::function<void(const DataAccessor&)>& aCallback, size_t aFramesToKeep = 2,
                               std::optional<size_t> aRateMs = {}) override;
        void Start() override;
        void RequestStart() override;
        void Stop() override;
        void RequestStop() override;
        void Wait() override;
        bool IsRunning() const override;
        PMA::ScopedTokenPtr OnRunningChanged(const std::function<void(bool)>& aCallback) override;
    };

}
