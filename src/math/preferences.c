#include "../../include/glpk.h"
#include "../../include/math/glp_utils.h"
#include <stdint.h>

void set_employee_preferences(glp_prob *lp, EmployeeVec *employee_vec,
                              FiveDimensionConfig *config) {
  int num_cols = glp_schedule_vec_size(config);

  glp_add_cols(lp, num_cols);

  // GLPK uses 1-based indexing
  for (int emp = 0; emp < config->num_employees; emp++) {
    for (int day = 0; day < config->num_sprint_days; day++) {
      for (int shift = 0; shift < config->num_shifts; shift++) {
        int employee_vec_idx = EMPLOYEE_VEC_INDEX(config, emp, day, shift);
        // Get preference value (second u8)
        // preference is irrelevant of the department, it only refers to the
        // shift
        double preference =
            employee_vec->vec[employee_vec_idx + 1]; // +1 to get preference

        for (int dep = 0; dep < config->num_departments; dep++) {
          int glpk_col = glp_schedule_vec_index(config, dep, emp, day, shift);
          // Set objective coefficient
          glp_set_obj_coef(lp, glpk_col, preference ? 1.0 : 0.0);

          // Set column bounds
          // Assuming binary decision variables (0 or 1)
          glp_set_col_bnds(lp, glpk_col, GLP_DB, 0.0, 1.0);

          // Set column kind (binary)
          glp_set_col_kind(lp, glpk_col, GLP_BV);
        }
      }
    }
  }
}
