
# 如果找不到 GTest，则使用 FetchContent 下载和构建库
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.13.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(byte_view_test 
    byte_view_test.cpp
)
target_include_directories(byte_view_test PRIVATE 
    ${PROJECT_SOURCE_DIR}/include
)
target_link_libraries(byte_view_test PRIVATE
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(byte_view_test)