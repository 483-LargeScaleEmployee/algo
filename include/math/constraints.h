#pragma once
#include "../glpk.h"
#include "../in/csv_importer.h"
#include "../in/input_data_funcs.h"

void add_availability_constraint(glp_prob *lp, EmployeeVec *employee_vec,
                                 FiveDimensionConfig *config, int* cur_row);

void add_department_needs_constraint(glp_prob *lp,
                                     DepartmentVec *department_vec,
                                     FiveDimensionConfig *config, InputData *input_data, int* cur_row);
