#include <stdio.h>
#include <stdbool.h>

#define MAX_SHIFTS_PER_WEEK 5
#define MAX_CONSECUTIVE_SHIFTS 2
#define SHIFTS_PER_DAY 3
#define DAYS_IN_WEEK 7

// Data structures to hold nurse information
typedef struct {
    int type;  // Nurse type (A=0, B=1, C=2)
    int availability[DAYS_IN_WEEK * SHIFTS_PER_DAY];  // 0 for not available, 1 for available
    int preferences[DAYS_IN_WEEK * SHIFTS_PER_DAY];  // 0 for non-preferred, 1 for preferred
    int days_off[DAYS_IN_WEEK];  // 1 for day off, 0 otherwise
} Nurse;

// Track shift data for each nurse
typedef struct {
    int shift_count;  // Number of shifts worked this week
    int consecutive_shifts;  // Number of consecutive shifts worked
    int last_worked_day;  // Last day the nurse worked
} NurseTracker;

// Hard and soft constraints checker function
bool can_assign(Nurse nurse, NurseTracker tracker, int day, int shift) {
    // Hard Constraint 1: Check if nurse has exceeded max shifts per week
    if (tracker.shift_count >= MAX_SHIFTS_PER_WEEK) {
        return false;
    }

    // Hard Constraint 2: Check if nurse has exceeded consecutive shifts and worked the previous day
    if (tracker.consecutive_shifts >= MAX_CONSECUTIVE_SHIFTS && tracker.last_worked_day == day - 1) {
        return false;
    }

    // Hard Constraint 3: Check if nurse is available for this shift
    if (nurse.availability[day * SHIFTS_PER_DAY + shift] == 0) {
        return false;
    }

    // Hard Constraint 4: Check if nurse requested the day off
    if (nurse.days_off[day] == 1) {
        return false;
    }

    // If all hard constraints are satisfied, return true (soft constraints can be handled separately)
    return true;
}


#include <string.h>

#define MAX_NURSES 1000
#define MAX_DEPARTMENTS 10

// Department structure to hold assigned nurses
typedef struct {
    int assigned_nurses[MAX_NURSES];  // List of assigned nurses (IDs)
    int assigned_count;  // Number of nurses assigned to the shift
} Department;

// Function to assign nurses to shifts
void assign_nurses_to_shifts(Nurse nurses[], NurseTracker trackers[], int shift_needs[MAX_DEPARTMENTS][DAYS_IN_WEEK][SHIFTS_PER_DAY][3], Department departments[MAX_DEPARTMENTS][DAYS_IN_WEEK][SHIFTS_PER_DAY]) {
    // Loop over days, shifts, and departments
    for (int day = 0; day < DAYS_IN_WEEK; day++) {
        for (int shift = 0; shift < SHIFTS_PER_DAY; shift++) {
            for (int department = 0; department < MAX_DEPARTMENTS; department++) {
                // Loop over nurse types (A=0, B=1, C=2)
                for (int emp_type = 0; emp_type < 3; emp_type++) {
                    int needed_count = shift_needs[department][day][shift][emp_type];  // Nurses needed for this type

                    // Loop through all nurses to find suitable ones
                    for (int nurse_id = 0; nurse_id < MAX_NURSES; nurse_id++) {
                        if (nurses[nurse_id].type == emp_type && can_assign(nurses[nurse_id], trackers[nurse_id], day, shift)) {
                            // Assign nurse to the shift if they can be assigned
                            if (departments[department][day][shift].assigned_count < needed_count) {
                                departments[department][day][shift].assigned_nurses[departments[department][day][shift].assigned_count] = nurse_id;
                                departments[department][day][shift].assigned_count++;

                                // Update nurse's shift count and last worked day
                                trackers[nurse_id].shift_count++;
                                trackers[nurse_id].last_worked_day = day;
                                
                                // Update consecutive shift count
                                if (trackers[nurse_id].last_worked_day == day - 1) {
                                    trackers[nurse_id].consecutive_shifts++;
                                } else {
                                    trackers[nurse_id].consecutive_shifts = 1;
                                }

                                // Stop assigning once the needed number of nurses is reached
                                if (departments[department][day][shift].assigned_count >= needed_count) {
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
