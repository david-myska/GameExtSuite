
set(SOURCE_FILES
				"game_enhancer/impl/memory_layout.cpp"
)

set(HEADER_FILES
				"game_enhancer/impl/memory_layout.h"
)

set(API_FILES
				"game_enhancer/memory_layout.h"
)

add_library(game_enhancer STATIC ${SOURCE_FILES})

set_property(TARGET game_enhancer PROPERTY CXX_STANDARD 20)

target_include_directories(game_enhancer PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${GE_INSTALL_INCLUDE_DIR}>
)

install(TARGETS game_enhancer DESTINATION ${GE_INSTALL_LIB_DIR})
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/game_enhancer
    DESTINATION ${GE_INSTALL_INCLUDE_DIR}
    FILES_MATCHING
        PATTERN "*.h"
    PATTERN "impl" EXCLUDE
)
