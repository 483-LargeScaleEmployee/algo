cmake_minimum_required(VERSION 3.10)
project(YourProjectName C)

set(CMAKE_C_STANDARD 11)

# Detect the operating system
if(WIN32)
    set(OS_SUBDIR "windows")
elseif(APPLE)
    set(OS_SUBDIR "mac")
elseif(UNIX)
    set(OS_SUBDIR "ubuntu")
else()
    message(FATAL_ERROR "Unsupported operating system")
endif()

# Set the output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/${OS_SUBDIR})

# Include directories
include_directories(include)

# Source files
file(GLOB_RECURSE SOURCES "src/*.c")

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})
