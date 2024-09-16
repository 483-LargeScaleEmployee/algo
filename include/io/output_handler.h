#pragma once

typedef struct StandardizedOutput {
  // Sample fields for now just to get to compile
  int some_data;
  char *some_string;
} StandardizedOutput;

void send_to_frontend(StandardizedOutput *output);
void destroy_standardized_output(StandardizedOutput *output);
