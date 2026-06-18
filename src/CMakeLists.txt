add_library(kvik_core INTERFACE)

target_include_directories(kvik_core INTERFACE
    ${CMAKE_SOURCE_DIR}/include
)

add_executable(kvik main.cpp)
target_link_libraries(kvik PRIVATE INTERFACE cxx_std_23)