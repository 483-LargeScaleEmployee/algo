#!/bin/bash

# Project structure configuration
SRC_DIR="src"
INCLUDE_DIR="include"
GLPK_DIR="glpk-4.65"
GLPK_DIR_W64="glpk-4.65/w64"  # Where GLPK files will be
OUTPUT="algo.exe"

# Compiler configuration
MINGW=x86_64-w64-mingw32-gcc
CFLAGS="-I${INCLUDE_DIR} -I${GLPK_DIR}/src -O2"
LDFLAGS="-L${GLPK_DIR_W64} -lglpk_4_65"

# Download GLPK if not present
if [ ! -d "${GLPK_DIR_W64}" ]; then
    echo "Downloading GLPK..."
    wget https://sourceforge.net/projects/winglpk/files/winglpk/GLPK-4.65/winglpk-4.65.zip
    unzip winglpk-4.65.zip
fi

# Copy DLL to execution directory if not present
if [ ! -f "glpk_4_65.dll" ]; then
    echo "Copying GLPK DLL..."
    cp ${GLPK_DIR_W64}/glpk_4_65.dll ./
fi

# Find all .c files in src directory
SRC_FILES=$(find ${SRC_DIR} -name "*.c")

# Compile the program
echo "Building ${OUTPUT}..."
${MINGW} ${CFLAGS} -o ${OUTPUT} ${SRC_FILES} ${LDFLAGS}

# Check if compilation succeeded
if [ $? -eq 0 ]; then
    echo "Build successful! Created ${OUTPUT}"
    echo "Make sure glpk_4_65.dll is in the same directory as ${OUTPUT}"
else
    echo "Build failed!"
    exit 1
fi
