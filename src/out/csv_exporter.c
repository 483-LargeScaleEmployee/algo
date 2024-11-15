#include "../../include/out/csv_exporter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Structure to hold grouped assignments
typedef struct {
    char* employee_name;
    int employee_type_idx;
    int department_idx;
    int* sprint_days;  // Array to hold days
    int* shifts;       // Array to hold corresponding shifts
    int num_entries;   // Number of days/shifts
    int capacity;      // Current capacity of arrays
} GroupedAssignment;

static int compare_assignments(const void *a, const void *b) {
    const ShiftAssignment *aa = (const ShiftAssignment *)a;
    const ShiftAssignment *bb = (const ShiftAssignment *)b;
    
    // First compare by employee name
    int name_cmp = strcmp(aa->employee_name, bb->employee_name);
    if (name_cmp != 0) {
        return name_cmp;
    }
    // Then compare by department index
    return aa->department_idx - bb->department_idx;
}

// Helper function to initialize a GroupedAssignment
static GroupedAssignment create_grouped_assignment(const ShiftAssignment* first) {
    GroupedAssignment group = {
        .employee_name = strdup(first->employee_name),
        .employee_type_idx = first->employee_type_idx,
        .department_idx = first->department_idx,
        .sprint_days = malloc(10 * sizeof(int)),  // Initial capacity of 10
        .shifts = malloc(10 * sizeof(int)),
        .num_entries = 0,
        .capacity = 10
    };
    return group;
}

// Helper function to add a day/shift to a GroupedAssignment
static void add_to_group(GroupedAssignment* group, int day, int shift) {
    if (group->num_entries >= group->capacity) {
        group->capacity *= 2;
        group->sprint_days = realloc(group->sprint_days, group->capacity * sizeof(int));
        group->shifts = realloc(group->shifts, group->capacity * sizeof(int));
    }
    group->sprint_days[group->num_entries] = day;
    group->shifts[group->num_entries] = shift;
    group->num_entries++;
}

// Helper function to free a GroupedAssignment
static void free_grouped_assignment(GroupedAssignment* group) {
    free(group->employee_name);
    free(group->sprint_days);
    free(group->shifts);
}

// Helper function to write tuple of days or shifts
static void write_tuple(FILE* file, const int* arr, int count) {
    fprintf(file, "(");
    for (int i = 0; i < count; i++) {
        fprintf(file, "%d%s", arr[i], (i < count - 1) ? "," : "");
    }
    fprintf(file, ")");
}

static bool write_machine_readable_csv(const char *output_dir,
                                     const OutputData *output,
                                     const InputData *data) {
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s/schedule_machine.csv", output_dir);
    FILE *file = fopen(filepath, "w");
    if (!file) {
        return false;
    }

    // Sort assignments first
    ShiftAssignment* sorted_assignments = malloc(output->num_assignments * sizeof(ShiftAssignment));
    memcpy(sorted_assignments, output->assignments, output->num_assignments * sizeof(ShiftAssignment));
    qsort(sorted_assignments, output->num_assignments, sizeof(ShiftAssignment), compare_assignments);

    // Create array for grouped assignments
    GroupedAssignment* groups = malloc(output->num_assignments * sizeof(GroupedAssignment));
    int num_groups = 0;

    // Group assignments
    for (int i = 0; i < output->num_assignments; i++) {
        const ShiftAssignment* curr = &sorted_assignments[i];
        
        // Check if we should add to existing group or create new one
        if (num_groups > 0 &&
            strcmp(groups[num_groups-1].employee_name, curr->employee_name) == 0 &&
            groups[num_groups-1].department_idx == curr->department_idx) {
            // Add to existing group
            add_to_group(&groups[num_groups-1], curr->sprint_day_idx + 1, curr->shift_idx);
        } else {
            // Create new group
            groups[num_groups] = create_grouped_assignment(curr);
            add_to_group(&groups[num_groups], curr->sprint_day_idx + 1, curr->shift_idx);
            num_groups++;
        }
    }

    // Write header
    fprintf(file, "employee_name,%s,department_name,sprint_days,shifts\n",
            data->metadata.employee_type_names[0]);

    // Write grouped data
    for (int i = 0; i < num_groups; i++) {
        const GroupedAssignment* group = &groups[i];
        
        fprintf(file, "%s,%s,%s,", 
                group->employee_name,
                data->metadata.employee_type_names[group->employee_type_idx],
                data->metadata.department_names[group->department_idx]);
        
        // Write days tuple
        write_tuple(file, group->sprint_days, group->num_entries);
        fprintf(file, ",");
        
        // Write shifts tuple
        write_tuple(file, group->shifts, group->num_entries);
        fprintf(file, "\n");
    }

    // Cleanup
    for (int i = 0; i < num_groups; i++) {
        free_grouped_assignment(&groups[i]);
    }
    free(groups);
    free(sorted_assignments);
    fclose(file);

    printf("Machine-readable schedule written to %s\n", filepath);
    return true;
}

