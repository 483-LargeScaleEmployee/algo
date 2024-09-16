// include/data_processor.h
#pragma once

typedef struct GenericData GenericData;
typedef struct StandardizedOutput StandardizedOutput;

typedef struct DataProcessor {
    void* (*create_from_generic_data)(GenericData*);
    void (*process_data)(void*);
    StandardizedOutput* (*convert_to_output)(void*);
    void (*destroy)(void*);
} DataProcessor;
