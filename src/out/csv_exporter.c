#include "../../include/out/csv_exporter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  char *employee_name;
  int employee_type_idx;
  int department_idx;
  int *sprint_days;
  int *shifts;
  int num_entries;
  int capacity;
} GroupedAssignment;

static int compare_assignments(const void *a, const void *b) {
  const ShiftAssignment *aa = (const ShiftAssignment *)a;
  const ShiftAssignment *bb = (const ShiftAssignment *)b;
  int name_cmp = strcmp(aa->employee_name, bb->employee_name);
  if (name_cmp != 0)
    return name_cmp;
  if (aa->department_idx != bb->department_idx)
    return aa->department_idx - bb->department_idx;
  return aa->sprint_day_idx - bb->sprint_day_idx;
}

static GroupedAssignment
create_grouped_assignment(const ShiftAssignment *first) {
  GroupedAssignment group = {.employee_name = strdup(first->employee_name),
                             .employee_type_idx = first->employee_type_idx,
                             .department_idx = first->department_idx,
                             .sprint_days = malloc(20 * sizeof(int)),
                             .shifts = malloc(20 * sizeof(int)),
                             .num_entries = 0,
                             .capacity = 20};
  return group;
}

static void add_to_group(GroupedAssignment *group, int day, int shift) {
  if (group->num_entries >= group->capacity) {
    group->capacity *= 2;
    group->sprint_days =
        realloc(group->sprint_days, group->capacity * sizeof(int));
    group->shifts = realloc(group->shifts, group->capacity * sizeof(int));
  }

  // Insert in sorted order
  int insert_pos = 0;
  while (insert_pos < group->num_entries &&
         group->sprint_days[insert_pos] < day) {
    insert_pos++;
  }

  // Shift existing elements
  for (int i = group->num_entries; i > insert_pos; i--) {
    group->sprint_days[i] = group->sprint_days[i - 1];
    group->shifts[i] = group->shifts[i - 1];
  }

  // Insert new elements
  group->sprint_days[insert_pos] = day;
  group->shifts[insert_pos] = shift;
  group->num_entries++;
}

static void free_grouped_assignment(GroupedAssignment *group) {
  free(group->employee_name);
  free(group->sprint_days);
  free(group->shifts);
}

// employee idx, dep_idx is already going to be constant
typedef struct {
  int sprint_day_one_idx;
  int *shift_idx;
} CsvElement;

static void write_csv_row(FILE *file, const GroupedAssignment *group,
                          const InputData *data) {
  // Write employee info columns
  fprintf(file, "%s,%s,%s,", group->employee_name,
          data->metadata.department_names[group->department_idx],
          data->metadata.employee_type_names[group->employee_type_idx]);

  CsvElement *csv_elements = malloc(group->num_entries * sizeof(CsvElement));
  int num_csv_elements = 0;
  // 1 if day is already written, 0 otherwise
  int *already_written_days = calloc(data->config.num_sprint_days, sizeof(int));

  // CsvElement essentially encompasses all shifts for dep, employee, day
  // convert all grouped assignment to csv element
  for (int i = 0; i < group->num_entries; i++) {

    // If day is already written, skip
    if (already_written_days[group->sprint_days[i]] == 1) {
      continue;
    }

    int cur_day = group->sprint_days[i]; // sprint days is already 1 indexed

    int *shift_idx = calloc(data->config.num_shifts, sizeof(int));
    shift_idx[group->shifts[i]] = 1;

    for (int j = i + 1; j < data->config.num_shifts; j++) {
      if (group->sprint_days[j] == cur_day) {
        shift_idx[group->shifts[j]] = 1;
      }
    }

    CsvElement csv_element = {.sprint_day_one_idx = cur_day,
                              .shift_idx = shift_idx};
    csv_elements[num_csv_elements++] = csv_element;
    already_written_days[cur_day] = 1;
  }

  // Write days tuple
  if (num_csv_elements == 1) {
    // Single day case
    fprintf(file, "\"(%d)\",", group->sprint_days[0]);

    char *binary_shift = malloc(data->config.num_shifts * sizeof(char));
    // initialize all to 0
    for (int i = 0; i < data->config.num_shifts; i++) {
      binary_shift[i] = '0';
      for (int j = 0; j < data->config.num_shifts; j++) {
        if (csv_elements[0].shift_idx[j] == 1) {
          binary_shift[j] = '1';
        }
      }
    }

    fprintf(file, "\"(%s)\"", binary_shift);
  } else {
    // Multiple days case - Write days
    fprintf(file, "\"(");
    for (int i = 0; i < num_csv_elements; i++) {
      fprintf(file, "%d%s", csv_elements[i].sprint_day_one_idx,
              (i < num_csv_elements - 1) ? ", " : "");
    }
    fprintf(file, ")\",");

    // Write shifts tuple with correct binary representation
    fprintf(file, "\"(");
    for (int i = 0; i < num_csv_elements; i++) {
      char *binary_shift = malloc(data->config.num_shifts * sizeof(char));
      // initialize all to 0
      for (int j = 0; j < data->config.num_shifts; j++) {
        binary_shift[j] = '0';
        for (int k = 0; k < data->config.num_shifts; k++) {
          if (csv_elements[i].shift_idx[k] == 1) {
            binary_shift[k] = '1';
          }
        }
      }

      fprintf(file, "%s%s", binary_shift,
              (i < num_csv_elements - 1) ? ", " : "");
    }
    fprintf(file, ")\"");
  }
  fprintf(file, "\n");
}

