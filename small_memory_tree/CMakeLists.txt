#
# Copyright (c) 2024 Waldemar Schneider (w-schneider1987 at web dot de)
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

add_library(small_memory_tree INTERFACE)

add_library(myproject::small_memory_tree ALIAS small_memory_tree)

find_package(Boost)
find_package(st_tree)
find_package(confu_algorithm)

target_link_libraries(
  small_memory_tree INTERFACE st_tree::st_tree Boost::headers
                              confu_algorithm::confu_algorithm)

target_include_directories(
  small_memory_tree INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
                              $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>)

target_compile_features(small_memory_tree INTERFACE cxx_std_20)

install(FILES smallMemoryTree.hxx util.hxx smallMemoryTreeLotsOfChildren.hxx
        DESTINATION include/small_memory_tree)
install(TARGETS small_memory_tree DESTINATION lib)