static bool write_human_readable_csv(const char *output_dir,
                                   const OutputData *output,
                                   const InputData *data) {
    // Use the same logic as machine readable but with different formatting
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s/schedule_human.csv", output_dir);
    FILE *file = fopen(filepath, "w");
    if (!file) {
        return false;
    }

    // Sort assignments first
    ShiftAssignment* sorted_assignments = malloc(output->num_assignments * sizeof(ShiftAssignment));
    memcpy(sorted_assignments, output->assignments, output->num_assignments * sizeof(ShiftAssignment));
    qsort(sorted_assignments, output->num_assignments, sizeof(ShiftAssignment), compare_assignments);

    // Create array for grouped assignments
    GroupedAssignment* groups = malloc(output->num_assignments * sizeof(GroupedAssignment));
    int num_groups = 0;

    // Group assignments
    for (int i = 0; i < output->num_assignments; i++) {
        const ShiftAssignment* curr = &sorted_assignments[i];
        
        if (num_groups > 0 &&
            strcmp(groups[num_groups-1].employee_name, curr->employee_name) == 0 &&
            groups[num_groups-1].department_idx == curr->department_idx) {
            add_to_group(&groups[num_groups-1], curr->sprint_day_idx + 1, curr->shift_idx);
        } else {
            groups[num_groups] = create_grouped_assignment(curr);
            add_to_group(&groups[num_groups], curr->sprint_day_idx + 1, curr->shift_idx);
            num_groups++;
        }
    }

    // Write header
    fprintf(file, "Employee,%s,Department,Days,Shifts\n",
            data->metadata.employee_type_names[0]);

    // Write grouped data
    for (int i = 0; i < num_groups; i++) {
        const GroupedAssignment* group = &groups[i];
        
        fprintf(file, "%s,%s,%s,", 
                group->employee_name,
                data->metadata.employee_type_names[group->employee_type_idx],
                data->metadata.department_names[group->department_idx]);
        
        write_tuple(file, group->sprint_days, group->num_entries);
        fprintf(file, ",");
        
        write_tuple(file, group->shifts, group->num_entries);
        fprintf(file, "\n");
    }

    // Cleanup
    for (int i = 0; i < num_groups; i++) {
        free_grouped_assignment(&groups[i]);
    }
    free(groups);
    free(sorted_assignments);
    fclose(file);

    printf("Human-readable schedule written to %s\n", filepath);
    return true;
}

bool export_csv(const char *output_dir, const OutputData output,
                const InputData *data) {
    if (!output.ran_successfully || !output.assignments ||
        output.num_assignments == 0) {
        return false;
    }

    bool machine_success = write_machine_readable_csv(output_dir, &output, data);
    if (!machine_success) {
        return false;
    }

    bool human_success = write_human_readable_csv(output_dir, &output, data);
    if (!human_success) {
        return false;
    }

    return true;
}
