#include "../include/io/csv_importer.h"
#include "../include/io/input_data_funcs.h"
#include "../include/io/output_handler.h"
#include "../include/math/math_processor.h"
#include "../include/ml/ml_processor.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
  const char *input_dir = NULL;
  bool use_ml = false; // Default to math processing

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--ml") == 0) {
      use_ml = true;
    } else if (input_dir == NULL) {
      input_dir = argv[i];
    } else {
      printf("Error: Unexpected argument '%s'\n", argv[i]);
      return 1;
    }
  }

  if (input_dir == NULL) {
    printf("Error: No input directory specified\n");
    return 1;
  }

  InputData data = import_csv(input_dir);

  printf("Random queries for testing:\n");
  printf("Is employee_available: %d\n", is_employee_available(&data, 0, 0, 0));
  printf("Does employee prefer: %d\n", does_employee_prefer(&data, 0, 0, 0));
  printf("Get needed employees: %d\n", get_needed_employees(&data, 0, 0, 0, 0));

  const DataProcessor *processor;

  // Conditional logic to decide between ML and Math paths
  if (use_ml) {
    processor = &ML_PROCESSOR;
  } else {
    processor = &MATH_PROCESSOR;
  }

  StandardizedOutput *output = processor->process(&data);

  send_to_frontend(output);

  printf("Program completed successfully!\n"); // Add this line

  return 0;
}
