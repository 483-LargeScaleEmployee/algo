#!/bin/bash

# Navigate to the project root
cd "$(dirname "$0")/.."

# Create build directory if it doesn't exist
mkdir -p build/mac

# Navigate to build directory
cd build/mac

# Generate Makefile
cmake ../..

# Build the project
make

# Check if build was successful
if [ $? -eq 0 ]; then
    echo "Build successful! Executable is in the build/mac directory."
else
    echo "Build failed. Please check the error messages above."
fi
