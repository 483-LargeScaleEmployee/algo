#include "../../include/io/csv_importer.h"
#include <complex.h>
#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

// employee_type -> employee_index -> sprint_day -> shift_index (0-3, index 4
// for if employee requested day off)
typedef struct {
  uint8_t cube[3][1000][14][4];
} EmployeeCube;

// 4 dimensions: department -> sprint day -> shift -> employee type -> employees
// needed
typedef struct {
  uint8_t cube[10][14][3][3];
} DataCube; // data cube is convention of how professor was referring to this

// String representations
static const char *SprintDayNames[] = {
    "Monday",        "Tuesday",        "Wednesday",     "Thursday",
    "Friday",        "Saturday",       "Sunday",        "Next Monday",
    "Next Tuesday",  "Next Wednesday", "Next Thursday", "Next Friday",
    "Next Saturday", "Next Sunday"};

static const char *ShiftNames[] = {"Night", "Morning", "Evening"};

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

int import_data_cube(const char *departments_dir, DataCube *cube) {
  if (cube == NULL) {
    fprintf(stderr, "Error: DataCube pointer is NULL\n");
    return -1;
  }
  memset(cube, 0, sizeof(DataCube));
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

InputData *import_csv(const char *input_dir) {
  char departments_dir[256];
  snprintf(departments_dir, sizeof(departments_dir), "%s/departments",
           input_dir);

  DataCube cube;
  import_data_cube(departments_dir, &cube);

  return NULL;
}

void destroy_generic_data(InputData *data) {
  // Implementation
}
