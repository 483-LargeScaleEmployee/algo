#include "../../include/ml/ml_processor.h"
#include "../../include/io/csv_importer.h"
#include <stdlib.h>

static StandardizedOutput *process(InputData *data) {
  // Process data
  return NULL;
}

const DataProcessor ML_PROCESSOR = {
    .process = process,
};
