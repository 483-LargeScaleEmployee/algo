#include "../../include/io/csv_importer.h"
#include <complex.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef enum {
  DAY_1,
  DAY_2,
  DAY_3,
  DAY_4,
  DAY_5,
  DAY_6,
  DAY_7,
  DAY_8,
  DAY_9,
  DAY_10,
  DAY_11,
  DAY_12,
  DAY_13,
  DAY_14
} SprintDay;

// 12-8, 8-4, 4-12
typedef enum { NIGHT, MORNING, EVENING } Shift;

typedef enum { A, B, C } EmployeeType;

typedef struct {
  bool available_shifts[3];
  Shift preferred_shift;
  bool request_day_off;
} EmployeeDay;

typedef struct {
  char name[50];
  EmployeeType employeeType;
  EmployeeDay employee_days[14];
} Employee;

// 4 dimensions: department -> day -> shift -> employee type -> employees needed
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

InputData *import_csv(const char *filename) {

  // Implementation
  return NULL;
}

void destroy_generic_data(InputData *data) {
  // Implementation
}
