#include "../include/io/csv_importer.h"
#include "../include/io/output_handler.h"
#include "../include/math/math_processor.h"
#include "../include/ml/ml_processor.h"
#include <stdio.h>

int main() {
  InputData *data = import_csv("../../input");

  return 0;

  const DataProcessor *processor;
  void *processed_data;

  const int use_ml = 1;
  // Conditional logic to decide between ML and Math paths
  if (use_ml) {
    processor = &ML_PROCESSOR;
  } else {
    processor = &MATH_PROCESSOR;
  }

  processed_data = processor->create_from_input_data(data);
  processor->process_data(processed_data);
  StandardizedOutput *output = processor->convert_to_output(processed_data);

  send_to_frontend(output);

  // Clean up
  processor->destroy(processed_data);
  destroy_standardized_output(output);
  destroy_generic_data(data);

  printf("Program completed successfully!\n"); // Add this line

  return 0;
}
