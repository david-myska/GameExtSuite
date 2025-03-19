#include "game_enhancer/impl/memory_processor.h"

#include <memory>
#include <string>

namespace GE
{
    void MemoryProcessorImpl::ReadLayout(const std::string& aLayoutType, size_t aFromAddress) {
        // Get space from MemoryStorage to write the memory to
        // Read the layout from the address
        // Resolve pointers
        // - If the pointer isn't resolved yet (at first just direct pointers, later also check if in range of the read memory)
        //   - Read its layout
        //   - Add mapping to the PointerResolver
        // Overwrite the pointer with the resolved address
        // Call the OnRefresh callback
    }

    void MemoryProcessorImpl::RegisterLayout(const std::string& aLayoutType, LayoutBuilder::Absolute::Layout aLayout)
    {
        // register layout into the storage, to know what size to allocate
    }

    void MemoryProcessorImpl::SetRefreshRate(size_t aRateMs) {}

    void MemoryProcessorImpl::OnRefresh(const std::string& aLayoutType, std::function<void(const DataStuff&)>) {}

    // TODO somewhere the connection to the Process needs to be done
    void MemoryProcessorImpl::Start() {}

    void MemoryProcessorImpl::Stop() {}
}
