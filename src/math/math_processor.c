#include "../../include/math/math_processor.h"
#include "../../include/glpk.h"
#include "../../include/math/constraints.h"
#include "../../include/math/glp_utils.h"
#include "../../include/math/preferences.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void main_solve(const InputData *data, const char *output_dir) {
  glp_prob *lp = glp_create_prob();
  glp_set_prob_name(lp, "Employee_Schedule");
  glp_set_obj_dir(lp, GLP_MAX);

  int cur_row = 1;
  set_employee_preferences(lp, &data->employee_vec, &data->config);
  add_availability_constraint(lp, &data->employee_vec, &data->config, &cur_row);

  printf("Problem size:\n");
  printf("Rows (constraints): %d\n", glp_get_num_rows(lp));
  printf("Columns (variables): %d\n", glp_get_num_cols(lp));
  printf("Non-zeros: %d\n", glp_get_num_nz(lp));

  glp_smcp smcp;
  glp_init_smcp(&smcp);
  smcp.msg_lev = GLP_MSG_ALL;
  smcp.meth = GLP_PRIMAL;
  smcp.presolve = GLP_ON;

  printf("\nSolving LP relaxation...\n");
  int lp_result = glp_simplex(lp, &smcp);
  if (lp_result != 0) {
    printf("Failed to solve LP relaxation: %d\n", lp_result);
    glp_delete_prob(lp);
    return;
  }

  int lp_status = glp_get_status(lp);
  if (lp_status != GLP_OPT) {
    printf("LP relaxation not optimal. Status: %d\n", lp_status);
    glp_delete_prob(lp);
    return;
  }

  printf("LP relaxation solved. Objective: %f\n", glp_get_obj_val(lp));

  glp_iocp iocp;
  glp_init_iocp(&iocp);
  iocp.msg_lev = GLP_MSG_ALL;
  iocp.presolve = GLP_ON;
  iocp.br_tech = GLP_BR_PCH;
  iocp.bt_tech = GLP_BT_BLB;
  iocp.fp_heur = GLP_ON;
  iocp.tm_lim = 60000;

  printf("\nSolving MIP problem...\n");
  int mip_result = glp_intopt(lp, &iocp);
  if (mip_result != 0) {
    printf("Failed to solve MIP problem: %d\n", mip_result);
    glp_delete_prob(lp);
    return;
  }

  int mip_status = glp_mip_status(lp);
  if (mip_status != GLP_OPT && mip_status != GLP_FEAS) {
    printf("MIP solution not optimal or feasible. Status: %d\n", mip_status);
    glp_delete_prob(lp);
    return;
  }

  double obj_val = glp_mip_obj_val(lp);
  printf("\nSolution found!\n");
  printf("Objective value: %f\n", obj_val);

  write_machine_readable_csv(output_dir, data, lp);
  write_human_readable_csv(output_dir, data, lp);

  glp_delete_prob(lp);
}

static StandardizedOutput *process(InputData *data, const char *output_dir) {
  main_solve(data, output_dir);
  return NULL;
}

const DataProcessor MATH_PROCESSOR = {
    .process = process,
};
