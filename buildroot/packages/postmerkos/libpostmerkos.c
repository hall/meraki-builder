#include "libpostmerkos.h"

#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


// has_poe returns true if the device is PoE-capable, false otherwise
bool has_poe() {
  // there's probably a better way to do this than guessing based on the device
  // name
  if (ends_with(get_name(), "P")) {
    return true;
  }

  return false;
}

// get_name returns the device model name
char *get_name() {
  char *name = malloc(sizeof(50));
  int found = 0;

  FILE *file = fopen(DEVICE_FILE, "r");
  char line[50];
  if (fgets(line, sizeof(line), file)) {
    if (starts_with(line, "MODEL=")) {
      // remove "MODEL=" prefix and "\n" suffix
       strncpy(name, (char *) line + 6, strlen(line)-7);
      found = 1;
    }
  }

  fclose(file);
  if (found) {
    return name;
  }
}

// get_time returns the current time in ISO 8601 format
char *get_time() {
  time_t now = time(NULL);
  struct tm *time_info = localtime(&now);
  static char timeString[256];
  strftime(timeString, sizeof(timeString), "%FT%TZ", time_info);
  return timeString;
}


// starts_with returns true if STR starts with PREFIX
bool starts_with(const char *str, const char *prefix) {
  if (!str || !prefix)
    return 0;
  size_t lenstr = strlen(str);
  size_t lenprefix = strlen(prefix);
  if (lenprefix > lenstr)
    return 0;
  return strncmp(prefix, str, lenprefix) == 0;
}

// ends_with returns true-ish if STR ends with SUFFIX
bool ends_with(const char *str, const char *suffix) {
  if (!str || !suffix)
    return 0;
  size_t lenstr = strlen(str);
  size_t lensuffix = strlen(suffix);
  if (lensuffix > lenstr)
    return 0;
  return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

// itoa converts NUM, in BASE, of max size BUFFER to string
char *itoa(int num, char *buffer, int base) {
  int current = 0;
  if (num == 0) {
    buffer[current++] = '0';
    buffer[current] = '\0';
    return buffer;
  }
  int num_digits = 0;
  if (num < 0) {
    if (base == 10) {
      num_digits++;
      buffer[current] = '-';
      current++;
      num *= -1;
    } else
      return NULL;
  }
  num_digits += (int)floor(log(num) / log(base)) + 1;
  while (current < num_digits) {
    int base_val = (int)pow(base, num_digits - 1 - current);
    int num_val = num / base_val;
    char value = num_val + '0';
    buffer[current] = value;
    current++;
    num -= base_val * num_val;
  }
  buffer[current] = '\0';
  return buffer;
}

// get_field gets field NUM from space-delimited LINE
const char *get_field(char *line, int num) {
  const char *tok;
  char linecopy[256];
  strcpy(linecopy, line);
  for (tok = strtok(linecopy, " "); tok && *tok; tok = strtok(NULL, " \n")) {
    if (!--num)
      return tok;
  }
  return NULL;
}
