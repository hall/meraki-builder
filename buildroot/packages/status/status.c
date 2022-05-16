#include "libpostmerkos.h"
#include "pd690xx.h"

#include <dirent.h>
#include <json-c/json.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

int main(int argc, char **argv) {
  bool poeCapable = hasPoe();

  struct json_object *jobj = json_object_new_object();
  json_object_object_add(jobj, "date", json_object_new_string(getTime()));

  struct json_object *jtemp = json_object_new_object();
  json_object_object_add(jobj, "temperature", jtemp);

  struct json_object *jtempsys = json_object_new_array();
  json_object_object_add(jtemp, "cpu", jtempsys);

  struct dirent *dp;
  DIR *dfd;
  char *dir = "/sys/class/thermal";
  if ((dfd = opendir(dir)) == NULL) {
    fprintf(stderr, "Can't open %s\n", dir);
    return 0;
  }

  char filename[100];
  while ((dp = readdir(dfd)) != NULL) {
    struct stat stbuf;
    sprintf(filename, "%s/%s", dir, dp->d_name);
    if (stat(filename, &stbuf) == -1) {
      printf("Unable to stat file: %s\n", filename);
      continue;
    }

    if (!startsWith(filename + strlen(dir) + 1, "thermal_")) {
      continue;
    }
    strcat(filename, "/temp");
    FILE *file = fopen(filename, "r");

    static char line[100];
    fgets(line, sizeof(line), file);
    // remove trailing newline
    line[strcspn(line, "\n")] = 0;
    json_object_array_add(jtempsys, json_object_new_int(atoi(line) / 1000));
    fclose(file);
  }

  if (poeCapable) {
    struct json_object *jtemppoe = json_object_new_array();
    json_object_object_add(jtemp, "poe", jtemppoe);
    for (int i = 0; i < 4; i++) {
      json_object_array_add(jtemppoe, json_object_new_int(get_temp(i)));
    }
  }

  struct json_object *jports = json_object_new_object();
  json_object_object_add(jobj, "ports", jports);

  FILE *file = fopen(PORTS_FILE, "r");
  char line[256];

  int p = -1;
  char buffer[256];
  while (fgets(line, sizeof(line), file)) {
    p++;
    if (p == 0) {
      // skip file header
      continue;
    }

    struct json_object *jport = json_object_new_object();
    json_object_object_add(jports, itoa(p, buffer, 10), jport);

    struct json_object *jportlink = json_object_new_object();
    json_object_object_add(jport, "link", jportlink);

    json_object_object_add(jportlink, "established",
                           json_object_new_boolean(atoi(getfield(line, 2))));
    json_object_object_add(jportlink, "speed",
                           json_object_new_int(atoi(getfield(line, 3))));

    if (poeCapable) {
      struct json_object *jportpoe = json_object_new_object();
      json_object_object_add(jport, "poe", jportpoe);
      json_object_object_add(jportpoe, "power",
                             json_object_new_int(port_power(p)));
    }
  }

  fclose(file);
  printf("%s", json_object_to_json_string_ext(
                   jobj, JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY));
  json_object_put(jobj); // Delete the json object
}
