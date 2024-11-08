#pragma once
#include "../in/csv_importer.h"
#include "../in/input_data_funcs.h"

int glp_schedule_vec_size(const FiveDimensionConfig *config);

int glp_schedule_vec_index(const FiveDimensionConfig *config, int dep, int emp,
                           int day, int shift);

void glp_schedule_vec_index_reverse(const FiveDimensionConfig *config,
                                    int glp_idx, int *dep, int *emp, int *day,
                                    int *shift);
