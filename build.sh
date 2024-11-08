#!/bin/bash

# Project structure configuration
SRC_DIR="src"
TEST_DIR="tests"  # New test directory
INCLUDE_DIR="include"
GLPK_DIR="glpk-4.65"
GLPK_DIR_W64="glpk-4.65/w64"
OUTPUT="algo.exe"
TEST_OUTPUT="run_tests.exe"  # New test executable

# Compiler configuration
MINGW=x86_64-w64-mingw32-gcc
CFLAGS="-I${INCLUDE_DIR} -I${GLPK_DIR}/src -O2"
LDFLAGS="-L${GLPK_DIR_W64} -lglpk_4_65"

# Function to download and setup GLPK
setup_glpk() {
    if [ ! -d "${GLPK_DIR_W64}" ]; then
        echo "Downloading GLPK..."
        wget https://sourceforge.net/projects/winglpk/files/winglpk/GLPK-4.65/winglpk-4.65.zip
        unzip winglpk-4.65.zip
    fi

    if [ ! -f "glpk_4_65.dll" ]; then
        echo "Copying GLPK DLL..."
        cp ${GLPK_DIR_W64}/glpk_4_65.dll ./
    fi
}

# Function to build main program
build_main() {
    echo "Building ${OUTPUT}..."
    SRC_FILES=$(find ${SRC_DIR} -name "*.c" ! -name "*_test.c")
    ${MINGW} ${CFLAGS} -o ${OUTPUT} ${SRC_FILES} ${LDFLAGS}
    return $?
}

# Function to build and run tests
build_and_run_tests() {
    echo "Building tests..."

    # Find all test files
    TEST_FILES=$(find ${TEST_DIR} -name "*_test.c")
    SRC_FILES=$(find ${SRC_DIR} -name "*.c" ! -name "main.c" ! -name "*_test.c")

    # Add test-specific flags
    TEST_CFLAGS="${CFLAGS} -I${TEST_DIR}"

    # Compile tests
    ${MINGW} ${TEST_CFLAGS} -o ${TEST_OUTPUT} ${TEST_FILES} ${SRC_FILES} ${LDFLAGS}

    if [ $? -eq 0 ]; then
        echo "Test build successful! Running tests..."
        wine ./${TEST_OUTPUT}
        return $?
    else
        echo "Test build failed!"
        return 1
    fi
}

# Main execution
setup_glpk

# Process command line arguments
case "$1" in
    "test")
        build_and_run_tests
        ;;
    *)
        build_main
        ;;
esac

# Check final status
if [ $? -eq 0 ]; then
    echo "Build successful!"
else
    echo "Build failed!"
    exit 1
fi
