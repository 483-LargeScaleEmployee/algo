#pragma once
#include "../io/csv_importer.h"
#include "../io/input_data_funcs.h"

int glp_employee_vec_index(FiveDimensionConfig *config, int emp, int day,
                           int shift);

void glp_employee_vec_index_reverse(FiveDimensionConfig *config, int glp_idx,
                                    int *emp, int *day, int *shift);
