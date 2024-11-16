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
    int capacity;
} GroupedAssignment;

static int compare_assignments(const void *a, const void *b) {
    const ShiftAssignment *aa = (const ShiftAssignment *)a;
    const ShiftAssignment *bb = (const ShiftAssignment *)b;
    int name_cmp = strcmp(aa->employee_name, bb->employee_name);
    if (name_cmp != 0) return name_cmp;
    if (aa->department_idx != bb->department_idx) return aa->department_idx - bb->department_idx;
    return aa->sprint_day_idx - bb->sprint_day_idx;
}

static GroupedAssignment create_grouped_assignment(const ShiftAssignment* first) {
    GroupedAssignment group = {
        .employee_name = strdup(first->employee_name),
        .employee_type_idx = first->employee_type_idx,
        .department_idx = first->department_idx,
        .sprint_days = malloc(20 * sizeof(int)),
        .shifts = malloc(20 * sizeof(int)),
        .num_entries = 0,
        .capacity = 20
    };
    return group;
}


static void free_grouped_assignment(GroupedAssignment* group) {
    free(group->employee_name);
    free(group->sprint_days);
    free(group->shifts);
}

static void write_csv_row(FILE* file, const GroupedAssignment* group, const InputData* data) {
    // Validate inputs
    if (!group || !data || !file) {
        fprintf(stderr, "Error: Invalid input to write_csv_row\n");
        return;
    }

    fprintf(file, "%s,%s,%s,", 
            group->employee_name,
            data->metadata.employee_type_names[group->employee_type_idx],
            data->metadata.department_names[group->department_idx]);
    
    // Write days
    if (group->num_entries == 1) {
        // Handle single entry case
        if (group->sprint_days[0] < 0 || group->shifts[0] < 0) {
            fprintf(stderr, "Warning: Invalid day (%d) or shift (%d)\n", 
                    group->sprint_days[0], group->shifts[0]);
            fprintf(file, "(0),(0)");  // Use default values for invalid data
        } else {
            fprintf(file, "(%d),", group->sprint_days[0]);
            fprintf(file, "(%d)", group->shifts[0]);
        }
    } else {
        // Write days tuple with commas
        fprintf(file, "(");
        for (int i = 0; i < group->num_entries; i++) {
            if (group->sprint_days[i] < 0) {
                fprintf(stderr, "Warning: Invalid day value: %d\n", group->sprint_days[i]);
                fprintf(file, "0");  // Use default value for invalid data
            } else {
                fprintf(file, "%d", group->sprint_days[i]);
            }
            if (i < group->num_entries - 1) {
                fprintf(file, ",");  // Use comma instead of space
            }
        }
        fprintf(file, "),");
        
        // Write shifts tuple with commas
        fprintf(file, "(");
        for (int i = 0; i < group->num_entries; i++) {
            if (group->shifts[i] < 0) {
                fprintf(stderr, "Warning: Invalid shift value: %d\n", group->shifts[i]);
                fprintf(file, "0");  // Use default value for invalid data
            } else {
                fprintf(file, "%d", group->shifts[i]);
            }
            if (i < group->num_entries - 1) {
                fprintf(file, ",");  // Use comma instead of space
            }
        }
        fprintf(file, ")");
    }
    fprintf(file, "\n");
}

