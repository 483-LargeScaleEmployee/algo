#pragma once
#include "../glpk.h"
#include "../in/csv_importer.h"
#include "../in/input_data_funcs.h"

void add_availability_constraint(glp_prob *lp, const EmployeeVec *employee_vec,
                                 const FiveDimensionConfig *config, int* cur_row);

void add_department_needs_constraint(glp_prob *lp,
                                     const DepartmentVec *department_vec,
                                     const FiveDimensionConfig *config, InputData *input_data, int* cur_row);
