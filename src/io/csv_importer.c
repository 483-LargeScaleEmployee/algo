#include "../../include/io/csv_importer.h"
#include <assert.h>
#include <complex.h>
#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>

MetadataStrings parse_metadata_csv(const char *input_dir,
                                   FiveDimensionConfig *config) {
  char filepath[256];
  char line[1024];
  FILE *file;
  MetadataStrings strings;

  strings.sprint_day_names = malloc(256 * sizeof(char *));
  strings.shift_names = malloc(256 * sizeof(char *));
  strings.employee_type_names = malloc(256 * sizeof(char *));
  strings.department_names = malloc(256 * sizeof(char *));

  config->num_sprint_days = 0;
  config->num_shifts = 0;
  config->num_employee_types = 0;
  config->num_departments = 0;

  snprintf(filepath, sizeof(filepath), "%s/metadata.csv", input_dir);
  file = fopen(filepath, "r");
  if (!file) {
    fprintf(stderr, "Failed to open %s\n", filepath);
    exit(1);
  }

  // Skip header
  fgets(line, sizeof(line), file);

  // Read each line
  while (fgets(line, sizeof(line), file)) {
    char type[64], name[1024];
    sscanf(line, "%[^,],%[^\n\r]", type, name);

    // Remove any quotes if present
    char *name_start = name;
    if (name[0] == '"') {
      name_start++;
      name[strlen(name) - 1] = '\0'; // Remove end quote
    }

    if (strcmp(type, "sprint_day") == 0) {
      strings.sprint_day_names[config->num_sprint_days] = strdup(name_start);
      config->num_sprint_days++;
    } else if (strcmp(type, "shift") == 0) {
      strings.shift_names[config->num_shifts] = strdup(name_start);
      config->num_shifts++;
    } else if (strcmp(type, "department_type") == 0) {
      strings.department_names[config->num_departments] = strdup(name_start);
      config->num_departments++;
    } else if (strcmp(type, "employee_type") == 0) {
      strings.employee_type_names[config->num_employee_types] =
          strdup(name_start);
      config->num_employee_types++;
    }
  }
  fclose(file);

  return strings;
}

char *parse_csv_field(char **line_ptr) {
  static char field[100];
  char *cur = *line_ptr;
  char *dest = field;
  bool in_quotes = false;

  while (*cur && (in_quotes || *cur != ',') && dest - field <= 100) {
    if (*cur == '"') {
      in_quotes = !in_quotes;
    } else {
      *dest++ = *cur;
    }
    cur++;
  }

  *dest = '\0';

  if (*cur == ',') {
    cur++;
  }

  *line_ptr = cur;
  return field;
}

int import_department_vec(const char *departments_dir, DepartmentVec *dep_vec,
                          MetadataStrings *metadata,
                          FiveDimensionConfig *config) {
  if (dep_vec == NULL) {
    printf("Error: DepartmentVec pointer is NULL\n");
    return -1;
  }

  // Iterate through known department names from metadata
  for (int j = 0; j < config->num_departments; j++) {
    char file_path[512];
    snprintf(file_path, sizeof(file_path), "%s/%s.csv", departments_dir,
             metadata->department_names[j]);

    FILE *file = fopen(file_path, "r");
    if (file == NULL) {
      printf("Error opening department file '%s': %s\n", file_path,
             strerror(errno));
      continue; // Skip to next department if file can't be opened
    }

    bool is_first_line = true;
    uint8_t sprint_day_idx;
    uint8_t shift_idx;
    char line[2048];
    while (fgets(line, sizeof(line), file)) {
      if (is_first_line) {
        is_first_line = false;
        continue;
      }

      char *line_ptr = line;
      char *field = parse_csv_field(&line_ptr);

      for (int i = 0; i < config->num_employee_types + 2; i++) {
        if (i == 0) {
          // find which sprint day it is
          for (int k = 0; k < config->num_sprint_days; k++) {
            if (strcmp(field, metadata->sprint_day_names[k]) == 0) {
              sprint_day_idx = k;
              break;
            }
          }
        } else if (i == 1) {
          // find which shift it is
          for (int k = 0; k < config->num_shifts; k++) {
            if (strcmp(field, metadata->shift_names[k]) == 0) {
              shift_idx = k;
              break;
            }
          }
        } else {
          uint8_t employee_type_idx = i - 2;

          dep_vec->vec[DEPARTMENT_VEC_INDEX(config, j, employee_type_idx,
                                            sprint_day_idx, shift_idx)] =
              atoi(field);
        }
        field = parse_csv_field(&line_ptr);
      }
    }

    fclose(file);
  }

  return 0;
}

