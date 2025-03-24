#pragma once

#include <atomic>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>
#include <deque>

#include "game_enhancer/impl/layout/frame_memory_storage.h"
#include "game_enhancer/memory_processor.h"
#include "pma/target_process.h"
#include "pma/utility/auto_attach.h"

namespace GE
{
    class MemoryProcessorImpl : public MemoryProcessor
    {
        std::unordered_map<std::string, std::function<size_t(PMA::MemoryAccessPtr aMemoryAccess)>> m_starterLayouts;
        std::unordered_map<std::string, LayoutBuilder::Absolute::Layout> m_layouts;

        size_t m_framesToKeep = 2;
        std::shared_ptr<std::deque<FrameMemoryStorage>> m_storedFrames;

        PMA::TargetProcessPtr m_targetProcess;
        PMA::AutoAttachPtr m_autoAttach;
        PMA::ScopedTokenPtr m_onAttachedToken;
        PMA::MemoryAccessPtr m_memoryAccess;

        size_t m_refreshRateMs = 100;
        std::jthread m_updateThread;
        std::function<void(const FrameAccessor&)> m_callback;
        std::atomic<bool> m_running = false;

        void Update();
        uint8_t* ReadLayout(const std::string& aLayoutType, size_t aFromAddress,
                            std::unordered_map<size_t, uint8_t*>& aPointerMap, FrameMemoryStorage& aCurrentFrameStorage);
        void EnsureNotRunning() const;

    public:
        MemoryProcessorImpl(PMA::TargetProcessPtr aTargetProcess);

        void RegisterLayout(const std::string& aLayoutType, LayoutBuilder::Absolute::Layout aLayout) override;
        void SetUpdateCallback(size_t aRateMs, const std::function<void(const FrameAccessor&)>& aCallback) override;
        void Start() override;
        void Stop() override;

        void AddStarterLayout(const std::string& aType,
                              const std::function<size_t(PMA::MemoryAccessPtr aMemoryAccess)>& aCallback) override;
        void SetFramesToKeep(size_t aFrames) override;
        void Initialize() override;
    };

}
