add_library(kvik_core INTERFACE)
add_library(kvik::core ALIAS kvik_core)

target_include_directories(kvik_core INTERFACE ${CMAKE_SOURCE_DIR}/include)
target_compile_features(kvik_core INTERFACE cxx_std_23)

add_executable(kvik main.cpp)
target_link_libraries(kvik PRIVATE kvik_core)