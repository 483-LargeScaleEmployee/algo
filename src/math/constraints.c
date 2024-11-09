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

// employees can only be scheduled for one specific element for each dep
// schrodingers cat analogy - employee can only be in one place at a time
void add_schrodingers_cat_constraint(glp_prob *lp, FiveDimensionConfig *config,
                                     int *cur_row) {
  int num_constraints =
      config->num_employees * config->num_sprint_days * config->num_shifts;
  int row = *cur_row;
  glp_add_rows(lp, num_constraints);

  for (int emp = 0; emp < config->num_employees; emp++) {
    for (int day = 0; day < config->num_sprint_days; day++) {
      for (int shift = 0; shift < config->num_shifts; shift++) {
        // Allocate arrays for the coefficient matrix
        // Add 1 to size for 1-based indexing
        int *ind = malloc((config->num_departments + 1) * sizeof(int));
        double *val = malloc((config->num_departments + 1) * sizeof(double));

        // Set up the constraint for this employee/day/shift combination
        // across all departments
        ind[0] = 0;   // Unused (GLPK 1-based indexing)
        val[0] = 0.0; // Unused

        // Fill arrays with column indices and coefficients
        for (int dep = 0; dep < config->num_departments; dep++) {
          ind[dep + 1] = glp_schedule_vec_index(config, dep, emp, day, shift);
          val[dep + 1] = 1.0; // Coefficient of 1 for each department variable
        }

        // Set constraint bounds:
        // sum of all department assignments for this emp/day/shift <= 1
        glp_set_row_bnds(lp, row, GLP_UP, 0.0, 1.0);

        // Add the constraint row to the matrix
        glp_set_mat_row(lp, row, config->num_departments, ind, val);

        // Free allocated memory
        free(ind);
        free(val);

        row++;
      }
    }
  }

  *cur_row = row;
}

void add_consecutive_shifts_constraint(glp_prob *lp,
                                       const EmployeeVec *employee_vec,
                                       const FiveDimensionConfig *config,
                                       int *cur_row) {
  int sliding_window_width = 3;

  // Calculate windows per employee
  int windows_per_employee =
      config->num_sprint_days * config->num_shifts - sliding_window_width + 1;
  int num_constraints =
      config->num_employees * windows_per_employee * config->num_departments;
  int row = *cur_row;
  glp_add_rows(lp, num_constraints);

  // Allocate arrays just for one window (plus 1 for GLPK's 1-based indexing)
  int *ind = malloc((sliding_window_width + 1) * sizeof(int));
  double *val = malloc((sliding_window_width + 1) * sizeof(double));

  for (int emp = 0; emp < config->num_employees; emp++) {
    for (int sliding_window_idx = 0; sliding_window_idx < windows_per_employee;
         sliding_window_idx++) {
      for (int dep = 0; dep < config->num_departments; dep++) {
        // Initialize arrays
        ind[0] = 0; // GLPK's 1-based indexing
        val[0] = 0.0;

        // Fill window slots
        for (int i = 0; i < sliding_window_width; i++) {
          int effective_idx = sliding_window_idx + i;
          int day = effective_idx / config->num_shifts;
          int shift = effective_idx % config->num_shifts;

          // Simple consecutive indexing for the window
          ind[i + 1] = glp_schedule_vec_index(config, dep, emp, day, shift);
          val[i + 1] = 1.0;
        }

        // Set constraint: sum of shifts in window <= 2
        glp_set_row_bnds(lp, row, GLP_UP, 0.0, 2.0);
        glp_set_mat_row(lp, row, sliding_window_width, ind, val);

        row++;
      }
    }
  }

  free(ind);
  free(val);
  *cur_row = row;
}

void add_department_needs_constraint(glp_prob *lp,
                                     const DepartmentVec *department_vec,
                                     const FiveDimensionConfig *config,
                                     const InputData *input_data,
                                     int *cur_row) {
  int num_constraints = config->num_departments * config->num_employee_types *
                        config->num_sprint_days * config->num_shifts;
  int row = *cur_row;
  glp_add_rows(lp, num_constraints);

  // Allocate maximum possible size for non-zero coefficients
  int *ind = malloc((config->num_employees + 1) * sizeof(int));
  double *val = malloc((config->num_employees + 1) * sizeof(double));

  for (int dep = 0; dep < config->num_departments; dep++) {
    for (int emp_type = 0; emp_type < config->num_employee_types; emp_type++) {
      for (int day = 0; day < config->num_sprint_days; day++) {
        for (int shift = 0; shift < config->num_shifts; shift++) {
          int dep_needs = department_vec->vec[DEPARTMENT_VEC_INDEX(
              config, dep, emp_type, day, shift)];

          // GLPK uses 1-based indexing
          ind[0] = 0;
          val[0] = 0.0;

          // Only include coefficients for matching employee types
          int non_zero_count = 0;
          for (int emp = 0; emp < config->num_employees; emp++) {
            if (input_data->employee_info[emp].employee_type == emp_type) {
              non_zero_count++;
              ind[non_zero_count] =
                  glp_schedule_vec_index(config, dep, emp, day, shift);
              val[non_zero_count] = 1.0;
            }
          }

          // Set the constraint: sum(matching_employees) = dep_needs
          glp_set_row_bnds(lp, row, GLP_FX, dep_needs, dep_needs);

          // Add the constraint row with only non-zero coefficients
          glp_set_mat_row(lp, row, non_zero_count, ind, val);

          row++;
        }
      }
    }
  }

  free(ind);
  free(val);
  *cur_row = row;
}
