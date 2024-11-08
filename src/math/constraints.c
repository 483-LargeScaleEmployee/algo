#include "../../include/glpk.h"
#include "../../include/math/glp_utils.h"
#include <stdint.h>

// employees can't be scheduled if they're unavailable
void add_availability_constraint(glp_prob *lp, EmployeeVec *employee_vec,
                                 FiveDimensionConfig *config, int *cur_row) {
  // For each employee, day, and shift, they can't be scheduled if they're
  // unavailable
  int num_constraints = glp_schedule_vec_size(config);

  int row = *cur_row;

  glp_add_rows(lp, num_constraints);

  for (int dep = 0; dep < config->num_departments; dep++) {
    for (int emp = 0; emp < config->num_employees; emp++) {
      for (int day = 0; day < config->num_sprint_days; day++) {
        for (int shift = 0; shift < config->num_shifts; shift++) {
          int vec_idx = EMPLOYEE_VEC_INDEX(config, emp, day, shift);
          int glpk_col = glp_schedule_vec_index(config, dep, emp, day, shift);

          uint8_t availability = employee_vec->vec[vec_idx];

          // Set constraint bounds
          glp_set_row_bnds(lp, row, GLP_UP, 0.0, availability ? 1.0 : 0.0);

          // Create arrays for the coefficient matrix:
          // ind[0] is unused (GLPK uses 1-based indexing)
          // ind[1] contains the column number for this employee/day/shift
          // variable
          int ind[2] = {0, glpk_col};

          // val[0] is unused (matches ind[0])
          // val[1] = 1.0 means this constraint is: 1.0 * variable <=
          // upper_bound
          double val[2] = {0.0, 1.0};

          glp_set_mat_row(lp, row, 1, ind, val);

          row++;
        }
      }
    }
  }
  *cur_row = row;
}

// // department needs must be met
// void add_department_needs_constraint(glp_prob *lp,
//                                      DepartmentVec *department_vec,
//                                      FiveDimensionConfig *config,
//                                      InputData *input_data, int *cur_row) {
//   // department * employee type * sprint day * shift, there will be a num of
//   // employees needed
//   int num_constraints = config->num_departments * config->num_employee_types *
//                         config->num_sprint_days * config->num_shifts;
//   glp_add_rows(lp, num_constraints);
//   int row = *cur_row;
//
//   // Allocate arrays for the coefficient matrix
//   // We need space for all employees since each could potentially fill this role
//   int *ind = malloc((config->num_employees + 1) * sizeof(int));
//   double *val = malloc((config->num_employees + 1) * sizeof(double));
//
//   for (int dep = 0; dep < config->num_departments; dep++) {
//     for (int emp_type = 0; emp_type < config->num_employee_types; emp_type++) {
//       for (int day = 0; day < config->num_sprint_days; day++) {
//         for (int shift = 0; shift < config->num_shifts; shift++) {
//           int vec_idx = DEPARTMENT_VEC_INDEX(config, dep, emp_type, day, shift);
//           uint8_t needed = department_vec->vec[vec_idx];
//
//           // Set up constraint: sum of assigned employees = needed
//           glp_set_row_bnds(lp, row, GLP_FX, needed, needed);
//
//           // For this constraint, we need to sum up all employees of the right
//           // type
//           int num_nonzero = 0;
//
//           for (int emp = 0; emp < config->num_employees; emp++) {
//             if (input_data->employee_info[emp].employee_type == emp_type) {
//               num_nonzero++;
//
//               // Calculate the column number for this employee/day/shift
//               int glpk_col = glp_employee_vec_index(config, emp, day, shift);
//
//               // reminder ind is indexed from 1, it is the column idx
//               // val is the coefficient, if an employee is scheduled here
//               // then it will count as 1.
//               ind[num_nonzero] = glpk_col;
//               val[num_nonzero] = 1.0; // Each employee counts as 1
//             }
//           }
//
//           glp_set_mat_row(lp, row, num_nonzero, ind, val);
//
//           row++;
//         }
//       }
//     }
//   }
//   *cur_row = row;
// }
