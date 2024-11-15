#include "../../include/out/csv_exporter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char* employee_name;
    int employee_type_idx;
    int department_idx;
    int* sprint_days;
    int* shifts;
    int num_entries;
} GroupedAssignment;

static int compare_assignments(const void *a, const void *b) {
    const ShiftAssignment *aa = (const ShiftAssignment *)a;
    const ShiftAssignment *bb = (const ShiftAssignment *)b;
    
    int name_cmp = strcmp(aa->employee_name, bb->employee_name);
    if (name_cmp != 0) return name_cmp;
    
    return aa->department_idx - bb->department_idx;
}

static void write_csv_line(FILE* file, 
                          const char* name, 
                          const char* type, 
                          const char* dept,
                          const int* days,
                          const int* shifts,
                          int count) {
    fprintf(file, "%s,%s,%s,(", name, type, dept);
    
    // Write days
    for (int i = 0; i < count; i++) {
        fprintf(file, "%d%s", days[i], (i < count - 1) ? " " : "");
    }
    
    fprintf(file, "),(");
    
    // Write shifts
    for (int i = 0; i < count; i++) {
        fprintf(file, "%d%s", shifts[i], (i < count - 1) ? " " : "");
    }
    
    fprintf(file, ")\n");  // End the line correctly
}

static bool write_schedule_csv(const char *output_dir,
                             const OutputData *output,
                             const InputData *data,
                             bool is_human_readable) {
    // Prepare the filepath
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s/%s", output_dir,
             is_human_readable ? "schedule_human.csv" : "schedule_machine.csv");
    
    FILE *file = fopen(filepath, "w");
    if (!file) return false;
    
    // Write header
    fprintf(file, "%s,%s,%s,sprint_days,shifts\n",
            is_human_readable ? "Employee" : "employee_name",
            "Title",
            is_human_readable ? "Department" : "department_name");
    
    // Sort assignments
    ShiftAssignment* sorted = malloc(output->num_assignments * sizeof(ShiftAssignment));
    memcpy(sorted, output->assignments, output->num_assignments * sizeof(ShiftAssignment));
    qsort(sorted, output->num_assignments, sizeof(ShiftAssignment), compare_assignments);
    
    // Process assignments
    int current_days[50];  // Temporary arrays for grouping
    int current_shifts[50];
    int count = 0;
    
    const char* current_name = sorted[0].employee_name;
    int current_dept = sorted[0].department_idx;
    
    for (int i = 0; i < output->num_assignments; i++) {
        const ShiftAssignment* curr = &sorted[i];
        
        // If still on the same employee and department
        if (strcmp(current_name, curr->employee_name) == 0 && 
            current_dept == curr->department_idx) {
            if (curr->sprint_day_idx >= 0) {  // Validate indices
                current_days[count] = curr->sprint_day_idx + 1;  // Make 1-indexed
                current_shifts[count] = curr->shift_idx;
                count++;
            } else {
                fprintf(stderr, "Error: Negative sprint_day_idx detected for employee %s\n", curr->employee_name);
                free(sorted);
                fclose(file);
                return false;
            }
        } else {
            // Write the previous group
            write_csv_line(file, 
                          current_name,
                          data->metadata.employee_type_names[sorted[i-1].employee_type_idx],
                          data->metadata.department_names[current_dept],
                          current_days,
                          current_shifts,
                          count);
            
            // Start new group
            current_name = curr->employee_name;
            current_dept = curr->department_idx;
            current_days[0] = curr->sprint_day_idx + 1;
            current_shifts[0] = curr->shift_idx;
            count = 1;
        }
    }
    
    // Write the last group
    write_csv_line(file, 
                   current_name,
                   data->metadata.employee_type_names[sorted[output->num_assignments-1].employee_type_idx],
                   data->metadata.department_names[current_dept],
                   current_days,
                   current_shifts,
                   count);
    
    free(sorted);
    fclose(file);
    return true;
}

bool export_csv(const char *output_dir, const OutputData output,
                const InputData *data) {
    if (!output.ran_successfully || !output.assignments ||
        output.num_assignments == 0) {
        return false;
    }
    
    // Write both formats
    bool machine_success = write_schedule_csv(output_dir, &output, data, false);
    if (!machine_success) return false;
    
    bool human_success = write_schedule_csv(output_dir, &output, data, true);
    if (!human_success) return false;
    
    return true;
}
