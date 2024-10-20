#include "../../include/io/input_data_funcs.h"
#include <stdbool.h>
#include <stdint.h>

// 0|No Available Shifts
// 1|Available Shift 2
// 2|Available Shift 1
// 3|Available Shifts 1 & 2
// 4|Available Shift 0
// 5|Available Shifts 0 & 2
// 6|Available Shifts 0 & 1
// 7|Available Shifts 0 & 1 & 2
// in this encoding, night is shift 0, morning is shift 1, evening is shift 2
bool is_employee_available(const InputData *data, uint8_t sprint_day_idx,
                           uint8_t shift, uint8_t employee_id) {
  uint8_t availability =
      data->employee_cube.cube[employee_id][sprint_day_idx][0];

  if (availability == 0) {
    return false;
  } else if (availability == 7) {
    return true;
  } else if (availability == 1 && shift == 2) {
    return true;
  } else if (availability == 2 && shift == 1) {
    return true;
  } else if (availability == 3 && (shift == 1 || shift == 2)) {
    return true;
  } else if (availability == 4 && shift == 0) {
    return true;
  } else if (availability == 5 && (shift == 0 || shift == 2)) {
    return true;
  } else if (availability == 6 && (shift == 0 || shift == 1)) {
    return true;
  }

  return data->employee_cube.cube[employee_id][sprint_day_idx][shift];
}

bool does_employee_prefer(const InputData *data, uint8_t sprint_day_idx,
                          uint8_t shift, uint8_t employee_id) {
  return data->employee_cube.cube[employee_id][sprint_day_idx][0] == shift;
}

uint8_t get_needed_employees(const InputData *data, uint8_t sprint_day_idx,
                             uint8_t shift, uint8_t employee_type,
                             uint8_t department_idx) {
  return data->department_cube
      .cube[department_idx][sprint_day_idx][shift][employee_type];
}

//
// TODO: implement these properly later based on dynamic problem bounds
//
int get_employee_count(const InputData *data) { return 1000; }

int get_department_count(const InputData *data) { return 10; }

int get_sprint_day_count(const InputData *data) { return 14; }

int get_shift_count(const InputData *data) { return 3; }

int get_employee_type_count(const InputData *data) { return 3; }
