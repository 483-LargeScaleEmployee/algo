#include "../../include/out/csv_exporter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int compare_assignments(const void *a, const void *b) {
  const ShiftAssignment *aa = (const ShiftAssignment *)a;
  const ShiftAssignment *bb = (const ShiftAssignment *)b;
  // First compare by employee name
  int name_cmp = strcmp(aa->employee_name, bb->employee_name);
  if (name_cmp != 0) {
    return name_cmp;
  }
  // Then compare by department index
  if (aa->department_idx != bb->department_idx) {
    return aa->department_idx - bb->department_idx;
  }
  // If name and department are the same, compare by sprint day
  if (aa->sprint_day_idx != bb->sprint_day_idx) {
    return aa->sprint_day_idx - bb->sprint_day_idx;
  }
  // Finally, compare by shift index
  return aa->shift_idx - bb->shift_idx;
}

static bool write_machine_readable_csv(const char *output_dir,
                                     const OutputData *output,
                                     const InputData *data) {
  char filepath[512];
  snprintf(filepath, sizeof(filepath), "%s/schedule_machine.csv", output_dir);
  FILE *file = fopen(filepath, "w");
  if (!file) {
    return false;
  }

  // Updated header with employee title instead of type
  fprintf(file, "employee_name,%s,department_name,sprint_day_idx,shift_idx\n",
          data->metadata.employee_type_names[0]); // Using first type name as column header

  for (int i = 0; i < output->num_assignments; i++) {
    const ShiftAssignment *assignment = &output->assignments[i];
    
    // Get the employee type name instead of index
    const char* employee_type_name = data->metadata.employee_type_names[assignment->employee_type_idx];
    
    // Get the department name instead of index
    const char* department_name = data->metadata.department_names[assignment->department_idx];
    
    // Adjust sprint day to be 1-indexed
    int sprint_day_1_indexed = assignment->sprint_day_idx + 1;

    fprintf(file, "%s,%s,%s,%d,%d\n", 
            assignment->employee_name,
            employee_type_name,
            department_name,
            sprint_day_1_indexed,
            assignment->shift_idx);
  }
  fclose(file);
  printf("Machine-readable schedule written to %s\n", filepath);
  return true;
}

static bool write_human_readable_csv(const char *output_dir,
                                   const OutputData *output,
                                   const InputData *data) {
  char filepath[512];
  snprintf(filepath, sizeof(filepath), "%s/schedule_human.csv", output_dir);
  FILE *file = fopen(filepath, "w");
  if (!file) {
    return false;
  }

  // Updated header with employee title instead of type
  fprintf(file, "Employee,%s,Department,Day,Shift\n",
          data->metadata.employee_type_names[0]); // Using first type name as column header

  // Create a sorted copy of assignments for output
  ShiftAssignment *sorted_assignments =
      malloc(output->num_assignments * sizeof(ShiftAssignment));
  if (!sorted_assignments) {
    fclose(file);
    return false;
  }
  memcpy(sorted_assignments, output->assignments,
         output->num_assignments * sizeof(ShiftAssignment));
  qsort(sorted_assignments, output->num_assignments, sizeof(ShiftAssignment),
        compare_assignments);

  for (int i = 0; i < output->num_assignments; i++) {
    const ShiftAssignment *assignment = &sorted_assignments[i];
    
    // Adjust sprint day to be 1-indexed
    int sprint_day_1_indexed = assignment->sprint_day_idx + 1;
    
    fprintf(file, "%s,%s,%s,%d,%s\n", 
            assignment->employee_name,
            data->metadata.employee_type_names[assignment->employee_type_idx],
            data->metadata.department_names[assignment->department_idx],
            sprint_day_1_indexed,
            data->metadata.shift_names[assignment->shift_idx]);
  }
  free(sorted_assignments);
  fclose(file);
  printf("Human-readable schedule written to %s\n", filepath);
  return true;
}

bool export_csv(const char *output_dir, const OutputData output,
                const InputData *data) {
  if (!output.ran_successfully || !output.assignments ||
      output.num_assignments == 0) {
    return false;
  }

  bool machine_success = write_machine_readable_csv(output_dir, &output, data);
  if (!machine_success) {
    return false;
  }

  bool human_success = write_human_readable_csv(output_dir, &output, data);
  if (!human_success) {
    return false;
  }

  return true;
}
