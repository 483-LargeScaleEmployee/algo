#include "io/csv_importer.h"
#include "io/output_handler.h"

#pragma once

typedef struct DataProcessor {
  StandardizedOutput *(*process)(InputData *);
} DataProcessor;
