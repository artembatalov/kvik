cmake_minimum_required(VERSION 3.20)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

project(kvik LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(src)

if(MSVC)
    target_compile_options(kvik PRIVATE /W4)
else()
    target_compile_options(kvik PRIVATE -Wall -Wextra)
endif()

enable_testing()
add_subdirectory(tests)
