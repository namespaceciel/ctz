cmake_minimum_required(VERSION 3.5)
project(CTZ LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(CTZ
        src/config.cpp
        src/task.cpp
        src/thread.cpp
)

target_include_directories(CTZ PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(CTZ ciellab)

add_subdirectory(third_party/ciellab)

string(COMPARE EQUAL ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR} PROJECT_IS_TOP_LEVEL)

# unit tests
option(CTZ_BUILD_TESTS "Build unit tests" ${PROJECT_IS_TOP_LEVEL})

if (CTZ_BUILD_TESTS)
    enable_testing()

    add_executable(CTZ_TEST
            test/task_test.cpp
            test/thread_test.cpp
    )

    add_subdirectory(third_party/googletest)

    target_link_libraries(CTZ_TEST CTZ GTest::gtest_main)
    include(GoogleTest)
    gtest_discover_tests(CTZ_TEST)

    target_compile_options(CTZ_TEST PUBLIC -Wall -Wextra -Werror)
endif ()