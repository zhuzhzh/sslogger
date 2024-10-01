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
set(SPDLOG_USE_STATIC_LIBS OFF)
find_package(spdlog REQUIRED)

# 在 find_package 之后添加这些行
if(NOT spdlog_FOUND)
    message(FATAL_ERROR "spdlog not found")
endif()

message(STATUS "spdlog include dir: ${spdlog_INCLUDE_DIRS}")
message(STATUS "spdlog libraries: ${spdlog_LIBRARIES}")

# 源文件
set(SOURCES src/sslogger.cc)

# 头文件
set(HEADERS include/ssln/sslogger.h)

# 可选头文件
set(OPTIONAL_HEADERS
    include/tl/optional.hpp
    include/nonstd/string_view.hpp
)

# 创建共享库
add_library(sslogger SHARED ${SOURCES})

# 设置包含目录
target_include_directories(sslogger PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    ${spdlog_INCLUDE_DIRS}
)

# 链接依赖库
target_link_libraries(sslogger PUBLIC fmt::fmt spdlog::spdlog)

# 添加编译定义
target_compile_definitions(sslogger PRIVATE
    SPDLOG_FMT_EXTERNAL
    SPDLOG_COMPILED_LIB
)

# 确保生成的共享库包含运行时路径信息
set_target_properties(sslogger PROPERTIES
    INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib64:/home/public/spdlog/lib64"
    BUILD_WITH_INSTALL_RPATH TRUE
    LINK_FLAGS "-Wl,--no-as-needed -Wl,--verbose"
)

# 安装选项
option(INSTALL_OPTIONAL_HEADERS "Install optional headers" ON)

# 安装库
install(TARGETS sslogger
    EXPORT ssloggerTargets
    LIBRARY DESTINATION lib64
    ARCHIVE DESTINATION lib64
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

# 安装主头文件
install(FILES ${HEADERS} DESTINATION include/ssln)

# 安装可选头文件
if(INSTALL_OPTIONAL_HEADERS)
    install(FILES ${OPTIONAL_HEADERS} DESTINATION include)
endif()

# 导出目标
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

# 打印链接库的信息
get_target_property(LINKED_LIBS sslogger LINK_LIBRARIES)
message(STATUS "Linked libraries: ${LINKED_LIBS}")

# 打印编译命令
set(CMAKE_VERBOSE_MAKEFILE ON)

# 启用测试
enable_testing()

# 添加测试目录
add_subdirectory(tests)