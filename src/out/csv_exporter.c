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
                                       const OutputData *output) {
  char filepath[512];
  snprintf(filepath, sizeof(filepath), "%s/schedule_machine.csv", output_dir);

  FILE *file = fopen(filepath, "w");
  if (!file) {
    return false;
  }

  fprintf(file, "employee_name,employee_type_idx,department_idx,sprint_day_idx,"
                "shift_idx\n");

  for (int i = 0; i < output->num_assignments; i++) {
    const ShiftAssignment *assignment = &output->assignments[i];
    fprintf(file, "%s,%d,%d,%d,%d\n", assignment->employee_name,
            assignment->employee_type_idx, assignment->department_idx,
            assignment->sprint_day_idx, assignment->shift_idx);
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

  fprintf(file, "Employee,Type,Department,Day,Shift\n");

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
    fprintf(file, "%s,%s,%s,%s,%s\n", assignment->employee_name,
            data->metadata.employee_type_names[assignment->employee_type_idx],
            data->metadata.department_names[assignment->department_idx],
            data->metadata.sprint_day_names[assignment->sprint_day_idx],
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

  bool machine_success = write_machine_readable_csv(output_dir, &output);
  if (!machine_success) {
    return false;
  }

  bool human_success = write_human_readable_csv(output_dir, &output, data);
  if (!human_success) {
    return false;
  }

  return true;
}
