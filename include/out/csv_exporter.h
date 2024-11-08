#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "../in/csv_importer.h"

typedef struct {
  char *employee_name;
  int employee_type_idx;
  int department_idx;
  int sprint_day_idx;
  int shift_idx;
} ShiftAssignment;

typedef struct {
  double objective_value;
  int num_assignments;
  ShiftAssignment *assignments;
  bool ran_successfully;
} OutputData;

bool export_csv(const char *output_dir, const OutputData output,
                const InputData *data);
