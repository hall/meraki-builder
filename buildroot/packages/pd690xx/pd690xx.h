#ifndef pd690xx
#define pd690xx

// All values sourced from Auto Mode PD690xx Registers Map

// Timer Value for Overload conditions
#define TIMER_OVERLOAD_AT 0x100E
#define TIMER_OVERLOAD_AF 0x1010

// Maximum Vmain Voltage Level Threshold
#define VMAIN_HT 0x12FE
// Minimum Vmain Voltage Level Threshold
#define VMAIN_AT_LT 0x1300
#define VMAIN_AF_LT 0x1302

// Total power budget
#define MASTER_CFG_POWER_BASE 0x138C
#define PM_MODE_SYS_FLAG 0x1160 // bit 6

// General use
#define GENERAL_USE_REG 0x0318
#define SW_CFG_REG 0x139E
#define I2C_COMM_EXT_SYNC 0x1318
#define EXT_EVENT_INTR 0x1144





int i2c_init();
void i2c_close();
int i2c_write(int, unsigned char, unsigned int, unsigned int);
unsigned char get_pd690xx_addr(int);
int pd690xx_bus(int);
int pd690xx_pres_count();
unsigned int port_base_addr(int, int);
int port_able(int, int);
int port_enable(int);
int port_disable(int);
int port_force(int);
int port_priority(int);
void list_all();

#endif
