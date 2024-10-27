#include "../../include/math/math_processor.h"
#include "../../include/glpk.h"
#include "../../include/math/constraints.h"
#include "../../include/math/glp_utils.h"
#include "../../include/math/preferences.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void main_solve(const InputData *data) {
  glp_prob *lp = glp_create_prob();
  glp_set_prob_name(lp, "Employee_Schedule");
  glp_set_obj_dir(lp, GLP_MAX); // Maximize preferred shift matching

  int cur_row = 1; // Start at row 1 (GLPK uses 1-based indexing)

  // Set up preferences and binary variables
  set_employee_preferences(lp, &data->employee_vec, &data->config);

  // Add constraints
  add_availability_constraint(lp, &data->employee_vec, &data->config, &cur_row);
  // add_department_needs_constraint(lp, &data->department_vec, &data->config,
  //                                 data, &cur_row);

  // Print problem size for debugging
  printf("Problem size:\n");
  printf("Rows (constraints): %d\n", glp_get_num_rows(lp));
  printf("Columns (variables): %d\n", glp_get_num_cols(lp));
  printf("Non-zeros: %d\n", glp_get_num_nz(lp));

  // Configure and solve LP relaxation
  glp_smcp smcp;
  glp_init_smcp(&smcp);
  smcp.msg_lev = GLP_MSG_ALL; // Show all messages for debugging
  smcp.meth = GLP_PRIMAL;     // Use primal simplex
  smcp.presolve = GLP_ON;     // Enable presolver

  printf("\nSolving LP relaxation...\n");
  int lp_result = glp_simplex(lp, &smcp);
  if (lp_result != 0) {
    printf("Failed to solve LP relaxation: %d\n", lp_result);
    glp_delete_prob(lp);
    return;
  }

  // Check LP solution status
  int lp_status = glp_get_status(lp);
  if (lp_status != GLP_OPT) {
    printf("LP relaxation not optimal. Status: %d\n", lp_status);
    glp_delete_prob(lp);
    return;
  }

  printf("LP relaxation solved. Objective: %f\n", glp_get_obj_val(lp));

  // Configure and solve integer problem
  glp_iocp iocp;
  glp_init_iocp(&iocp);
  iocp.msg_lev = GLP_MSG_ALL; // Show all messages
  iocp.presolve = GLP_ON;     // Enable presolver
  iocp.br_tech = GLP_BR_PCH;  // Hybrid branching
  iocp.bt_tech = GLP_BT_BLB;  // Best local bound
  iocp.fp_heur = GLP_ON;      // Enable feasibility pump
  iocp.tm_lim = 60000;        // 60 second time limit

  printf("\nSolving MIP problem...\n");
  int mip_result = glp_intopt(lp, &iocp);
  if (mip_result != 0) {
    printf("Failed to solve MIP problem: %d\n", mip_result);
    glp_delete_prob(lp);
    return;
  }

  // Check MIP solution status
  int mip_status = glp_mip_status(lp);
  if (mip_status != GLP_OPT) {
    printf("MIP solution not optimal. Status: %d\n", mip_status);
    if (mip_status == GLP_FEAS) {
      printf("But feasible solution found.\n");
    }
    glp_delete_prob(lp);
    return;
  }

  // Process results
  double obj_val = glp_mip_obj_val(lp);
  printf("\nSolution found!\n");
  printf("Objective value: %f\n", obj_val);

  // Get solution values
  int num_vars = glp_get_num_cols(lp);
  int assignments = 0;
  for (int i = 1; i <= num_vars; i++) {
    double val = glp_mip_col_val(lp, i);
    if (val > 0.5) {
      int emp, day, shift;
      glp_employee_vec_index_reverse(&data->config, i, &emp, &day, &shift);
      printf("Employee %d (%s) assigned to day %d, shift %d\n", emp,
             data->employee_info[emp].name, day, shift);
    }
  }
  printf("\nTotal assignments: %d\n", assignments);

  glp_delete_prob(lp);
}

static StandardizedOutput *process(InputData *data) {
  main_solve(data);
  return NULL;
}

const DataProcessor MATH_PROCESSOR = {
    .process = process,
};
