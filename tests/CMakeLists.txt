message(STATUS "CTDB testing enabled")

find_package(Catch2 3 REQUIRED)

file(GLOB_RECURSE TESTS_SOURCES  LINK_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp")

add_executable(tests ${TESTS_SOURCES})

target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)
target_link_libraries(tests PUBLIC ctdb)
target_compile_features(tests PUBLIC cxx_std_20)

add_custom_target(test tests DEPENDS tests)