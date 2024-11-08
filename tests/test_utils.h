#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <stdio.h>
#include <stdlib.h>

#define TEST_ASSERT(condition, message)                                        \
  do {                                                                         \
    if (!(condition)) {                                                        \
      fprintf(stderr, "\n    Assertion failed: %s\n", message);                    \
      fprintf(stderr, "    File: %s, Line: %d\n\n", __FILE__, __LINE__);             \
      exit(1);                                                                 \
    }                                                                          \
  } while (0)

#define TEST_EQUALS(expected, actual, message)                                 \
  TEST_ASSERT((expected) == (actual), message)

#define TEST_START(name) printf("Running test: %s...\n", name)

#define TEST_END(num) printf("Passed %d tests\n", num)

#endif // TEST_UTILS_H
