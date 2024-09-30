#pragma once

typedef struct InputData InputData;

InputData *import_csv(const char *input_dir);
void destroy_generic_data(InputData *data);