static void add_to_group(GroupedAssignment* group, int day, int shift) {
    // Validate inputs
    if (!group) {
        fprintf(stderr, "Error: Invalid group pointer\n");
        return;
    }
    
    if (day < 0 || shift < 0) {
        fprintf(stderr, "Warning: Invalid day (%d) or shift (%d)\n", day, shift);
        return;
    }
    
    if (group->num_entries >= group->capacity) {
        group->capacity *= 2;
        group->sprint_days = realloc(group->sprint_days, group->capacity * sizeof(int));
        group->shifts = realloc(group->shifts, group->capacity * sizeof(int));
        
        if (!group->sprint_days || !group->shifts) {
            fprintf(stderr, "Error: Memory allocation failed\n");
            return;
        }
    }
    
    // Insert in sorted order
    int insert_pos = 0;
    while (insert_pos < group->num_entries && group->sprint_days[insert_pos] < day) {
        insert_pos++;
    }
    
    // Shift existing elements
    for (int i = group->num_entries; i > insert_pos; i--) {
        group->sprint_days[i] = group->sprint_days[i-1];
        group->shifts[i] = group->shifts[i-1];
    }
    
    // Insert new elements
    group->sprint_days[insert_pos] = day;
    group->shifts[insert_pos] = shift;
    group->num_entries++;
}

static bool process_assignments(FILE* file, const OutputData* output, const InputData* data) {
    // Sort assignments
    ShiftAssignment* sorted_assignments = malloc(output->num_assignments * sizeof(ShiftAssignment));
    memcpy(sorted_assignments, output->assignments, output->num_assignments * sizeof(ShiftAssignment));
    qsort(sorted_assignments, output->num_assignments, sizeof(ShiftAssignment), compare_assignments);

    GroupedAssignment* groups = malloc(output->num_assignments * sizeof(GroupedAssignment));
    int num_groups = 0;

    const ShiftAssignment* curr = &sorted_assignments[0];
    groups[0] = create_grouped_assignment(curr);
    add_to_group(&groups[0], curr->sprint_day_idx + 1, curr->shift_idx); // Add 1 to make 1-indexed
    num_groups = 1;

    // Process remaining assignments
    for (int i = 1; i < output->num_assignments; i++) {
        curr = &sorted_assignments[i];
        if (strcmp(groups[num_groups-1].employee_name, curr->employee_name) == 0 &&
            groups[num_groups-1].department_idx == curr->department_idx) {
            // Add to existing group
            add_to_group(&groups[num_groups-1], curr->sprint_day_idx + 1, curr->shift_idx); // Add 1 to make 1-indexed
        } else {
            // Write previous group
            write_csv_row(file, &groups[num_groups-1], data);
            // Start new group
            groups[num_groups] = create_grouped_assignment(curr);
            add_to_group(&groups[num_groups], curr->sprint_day_idx + 1, curr->shift_idx); // Add 1 to make 1-indexed
            num_groups++;
        }
    }

    // Write last group
    if (num_groups > 0) {
        write_csv_row(file, &groups[num_groups-1], data);
    }

    // Cleanup
    for (int i = 0; i < num_groups; i++) {
        free_grouped_assignment(&groups[i]);
    }
    free(groups);
    free(sorted_assignments);
    
    return true;
}

static bool write_machine_readable_csv(const char *output_dir,
                                     const OutputData *output,
                                     const InputData *data) {
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s/schedule_machine.csv", output_dir);
    FILE *file = fopen(filepath, "w");
    if (!file) return false;

    fprintf(file, "employee_name,%s,department_name,sprint_days,shifts\n",
            data->metadata.employee_type_names[0]);

    bool success = process_assignments(file, output, data);
    fclose(file);
    return success;
}

static bool write_human_readable_csv(const char *output_dir,
                                   const OutputData *output,
                                   const InputData *data) {
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s/schedule_human.csv", output_dir);
    FILE *file = fopen(filepath, "w");
    if (!file) return false;

    fprintf(file, "Employee,%s,Department,Days,Shifts\n",
            data->metadata.employee_type_names[0]);

    bool success = process_assignments(file, output, data);
    fclose(file);
    return success;
}

bool export_csv(const char *output_dir, const OutputData output,
                const InputData *data) {
    if (!output.ran_successfully || !output.assignments ||
        output.num_assignments == 0) {
        return false;
    }

    bool machine_success = write_machine_readable_csv(output_dir, &output, data);
    if (!machine_success) return false;

    bool human_success = write_human_readable_csv(output_dir, &output, data);
    if (!human_success) return false;

    return true;
}
