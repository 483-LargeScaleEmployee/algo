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
    return aa->department_idx - bb->department_idx;
}

static GroupedAssignment create_grouped_assignment(const ShiftAssignment* first) {
    GroupedAssignment group = {
        .employee_name = strdup(first->employee_name),
        .employee_type_idx = first->employee_type_idx,
        .department_idx = first->department_idx,
        .sprint_days = malloc(20 * sizeof(int)),  // Increased initial capacity
        .shifts = malloc(20 * sizeof(int)),
        .num_entries = 0,
        .capacity = 20
    };
    return group;
}

static void add_to_group(GroupedAssignment* group, int day, int shift) {
    if (group->num_entries >= group->capacity) {
        group->capacity *= 2;
        group->sprint_days = realloc(group->sprint_days, group->capacity * sizeof(int));
        group->shifts = realloc(group->shifts, group->capacity * sizeof(int));
    }
    
    // Sort the days while inserting
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

static void free_grouped_assignment(GroupedAssignment* group) {
    free(group->employee_name);
    free(group->sprint_days);
    free(group->shifts);
}

static void write_csv(FILE* file, const GroupedAssignment* group, const InputData* data, bool is_human_readable) {
    // Write employee name, type, and department
    fprintf(file, "%s,%s,%s,\"(",
            group->employee_name,
            data->metadata.employee_type_names[group->employee_type_idx],
            data->metadata.department_names[group->department_idx]);
    
    // Write days tuple
    for (int i = 0; i < group->num_entries; i++) {
        fprintf(file, "%d%s", 
                group->sprint_days[i],
                (i < group->num_entries - 1) ? " " : "");
    }
    fprintf(file, ")\",\"(");
    
    // Write shifts tuple
    for (int i = 0; i < group->num_entries; i++) {
        fprintf(file, "%d%s", 
                group->shifts[i],
                (i < group->num_entries - 1) ? " " : "");
    }
    fprintf(file, ")\"\n");
}

static bool write_machine_readable_csv(const char *output_dir,
                                     const OutputData *output,
                                     const InputData *data) {
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s/schedule_machine.csv", output_dir);
    FILE *file = fopen(filepath, "w");
    if (!file) return false;

    // Sort assignments
    ShiftAssignment* sorted_assignments = malloc(output->num_assignments * sizeof(ShiftAssignment));
    memcpy(sorted_assignments, output->assignments, output->num_assignments * sizeof(ShiftAssignment));
    qsort(sorted_assignments, output->num_assignments, sizeof(ShiftAssignment), compare_assignments);

    // Write header
    fprintf(file, "employee_name,%s,department_name,sprint_days,shifts\n",
            data->metadata.employee_type_names[0]);

    // Group and write assignments
    GroupedAssignment* groups = malloc(output->num_assignments * sizeof(GroupedAssignment));
    int num_groups = 0;

    for (int i = 0; i < output->num_assignments; i++) {
        const ShiftAssignment* curr = &sorted_assignments[i];
        
        if (num_groups > 0 &&
            strcmp(groups[num_groups-1].employee_name, curr->employee_name) == 0 &&
            groups[num_groups-1].department_idx == curr->department_idx) {
            add_to_group(&groups[num_groups-1], curr->sprint_day_idx + 1, curr->shift_idx);
        } else {
            if (num_groups > 0) {
                write_csv(file, &groups[num_groups-1], data, false);
            }
            groups[num_groups] = create_grouped_assignment(curr);
            add_to_group(&groups[num_groups], curr->sprint_day_idx + 1, curr->shift_idx);
            num_groups++;
        }
    }

    // Write last group if exists
    if (num_groups > 0) {
        write_csv(file, &groups[num_groups-1], data, false);
    }

    // Cleanup
    for (int i = 0; i < num_groups; i++) {
        free_grouped_assignment(&groups[i]);
    }
    free(groups);
    free(sorted_assignments);
    fclose(file);

    return true;
}

static bool write_human_readable_csv(const char *output_dir,
                                   const OutputData *output,
                                   const InputData *data) {
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s/schedule_human.csv", output_dir);
    FILE *file = fopen(filepath, "w");
    if (!file) return false;

    // Write header
    fprintf(file, "Employee,%s,Department,Days,Shifts\n",
            data->metadata.employee_type_names[0]);

    // Sort assignments
    ShiftAssignment* sorted_assignments = malloc(output->num_assignments * sizeof(ShiftAssignment));
    memcpy(sorted_assignments, output->assignments, output->num_assignments * sizeof(ShiftAssignment));
    qsort(sorted_assignments, output->num_assignments, sizeof(ShiftAssignment), compare_assignments);

    // Group and write assignments
    GroupedAssignment* groups = malloc(output->num_assignments * sizeof(GroupedAssignment));
    int num_groups = 0;

    for (int i = 0; i < output->num_assignments; i++) {
        const ShiftAssignment* curr = &sorted_assignments[i];
        
        if (num_groups > 0 &&
            strcmp(groups[num_groups-1].employee_name, curr->employee_name) == 0 &&
            groups[num_groups-1].department_idx == curr->department_idx) {
            add_to_group(&groups[num_groups-1], curr->sprint_day_idx + 1, curr->shift_idx);
        } else {
            if (num_groups > 0) {
                write_csv(file, &groups[num_groups-1], data, true);
            }
            groups[num_groups] = create_grouped_assignment(curr);
            add_to_group(&groups[num_groups], curr->sprint_day_idx + 1, curr->shift_idx);
            num_groups++;
        }
    }

    // Write last group if exists
    if (num_groups > 0) {
        write_csv(file, &groups[num_groups-1], data, true);
    }

    // Cleanup
    for (int i = 0; i < num_groups; i++) {
        free_grouped_assignment(&groups[i]);
    }
    free(groups);
    free(sorted_assignments);
    fclose(file);

    return true;
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
