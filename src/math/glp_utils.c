#include "../../include/math/glp_utils.h"

int glp_schedule_vec_size(FiveDimensionConfig *config) {
  return config->num_departments * config->num_employees *
         config->num_sprint_days * config->num_shifts;
}

int glp_schedule_vec_index(FiveDimensionConfig *config, int dep, int emp,
                           int day, int shift) {
  return ((dep)*config->num_employees * config->num_sprint_days *
              config->num_shifts +
          (emp)*config->num_sprint_days * config->num_shifts +
          (day)*config->num_shifts + (shift)) +
         1; // +1 for GLPK 1-based indexing
}

// Takes a GLPK index and returns emp, day, shift components
// This is necessary for reading the solution from GLPK
void glp_schedule_vec_index_reverse(FiveDimensionConfig *config, int glp_idx,
                                    int *emp, int *dep, int *day, int *shift) {
  // Subtract 1 to handle GLPK 1-based indexing
  glp_idx--;

  // Undo the forward calculation:
  // idx = (dep * employees * sprint_days * shifts) + (emp * sprint_days *
  // shifts) + (day * shifts) + shift
  int elements_per_dep =
      config->num_employees * config->num_sprint_days * config->num_shifts;
  *dep = glp_idx / elements_per_dep;

  glp_idx %= elements_per_dep; // remove department portion, just left with
                               // employees * days * shifts

  *emp = glp_idx / (config->num_sprint_days * config->num_shifts);
  glp_idx %= config->num_sprint_days *
             config->num_shifts; // remove employee portion,
                                 // just left with days * shifts

  *day = glp_idx / config->num_shifts;
  glp_idx %= config->num_shifts; // remove days portion, just left with shifts

  *shift = glp_idx;
}
