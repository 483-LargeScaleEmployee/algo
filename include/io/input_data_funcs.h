#include "csv_importer.h"
#include <stdbool.h>
#pragma once

bool is_employee_available(const InputData *data, uint8_t employee_idx,
                           uint8_t sprint_day_idx, uint8_t shift_idx);

bool does_employee_prefer(const InputData *data, uint8_t employee_idx,
                           uint8_t sprint_day_idx, uint8_t shift_idx);

uint8_t get_employee_type(const InputData *data, uint8_t employee_id);

uint8_t get_needed_employees(const InputData *data, uint8_t department_idx,
                             uint8_t employee_type_idx, uint8_t sprint_day_idx,
                             uint8_t shift_idx);
