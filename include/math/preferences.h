#pragma once
#include "../glpk.h"
#include "../io/csv_importer.h"
#include "../io/input_data_funcs.h"

void set_employee_preferences(glp_prob *lp, EmployeeVec *employee_vec,
                              FiveDimensionConfig *config);

