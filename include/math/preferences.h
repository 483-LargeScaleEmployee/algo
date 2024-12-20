#pragma once
#include "../glpk.h"
#include "../in/csv_importer.h"
#include "../in/input_data_funcs.h"

void set_employee_preferences(glp_prob *lp, const EmployeeVec *employee_vec,
                              const FiveDimensionConfig *config);
