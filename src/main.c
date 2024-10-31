#include "../include/in/csv_importer.h"
#include "../include/math/math_processor.h"
#include "../include/out/csv_exporter.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

void print_all_department_headers(const InputData *data) {
  for (int i = 0; i < data->config.num_departments; i++) {
    printf("Department %d: %s\n", i, data->metadata.department_names[i]);
  }
  for (int i = 0; i < data->config.num_employee_types; i++) {
    printf("Employee Type %d: %s\n", i, data->metadata.employee_type_names[i]);
  }
  for (int i = 0; i < data->config.num_sprint_days; i++) {
    printf("Sprint Day %d: %s\n", i, data->metadata.sprint_day_names[i]);
  }
  for (int i = 0; i < data->config.num_shifts; i++) {
    printf("Shift %d: %s\n", i, data->metadata.shift_names[i]);
  }
}

void print_all_employee_info(const InputData *data) {
  for (int i = 0; i < data->config.num_employees; i++) {
    printf("Employee %d: %s, type: %s\n", i, data->employee_info[i].name,
           data->metadata
               .employee_type_names[data->employee_info[i].employee_type]);
  }
}

int main(int argc, char *argv[]) {
  const char *input_dir = NULL;
  const char *output_dir = NULL;
  bool use_ml = false; // Default to math processing

  printf("Expected usage: ./algo.exe <input_dir> <output_dir>\n");
  printf("Surround directories with quotes\n");

  // this no longer supports ml, that will be a python script
  for (int i = 1; i < argc; i++) {
    if (input_dir == NULL) {
      input_dir = argv[i];
    } else if (output_dir == NULL) {
      output_dir = argv[i];
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

  // print_all_department_headers(data);
  // print_all_employee_info(data);

  const DataProcessor *processor = &MATH_PROCESSOR;
  OutputData output_data = processor->process(data, output_dir);
  export_csv(output_dir, output_data);

  printf("Program completed successfully!\n"); // Add this line

  return 0;
}
