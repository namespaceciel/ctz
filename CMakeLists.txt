cmake_minimum_required(VERSION 3.5)
project(CTZ LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CTZ_LIST
        src/config.cpp
)

add_library(CTZ ${CTZ_LIST})

target_include_directories(CTZ PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(CTZ ciellab)

add_subdirectory(third_party/ciellab)

#string(COMPARE EQUAL ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR} PROJECT_IS_TOP_LEVEL)
#
#option(CTZ_BUILD_TESTS "Build unit tests" ${PROJECT_IS_TOP_LEVEL})
#
#if (CTZ_BUILD_TESTS)
#    enable_testing()
#    add_subdirectory(test)
#endif ()