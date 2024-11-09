#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Assert that a condition is true
 * @param condition The condition to check
 * @param message The message to display if the assertion fails
 */
#define TEST_ASSERT(condition, message)                                        \
  do {                                                                         \
    if (!(condition)) {                                                        \
      fprintf(stderr,                                                          \
              "\nAssertion failed\n"                                           \
              "  Message:  %s\n"                                               \
              "  Location: %s:%d\n"                                            \
              "  Function: %s\n\n",                                            \
              message, __FILE__, __LINE__, __func__);                          \
      exit(1);                                                                 \
    }                                                                          \
  } while (0)

/**
 * @brief Assert that two values are equal
 * @param expected The expected value
 * @param actual The actual value
 * @param message The message to display if the assertion fails
 */
#define TEST_EQUALS(expected, actual, message)                                 \
  do {                                                                         \
    if (!((expected) == (actual))) {                                           \
      fprintf(stderr,                                                          \
              "\nEquality assertion failed\n"                                  \
              "  Message:  %s\n"                                               \
              "  Expected: %d\n"                                               \
              "  Actual:   %d\n"                                               \
              "  Location: %s:%d\n"                                            \
              "  Function: %s\n\n",                                            \
              message, (int)(expected), (int)(actual),                         \
              __FILE__, __LINE__, __func__);                                   \
      exit(1);                                                                 \
    }                                                                          \
  } while (0)

/**
 * @brief Assert that two strings are equal
 * @param expected The expected string
 * @param actual The actual string
 * @param message The message to display if the assertion fails
 */
#define TEST_STRING_EQUALS(expected, actual, message)                          \
  do {                                                                         \
    if (strcmp((expected), (actual)) != 0) {                                   \
      fprintf(stderr,                                                          \
              "\nString equality assertion failed\n"                           \
              "  Message:  %s\n"                                               \
              "  Expected: \"%s\"\n"                                           \
              "  Actual:   \"%s\"\n"                                           \
              "  Location: %s:%d\n"                                            \
              "  Function: %s\n\n",                                            \
              message, expected, actual,                                        \
              __FILE__, __LINE__, __func__);                                   \
      exit(1);                                                                 \
    }                                                                          \
  } while (0)

/**
 * @brief Start a test suite
 * @param name The name of the test suite
 */
#define TEST_START(name)                                                       \
  printf("Running test: %s...\n", name)

/**
 * @brief End a test suite
 * @param num The number of tests that passed
 */
#define TEST_END(num)                                                          \
  printf("Passed %d tests\n", num)

#endif // TEST_UTILS_H
