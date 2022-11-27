#include "configd.h"
#include <libpd690xx.h>
#include <pd690xx_meraki.h>
#include <libpostmerkos.h>

#include <dirent.h>
#include <fcntl.h>
#include <json-c/json.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <libgen.h>
// #include <sys/inotify.h>

struct pd690xx_cfg pd690xx = {
    // i2c_fds
    {-1, -1},
    // pd690xx_addrs
    {PD690XX0_I2C_ADDR, PD690XX1_I2C_ADDR, PD690XX2_I2C_ADDR, PD690XX3_I2C_ADDR},
    // pd690xx_pres
    {0, 0, 0 ,0}
};

// read config from click filesystem
struct json_object* read_config(struct pd690xx_cfg *pd690xx) {
  bool poeCapable = has_poe();

  struct json_object *jobj = json_object_new_object();

  struct json_object *jports = json_object_new_object();
  json_object_object_add(jobj, "ports", jports);

  FILE *pfile = fopen(PORTS_FILE, "r");
  FILE *vfile = fopen(VLANS_FILE, "r");
  char pline[256];
  char vline[256];
  char port[100][100];
  char vlan[100][100];

  char *context = NULL;

  int p = -1;
  char buffer[256];
  while (fgets(pline, sizeof(pline), pfile)) {
    fgets(vline, sizeof(vline), vfile);

    p++;
    if (p == 0) {
      // skip file header
      continue;
    }

    // remove newline at end of line
    int len = strlen(vline);
    if (len > 0 && vline[len - 1] == '\n')
      vline[len - 1] = '\0';
    len = strlen(pline);
    if (len > 0 && pline[len - 1] == '\n')
      pline[len - 1] = '\0';

    struct json_object *jport = json_object_new_object();
    json_object_object_add(jports, itoa(p, buffer, 10), jport);

    if (poeCapable) {
      struct json_object *jportpoe = json_object_new_object();
      json_object_object_add(jport, "poe", jportpoe);
      json_object_object_add(jportpoe, "enabled",
                             json_object_new_boolean(port_state(pd690xx, p)));
      // json_object_object_add(jportpoe, "mode",
                            //  json_object_new_string(port_type_str(*pd690xx, p)));
    }

    // populate array of values from the current line
    int num = 0;
    char *token = strtok_r(vline, " ", &context);
    while (token != NULL) {
      strcpy(vlan[num], token);
      num++;
      token = strtok_r(NULL, " ", &context);
    }

    struct json_object *jportvlan = json_object_new_object();
    json_object_object_add(jport, "vlan", jportvlan);
    json_object_object_add(jportvlan, "pvid",
                           json_object_new_int(atoi(vlan[5])));
    json_object_object_add(jportvlan, "allowed",
                           json_object_new_string(vlan[10]));
  }

  fclose(pfile);
  fclose(vfile);
  return jobj;
}

// write config to click filesystem
int write_config(struct pd690xx_cfg *pd690xx, struct json_object *json) {

  // get all top-level keys
  json_object_object_foreach(json, key, value) {

    if (strcmp(key, "ports") == 0) {
      // get all ports
      json_object_object_foreach(value, port, portconfig) {
        // printf("%s  -> %s\n", port, json_object_get_string(config));

        // get config for ports
        json_object_object_foreach(portconfig, item, itemconfig) {
          // printf("%s  -> %s\n", key, json_object_get_string(value));

          if (strcmp(item, "poe") == 0) {
            // get poe config
            json_object_object_foreach(itemconfig, poeitem, poeitemconfig) {

              if (strcmp(poeitem, "enabled") == 0) {
                printf("poe state is %d for port %s\n", port_state(pd690xx, atoi(port)),
                       port);
                if (poeitemconfig && !port_state(pd690xx, atoi(port))) {
                  printf("poe enabled for port %s\n", port);
                } else if (!poeitemconfig && port_state(pd690xx, atoi(port))) {
                  printf("poe disabled for port %s\n", port);
                }
              }
            }
          }
        }
      }
    }
  }
}

