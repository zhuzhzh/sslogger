cmake_minimum_required(VERSION 3.10)
project(sslogger VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置安装前缀
if(NOT CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX "/home/public/ssln")
endif()

list(APPEND CMAKE_PREFIX_PATH "/home/public/fmt" "/home/public/spdlog")

# 查找依赖库
find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)

# 在 find_package 之后添加这些行
if(NOT spdlog_FOUND)
    message(FATAL_ERROR "spdlog not found")
endif()

message(STATUS "spdlog include dir: ${spdlog_INCLUDE_DIRS}")
message(STATUS "spdlog libraries: ${spdlog_LIBRARIES}")

# 创建 INTERFACE 库（header-only 库）
add_library(sslogger INTERFACE)

# 设置包含目录
target_include_directories(sslogger 
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# 链接依赖库
target_link_libraries(sslogger 
    INTERFACE 
        fmt::fmt 
        spdlog::spdlog
)

# 添加编译定义
target_compile_definitions(sslogger 
    INTERFACE
        SPDLOG_FMT_EXTERNAL
        SPDLOG_COMPILED_LIB
)

# 安装选项
option(BUILD_EXAMPLES "Build example programs" ON)

# 安装头文件
install(DIRECTORY include/ssln
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

# 导出目标
install(TARGETS sslogger
    EXPORT ssloggerTargets
    INCLUDES DESTINATION include
)

install(EXPORT ssloggerTargets
    FILE ssloggerTargets.cmake
    NAMESPACE sslogger::
    DESTINATION lib/cmake/sslogger
)

# 创建配置文件
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/ssloggerConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ssloggerConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/ssloggerConfig.cmake"
    INSTALL_DESTINATION lib/cmake/sslogger
)

# 安装配置文件
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/ssloggerConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/ssloggerConfigVersion.cmake"
    DESTINATION lib/cmake/sslogger
)

# 添加示例程序
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# 启用测试
enable_testing()
add_subdirectory(tests)