# Add path to find packages
list(APPEND CMAKE_PREFIX_PATH "/home/public/quill" "/home/public/googletest")

# Find required packages
find_package(GTest REQUIRED)
find_package(quill REQUIRED)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set build type
set(CMAKE_BUILD_TYPE Debug)

# Test sources
set(TEST_SOURCES
    test_basic.cc
    test_hex_logging.cc
    test_stopwatch.cc
    test_verbosity.cc
    test_env.cc
)

# Add test executables
foreach(test_source ${TEST_SOURCES})
    # Get the test name without extension
    get_filename_component(test_name ${test_source} NAME_WE)
    
    # Add executable if it doesn't exist
    if(NOT TARGET ${test_name})
        add_executable(${test_name} ${test_source})
        
        # Link with sslogger and GTest
        target_link_libraries(${test_name}
            PRIVATE
                sslogger
                GTest::GTest
                GTest::Main
        )
        
        # Set compile definitions for tests
        #target_compile_definitions(${test_name}
        #    PRIVATE
        #        QUILL_COMPILE_ACTIVE_LOG_LEVEL=QUILL_COMPILE_ACTIVE_LOG_LEVEL_TRACE_L3
        #)
        
        # Add test to CTest
        add_test(
            NAME ${test_name}
            COMMAND ${test_name}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
    endif()
endforeach()

# Create test logs directory
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/logs)
