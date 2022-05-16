#include "libpd690xx.h"
#include "libpostmerkos.h"
#include <math.h>

#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>

#include <linux/i2c-dev.h>
#include <linux/i2c.h>

// getopt
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>

// basename
#include <libgen.h>

// Global array of file descriptors used to talk to the I2C bus 1 and 2
int i2c_fds[2] = {-1, -1};
// detect if the pd690xx devices are present
int pd690xx_pres[4] = {0, 0, 0, 0};
// hold pd690xx addresses
u8 pd690xx_addrs[4] = {PD690XX0_I2C_ADDR, PD690XX1_I2C_ADDR, PD690XX2_I2C_ADDR,
                       PD690XX3_I2C_ADDR};
// define the static part of the file path
const char *i2c_fname_base = "/dev/i2c-";
char i2c_fname[11];


int pd690xx_fd(int port) {
  switch (port / 12) {
  case 0:
  case 1:
  case 2:
  case 3:
  default:
    return i2c_fds[0];
    break;
  }
}

unsigned int port_base_addr(int type, int port) {
  u16 port_base;
  switch (type) {
  case PORT_POWER:
    port_base = PORT_POWER_BASE;
    break;
  case PORT_CONFIG:
  default:
    port_base = PORT_CR_BASE;
  }
  return (port_base - 2) + (unsigned int)(port * 2);
}

unsigned char get_pd690xx_addr(int port) {
  // check the port number AND verify that the pd690xx is present
  // on the bus before returning the address
  int select = abs(port / 12);
  // can't have more than 4 pd690xx in the switch
  // port number is too high
  if (select > 4) {
    return 0;
  }
  switch (pd690xx_pres[select]) {
  case 0:
    // selected pd690xx is not present
    return 0;
    break;
  case 1:
    // pd690xx is present, return I2C address
    return pd690xx_addrs[select];
    break;
  }
  // if (0 < port < 12) && (pd690xx_pres[0] == 1) {
  //     return PD690XX0_I2C_ADDR;
  // } elif (12 < port < 24) && (pd690xx_pres[1] == 1) {
  //     return PD690XX1_I2C_ADDR;
  // } elif (24 < port < 36) && (pd690xx_pres[2] == 1) {
  //     return PD690XX2_I2C_ADDR;
  // } elif (36 < port < 48) && (pd690xx_pres[3] == 1) {
  //     return PD690XX3_I2C_ADDR;
  // }
  // port number was bogus or pd690xx was not present on bus
  return 0;
}

// Read the given I2C slave device's register and return the read value in
// `*result`:
int i2c_read(int i2c_fd, u8 slave_addr, u16 reg, u16 *result) {
  int retval;
  u8 outbuf[2], inbuf[2];
  struct i2c_msg msgs[2];
  struct i2c_rdwr_ioctl_data msgset[1];

  msgs[0].addr = slave_addr;
  msgs[0].flags = 0;
  msgs[0].len = 2;
  msgs[0].buf = outbuf;

  msgs[1].addr = slave_addr;
  msgs[1].flags = I2C_M_RD;
  msgs[1].len = 2;
  msgs[1].buf = inbuf;

  msgset[0].msgs = msgs;
  msgset[0].nmsgs = 2;

  outbuf[0] = (reg >> 8) & 0xFF;
  outbuf[1] = reg & 0xFF;

  inbuf[0] = 0;
  inbuf[1] = 0;

  *result = 0;
  if (ioctl(i2c_fd, I2C_RDWR, &msgset) < 0) {
// we're going to relegate this to debug
// since it is expected to fire during pd690xx detection
#ifdef DEBUG
    perror("ioctl(I2C_RDWR) in i2c_read");
#endif
    return -1;
  }
#ifdef DEBUG
  // print raw I2C response as hex
  // otherwise you can always use kernel i2c tracing
  printf("I2C data: %02X%02X\n", inbuf[0], inbuf[1]);
#endif
  *result = (u16)inbuf[0] << 8 | (u16)inbuf[1];
  return 0;
}

int i2c_init(void) {
  int i2c_fd;
  for (int i2c_dev = 0; i2c_dev < 1; i2c_dev++) {
    snprintf(i2c_fname, sizeof(i2c_fname), "%s%d", i2c_fname_base, i2c_dev + 1);
    if ((i2c_fd = open(i2c_fname, O_RDWR)) < 0) {
      // couldn't open the device
      i2c_fds[i2c_dev] = -1;
    } else {
      // save the file descriptor in the array of global file descriptors
      i2c_fds[i2c_dev] = i2c_fd;
      u16 res;
      switch (i2c_dev) {
      case 0:
#ifdef DEBUG
        printf("/dev/i2c-0\n");
#endif
        for (int i = 0; i < 4; i++) {
// read the CFGC_ICVER register and if we get a response
// mark the pd690xx as present
#ifdef DEBUG
          printf("Probing I2C address %02X\n", pd690xx_addrs[i]);
#endif
          i2c_read(i2c_fd, pd690xx_addrs[i], CFGC_ICVER, &res);
          if ((res >> 10) > 0) {
            pd690xx_pres[i] = 1;
          }
        }
        break;
      case 1:
#ifdef DEBUG
        printf("/dev/i2c-1\n");
#endif
        for (int i = 2; i < 4; i++) {
#ifdef DEBUG
          printf("Probing I2C address %02X\n", pd690xx_addrs[i]);
#endif
          i2c_read(i2c_fd, pd690xx_addrs[i], CFGC_ICVER, &res);
          if ((res >> 10) > 0) {
            pd690xx_pres[i] = 1;
          }
        }
        break;
      }
    }
  }
#ifdef DEBUG
  printf("Detected %d pd690xx\n", pd690xx_pres_count());
#endif
}

