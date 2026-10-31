cmake_minimum_required(VERSION 3.20)

project(c_lab
    VERSION 0.1.0
    DESCRIPTION "C Lab: 从能跑到跑对"
    LANGUAGES C
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CompilerWarnings)
include(Sanitizers)

option(CLAB_BUILD_TESTS "Build test executables" ON)

if(CLAB_BUILD_TESTS)
    enable_testing()
endif()

add_subdirectory(common)

# 各章的项目按阶段逐步加入；第 00 章基线为 Windows-only，不进入构建。
add_subdirectory(projects/01-webserver/stages/01-portable)
