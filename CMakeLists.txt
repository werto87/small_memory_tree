# Copyright (c) 2024 Waldemar Schneider (w-schneider1987 at web dot de)
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.md or copy at http://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.15.7)
if(NOT DEFINED CMAKE_CXX_STANDARD) # Only set the cxx_standard if it is not set
                                   # by someone else
  set(CMAKE_CXX_STANDARD 20)
endif()
project(
  myproject
  VERSION 0.0.1
  LANGUAGES CXX C)

include(cmake/PreventInSourceBuilds.cmake)
include(ProjectOptions.cmake)
myproject_setup_options()
myproject_local_options()
target_compile_features(myproject_options
                        INTERFACE cxx_std_${CMAKE_CXX_STANDARD})
add_library(myproject::myproject_options ALIAS myproject_options)
add_library(myproject::myproject_warnings ALIAS myproject_warnings)
add_subdirectory(small_memory_tree)
option(BUILD_TESTS "enable tests" OFF)
if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(test)
endif(BUILD_TESTS)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_custom_target(copy_compile_commands_json ALL
                  DEPENDS ${CMAKE_SOURCE_DIR}/compile_commands.json)

add_custom_command(
  DEPENDS ${CMAKE_BINARY_DIR}/compile_commands.json
  OUTPUT ${CMAKE_SOURCE_DIR}/compile_commands.json
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/compile_commands.json
          ${CMAKE_SOURCE_DIR}/compile_commands.json
  COMMENT
    "copy compile_commands.json build to compile_commands.json project root so tools can find it more easily"
)
