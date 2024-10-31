#include "../../include/math/math_processor.h"
#include "../../include/glpk.h"
#include "../../include/math/constraints.h"
#include "../../include/math/glp_utils.h"
#include "../../include/math/preferences.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

OutputData main_solve(const InputData *data, const char *output_dir) {
  glp_prob *lp = glp_create_prob();
  glp_set_prob_name(lp, "Employee_Schedule");
  glp_set_obj_dir(lp, GLP_MAX);

  int cur_row = 1;
  set_employee_preferences(lp, &data->employee_vec, &data->config);
  add_availability_constraint(lp, &data->employee_vec, &data->config, &cur_row);
  // add_department_needs_constraint(lp, &data->department_vec, &data->config,
  //                                 data, &cur_row);

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
    return (OutputData){0, 0, NULL, false};
  }

  int lp_status = glp_get_status(lp);
  if (lp_status != GLP_OPT) {
    printf("LP relaxation not optimal. Status: %d\n", lp_status);
    glp_delete_prob(lp);
    return (OutputData){0, 0, NULL, false};
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
    return (OutputData){0, 0, NULL, false};
  }

  int mip_status = glp_mip_status(lp);
  if (mip_status != GLP_OPT && mip_status != GLP_FEAS) {
    printf("MIP solution not optimal or feasible. Status: %d\n", mip_status);
    glp_delete_prob(lp);
    return (OutputData){0, 0, NULL, false};
  }

  double obj_val = glp_mip_obj_val(lp);
  printf("\nSolution found!\n");
  printf("Objective value: %f\n", obj_val);

  OutputData output_data = {.objective_value = obj_val,
                            .num_assignments = 0,
                            .assignments = NULL,
                            .ran_successfully = true};

  int num_vars = glp_get_num_cols(lp);
  for (int i = 1; i < num_vars; i++) {
    double val = glp_mip_col_val(lp, i);
    if (val > 0.5) {
      int emp, day, shift;
      glp_employee_vec_index_reverse(&data->config, i, &emp, &day, &shift);
      output_data.num_assignments++;
      output_data.assignments =
          realloc(output_data.assignments,
                  output_data.num_assignments * sizeof(ShiftAssignment));
      output_data.assignments[output_data.num_assignments - 1].employee_name =
          data->employee_info[emp].name;
      output_data.assignments[output_data.num_assignments - 1].sprint_day_idx =
          day;
      output_data.assignments[output_data.num_assignments - 1].shift_idx =
          shift;
    }
  }

  glp_delete_prob(lp);
  return output_data;
}

static OutputData process(InputData *data, const char *output_dir) {
  return main_solve(data, output_dir);
}

const DataProcessor MATH_PROCESSOR = {
    .process = process,
};
