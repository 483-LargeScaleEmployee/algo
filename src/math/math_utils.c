#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

static size_t get_total_shift_vec_indices() { return 1000 * 14 * 3; }

static size_t get_total_constraint_rows() { return 10 * 3 * 14 * 3; }

static size_t get_shift_vec_idx(int employee_idx, int sprint_day_idx,
                                int shift_idx) {
  return employee_idx * 14 * 3 + sprint_day_idx * 3 + shift_idx;
}

static size_t get_constraint_row_idx(int department_idx, int employee_type_idx,
                                     int sprint_day_idx, int shift_idx) {
  return department_idx * 3 * 14 * 3 + employee_type_idx * 14 * 3 +
         sprint_day_idx * 3 + shift_idx;
}
