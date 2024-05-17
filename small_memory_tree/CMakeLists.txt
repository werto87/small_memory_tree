#
# Copyright (c) 2024 Waldemar Schneider (w-schneider1987 at web dot de)
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE.md or copy at http://www.boost.org/LICENSE_1_0.txt)

add_library(small_memory_tree INTERFACE)


find_package(Boost)
find_package(confu_algorithm)
set(OptionalLibrary)
if(WITH_ST_TREE)
target_compile_definitions(small_memory_tree INTERFACE SMALL_MEMORY_TREE_WITH_ST_TREE)
  find_package(st_tree)
  list(APPEND OptionalLibrary st_tree::st_tree)
endif()
if(WITH_STLPLUS_TREE)
  target_compile_definitions(small_memory_tree INTERFACE SMALL_MEMORY_TREE_WITH_STLPLUS_TREE)
  find_package(stlplus)
  list(APPEND OptionalLibrary stlplus::stlplus)
endif()


set(LibraryToAlwaysLink Boost::headers confu_algorithm::confu_algorithm )
target_link_libraries(small_memory_tree INTERFACE ${LibraryToAlwaysLink} ${OptionalLibrary})

target_include_directories(small_memory_tree INTERFACE ${CMAKE_SOURCE_DIR})

target_compile_features(small_memory_tree INTERFACE cxx_std_20)

set(HeaderToAlwaysInstall smallMemoryTree.hxx smallMemoryTreeAdapter.hxx)
set(OptionalHeader)
if(WITH_ST_TREE)
  list(APPEND OptionalHeader stTree.hxx) 
endif()
if(WITH_STLPLUS_TREE)
  list(APPEND OptionalHeader stlplusTree.hxx) 
endif()
install(FILES ${HeaderToAlwaysInstall} ${OptionalHeader} DESTINATION include/small_memory_tree)


install(TARGETS small_memory_tree DESTINATION lib)
