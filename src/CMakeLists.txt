
include(FetchContent)

FetchContent_Declare(
  pma
  GIT_REPOSITORY https://github.com/david-myska/ProcessMemoryAccess.git
  GIT_TAG        v1.0.0
)
FetchContent_MakeAvailable(pma)

FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG        v1.15.3
)
FetchContent_MakeAvailable(spdlog)

set(SOURCE_FILES
				"game_enhancer/impl/memory_processor.cpp"
				"game_enhancer/impl/data_accessor.cpp"
				"game_enhancer/impl/layout/memory_layout_builder.cpp"
				"game_enhancer/impl/layout/frame_memory_storage.cpp"
				"game_enhancer/impl/achis/conditions.cpp"
				"game_enhancer/impl/backup/backup_engine.cpp"
)

set(HEADER_FILES
				"game_enhancer/impl/memory_processor.h"
				"game_enhancer/impl/data_accessor.h"
				"game_enhancer/impl/layout/memory_layout_builder.h"
				"game_enhancer/impl/layout/frame_memory_storage.h"
				"game_enhancer/impl/backup/backup_engine.h"
)

set(API_FILES
				"game_enhancer/memory_layout_builder.h"
				"game_enhancer/memory_processor.h"
				"game_enhancer/data_accessor.h"
				"game_enhancer/backup/backup_engine.h"
)

add_library(game_ext_suite STATIC ${SOURCE_FILES})

set_property(TARGET game_ext_suite PROPERTY CXX_STANDARD 20)

target_include_directories(game_ext_suite PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${GE_INSTALL_INCLUDE_DIR}>
	PUBLIC ${spdlog_SOURCE_DIR}/include
)

target_link_libraries(game_ext_suite PUBLIC PMA::pma spdlog::spdlog)


install(TARGETS game_ext_suite DESTINATION ${GE_INSTALL_LIB_DIR})
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/game_ext_suite
    DESTINATION ${GE_INSTALL_INCLUDE_DIR}
    FILES_MATCHING
        PATTERN "*.h"
    PATTERN "impl" EXCLUDE
)
