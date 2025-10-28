cmake_minimum_required(VERSION 3.10)
project(HikRobotCamera VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# HikRobot MVS SDK paths
# Default installation path on Ubuntu
set(MVS_SDK_PATH "/opt/MVS" CACHE PATH "Path to HikRobot MVS SDK")

# Include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${MVS_SDK_PATH}/include
)

# Library directories
link_directories(
    ${MVS_SDK_PATH}/lib/64
)

# Optional: OpenCV support
option(USE_OPENCV "Build with OpenCV support for image display/save" ON)

if(USE_OPENCV)
    find_package(OpenCV QUIET)
    if(OpenCV_FOUND)
        message(STATUS "OpenCV found: ${OpenCV_VERSION}")
        include_directories(${OpenCV_INCLUDE_DIRS})
        add_definitions(-DUSE_OPENCV)
    else()
        message(WARNING "OpenCV not found. Building without OpenCV support.")
        set(USE_OPENCV OFF)
    endif()
endif()

# Source files
set(CAMERA_SOURCES
    cpp/HikRobotCamera.cpp
)

# Create library
add_library(hikrobot_camera SHARED ${CAMERA_SOURCES})
target_link_libraries(hikrobot_camera
    MvCameraControl
    pthread
)

# Example executable
add_executable(cpp_example examples/cpp_example.cpp)
target_link_libraries(cpp_example
    hikrobot_camera
    MvCameraControl
    pthread
)

if(USE_OPENCV AND OpenCV_FOUND)
    target_link_libraries(cpp_example ${OpenCV_LIBS})
endif()

# Install targets
install(TARGETS hikrobot_camera cpp_example
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

# Print configuration summary
message(STATUS "")
message(STATUS "==================== Configuration Summary ====================")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "MVS SDK path: ${MVS_SDK_PATH}")
message(STATUS "OpenCV support: ${USE_OPENCV}")
if(USE_OPENCV AND OpenCV_FOUND)
    message(STATUS "OpenCV version: ${OpenCV_VERSION}")
    message(STATUS "OpenCV libs: ${OpenCV_LIBS}")
endif()
message(STATUS "===============================================================")
message(STATUS "")
