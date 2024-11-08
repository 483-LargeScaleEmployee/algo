#!/bin/bash

# Project structure configuration
SRC_DIR="src"
TEST_DIR="tests"
INCLUDE_DIR="include"
GLPK_DIR="glpk-4.65"
GLPK_DIR_W64="glpk-4.65/w64"
OUTPUT="algo.exe"
TEST_OUTPUT="run_tests.exe"

# Compiler configuration
MINGW=x86_64-w64-mingw32-gcc
CFLAGS="-I${INCLUDE_DIR} -I${GLPK_DIR}/src -O2"
LDFLAGS="-L${GLPK_DIR_W64} -lglpk_4_65"

build_and_run_tests() {
    echo "Building tests..."

    # Find all source files except main.c and test files
    SRC_FILES=$(find ${SRC_DIR} -name "*.c" ! -name "main.c" ! -name "test_*.c")

    # Find all test files
    TEST_FILES=$(find ${TEST_DIR} -name "test_*.c")

    # Check if we found any test files
    if [ -z "$TEST_FILES" ]; then
        echo "No test files found in ${TEST_DIR}!"
        exit 1
    fi

    echo "Found test files:"
    echo "$TEST_FILES" | sed 's/^/  /'

    # Compile test runner as a console application
    ${MINGW} ${CFLAGS} -o ${TEST_OUTPUT} \
        ${TEST_FILES} \
        ${SRC_FILES} \
        ${LDFLAGS}

    if [ $? -eq 0 ]; then
        echo "Test build successful! Running tests..."
        wine ./${TEST_OUTPUT}
        return $?
    else
        echo "Test build failed!"
        return 1
    fi
}

build_and_run_tests
