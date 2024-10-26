#include "../../include/math/math_processor.h"
#include "math_utils.c"
#include "../../include/glpk.h"
#include <stdio.h>
#include <stdlib.h>

int example_solve() {
  glp_prob *lp;
  int ia[1 + 1000], ja[1 + 1000];
  double ar[1 + 1000];

  // Create problem
  lp = glp_create_prob();
  glp_set_prob_name(lp, "sample");
  glp_set_obj_dir(lp, GLP_MAX);

  // Define columns (variables)
  glp_add_cols(lp, 2);
  glp_set_col_name(lp, 1, "x1");
  glp_set_col_name(lp, 2, "x2");
  glp_set_col_bnds(lp, 1, GLP_LO, 0.0, 0.0); // x1 >= 0
  glp_set_col_bnds(lp, 2, GLP_LO, 0.0, 0.0); // x2 >= 0
  glp_set_obj_coef(lp, 1, 10.0);             // cost coefficient in objective
  glp_set_obj_coef(lp, 2, 6.0);

  // Define constraints
  glp_add_rows(lp, 2);
  glp_set_row_name(lp, 1, "c1");
  glp_set_row_name(lp, 2, "c2");
  glp_set_row_bnds(lp, 1, GLP_UP, 0.0, 100.0); // constraint <= 100
  glp_set_row_bnds(lp, 2, GLP_UP, 0.0, 600.0); // constraint <= 600

  // Load constraint matrix
  ia[1] = 1, ja[1] = 1, ar[1] = 1.0;  // a[1,1] = 1
  ia[2] = 1, ja[2] = 2, ar[2] = 1.0;  // a[1,2] = 1
  ia[3] = 2, ja[3] = 1, ar[3] = 10.0; // a[2,1] = 10
  ia[4] = 2, ja[4] = 2, ar[4] = 4.0;  // a[2,2] = 4
  glp_load_matrix(lp, 4, ia, ja, ar);

  // Solve the problem
  glp_simplex(lp, NULL);

  // Retrieve solution
  double z = glp_get_obj_val(lp);
  double x1 = glp_get_col_prim(lp, 1);
  double x2 = glp_get_col_prim(lp, 2);

  printf("\nObj = %g; x1 = %g; x2 = %g\n", z, x1, x2);

  // Free memory
  glp_delete_prob(lp);
  return 0;
}

static StandardizedOutput *process(InputData *data) {
  // glp_prob *lp;

  size_t total_constraint_rows = get_total_constraint_rows();
  // a[ia[i], ja[i]] = ar[i], so if i = 1, then this is the val of the
  // constraint of the first row and first column
  int ia[1 + total_constraint_rows], ja[1 + total_constraint_rows],
      ar[1 + total_constraint_rows];

  example_solve();

  // Process data
  return NULL;
}

const DataProcessor MATH_PROCESSOR = {
    .process = process,
};
