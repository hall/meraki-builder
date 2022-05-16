#include "libpostmerkos.h"

#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


// hasPoe returns true if the device is PoE-capable, false otherwise
bool hasPoe() {
  // there's probably a better way to do this than guessing based on the device
  // name
  bool has = false;

  FILE *file = fopen(DEVICE_FILE, "r");
  char line[100];
  if (fgets(line, sizeof(line), file)) {
    if (startsWith(line, "MODEL=") && endsWith(line, "P\n")) {
      has = true;
    }
  }

  fclose(file);
  return has;
}

// getTime returns the current time in ISO 8601 format
char *getTime() {
  time_t now = time(NULL);
  struct tm *time_info = localtime(&now);
  static char timeString[256];
  strftime(timeString, sizeof(timeString), "%FT%TZ", time_info);
  return timeString;
}


// startsWith returns true if STR starts with PREFIX
bool startsWith(const char *str, const char *prefix) {
  if (!str || !prefix)
    return 0;
  size_t lenstr = strlen(str);
  size_t lenprefix = strlen(prefix);
  if (lenprefix > lenstr)
    return 0;
  return strncmp(prefix, str, lenprefix) == 0;
}

// endsWith returns true-ish if STR ends with SUFFIX
bool endsWith(const char *str, const char *suffix) {
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

// getfield gets field NUM from space-delimited LINE
const char *getfield(char *line, int num) {
  const char *tok;
  char linecopy[256];
  strcpy(linecopy, line);
  for (tok = strtok(linecopy, " "); tok && *tok; tok = strtok(NULL, " \n")) {
    if (!--num)
      return tok;
  }
  return NULL;
}
