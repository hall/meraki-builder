#include <libpostmerkos.h>
#include <libpd690xx.h>
#include "configd.h"

#include <dirent.h>
#include <json-c/json.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
// #include <sys/inotify.h>


// read config from click filesystem
struct json_object *read_config() {
  bool poeCapable = hasPoe();

  struct json_object *jobj = json_object_new_object();

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

    if (poeCapable) {
      struct json_object *jportpoe = json_object_new_object();
      json_object_object_add(jport, "poe", jportpoe);
      json_object_object_add(jportpoe, "enabled",
                             json_object_new_boolean(port_state(p)));
      json_object_object_add(jportpoe, "mode",
                             json_object_new_string(port_type_str(p)));
    }
  }

  fclose(file);
  return jobj;
}

// write config to click filesystem
int write_config(struct json_object *json) {

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
                printf("poe state is %d for port %s\n", port_state(atoi(port)), port);
                if (poeitemconfig && !port_state(atoi(port))) {
                  printf("poe enabled for port %s\n", port);
                } else if (!poeitemconfig && port_state(atoi(port))) {
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

// poll config file for changes
void poll() {
  time_t mtime;
  time(&mtime);
  struct stat st;
  struct json_object *current = read_config();
  struct json_object *modified;

  int file = open(CONFIG_FILE, O_RDONLY);
  if(file == -1) { 
    printf("unable to open file\n"); 
    return; 
  } 


  while(true) {
    // get file modification time
    if(fstat(file, &st))   { 
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
        write_config(modified);
      }
    }

    sleep(1);
  }

}

int main(int argc, char **argv) {

  // create config file if it doesn't exist
  if(access(CONFIG_FILE, F_OK) != 0) {
    printf("config file (%s) does not exist, creating\n", CONFIG_FILE);
    FILE *file = fopen(CONFIG_FILE, "w");
    const char * json = json_object_to_json_string_ext(
                   read_config(), JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY);
    fprintf(file, json);
    fclose(file);
  }

  // inotify is not currently enabled and returns the following error:
  //   inotify_init: Function not implemented
  // so lets poll for now
  poll();

}
