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

    struct Enabler
    {
    };

    using LayoutId = std::string;

    struct MainLayoutCallbacks
    {
        std::function<PMA::MemoryAddress(PMA::MemoryAccessPtr)> m_baseLocator;
        std::optional<std::function<void(const DataAccessor&, Enabler&)>> m_enabler;
        std::optional<std::function<void(std::shared_ptr<DataAccessor>)>> m_onReady;
        std::optional<std::function<void(std::shared_ptr<DataAccessor>)>> m_onDisabled;
    };

    /*
     * Every pointer, whose type has been registered as a layout, is automatically resolved.
     */
    struct MemoryProcessor
    {
        static [[nodiscard]] MemoryProcessorPtr Create(PMA::TargetProcessPtr aTargetProcess);

        /*
         * Registers layouts into the framework. Registered layouts can be used by other layouts and framework understands how to
         * read them.
         */
        virtual void RegisterLayout(const LayoutId& aId, LayoutBuilder::Absolute::Layout aLayout) = 0;
        // virtual void RegisterLayout(const std::string& aId, LayoutBuilder::Relative::Layout aLayout) = 0;

        /*
         * Sets aLayoutId as a MainLayout.
         * Main layouts are the starting point for each memory read. They are processed sequentially in the order of addition.
         * First MainLayout cannot be disabled and is always read.
         * Full cycle:
         * - For each MainLayout:
         *     - Check if the Mainlayout is Enabled (if not, stop processing this layout)
         *     - Run the BaseLocatorCallback to find the starting MemoryAddress
         *     - Read TargetProcess' memory according to this layout
         *     - Run the EnablerCallback to Enable/Disable subsequent MainLayouts
         * - For each MainLayout:
         *     - Check if enough frames stored
         *         - Yes: Once run the OnReadyCallback
         * - Run UpdateCallback
         *
         * aLayoutId - Previously registered layout
         * aAnchorCallback - Should return address of where the layout starts
         * aEnablerCallback -
         * TODO EnablerCallback could optionally pass some value to the aAnchorCallback of other layouts,
         * ie when decide menu vs game, could pass the address if it is saved already in this layout to avoid repeated reads
                                        -> const std::function<size_t(PMA::MemoryAccessPtr, std::optional<PMA::MemoryAddress>)>&
         aAnchorCallback, Enabler class should on activate take optional value that will be passed
         */
        virtual void AddMainLayout(const LayoutId& aLayoutId, const MainLayoutCallbacks& aCallbacks) = 0;

        /*
         * Update callback is called for the first time after the first 'aFramesToKeep' frames were read.
         * After that, it is called every 'aRateMs' milliseconds.
         * aCallback - Main processing callback called once per read frame
         * aFramesToKeep - Set how many frames to keep in memory. Default: 2
         * aRateMs - Sets how often the Update callback will be called. Default: 1000 / aFramesToKeep
         */
        virtual void SetUpdateCallback(const std::function<void(const DataAccessor&)>& aCallback, size_t aFramesToKeep = 2,
                                       std::optional<size_t> aRateMs = {}) = 0;

        /*
         * OnReady callback is called after MemoryProcessor successfully started main loop and first 'FramesToKeep' frames were
         * read. In this callback, setup the SharedState and any helper classes that require DataAccessor to be fully initialized.
         */
        // virtual void SetOnReadyCallback(const std::function<void(std::shared_ptr<DataAccessor>)>& aCallback) = 0;

        /*
         * Starts the main loop. Update callbacks will be called at the specified refresh rate.
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
