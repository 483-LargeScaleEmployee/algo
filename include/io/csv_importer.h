#pragma once
#include <stdint.h>

typedef struct {
  char name[100];
  uint8_t employee_type;
} EmployeeInfo;

// employee_index -> sprint_day ->
// first u8 is for available shifts: 8 possible options. from 0 shifts to all 3
// 0|No Available Shifts
// 1|Available Shift 2
// 2|Available Shift 1
// 3|Available Shifts 1 & 2
// 4|Available Shift 0
// 5|Available Shifts 0 & 2
// 6|Available Shifts 0 & 1
// 7|Available Shifts 0 & 1 & 2
// in this encoding, night is shift 0, morning is shift 1, evening is shift 2
//
// second u8 is for preferred shift: 0, 1, 2 for night, morning,
typedef struct {
  uint8_t cube[1000][14][2];
} EmployeeCube;

// 4 dimensions: department -> sprint day -> shift -> employee type -> employees
// needed
typedef struct {
  uint8_t cube[10][14][3][3];
} DepartmentCube;

static const char *SprintDayNames[] = {
    "Monday",        "Tuesday",        "Wednesday",     "Thursday",
    "Friday",        "Saturday",       "Sunday",        "Next Monday",
    "Next Tuesday",  "Next Wednesday", "Next Thursday", "Next Friday",
    "Next Saturday", "Next Sunday"};

static const char *ShiftNames[] = {"Night", "Morning", "Evening"};

static const char *EmployeeTypes[] = {"Nurse", "Doctor", "Admin"};

typedef struct {
  EmployeeInfo employee_info[1000];
  EmployeeCube employee_cube;
  DepartmentCube department_cube;
} InputData;

InputData import_csv(const char *input_dir);
