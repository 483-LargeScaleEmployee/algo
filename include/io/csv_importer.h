#pragma once
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  char name[256];
  uint8_t employee_type;
} EmployeeInfo;

typedef struct {
  size_t num_employees;
  size_t num_sprint_days;
  size_t num_shifts;
  size_t num_departments;
  size_t num_employee_types;
} FiveDimensionConfig;

// employee_index -> sprint_day -> shift -> 2 u8s
//
// first u8 is if they are available
// second u8 is if they prefer
//
// employee vec: (num employees) x (num sprint days) x (num shifts)
typedef struct {
  uint8_t *vec;
} EmployeeVec;

#define EMPLOYEE_VEC_INDEX(config, employee_idx, sprint_day_idx, shift_idx)    \
  ((employee_idx) * config->num_sprint_days * 3 + (sprint_day_idx) * 3 +       \
   (shift_idx)) *                                                              \
      2 // 2 u8s per entry

// department_idx -> employee_type -> sprint_day -> shift_idx -> u8
// where u8 is the num of employees of that type needed on the sprint day on the
// shift in the department
typedef struct {
  uint8_t *vec;
} DepartmentVec;

#define DEPARTMENT_VEC_INDEX(config, department_idx, employee_type_idx,        \
                             sprint_day_idx, shift_idx)                        \
  ((department_idx) * config->num_employee_types * config->num_sprint_days *   \
       config->num_shifts +                                                    \
   (employee_type_idx) * config->num_sprint_days * config->num_shifts +        \
   (sprint_day_idx) * config->num_shifts + (shift_idx))

typedef struct {
  char **sprint_day_names;
  char **shift_names;
  char **employee_type_names;
  char **department_names;
} MetadataStrings;

typedef struct InputData {
  EmployeeInfo *employee_info;
  MetadataStrings metadata;
  FiveDimensionConfig config;
  EmployeeVec employee_vec;
  DepartmentVec department_vec;
} InputData;

InputData *import_csv(const char *input_dir);

void free_input_data(InputData *data);
