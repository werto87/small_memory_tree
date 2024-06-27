# Copyright (c) 2024 Waldemar Schneider (w-schneider1987 at web dot de)
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.md or copy at http://www.boost.org/LICENSE_1_0.txt)
 

add_executable(_test benchmarkTest.cxx stlplusTree.cxx
stTree.cxx)
find_package(Catch2)
target_link_libraries(_test myproject_options myproject_warnings
                      small_memory_tree Catch2::Catch2WithMain)
target_include_directories(_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
                                         ${CMAKE_SOURCE_DIR})

add_test(NAME _test COMMAND _test -r junit)
