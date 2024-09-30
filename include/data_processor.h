// include/data_processor.h
#pragma once

typedef struct InputData InputData;
typedef struct StandardizedOutput StandardizedOutput;

typedef struct DataProcessor {
  void *(*create_from_input_data)(InputData *);
  void (*process_data)(void *);
  StandardizedOutput *(*convert_to_output)(void *);
  void (*destroy)(void *);
} DataProcessor;