void i2c_close() {
  // loop through i2c-1 and i2c-2 and close them if they were open
  for (int i = 0; i < 2; i++) {
    if (i2c_fds[i] != -1) {
      close(i2c_fds[i]);
    }
  }
}
int pd690xx_pres_count(void) {
  int total = 0;
  for (int i = 0; i < 4; i++) {
    if (pd690xx_pres[i] == 1) {
      total++;
    }
  }
  return total;
}
// Write to an I2C slave device's register:
int i2c_write(int i2c_fd, u8 slave_addr, u16 reg, u16 data) {
  int retval;
  u8 outbuf[4];

  struct i2c_msg msgs[1];
  struct i2c_rdwr_ioctl_data msgset[1];

  outbuf[0] = (reg >> 8) & 0xFF;
  outbuf[1] = reg & 0xFF;
  outbuf[2] = (data >> 8) & 0xFF;
  outbuf[3] = data & 0xFF;

  msgs[0].addr = slave_addr;
  msgs[0].flags = 0;
  msgs[0].len = 4;
  msgs[0].buf = outbuf;

  msgset[0].msgs = msgs;
  msgset[0].nmsgs = 1;

  if (ioctl(i2c_fd, I2C_RDWR, &msgset) < 0) {
// we're going to relegate this to debug
// since it is expected to fire during pd690xx detection
#ifdef DEBUG
    perror("ioctl(I2C_RDWR) in i2c_write");
#endif
    return -1;
  }

  return 0;
}

float port_power(int port) {
  i2c_init();
  u16 res;
  u8 pd_addr = get_pd690xx_addr(port);
  if (pd_addr == 0) {
    return -1;
  }
  int i2c_fd = pd690xx_fd(port);
  i2c_read(i2c_fd, pd_addr, port_base_addr(PORT_POWER, port), &res);
  i2c_close();
  return (float)res / 10;
}

int get_power(int port) {
  u16 res;
  // only -p was specified, get system power
  if (port == 0) {
    float total = 0;
    int pd690xx_count = pd690xx_pres_count();
    for (int i = 0; i < pd690xx_count; i++) {
      i2c_write(pd690xx_fd(i * 12), pd690xx_addrs[i], UPD_POWER_MGMT_PARAMS,
                0x01);
      usleep(100000);
      i2c_read(pd690xx_fd(i * 12), pd690xx_addrs[i], SYS_TOTAL_POWER, &res);
      total += (float)res / 10;
    }
    printf("Total: %.1f W\n", total);
  } else {
    // optarg was a port number, so get power for a specific port
    float power = port_power(port);
    printf("Port %d: %.1f W\n", port, power);
  }
  return 0;
}

int get_temp(int i) {
  i2c_init();
  u16 res;
  int pd690xx_count = pd690xx_pres_count();
  if (i > pd690xx_count) {
    return -1;
  }
  // this seems _kind of_ sane, but I'm not sure
  // the datasheet has two different versions of the formula
  // and nothing about a sign bit, since it's -200 to 400 C
  i2c_read(pd690xx_fd(i * 12), pd690xx_addrs[i], AVG_JCT_TEMP, &res);
  i2c_close();
  return (((int)res - 684) / -1.514) - 40;
}


int port_state(int port) {
  u16 res;
  int port_enabled = -1;
  u8 pd_addr = get_pd690xx_addr(port);
  if (pd_addr == 0) {
    return -1;
  }
  int i2c_fd = pd690xx_fd(port);
  i2c_read(i2c_fd, pd_addr, port_base_addr(PORT_CONFIG, port), &res);
  port_enabled = res & 0x03;
#ifdef DEBUG
  switch (port_enabled) {
  case PORT_DISABLED:
    printf("Port %d: disabled\n", port);
    break;
  case PORT_ENABLED:
    printf("Port %d: enabled\n", port);
    break;
  case PORT_FORCED:
    printf("Port %d: forced\n", port);
    break;
  default:
    printf("Port %d: unknown (%d)\n", port, port_enabled);
  }
#endif
  return port_enabled;
}

int port_type(int port) {
  u16 res;
  int port_mode = -1;
  u8 pd_addr = get_pd690xx_addr(port);
  if (pd_addr == 0) {
    return port_mode;
  }
  int i2c_fd = pd690xx_fd(port);
  i2c_read(i2c_fd, pd_addr, port_base_addr(PORT_CONFIG, port), &res);
  port_mode = (res & 0x30) >> 4;

  return port_mode;
}

const char * port_type_str(int port) {
    char buffer[256];

    switch (port_type(port)) {
    case PORT_MODE_AF:
      return "af";
    case PORT_MODE_AT:
      return "at";
    default:
      return itoa(port_type(port), buffer, 10);
    }
}