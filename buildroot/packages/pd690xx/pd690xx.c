#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>

// getopt
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>

// basename
#include <libgen.h>

// register map
#include "libpd690xx.h"
#include "pd690xx.h"
#include "pd690xx_meraki.h"
#include "libpd690xx.h"

// Copyright(C) 2020-2022 - Hal Martin <hal.martin@gmail.com>

// portions adapted from
// https://gist.github.com/JamesDunne/9b7fbedb74c22ccc833059623f47beb7

int port_able(int op, int port) {
  const char *operation[4] = {"disabl", "enabl", "forc", "reserv"};
  u16 res;
  u16 reg;
  u8 pd_addr = get_pd690xx_addr(port);
  if (pd_addr == 0) {
    return -1;
  }
  u16 port_addr = port_base_addr(PORT_CONFIG, port);
#ifdef DEBUG
  printf("Port addr: %02X\n", port_addr);
#endif
  int i2c_fd = pd690xx_fd(port);
  i2c_read(i2c_fd, pd_addr, port_addr, &reg);
  switch (op) {
  case PORT_DISABLED:
    res = i2c_write(i2c_fd, pd_addr, port_addr, reg & 0xFC);
    break;
  case PORT_ENABLED:
    res = i2c_write(i2c_fd, pd_addr, port_addr, (reg & 0xFD) | 0x01);
    break;
  case PORT_FORCED:
    res = i2c_write(i2c_fd, pd_addr, port_addr, (reg & 0xFC) | 0x02);
    break;
  }
  if (res != 0) {
    printf("Error %sing port %d\n", operation[op], port);
    return -1;
  }
  // sleep before we poll the port register
  usleep(100000);
  i2c_read(i2c_fd, pd_addr, port_addr, &reg);
  printf("Port %d: %sed\n", port, operation[(reg & 0x03)]);
  return 0;
}

int port_disable(int port) { return port_able(PORT_DISABLED, port); }

int port_enable(int port) { return port_able(PORT_ENABLED, port); }

int port_force(int port) { return port_able(PORT_FORCED, port); }

int port_reset(int port) {
  port_disable(port);
  usleep(2000000);
  port_enable(port);
  return 0;
}

int port_priority(int port) {
  u16 res;
  int port_prio = -1;
  u8 pd_addr = get_pd690xx_addr(port);
  if (pd_addr == 0) {
    return -1;
  }
  int i2c_fd = pd690xx_fd(port);
  i2c_read(i2c_fd, pd_addr, port_base_addr(PORT_CONFIG, port), &res);
  port_prio = res & 0xC0;
#ifdef DEBUG
  switch (port_prio) {
  case PORT_PRIO_CRIT:
    printf("Port %d: Critical\n", port);
    break;
  case PORT_PRIO_HIGH:
    printf("Port %d: High\n", port);
    break;
  case PORT_PRIO_LOW:
    printf("Port %d: Low\n", port);
    break;
  default:
    printf("Port %d: unknown (%d)\n", port, port_prio);
  }
#endif
  return port_prio;
}

int print_temp(void) {
  int pd690xx_count = pd690xx_pres_count();
  // this seems _kind of_ sane, but I'm not sure
  // the datasheet has two different versions of the formula
  // and nothing about a sign bit, since it's -200 to 400 C
  for (int i = 0; i < pd690xx_count; i++) {
    printf("%.1f C\n", get_temp(i));
  }
  return 0;
}

