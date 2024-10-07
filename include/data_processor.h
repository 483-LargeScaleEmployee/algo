#include "io/csv_importer.h"
#include "io/output_handler.h"

#pragma once

typedef struct DataProcessor {
  void *(*create_from_input_data)(InputData *);
  void (*process_data)(void *);
  StandardizedOutput *(*convert_to_output)(void *);
} DataProcessor;
