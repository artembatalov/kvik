include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/releases/download/v1.16.0/googletest-1.16.0.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP ON
    FIND_PACKAGE_ARGS NAMES GTest
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
    database-tests
    parser_tests.cpp
    database_test.cpp
    data_test.cpp
    type_test.cpp
    utils_test.cpp
)

target_link_libraries(
    database-tests
    GTest::gtest_main
    GTest::gmock_main
)

target_include_directories(database-tests PUBLIC ${PROJECT_SOURCE_DIR})

include(GoogleTest)

gtest_discover_tests(database-tests)