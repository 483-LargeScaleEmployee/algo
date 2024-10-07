#include "../../include/io/csv_importer.h"
#include <complex.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#define mkdir(path, mode) _mkdir(path)

struct dirent {
  char d_name[MAX_PATH];
};

typedef struct {
  WIN32_FIND_DATA find_data;
  HANDLE find_handle;
  struct dirent entry;
  int eof;
} DIR;

DIR *opendir(const char *name);
struct dirent *readdir(DIR *dir);
int closedir(DIR *dir);

DIR *opendir(const char *name) {
  DIR *dir = malloc(sizeof(DIR));
  if (dir) {
    char path[MAX_PATH];
    snprintf(path, sizeof(path), "%s\\*", name);
    dir->find_handle = FindFirstFile(path, &dir->find_data);
    dir->eof = (dir->find_handle == INVALID_HANDLE_VALUE);
  }
  return dir;
}

struct dirent *readdir(DIR *dir) {
  if (dir->eof) return NULL;
  strcpy(dir->entry.d_name, dir->find_data.cFileName);
  dir->eof = !FindNextFile(dir->find_handle, &dir->find_data);
  return &dir->entry;
}

int closedir(DIR *dir) {
  if (dir) {
    if (dir->find_handle != INVALID_HANDLE_VALUE) {
      FindClose(dir->find_handle);
    }
    free(dir);
  }
  return 0;
}
#else
#include <dirent.h>
#include <sys/stat.h>
#endif

const int DEPARTMENT_COUNT = 10;

uint8_t get_employee_type_index(const char *employee_type) {
  for (uint8_t i = 0; i < 3; i++) {
    if (strcmp(employee_type, EmployeeTypes[i]) == 0)
      return i;
  }
  return -1;
}

uint8_t get_sprint_day_index(const char *day) {
  for (uint8_t i = 0; i < 14; i++) {
    if (strcmp(day, SprintDayNames[i]) == 0)
      return i;
  }
  return -1;
}

uint8_t get_shift_index(const char *shift) {
  for (uint8_t i = 0; i < 3; i++) {
    if (strcmp(shift, ShiftNames[i]) == 0)
      return i;
  }
  return -1;
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

int import_employee_cube(const char *employee_file_path, EmployeeCube *cube,
                         EmployeeInfo *employee_info) {
  if (cube == NULL) {
    fprintf(stderr, "Error: EmployeeCube pointer is NULL\n");
    return -1;
  }
  memset(cube, 0, sizeof(EmployeeCube));

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

    char *newline = strchr(line, '\n');
    if (newline) {
      *newline = '\0';
    }

    char *line_ptr = line;
    int field_count = 0;
    while (field_count < 14) {
      char *field = parse_csv_field(&line_ptr);
      if (field_count == 0) {
        strcpy(employee_info[employee_index].name, field);
      } else if (field_count == 1) {
        employee_info[employee_index].employee_type =
            get_employee_type_index(field);
      } else {
        uint32_t sprint_day_index = field_count - 2;
        // need to parse, first the array, then the shift, then a boolean
        // the field is a string, each subfield is separated by a space
        uint32_t subfield_index = 0;
        char *subfield = strtok(field, " ");
        while (subfield != NULL) {
          if (subfield_index == 0) {
            cube->cube[employee_index][sprint_day_index][0] = atoi(subfield);
          } else if (subfield_index == 1) {
            // this is the preferred shift, it will match one of ShiftNames
            if (strcmp(subfield, "Night") == 0) {
              cube->cube[employee_index][sprint_day_index][1] = 0;
            } else if (strcmp(subfield, "Morning") == 0) {
              cube->cube[employee_index][sprint_day_index][1] = 1;
            } else if (strcmp(subfield, "Evening") == 0) {
              cube->cube[employee_index][sprint_day_index][1] = 2;
            }
          } else if (subfield_index == 2) {
            // True means they have requested day off so we set available shifts
            // to 0, indicating they have no available shifts
            if (strcmp(subfield, "True") == 0) {
              cube->cube[employee_index][sprint_day_index][0] = 0;
            }
          }
          subfield = strtok(NULL, " ");
          subfield_index++;
        }
      }
      field_count++;
    }
    employee_index++;
  }

  fclose(file);
  return 0;
}

int import_department_cube(const char *departments_dir, DepartmentCube *cube) {
  if (cube == NULL) {
    fprintf(stderr, "Error: DepartmentCube pointer is NULL\n");
    return -1;
  }
  memset(cube, 0, sizeof(DepartmentCube));
  DIR *dir = opendir(departments_dir);
  if (dir == NULL) {
    fprintf(stderr, "Error opening directory %s: %s\n", departments_dir,
            strerror(errno));
    return -1; // Changed from NULL to -1 to match the function return type
  }
  struct dirent *entry;
  uint8_t department_index = 0;
  while ((entry = readdir(dir)) != NULL) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
      continue;
    }
    char file_path[512];
    snprintf(file_path, sizeof(file_path), "%s/%s", departments_dir,
             entry->d_name);
    FILE *file = fopen(file_path, "r");
    if (file == NULL) {
      fprintf(stderr, "Error opening file %s: %s\n", file_path,
              strerror(errno));
      continue;
    }
    bool is_first_line = true;
    char line[2048];
    while (fgets(line, sizeof(line), file)) {
      if (is_first_line) {
        is_first_line = false;
        continue;
      }
      char sprint_day[20], shift_time[10];
      int count_type_a, count_type_b, count_type_c; // Changed to int for sscanf
      if (sscanf(line, "%19[^,],%9[^,],%d,%d,%d\n", sprint_day, shift_time,
                 &count_type_a, &count_type_b, &count_type_c) == 5) {
        uint8_t sprint_day_index = get_sprint_day_index(sprint_day);
        uint8_t shift_index = get_shift_index(shift_time);
        cube->cube[department_index][sprint_day_index][shift_index][0] =
            (uint8_t)count_type_a;
        cube->cube[department_index][sprint_day_index][shift_index][1] =
            (uint8_t)count_type_b;
        cube->cube[department_index][sprint_day_index][shift_index][2] =
            (uint8_t)count_type_c;
      } else {
        fprintf(stderr, "Invalid line format in file %s: %s", entry->d_name,
                line);
      }
    }
    fclose(file);
    department_index++;
  }
  closedir(dir);
  return 0;
}

InputData import_csv(const char *input_dir) {
  char departments_dir[256];
  snprintf(departments_dir, sizeof(departments_dir), "%s/departments",
           input_dir);

  DepartmentCube departments_cube;
  import_department_cube(departments_dir, &departments_cube);

  char employee_path[256];
  strcpy(employee_path, input_dir);
  strcat(employee_path, "/employees.csv");

  EmployeeCube employee_cube;
  EmployeeInfo employee_info[1000];
  import_employee_cube(employee_path, &employee_cube, employee_info);

  InputData data = {.employee_info = *employee_info,
                    .employee_cube = employee_cube,
                    .department_cube = departments_cube};
  return data;
}
