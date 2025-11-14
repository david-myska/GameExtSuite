
# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        6910c9d9165801d8827d628cb72eb7ea9dd538c5 # v1.16.0
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(ge_tests "ge_test.cpp" "fixtures/ge_fixture.cpp")

target_link_libraries(
  ge_tests
  game_ext_suite
  GTest::gtest_main
  GTest::gmock_main
  spdlog::spdlog
)

install(TARGETS ge_tests DESTINATION ${GE_INSTALL_BIN_DIR})

include(GoogleTest)
gtest_discover_tests(ge_tests)
