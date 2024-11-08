#include "test_utils.h"

void test_glp_utils(void) {
  TEST_START("test_glp_utils");

  TEST_EQUALS(1, 1, "1 should equal 1");
  TEST_EQUALS(1, 2, "1 should not equal 2");

  TEST_END();
}
