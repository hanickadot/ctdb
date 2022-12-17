message(STATUS "CTDB testing enabled")

find_package(Catch2 3 REQUIRED)

add_executable(tests test.cpp)

target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)

add_custom_target(test tests DEPENDS tests)