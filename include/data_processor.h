#include "io/csv_importer.h"

#pragma once

typedef struct {
  uint8_t *vec;
} StandardizedOutput;


typedef struct DataProcessor {
  StandardizedOutput *(*process)(InputData *);
} DataProcessor;
