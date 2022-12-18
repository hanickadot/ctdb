cmake_minimum_required(VERSION 3.14)

project(CTDB VERSION 1.0 LANGUAGES CXX)

list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/support/cmake")

include(colors)
include(pedantic)

add_subdirectory(include)


option(CTDB_TESTS "Enable CTDB testing" ON)

if(CTDB_TESTS)
  add_subdirectory(tests)
endif()