void list_all() {
  u16 res;
  printf("Port\tStatus\t\tType\tPriority\tPower\n");
  int total_ports = 12 * pd690xx_pres_count();
  for (int i = 1; i <= total_ports; i++) {
    // print port number
    printf("%d\t", i);
    // print port status
    switch (port_state(i)) {
    case PORT_DISABLED:
      printf("Disabled\t");
      break;
    case PORT_ENABLED:
      printf("Enabled\t\t");
      break;
    case PORT_FORCED:
      printf("Forced\t\t");
      break;
    default:
      printf("Unknown (%d)\t", port_state(i));
void list_all(struct pd690xx_cfg *pd690xx) {
    printf("Port\tStatus\t\tType\tPriority\tPower\n");
    int total_ports = 12*pd690xx_pres_count(pd690xx);
    for (int i=1; i<=total_ports; i++) {
        // print port number
        printf("%d\t", i);
        // print port status
        switch(port_state(pd690xx, i)) {
            case PORT_DISABLED:
                printf("Disabled\t");
                break;
            case PORT_ENABLED:
                printf("Enabled\t\t");
                break;
            case PORT_FORCED:
                printf("Forced\t\t");
                break;
            default:
                printf("Unknown\t");
        }
        // print port type
        switch(port_type(pd690xx, i)) {
            case PORT_MODE_AF:
                printf("af\t");
                break;
            case PORT_MODE_AT:
                printf("at\t");
                break;
            default:
                printf("Unknown\t");
        }
        // print port priority
        switch(port_priority(pd690xx, i)) {
            case PORT_PRIO_CRIT:
                printf("Critical\t");
                break;
            case PORT_PRIO_HIGH:
                printf("High\t\t");
                break;
            case PORT_PRIO_LOW:
                printf("Low\t\t");
                break;
            default:
                printf("Unknown\t");
        }
        // print port power
        printf("%.1f\n", port_power(pd690xx, i));
    }
    // print port type
    printf("%s\t", port_type_str(i));
    // print port priority
    switch (port_priority(i)) {
    case PORT_PRIO_CRIT:
      printf("Critical\t");
      break;
    case PORT_PRIO_HIGH:
      printf("High\t\t");
      break;
    case PORT_PRIO_LOW:
      printf("Low\t\t");
      break;
    default:
      printf("Unknown (%d)\t", port_priority(i));
    }
    // print port power
    printf("%.1f\n", port_power(i));
  }
}

void usage(char **argv) {
  // there are SO discussions about whether to bake in the name
  // or get it from the executable path, for now, use the basename
  char *binary = basename(argv[0]);
  printf("Usage: %s [OPTIONS]\n", binary);
  printf("Options:\n");
  printf("\t-d <PORT>\tDisable PoE on port PORT\n");
  printf("\t-e <PORT>\tEnable PoE on port PORT\n");
  printf("\t-f <PORT>\tForce PoE on port PORT\n");
  printf("\t-h\t\tProgram usage\n");
  printf("\t-l\t\tList all port statuses\n");
  printf("\t-p [PORT]\tPrint PoE power consumption (system total, or on port "
         "PORT)\n");
  printf("\t-r <PORT>\tReset PoE on port PORT\n");
  printf("\t-s\t\tDisplay PoE voltage\n");
  printf("\t-t\t\tDisplay average junction temperature (deg C) of pd690xx\n");
}

int main(int argc, char **argv) {
  int port = -1;
  int ret_val = 0;
  int c;
  // https://www.gnu.org/savannah-checkouts/gnu/libc/manual/html_node/Example-of-Getopt.html
  if (argc == 1) {
    usage(argv);
    return 0;
  }
  while ((c = getopt(argc, argv, "htsld:e:f:r:p::")) != -1) {
    switch (c) {
    case 'd':
    case 'e':
    case 'f':
    case 'l':
    case 'r':
    case 's':
    case 't':
    case 'p':
    case 'P':
      i2c_init();
      if (pd690xx_pres_count() > 0) {
        switch (c) {
        case 'd':
          port_disable(atoi(optarg));
          break;
        case 'e':
          port_enable(atoi(optarg));
          break;
        case 'f':
          port_force(atoi(optarg));
          break;
        case 'l':
          list_all();
          break;
        case 'r':
          port_reset(atoi(optarg));
          break;
        case 's':
          get_voltage();
          break;
        case 't':
          print_temp();
int main (int argc, char **argv) {
    int port = -1;
    int c;
    char disable = 0, enable = 0, force= 0, list = 0, power = 0, reset = 0, voltage = 0, temp = 0, debug = 0;

    // https://www.gnu.org/savannah-checkouts/gnu/libc/manual/html_node/Example-of-Getopt.html
    if (argc == 1) {
        usage(argv);
        return 0;
    }
    while (( c = getopt (argc, argv, "htsld:e:f:r:p:v::")) != -1) {
      switch(c) {
        case 'd':
          disable = 1;
          port = atoi(optarg);
          break;
        case 'e':
          enable = 1;
          port = atoi(optarg);
          break;
        case 'f':
          force = 1;
          port = atoi(optarg);
          break;
        case 'l':
          list = 1;
          break;
        case 'p':
          power = 1;
          if (argv[optind] == NULL) {
            port = 0;
          } else {
            // optarg doesn't seem to work if the
            // option has an optional value
            // dirty workaround
            port = atoi(argv[optind]);
          }
          get_power(port);
          break;
        }
      } else {
        ret_val = 1;
              // optarg doesn't seem to work if the
              // option has an optional value
              // dirty workaround
              port = atoi(argv[optind]);
          }
          break;
        case 'r':
          reset = 1;
          port = atoi(optarg);
          break;
        case 's':
          voltage = 1;
          break;
        case 't':
          temp = 1;
          break;
        case 'v':
          debug = 1;
          break;
        case 'h':
        case '?':
          usage(argv);
          break;
        default:
          usage(argv);
          break;
      }
      break;
    case 'h':
    case '?':
      usage(argv);
      break;
    default:
      usage(argv);
      ret_val = 1;
      break;
    }
  }

  // close the file descriptor when exiting
  i2c_close();
  return ret_val;
    struct pd690xx_cfg pd690xx = {
        // i2c_fds
        {-1, -1},
        // pd690xx_addrs
        {PD690XX0_I2C_ADDR, PD690XX1_I2C_ADDR, PD690XX2_I2C_ADDR, PD690XX3_I2C_ADDR},
        // pd690xx_pres
        {0, 0, 0 ,0}
    };

    // first, enable debugging if requested
    if (debug) {
        enable_debug();
    }

    i2c_init(&pd690xx);

    // check if we detected any pd690xx present
    if (pd690xx_pres_count(&pd690xx) == 0) {
        // we didn't, so exit with an error
        return 1;
    }

    if (disable) {
        if (port < 1 || port > 48) { return 1; };
        port_disable(&pd690xx, port);
    }
    if (enable) {
        if (port < 1 || port > 48) { return 1; };
        port_enable(&pd690xx, port);
    }
    if (force) {
        if (port < 1 || port > 48) { return 1; };
        port_force(&pd690xx, port);
    }
    if (reset) {
        if (port < 1 || port > 48) { return 1; };
        port_reset(&pd690xx, port);
    }
    if (list) {
        list_all(&pd690xx);
    }
    if (power) {
        if (port < 1 || port > 48) { return 1; };
        get_power(&pd690xx, port);
    }
    if (voltage) {
        get_voltage(&pd690xx);
    }
    if (temp) {
        get_temp(&pd690xx);
    }

    i2c_close(&pd690xx);
}
