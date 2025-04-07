#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "game_enhancer/data_accessor.h"
#include "game_enhancer/memory_layout_builder.h"
#include "pma/target_process.h"

namespace GE
{
    struct MemoryProcessor;
    using MemoryProcessorPtr = std::unique_ptr<MemoryProcessor>;

    /*
     * Every pointer, whose type has been registered as a layout, is automatically resolved.
     */
    struct MemoryProcessor
    {
        static [[nodiscard]] MemoryProcessorPtr Create(PMA::TargetProcessPtr aTargetProcess);

        /*
         * Registers possible starter layouts and layouts that pointers can point at
         */
        virtual void RegisterLayout(const std::string& aType, LayoutBuilder::Absolute::Layout aLayout) = 0;
        // virtual void RegisterLayout(const std::string& aType, LayoutBuilder::Relative::Layout aLayout) = 0;

        /*
         * Update callback is called for the first time after the first 'FramesToKeep' frames were read.
         * After that, it is called every 'aRateMs' milliseconds.
         * Calculation from FrameRate to UpdateRate: 1000 / FrameRate
         */
        virtual void SetUpdateCallback(size_t aRateMs, const std::function<void(const DataAccessor&)>& aCallback) = 0;

        /*
         * Adds a starting layout that will be used to resolve pointers and read their layouts
         * Specifying refresh rate for a layout will cause it to be completely separated from the main loop.
         *   - Useful for data that doesn't need to be read as ofter as the main loop and doesn't have complex pointer structures
         *   - Improper use can cause significant performance issues
         */
        virtual void AddStarterLayout(const std::string& aType,
                                      const std::function<size_t(PMA::MemoryAccessPtr aMemoryAccess)>& aCallback) = 0;

        /*
         * Set how many frames to keep in memory.
         * This decides how far back in time the data can be accessed during each refresh.
         */
        virtual void SetFramesToKeep(size_t aFrames) = 0;

        /*
         * Prepares the MemoryProcessor for the main loop.
         * Prepares required memory, might do some optimizations, etc.
         */
        virtual void Initialize() = 0;

        /*
         * OnReady callback is called after MemoryProcessor successfully started main loop and first 'FramesToKeep' frames were
         * read. In this callback, setup the SharedState and any helper classes that require DataAccessor to be fully initialized.
         */
        virtual void SetOnReadyCallback(const std::function<void(std::shared_ptr<DataAccessor>)>& aCallback) = 0;

        /*
         * Starts the main loop. OnRefresh callbacks will be called at the specified refresh rate.
         */
        virtual void Start() = 0;

        /*
         * Stops the main loop. OnRefresh callbacks will no longer be called.
         * This call is blocking and returns when the main loop is fully stopped.
         */
        virtual void Stop() = 0;

        /*
         * Stops the main loop. OnRefresh callbacks will no longer be called.
         * Returns immediately.
         */
        virtual void RequestStop() = 0;

        /*
         * Waits for the main loop to finish. This is a blocking call.
         * This call returns under the following conditions:
         * - Stop() was called from other thread
         * - Connection to the target process was lost (target process was closed, unexpected error, ...)
         */
        virtual void Wait() = 0;
    };
}
