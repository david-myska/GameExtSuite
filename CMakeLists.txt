cmake_minimum_required (VERSION 3.20)

project ("GameExtSuite")

set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})

set(GE_INSTALL_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(GE_INSTALL_BIN_DIR ${PROJECT_SOURCE_DIR}/bin)
set(GE_INSTALL_LIB_DIR ${PROJECT_SOURCE_DIR}/lib)

add_subdirectory(src)
add_subdirectory(test)

