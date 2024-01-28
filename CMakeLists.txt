cmake_minimum_required(VERSION 3.15.7)
project(project VERSION 1.0.0
        DESCRIPTION project
        LANGUAGES CXX
        )
include(cmake/Cache.cmake)
add_library(project_warning INTERFACE)
include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warning)
add_library(project_options INTERFACE)
include(cmake/StaticAnalyzers.cmake)
include(cmake/Sanitizers.cmake)
enable_sanitizers(project_options)
add_subdirectory(small_memory_tree)
OPTION(BUILD_TESTS "enable tests" OFF)
IF (BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
ENDIF (BUILD_TESTS)
