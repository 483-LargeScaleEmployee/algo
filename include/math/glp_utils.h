#pragma once
#include "../in/csv_importer.h"
#include "../in/input_data_funcs.h"

int glp_schedule_vec_size(FiveDimensionConfig *config);

int glp_schedule_vec_index(FiveDimensionConfig *config, int dep, int emp,
                           int day, int shift);

void glp_schedule_vec_index_reverse(FiveDimensionConfig *config, int glp_idx,
                                    int *dep, int *emp, int *day, int *shift);
