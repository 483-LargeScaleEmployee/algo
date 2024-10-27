#include "../include/io/csv_importer.h"
#include "../include/io/input_data_funcs.h"
#include "../include/math/math_processor.h"
#include "../include/ml/ml_processor.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
  const char *input_dir = NULL;
  bool use_ml = false; // Default to math processing

  printf("Expected usage: ./algo.exe <input_dir> [--ml | --math]\n");
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--ml") == 0) {
      use_ml = true;
    } else if (strcmp(argv[i], "--math") == 0) {
      use_ml = false;
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

  InputData *data = import_csv(input_dir);

  // for (int i = 0; i < data->config.num_departments; i++) {
  //   printf("Department %d: %s\n", i, data->metadata.department_names[i]);
  // }
  // for (int i = 0; i < data->config.num_employee_types; i++) {
  //   printf("Employee Type %d: %s\n", i, data->metadata.employee_type_names[i]);
  // }
  // for (int i = 0; i < data->config.num_sprint_days; i++) {
  //   printf("Sprint Day %d: %s\n", i, data->metadata.sprint_day_names[i]);
  // }
  // for (int i = 0; i < data->config.num_shifts; i++) {
  //   printf("Shift %d: %s\n", i, data->metadata.shift_names[i]);
  // }

  const DataProcessor *processor;
  // Conditional logic to decide between ML and Math paths
  if (use_ml) {
    processor = &ML_PROCESSOR;
  } else {
    processor = &MATH_PROCESSOR;
  }
  StandardizedOutput *output = processor->process(data);

  // send_to_frontend(output);

  printf("Program completed successfully!\n"); // Add this line

  return 0;
}
