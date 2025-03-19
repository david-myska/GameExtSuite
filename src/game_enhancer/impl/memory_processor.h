#pragma once

#include <string>
#include <memory>

#include "game_enhancer/memory_processor.h"

namespace GE
{
    class FrameMemoryStorage;
    class MemoryStorage;
    class PointerResolver;
    class LayoutReader;

    class MemoryProcessorImpl : public MemoryProcessor
    {
        std::shared_ptr<FrameMemoryStorage> m_frameMemoryStorage;
        std::shared_ptr<MemoryStorage> m_memoryStorage;
        std::shared_ptr<PointerResolver> m_pointerResolver;
        std::shared_ptr<LayoutReader> m_layoutReader;

        void ReadLayout(const std::string& aLayoutType, size_t aFromAddress);

    public:
        MemoryProcessorImpl() = default;

        void RegisterLayout(const std::string& aLayoutType, LayoutBuilder::Absolute::Layout aLayout) override;
        void SetRefreshRate(size_t aRateMs) override;
        void OnRefresh(const std::string& aLayoutType, std::function<void(const DataStuff&)>) override;
        void Start() override;
        void Stop() override;
    };

}
