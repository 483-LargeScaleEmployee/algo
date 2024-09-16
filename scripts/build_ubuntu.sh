#!/bin/bash

# Navigate to the project root
cd "$(dirname "$0")/.."

# Create build directory if it doesn't exist
mkdir -p build/ubuntu

# Navigate to build directory
cd build/ubuntu

# Generate Makefile
cmake ../..

# Build the project
make

# Check if build was successful
if [ $? -eq 0 ]; then
    echo "Build successful! Executable is in the build/ubuntu directory."
else
    echo "Build failed. Please check the error messages above."
fi
