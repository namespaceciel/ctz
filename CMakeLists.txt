cmake_minimum_required(VERSION 3.12)
project(ctz LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose build type: Debug/Release/RelWithDebInfo/MinSizeRel." FORCE)
endif (NOT CMAKE_BUILD_TYPE)

set(FETCHCONTENT_BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party")

include(FetchContent)
FetchContent_Declare(
    ciellab
    GIT_REPOSITORY https://github.com/namespaceciel/ciellab.git
    GIT_TAG        8eafa0473da058280c286c53b22f3aba3484ace1
)
FetchContent_MakeAvailable(ciellab)

option(CTZ_BUILD_TESTS    "Build unit tests" ${PROJECT_IS_TOP_LEVEL})
option(CTZ_BUILD_EXAMPLES "Build examples"   ${PROJECT_IS_TOP_LEVEL})

set_source_files_properties(
    src/osfiber_aarch64.S
    src/osfiber_arm.S
    src/osfiber_x64.S
    PROPERTIES LANGUAGE C
)

add_library(ctz STATIC
    src/conditionvariable.cpp
    src/event.cpp
    src/fiber.cpp
    src/osfiber_aarch64.c
    src/osfiber_aarch64.S
    src/osfiber_arm.c
    src/osfiber_arm.S
    src/osfiber_x64.c
    src/osfiber_x64.S
    src/osfiber.cpp
    src/ticket.cpp
    src/waitgroup.cpp
    src/worker.cpp
)

target_include_directories(ctz PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(ctz
    ciellab
)

target_compile_options(ctz PUBLIC -fno-exceptions)

# unit tests

if (CTZ_BUILD_TESTS)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        58d77fa8070e8cec2dc1ed015d66b454c8d78850 # release-1.12.1, last version supporting C++11
    )
    FetchContent_MakeAvailable(googletest)

    enable_testing()

    add_executable(ctz_test
        test/dag_test.cpp
        test/event_test.cpp
        test/osfiber_test.cpp
        test/scheduler_test.cpp
        test/ticket_test.cpp
        test/waitgroup_test.cpp
    )

    target_link_libraries(ctz_test
        ctz
        GTest::gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(ctz_test)

    target_compile_options(ctz_test PUBLIC -Wall -Wextra -Werror)
endif ()

# examples

if(CTZ_BUILD_EXAMPLES)
    function(build_example target)
        add_executable(ctz_${target} example/${target}.cpp)
        target_link_libraries(ctz_${target} ctz)
        target_compile_options(ctz_${target} PUBLIC -Wall -Wextra -Werror)
    endfunction(build_example)

    build_example(event_example)
    build_example(ticket_example)
    build_example(waitgroup_example)
endif(CTZ_BUILD_EXAMPLES)
