#include "../../include/io/csv_importer.h"
#include <complex.h>
#include <dirent.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>

static void free_metadata_strings(MetadataStrings *strings,
                                  FiveDimensionConfig *config) {
  if (!strings)
    return;

  for (size_t i = 0; i < config->num_sprint_days; i++) {
    free(strings->sprint_day_names[i]);
  }
  for (size_t i = 0; i < config->num_shifts; i++) {
    free(strings->shift_names[i]);
  }
  for (size_t i = 0; i < config->num_departments; i++) {
    free(strings->department_names[i]);
  }
  for (size_t i = 0; i < config->num_employee_types; i++) {
    free(strings->employee_type_names[i]);
  }

  free(strings->sprint_day_names);
  free(strings->shift_names);
  free(strings->department_names);
  free(strings->employee_type_names);
  free(strings);
}

static MetadataStrings *parse_metadata_csv(const char *input_dir,
                                           FiveDimensionConfig *config) {
  char filepath[256];
  char line[1024];
  FILE *file;
  MetadataStrings *strings = malloc(sizeof(MetadataStrings));

  strings->sprint_day_names = malloc(1024 * sizeof(char *));
  strings->shift_names = malloc(1024 * sizeof(char *));
  strings->employee_type_names = malloc(1024 * sizeof(char *));
  strings->department_names = malloc(1024 * sizeof(char *));

  config->num_sprint_days = 0;
  config->num_shifts = 0;
  config->num_employee_types = 0;
  config->num_departments = 0;

  snprintf(filepath, sizeof(filepath), "%s/metadata.csv", input_dir);
  file = fopen(filepath, "r");
  if (!file) {
    fprintf(stderr, "Failed to open %s\n", filepath);
    free_metadata_strings(strings, config);
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
      strings->sprint_day_names[config->num_sprint_days] = strdup(name_start);
      config->num_sprint_days++;
    } else if (strcmp(type, "shift") == 0) {
      strings->shift_names[config->num_shifts] = strdup(name_start);
      config->num_shifts++;
    } else if (strcmp(type, "department_type") == 0) {
      strings->department_names[config->num_departments] = strdup(name_start);
      config->num_departments++;
    } else if (strcmp(type, "employee_type") == 0) {
      strings->employee_type_names[config->num_employee_types] =
          strdup(name_start);
      config->num_employee_types++;
    }
  }
  fclose(file);

  return strings;
}

InputData *import_csv(const char *input_dir) {
  InputData *data = malloc(sizeof(InputData));

  char filepath[256];
  char line[1024];
  FILE *file;

  FiveDimensionConfig *config = &data->config;
  MetadataStrings *metadata = parse_metadata_csv(input_dir, config);

  // Print all parsed metadata
  printf("Sprint Days (%zu):\n", config->num_sprint_days);
  for (size_t i = 0; i < config->num_sprint_days; i++) {
    printf("  %zu: %s\n", i, metadata->sprint_day_names[i]);
  }

  printf("\nShifts (%zu):\n", config->num_shifts);
  for (size_t i = 0; i < config->num_shifts; i++) {
    printf("  %zu: %s\n", i, metadata->shift_names[i]);
  }

  printf("\nDepartments (%zu):\n", config->num_departments);
  for (size_t i = 0; i < config->num_departments; i++) {
    printf("  %zu: %s\n", i, metadata->department_names[i]);
  }

  printf("\nEmployee Types (%zu):\n", config->num_employee_types);
  for (size_t i = 0; i < config->num_employee_types; i++) {
    printf("  %zu: %s\n", i, metadata->employee_type_names[i]);
  }

  // Copy metadata into data structure
  data->metadata = *metadata;

  // Free the original metadata struct (but not its contents since they were
  // copied)
  free(metadata);

  return NULL;
}
