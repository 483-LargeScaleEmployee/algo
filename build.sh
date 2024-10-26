#!/bin/bash

# Project structure configuration
SRC_DIR="src"
INCLUDE_DIR="include"
OUTPUT="algo.exe"

# Compiler configuration
# sudo apt-get install mingw-w64
MINGW=x86_64-w64-mingw32-gcc
CFLAGS="-I${INCLUDE_DIR} -O2"

# Find all .c files in src directory
SRC_FILES=$(find ${SRC_DIR} -name "*.c")

# Compile the program
echo "Building ${OUTPUT}..."
${MINGW} ${CFLAGS} -o ${OUTPUT} ${SRC_FILES}

# Check if compilation succeeded
if [ $? -eq 0 ]; then
    echo "Build successful! Created ${BUILD_DIR}/${OUTPUT}"
else
    echo "Build failed!"
    exit 1
fi
