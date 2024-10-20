#include "../../include/math/math_processor.h"
#include "../../include/io/input_data_funcs.h"
#include "math_utils.c"
// #include <glpk.h>
#include <stdlib.h>

static StandardizedOutput *process(InputData *data) {
  // glp_prob *lp;

  size_t total_constraint_rows = get_total_constraint_rows();
  // a[ia[i], ja[i]] = ar[i], so if i = 1, then this is the val of the
  // constraint of the first row and first column
  int ia[1 + total_constraint_rows], ja[1 + total_constraint_rows],
      ar[1 + total_constraint_rows];

  // lp = glp_create_prob();
  // glp_set_prob_name(lp, "shift_scheduling");
  // glp_set_obj_dir(lp, GLP_MAX);

  // Process data
  return NULL;
}

const DataProcessor MATH_PROCESSOR = {
    .process = process,
};