// Helper function to convert shift index to correct binary representation
static void get_binary_shift_string(int shift_idx, char *output) {
  // Initialize binary string with all zeros
  strcpy(output, "000");

  // Set the appropriate position to 1 based on shift index
  // shift 0 -> "100", shift 1 -> "010", shift 2 -> "001"
  if (shift_idx >= 0 && shift_idx < 3) {
    output[shift_idx] = '1'; // Direct mapping of index to position
  }
}

static bool process_assignments(FILE *file, const OutputData *output,
                                const InputData *data) {
  // Sort assignments
  ShiftAssignment *sorted_assignments =
      malloc(output->num_assignments * sizeof(ShiftAssignment));
  memcpy(sorted_assignments, output->assignments,
         output->num_assignments * sizeof(ShiftAssignment));
  qsort(sorted_assignments, output->num_assignments, sizeof(ShiftAssignment),
        compare_assignments);

  GroupedAssignment *groups =
      malloc(output->num_assignments * sizeof(GroupedAssignment));
  int num_groups = 0;

  const ShiftAssignment *curr = &sorted_assignments[0];
  groups[0] = create_grouped_assignment(curr);
  add_to_group(&groups[0], curr->sprint_day_idx + 1,
               curr->shift_idx); // Add 1 to make 1-indexed
  num_groups = 1;

  // Process remaining assignments
  for (int i = 1; i < output->num_assignments; i++) {
    curr = &sorted_assignments[i];
    if (strcmp(groups[num_groups - 1].employee_name, curr->employee_name) ==
            0 &&
        groups[num_groups - 1].department_idx == curr->department_idx) {
      // Add to existing group
      add_to_group(&groups[num_groups - 1], curr->sprint_day_idx + 1,
                   curr->shift_idx); // Add 1 to make 1-indexed
    } else {
      // Write previous group
      write_csv_row(file, &groups[num_groups - 1], data);
      // Start new group
      groups[num_groups] = create_grouped_assignment(curr);
      add_to_group(&groups[num_groups], curr->sprint_day_idx + 1,
                   curr->shift_idx); // Add 1 to make 1-indexed
      num_groups++;
    }
  }

  // Write last group
  if (num_groups > 0) {
    write_csv_row(file, &groups[num_groups - 1], data);
  }

  // Cleanup
  for (int i = 0; i < num_groups; i++) {
    free_grouped_assignment(&groups[i]);
  }
  free(groups);
  free(sorted_assignments);

  return true;
}

static bool write_csv(const char *output_dir, const OutputData *output,
                      const InputData *data) {
  char filepath[512];
  snprintf(filepath, sizeof(filepath), "%s/schedule.csv", output_dir);
  FILE *file = fopen(filepath, "w");
  if (!file)
    return false;

  fprintf(file, "EmployeeName,Department,Role,Day,Shift\n");

  bool success = process_assignments(file, output, data);
  fclose(file);
  return success;
}

bool export_csv(const char *output_dir, const OutputData output,
                const InputData *data) {
  if (!output.ran_successfully || !output.assignments ||
      output.num_assignments == 0) {
    return false;
  }

  bool human_success = write_csv(output_dir, &output, data);
  if (!human_success)
    return false;

  return true;
}