int import_employee_vec(const char *employee_file_path,
                        EmployeeVec *employee_vec, EmployeeInfo *employee_info,
                        const MetadataStrings *metadata,
                        FiveDimensionConfig *config) {
  if (employee_vec == NULL) {
    fprintf(stderr, "Error: EmployeeVec pointer is NULL\n");
    return -1;
  }

  FILE *file = fopen(employee_file_path, "r");
  if (file == NULL) {
    fprintf(stderr, "Error opening file: %s\n", employee_file_path);
    return -1;
  }

  char line[4096];
  int employee_index = -1;
  while (fgets(line, sizeof(line), file)) {
    if (employee_index == -1) {
      employee_index++;
      continue;
    }

    char *line_ptr = line;
    char *field = parse_csv_field(&line_ptr);

    // +2 for name and employee type
    for (int i = 0; i < config->num_sprint_days + 2; i++) {
      if (i == 0) {
        strcpy(employee_info[employee_index].name, field);
      } else if (i == 1) {
        for (int j = 0; j < config->num_employee_types; j++) {
          if (strcmp(field, metadata->employee_type_names[j]) == 0) {
            employee_info[employee_index].employee_type = j;
            break;
          }
        }
      } else {
        int sprint_day_idx = i - 2;
        // there will be two binary strings: like 001, 100
        // split field into two binary strings
        char field_copy[100];
        strcpy(field_copy, field);
        char *subfield = strtok(field_copy, " ");

        // Process first binary string (availability)
        // If they have requested day off, then it is X, since we use
        // calloc, we don't need to set anything to 0
        bool skip_next = false;
        if (subfield != NULL) {
          if (subfield[0] == 'X') {
            // set all to 0 in both
            for (int k = 0; k < config->num_shifts; k++) {
              employee_vec->vec[EMPLOYEE_VEC_INDEX(config, employee_index,
                                                   sprint_day_idx, k)] = 0;
              employee_vec->vec[EMPLOYEE_VEC_INDEX(config, employee_index,
                                                   sprint_day_idx, k) +
                                1] = 0;
            }
            skip_next = true;
          } else {
            for (int k = 0; k < config->num_shifts && subfield[k] != '\0';
                 k++) {

              if (subfield[k] == '1') {
                employee_vec->vec[EMPLOYEE_VEC_INDEX(config, employee_index,
                                                     sprint_day_idx, k)] = 1;
              } else if (subfield[k] == '0') {
                employee_vec->vec[EMPLOYEE_VEC_INDEX(config, employee_index,
                                                     sprint_day_idx, k)] = 0;
              } else if (subfield[k] != '0') {
                break;
              }
            }
          }
        }

        if (!skip_next) {
          // Get and process second binary string (preference)
          subfield = strtok(NULL, " ");

          if (subfield != NULL) {
            for (int k = 0; k < config->num_shifts && subfield[k] != '\0';
                 k++) {
              if (subfield[k] == '1') {
                employee_vec->vec[EMPLOYEE_VEC_INDEX(config, employee_index,
                                                     sprint_day_idx, k) +
                                  1] = 1;
              } else if (subfield[k] == '0') {
                employee_vec->vec[EMPLOYEE_VEC_INDEX(config, employee_index,
                                                     sprint_day_idx, k) +
                                  1] = 0;
              } else if (subfield[k] != '0') {
                break;
              }
            }
          }
        }
      }
      field = parse_csv_field(&line_ptr);
      // parse_csv field always just gets the next field
    }
    employee_index++;
  }
  fclose(file);
  return 0;
}

int handle_department_data(FiveDimensionConfig *config, InputData *data,
                           const char *input_dir) {
  //
  // Allocate department vector
  //
  size_t department_vec_size = config->num_departments *
                               config->num_employee_types *
                               config->num_sprint_days * config->num_shifts;
  printf("Department vec size: %zu\n", department_vec_size);
  data->department_vec.vec = malloc(department_vec_size * sizeof(uint8_t));

  //
  // Import department data
  //
  char departments_dir[256];
  snprintf(departments_dir, sizeof(departments_dir), "%s/departments",
           input_dir);
  int result = import_department_vec(departments_dir, &data->department_vec,
                                     &data->metadata, config);
  if (result != 0) {
    printf("Failed to import department data\n");
    return -1;
  }
  return 0;
}

int handle_employee_data(FiveDimensionConfig *config, InputData *data,
                         const char *input_dir) {
  data->employee_info = malloc(config->num_employees * sizeof(EmployeeInfo));
  //
  // Allocate employee vector
  //
  size_t employee_vec_size = config->num_employees * config->num_sprint_days *
                             config->num_shifts * 2; // 2 u8s per entry
  data->employee_vec.vec = malloc(employee_vec_size * sizeof(uint8_t));

  printf("Department 0 name: %s\n", data->metadata.department_names[0]);

  //
  // Import employee data
  //
  char filepath[256];
  snprintf(filepath, sizeof(filepath), "%s/employees.csv", input_dir);
  int result =
      import_employee_vec(filepath, &data->employee_vec, data->employee_info,
                          &data->metadata, config);

  if (result != 0) {
    printf("Failed to import employee data\n");
    return -1;
  }
  return 0;
}

InputData *import_csv(const char *input_dir) {
  InputData *data = malloc(sizeof(InputData));

  FiveDimensionConfig *config = &data->config;
  data->metadata = parse_metadata_csv(input_dir, config);
  // Free the original metadata struct (but not its contents since they were
  // copied)

  handle_department_data(config, data, input_dir);
  // handle_employee_data(config, data, input_dir);

  return data;
}
