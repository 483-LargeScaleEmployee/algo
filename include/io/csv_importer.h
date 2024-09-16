#pragma once

typedef struct GenericData GenericData;

GenericData* import_csv(const char* filename);
void destroy_generic_data(GenericData* data);
