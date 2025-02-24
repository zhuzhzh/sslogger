cmake_minimum_required(VERSION 3.10)
project(sslogger VERSION 1.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set install prefix
if(NOT CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX "/home/public/ssln")
endif()

list(APPEND CMAKE_PREFIX_PATH "/home/public/quill")

# Find dependencies
find_package(quill REQUIRED)

# Source files
set(SSLOGGER_SOURCES
    src/sslogger.cc
)

set(CMAKE_BUILD_TYPE Debug)

# Create library
add_library(sslogger ${SSLOGGER_SOURCES})

# Set include directories
target_include_directories(sslogger
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Link dependencies
target_link_libraries(sslogger
    PUBLIC
        quill::quill
)

# Add compile definitions
target_compile_definitions(sslogger 
    PUBLIC
    QUILL_COMPILE_ACTIVE_LOG_LEVEL=QUILL_COMPILE_ACTIVE_LOG_LEVEL_TRACE_L3
)

# Installation options
option(BUILD_EXAMPLES "Build example programs" ON)

# Install headers
install(DIRECTORY include/ssln
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

# Install library and export targets
install(TARGETS sslogger
    EXPORT ssloggerTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(EXPORT ssloggerTargets
    FILE ssloggerTargets.cmake
    NAMESPACE sslogger::
    DESTINATION lib/cmake/sslogger
)

# Create config files
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

# Install config files
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/ssloggerConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/ssloggerConfigVersion.cmake"
    DESTINATION lib/cmake/sslogger
)

# Add examples
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Enable testing
enable_testing()
add_subdirectory(tests)
