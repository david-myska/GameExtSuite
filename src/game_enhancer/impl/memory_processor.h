#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "game_enhancer/memory_processor.h"
#include "game_enhancer/impl/layout/frame_memory_storage.h"

namespace GE
{
    class LayoutReader;

    class MemoryProcessorImpl : public MemoryProcessor
    {
        std::unordered_map<std::string, std::function<size_t()>> m_starterLayouts;
        std::unordered_map<std::string, LayoutBuilder::Absolute::Layout> m_layouts;

        size_t m_refreshRateMs 100;
        size_t m_framesToKeep = 2;
        std::list<FrameMemoryStorage> m_storedFrames;
        // TODO substitute with pure PMA
        std::shared_ptr<LayoutReader> m_layoutReader;

        void Update();
        void ReadLayout(const std::string& aLayoutType, size_t aFromAddress, std::unordered_map<size_t, size_t>& aPointerMap);
        uint8_t* ReadLayoutRecursive(const std::string& aLayoutType, size_t aFromAddress,
                                     std::unordered_map<size_t, size_t>& aPointerMap);

    public:
        MemoryProcessorImpl() = default;

        void RegisterLayout(const std::string& aLayoutType, LayoutBuilder::Absolute::Layout aLayout) override;
        void SetRefreshRate(size_t aRateMs) override;
        void OnRefresh(const std::string& aLayoutType, std::function<void(const DataStuff&)>) override;
        void Start() override;
        void Stop() override;

        void AddStarterLayout(const std::string& aType, const std::function<void(const DataStuff&)>& aCallback) override;
        void SetFramesToKeep(size_t aFrames) override;
        void Initialize() override;
    };

}
