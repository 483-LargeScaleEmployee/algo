#include "in/csv_importer.h"
#include "out/csv_exporter.h"

#pragma once

typedef struct DataProcessor {
  OutputData (*process)(InputData *, const char *output_dir);
} DataProcessor;
