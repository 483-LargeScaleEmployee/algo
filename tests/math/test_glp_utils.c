#include "../../include/math/glp_utils.h"
#include "../test_utils.h"

// Helper function to create a standard test configuration
static FiveDimensionConfig create_test_config(void) {
  return (FiveDimensionConfig){.num_departments = 2,
                               .num_employees = 3,
                               .num_sprint_days = 5,
                               .num_shifts = 2};
}

static void test_vec_size(void) {
  FiveDimensionConfig config = create_test_config();

  // Normal configuration
  int expected_size = 2 * 3 * 5 * 2; // dep * emp * days * shifts
  int actual_size = glp_schedule_vec_size(&config);
  TEST_EQUALS(expected_size, actual_size,
              "Vector size should match configuration dimensions");

  // Single department configuration
  config.num_departments = 1;
  expected_size = 1 * 3 * 5 * 2;
  actual_size = glp_schedule_vec_size(&config);
  TEST_EQUALS(expected_size, actual_size,
              "Vector size should work with single department");

  // Minimal configuration
  config.num_employees = 1;
  config.num_sprint_days = 1;
  config.num_shifts = 1;
  TEST_EQUALS(1, glp_schedule_vec_size(&config),
              "Vector size should work with minimal configuration");
}

static void test_vec_index(void) {
  FiveDimensionConfig config = create_test_config();

  // Test first position
  int idx = glp_schedule_vec_index(&config, 0, 0, 0, 0);
  TEST_EQUALS(1, idx, "First position should return 1 (GLPK 1-based indexing)");

  // Test last position
  idx = glp_schedule_vec_index(&config, 1, 2, 4, 1);
  int expected_size =
      config.num_employees * config.num_sprint_days * config.num_shifts * 2;
  TEST_EQUALS(expected_size, idx, "Last position should match total size");

  // Test middle position
  idx = glp_schedule_vec_index(&config, 0, 1, 2, 1);
  int expected_middle =
      (0 * config.num_employees * config.num_sprint_days * config.num_shifts +
       1 * config.num_sprint_days * config.num_shifts + 2 * config.num_shifts +
       1) +
      1;
  TEST_EQUALS(expected_middle, idx,
              "Middle position should calculate correctly");
}

static void test_vec_index_reverse(void) {
  FiveDimensionConfig config = create_test_config();
  int dep, emp, day, shift;

  // Test roundtrip for all positions
  for (int d = 0; d < config.num_departments; d++) {
    for (int e = 0; e < config.num_employees; e++) {
      for (int day_idx = 0; day_idx < config.num_sprint_days; day_idx++) {
        for (int s = 0; s < config.num_shifts; s++) {
          int idx = glp_schedule_vec_index(&config, d, e, day_idx, s);
          glp_schedule_vec_index_reverse(&config, idx, &dep, &emp, &day,
                                         &shift);

          TEST_EQUALS(d, dep, "Department should match after roundtrip");
          TEST_EQUALS(e, emp, "Employee should match after roundtrip");
          TEST_EQUALS(day_idx, day, "Day should match after roundtrip");
          TEST_EQUALS(s, shift, "Shift should match after roundtrip");
        }
      }
    }
  }

  // Test first position
  glp_schedule_vec_index_reverse(&config, 1, &emp, &dep, &day, &shift);
  TEST_EQUALS(0, dep, "First position department should be 0");
  TEST_EQUALS(0, emp, "First position employee should be 0");
  TEST_EQUALS(0, day, "First position day should be 0");
  TEST_EQUALS(0, shift, "First position shift should be 0");

  // Test last position
  int last_idx = config.num_departments * config.num_employees *
                 config.num_sprint_days * config.num_shifts;
  glp_schedule_vec_index_reverse(&config, last_idx, &dep, &emp, &day, &shift);
  TEST_EQUALS(config.num_departments - 1, dep,
              "Last position department should be max");
  TEST_EQUALS(config.num_employees - 1, emp,
              "Last position employee should be max");
  TEST_EQUALS(config.num_sprint_days - 1, day,
              "Last position day should be max");
  TEST_EQUALS(config.num_shifts - 1, shift,
              "Last position shift should be max");
}

void test_glp_utils(void) {
  TEST_START("test_glp_utils");

  test_vec_size();
  test_vec_index();
  test_vec_index_reverse();

  TEST_END(3);
}
