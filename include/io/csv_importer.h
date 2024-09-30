#pragma once

typedef struct InputData InputData;

InputData* import_csv(const char* filename);
void destroy_generic_data(InputData* data);
