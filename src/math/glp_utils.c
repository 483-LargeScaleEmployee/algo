#include "../../include/math/glp_utils.h"

int glp_employee_vec_index(FiveDimensionConfig *config, int emp, int day,
                           int shift) {
  return ((emp)*config->num_sprint_days * config->num_shifts +
          (day)*config->num_shifts + (shift)) +
         1; // +1 for GLPK 1-based indexing
}

// Takes a GLPK index and returns emp, day, shift components
// This is necessary for reading the solution from GLPK
void glp_employee_vec_index_reverse(FiveDimensionConfig *config, int glp_idx,
                                    int *emp, int *day, int *shift) {
  // Subtract 1 to handle GLPK 1-based indexing
  glp_idx--;

  // Undo the forward calculation:
  // idx = (emp * sprint_days * shifts) + (day * shifts) + shift
  int shifts_per_emp = config->num_sprint_days * config->num_shifts;

  *emp = glp_idx / shifts_per_emp;
  glp_idx %=
      shifts_per_emp; // remove employee portion, just left with days * shifts

  *day = glp_idx / config->num_shifts; // remove days portion, just left with shifts
  *shift = glp_idx % config->num_shifts;
}
