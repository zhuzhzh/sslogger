list(APPEND CMAKE_PREFIX_PATH "/home/public/fmt" "/home/public/spdlog" "/home/public/googletest" "/home/public/ssln")

# 查找 GTest 包
find_package(GTest REQUIRED)

# 包含 Google Test 的头文件
include_directories(${GTEST_INCLUDE_DIRS})

# 获取所有测试文件
file(GLOB_RECURSE TEST_SOURCES "test_*.cc")

# 为每个测试文件创建一个可执行文件
foreach(TEST_FILE ${TEST_SOURCES})
    # 获取文件名（不包含扩展名）作为测试名
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    
    # 添加可执行文件
    add_executable(${TEST_NAME} ${TEST_FILE})
    
    # 链接 Google Test 和您的日志库
    target_link_libraries(${TEST_NAME} 
        PRIVATE 
        GTest::GTest 
        GTest::Main
        sslogger
    )
    
    # 将测试添加到 CTest
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()

# 可选：设置一些通用的编译选项
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 可选：为所有测试添加包含目录
include_directories(${CMAKE_SOURCE_DIR}/include)