# 示例程序的 CMakeLists.txt
add_executable(example_basic example_basic.cc)
target_link_libraries(example_basic 
    PRIVATE 
        sslogger
        spdlog::spdlog
        fmt::fmt
)

add_executable(example_advanced example_advanced.cc)
target_link_libraries(example_advanced 
    PRIVATE 
        sslogger
        spdlog::spdlog
        fmt::fmt
)

add_executable(example_stopwatch example_stopwatch.cc)
target_link_libraries(example_stopwatch 
    PRIVATE 
        sslogger
        spdlog::spdlog
        fmt::fmt
)

# 添加运行时库搜索路径
set_target_properties(example_basic example_advanced example_stopwatch
    PROPERTIES
    INSTALL_RPATH "${CMAKE_BINARY_DIR}"
    BUILD_WITH_INSTALL_RPATH TRUE
)

# Ensure examples can find header files
target_include_directories(example_basic
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

target_include_directories(example_advanced
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

target_include_directories(example_stopwatch
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)