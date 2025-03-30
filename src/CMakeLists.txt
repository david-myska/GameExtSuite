
include(FetchContent)

FetchContent_Declare(
  pma
  GIT_REPOSITORY https://github.com/david-myska/ProcessMemoryAccess.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(pma)

set(SOURCE_FILES
				"game_enhancer/impl/memory_processor.cpp"
				"game_enhancer/impl/frame_accessor.cpp"
				"game_enhancer/impl/layout/memory_layout_builder.cpp"
				"game_enhancer/impl/layout/frame_memory_storage.cpp"
)

set(HEADER_FILES
				"game_enhancer/impl/memory_processor.h"
				"game_enhancer/impl/frame_accessor.h"
				"game_enhancer/impl/layout/memory_layout_builder.h"
				"game_enhancer/impl/layout/frame_memory_storage.h"
)

set(API_FILES
				"game_enhancer/memory_layout_builder.h"
				"game_enhancer/memory_processor.h"
				"game_enhancer/frame_accessor.h"
)

add_library(game_enhancer STATIC ${SOURCE_FILES})

set_property(TARGET game_enhancer PROPERTY CXX_STANDARD 20)

target_include_directories(game_enhancer PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${GE_INSTALL_INCLUDE_DIR}>
)

target_link_libraries(game_enhancer PUBLIC PMA::pma)


install(TARGETS game_enhancer DESTINATION ${GE_INSTALL_LIB_DIR})
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/game_enhancer
    DESTINATION ${GE_INSTALL_INCLUDE_DIR}
    FILES_MATCHING
        PATTERN "*.h"
    PATTERN "impl" EXCLUDE
)
