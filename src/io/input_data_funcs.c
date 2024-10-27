#include "../../include/io/input_data_funcs.h"
#include <stdbool.h>
#include <stdint.h>

// employee_index -> sprint_day -> shift -> 2 u8s
//
// first u8 is if they are available
// second u8 is if they prefer
//
// employee vec: (num employees) x (num sprint days) x (num shifts)
bool is_employee_available(const InputData *data, uint8_t employee_idx,
                           uint8_t sprint_day_idx, uint8_t shift_idx) {

  const FiveDimensionConfig *config = &data->config;
  uint8_t availability = data->employee_vec.vec[EMPLOYEE_VEC_INDEX(
      config, employee_idx, sprint_day_idx, shift_idx)];

  if (availability == 0) {
    return false;
  } else {
    return true;
  }
}

bool does_employee_prefer(const InputData *data, uint8_t employee_idx,
                          uint8_t sprint_day_idx, uint8_t shift_idx) {
  const FiveDimensionConfig *config = &data->config;
  uint8_t preference =
      data->employee_vec.vec[EMPLOYEE_VEC_INDEX(config, employee_idx,
                                                sprint_day_idx, shift_idx) +
                             1];

  printf("preference: %d\n", preference);

  if (preference == 0) {
    return false;
  } else {
    return true;
  }
}

uint8_t get_employee_type(const InputData *data, uint8_t employee_idx) {
  return data->employee_info[employee_idx].employee_type;
}

uint8_t get_needed_employees(const InputData *data, uint8_t department_idx,
                             uint8_t employee_type_idx, uint8_t sprint_day_idx,
                             uint8_t shift_idx) {
  const FiveDimensionConfig *config = &data->config;
  return data->department_vec.vec[DEPARTMENT_VEC_INDEX(
      config, department_idx, employee_type_idx, sprint_day_idx, shift_idx)];
}