void read_status(struct pd690xx_cfg *pd690xx) {
  bool poeCapable = has_poe();

  struct json_object *jobj = json_object_new_object();
  json_object_object_add(jobj, "date", json_object_new_string(get_time()));
  json_object_object_add(jobj, "device", json_object_new_string(get_name()));

  struct json_object *jtemp = json_object_new_object();
  json_object_object_add(jobj, "temperature", jtemp);

  struct json_object *jtempsys = json_object_new_array();
  json_object_object_add(jtemp, "cpu", jtempsys);

  struct dirent *dp;
  DIR *dfd;
  char *dir = "/sys/class/thermal";
  if ((dfd = opendir(dir)) == NULL) {
    fprintf(stderr, "Can't open %s\n", dir);
    exit(1);
  }

  char filename[100];
  while ((dp = readdir(dfd)) != NULL) {
    struct stat stbuf;
    sprintf(filename, "%s/%s", dir, dp->d_name);
    if (stat(filename, &stbuf) == -1) {
      printf("Unable to stat file: %s\n", filename);
      continue;
    }

    if (!starts_with(filename + strlen(dir) + 1, "thermal_")) {
      continue;
    }
    strcat(filename, "/temp");
    FILE *file = fopen(filename, "r");

    static char line[100];
    fgets(line, sizeof(line), file);
    // remove trailing newline
    line[strcspn(line, "\n")] = 0;
    json_object_array_add(jtempsys, json_object_new_double(atoi(line) / 1000.0));
    fclose(file);
  }

  if (poeCapable) {
    struct json_object *jtemppoe = json_object_new_array();
    json_object_object_add(jtemp, "poe", jtemppoe);
    for (int i = 0; i < 4; i++) {
      json_object_array_add(jtemppoe, json_object_new_double(get_temp(pd690xx, i)));
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
                           json_object_new_boolean(atoi(get_field(line, 2))));
    json_object_object_add(jportlink, "speed",
                           json_object_new_int(atoi(get_field(line, 3))));

    if (poeCapable) {
      struct json_object *jportpoe = json_object_new_object();
      json_object_object_add(jport, "poe", jportpoe);
      json_object_object_add(jportpoe, "power",
                             json_object_new_int(port_power(pd690xx, p)));
    }
  }

  fclose(file);
  printf("%s", json_object_to_json_string_ext(
                   jobj, JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY));
  json_object_put(jobj); // Delete the json object
}

// poll config file for changes
void poll(struct pd690xx_cfg *pd690xx) {
  time_t mtime;
  time(&mtime);
  struct stat st;
  struct json_object *current = read_config(pd690xx);
  struct json_object *modified;

  int file = open(CONFIG_FILE, O_RDONLY);
  if (file == -1) {
    printf("unable to open file\n");
    return;
  }

  while (true) {
    // get file modification time
    if (fstat(file, &st)) {
      printf("fstat error: [%s]\n", strerror(0));
      close(file);
      return;
    }

    if (difftime(st.st_mtime, mtime) > 0) {
      // printf("file modified\n");
      mtime = st.st_mtime;
      modified = json_object_from_file(CONFIG_FILE);
      if (!json_object_equal(current, modified)) {
        current = modified;
        // printf("object changed\n");
        write_config(pd690xx, modified);
      }
    }

    sleep(1);
  }
}

void run_daemon(struct pd690xx_cfg *pd690xx) {

  // create config file if it doesn't exist
  if (access(CONFIG_FILE, F_OK) != 0) {
    printf("config file (%s) does not exist, creating\n", CONFIG_FILE);
    FILE *file = fopen(CONFIG_FILE, "w");
    const char *json = json_object_to_json_string_ext(
        read_config(pd690xx), JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY);
    fprintf(file, json);
    fclose(file);
    printf("config file created\n");
  }

  // inotify is not currently enabled and returns the following error:
  //   inotify_init: Function not implemented
  // so lets poll for now
  poll(pd690xx);

}

void usage(char **argv) {
    printf("Usage: %s [OPTIONS]\n", basename(argv[0]));
    printf("\tRead switch status or get/set config to/from `/etc/switch.json`\n");
    printf("Options:\n");
    printf("\t-d\tWatch for config changes in daemon mode\n");
    printf("\t-g\tGet switch status (default)\n");
    printf("\t-h\tProgram usage\n");
}

int main(int argc, char **argv) {
  i2c_init(&pd690xx);
    int c;

    while ((c = getopt (argc, argv, "hdg::")) != -1) {
      switch(c) {
        case 'd':
          run_daemon(&pd690xx);
          break;
        case 'h':
        case '?':
          usage(argv);
          break;
        default:
        case 'g':
          read_status(&pd690xx);
          break;
      }
    }
}
