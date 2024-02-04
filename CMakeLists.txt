cmake_minimum_required(VERSION 3.5)
project(CTZ LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        REQUIRED)

set_source_files_properties(
        src/osfiber_asm_aarch64.S
        src/osfiber_asm_arm.S
        src/osfiber_asm_loongarch64.S
        src/osfiber_asm_mips64.S
        src/osfiber_asm_ppc64.S
        src/osfiber_asm_rv64.S
        src/osfiber_asm_x64.S
        src/osfiber_asm_x86.S
        PROPERTIES LANGUAGE C
)

add_library(CTZ
        src/config.cpp
        src/fiber.cpp
        src/scheduler.cpp
        src/worker.cpp
        src/osfiber_asm.cpp
        src/osfiber_windows.cpp
        src/osfiber_aarch64.c
        src/osfiber_arm.c
        src/osfiber_asm_aarch64.S
        src/osfiber_asm_arm.S
        src/osfiber_asm_loongarch64.S
        src/osfiber_asm_mips64.S
        src/osfiber_asm_ppc64.S
        src/osfiber_asm_rv64.S
        src/osfiber_asm_x64.S
        src/osfiber_asm_x86.S
        src/osfiber_loongarch64.c
        src/osfiber_mips64.c
        src/osfiber_ppc64.c
        src/osfiber_rv64.c
        src/osfiber_x64.c
        src/osfiber_x86.c
        src/osfiber_emscripten.cpp
        src/waitgroup.cpp
        src/conditionvariable.cpp
        src/event.cpp
        src/ticket.cpp
)

target_include_directories(CTZ PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(CTZ
        ciellab
        luaopener
        Qt::Core
        Qt::Gui
        Qt::Widgets
)

add_subdirectory(third_party/ciellab)
add_subdirectory(third_party/luaopener)

# unit tests
option(CTZ_BUILD_TESTS "Build unit tests" ${PROJECT_IS_TOP_LEVEL})

if (CTZ_BUILD_TESTS)
    enable_testing()

    add_executable(CTZ_TEST
            test/osfiber_test.cpp
            test/scheduler_test.cpp
            test/waitgroup_test.cpp
            test/event_test.cpp
            test/dag_test.cpp
            test/ticket_test.cpp
    )

    add_subdirectory(third_party/googletest)

    target_link_libraries(CTZ_TEST
            CTZ
            GTest::gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(CTZ_TEST)

    target_compile_options(CTZ_TEST PUBLIC -Wall -Wextra -Werror)
endif ()

# qt gui TODO
if (false)
add_executable(CTZ_QT
        qt/main.cpp
        qt/scheduler_window.cpp
        qt/work_widget.cpp
        qt/new_built.cpp
)

target_link_libraries(CTZ_QT CTZ)
endif ()