add_library(ctdb INTERFACE)

target_compile_features(ctdb INTERFACE cxx_std_23)
target_include_directories(ctdb INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})