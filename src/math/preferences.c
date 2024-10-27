#include "../../include/glpk.h"
#include "../../include/math/glp_utils.h"
#include <stdint.h>

void set_employee_preferences(glp_prob *lp, EmployeeVec *employee_vec,
                              FiveDimensionConfig *config) {
  int num_cols =
      config->num_employees * config->num_sprint_days * config->num_shifts;

  glp_add_cols(lp, num_cols);

  // GLPK uses 1-based indexing
  for (int emp = 0; emp < config->num_employees; emp++) {
    for (int day = 0; day < config->num_sprint_days; day++) {
      for (int shift = 0; shift < config->num_shifts; shift++) {
        int vec_idx = EMPLOYEE_VEC_INDEX(config, emp, day, shift);
        int glpk_col = glp_employee_vec_index(config, emp, day, shift);
        // Get preference value (second u8)
        double preference =
            employee_vec->vec[vec_idx + 1]; // +1 to get preference

        // Set objective coefficient
        glp_set_obj_coef(lp, glpk_col, preference ? 1.0 : -1.0);

        // Set column bounds
        // Assuming binary decision variables (0 or 1)
        glp_set_col_bnds(lp, glpk_col, GLP_DB, 0.0, 1.0);

        // Set column kind (binary)
        glp_set_col_kind(lp, glpk_col, GLP_BV);
      }
    }
  }
}
