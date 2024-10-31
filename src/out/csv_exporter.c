#include "../../include/out/csv_exporter.h"
#include "../../include/glpk.h"
#include "../../include/math/glp_utils.h"
#include "../../include/out/csv_exporter.h"
#include <string.h>

void cleanup_standardized_output(OutputData *output) {
  if (output->assignments) {
    for (int i = 0; i < output->num_assignments; i++) {
      free(output->assignments[i].employee_name);
    }
    free(output->assignments);
  }
}

static int compare_assignments(const void *a, const void *b) {
  typedef struct {
    const char *employee_name;
    const char *employee_type;
    int day;
    int shift;
  } Assignment;

  const Assignment *aa = (const Assignment *)a;
  const Assignment *bb = (const Assignment *)b;

  if (aa->day != bb->day)
    return aa->day - bb->day;
  if (aa->shift != bb->shift)
    return aa->shift - bb->shift;
  return strcmp(aa->employee_name, bb->employee_name);
}

void write_machine_readable_csv(const char *output_dir, const InputData *data,
                                glp_prob *lp) {
  char filepath[512];
  snprintf(filepath, sizeof(filepath), "%s/schedule_machine.csv", output_dir);

  FILE *file = fopen(filepath, "w");
  if (!file) {
    printf("Error: Could not open %s for writing\n", filepath);
    return;
  }

  fprintf(file, "employee_idx,employee_type_idx,day_idx,shift_idx\n");

  int num_vars = glp_get_num_cols(lp);
  for (int i = 1; i <= num_vars; i++) {
    double val = glp_mip_col_val(lp, i);
    if (val > 0.5) {
      int emp, day, shift;
      glp_employee_vec_index_reverse(&data->config, i, &emp, &day, &shift);
      fprintf(file, "%d,%d,%d,%d\n", emp,
              data->employee_info[emp].employee_type, day, shift);
    }
  }
  fclose(file);
  printf("Machine-readable schedule written to %s\n", filepath);
}

void write_human_readable_csv(const char *output_dir, const InputData *data,
                              glp_prob *lp) {
  char filepath[512];
  snprintf(filepath, sizeof(filepath), "%s/schedule_human.csv", output_dir);

  FILE *file = fopen(filepath, "w");
  if (!file) {
    printf("Error: Could not open %s for writing\n", filepath);
    return;
  }

  fprintf(file, "Employee,Type,Day,Shift\n");

  typedef struct {
    const char *employee_name;
    const char *employee_type;
    int day;
    int shift;
  } Assignment;

  Assignment *assignments = malloc(glp_get_num_cols(lp) * sizeof(Assignment));
  if (!assignments) {
    printf("Error: Failed to allocate memory for assignments\n");
    fclose(file);
    return;
  }

  int num_assignments = 0;
  int num_vars = glp_get_num_cols(lp);

  for (int i = 1; i <= num_vars; i++) {
    double val = glp_mip_col_val(lp, i);
    if (val > 0.5) {
      int emp, day, shift;
      glp_employee_vec_index_reverse(&data->config, i, &emp, &day, &shift);

      assignments[num_assignments].employee_name =
          data->employee_info[emp].name;
      assignments[num_assignments].employee_type =
          data->metadata
              .employee_type_names[data->employee_info[emp].employee_type];
      assignments[num_assignments].day = day;
      assignments[num_assignments].shift = shift;
      num_assignments++;
    }
  }

  qsort(assignments, num_assignments, sizeof(Assignment), compare_assignments);

  for (int i = 0; i < num_assignments; i++) {
    fprintf(file, "%s,%s,%s,%s\n", assignments[i].employee_name,
            assignments[i].employee_type,
            data->metadata.sprint_day_names[assignments[i].day],
            data->metadata.shift_names[assignments[i].shift]);
  }

  free(assignments);
  fclose(file);
  printf("Human-readable schedule written to %s\n", filepath);
}

bool export_csv(const char *output_dir, const OutputData output) {
  if (!output.ran_successfully) {
    printf("Error: Cannot export CSV, algorithm did not run successfully\n");
    return false;
  }

  printf("Exporting CSV files...\n");

  return true;
}
