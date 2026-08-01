cmake_minimum_required(VERSION 3.20)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

project(kvik LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(lib)

add_subdirectory(src)

target_compile_options(kvik PRIVATE -Wall -Wextra -Werror)

enable_testing()
add_subdirectory(tests)
