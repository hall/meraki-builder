#ifndef libpd690xx
#define libpd690xx

typedef unsigned char u8;
typedef unsigned int u16;

// gathered from I2C trace of libpoecore
#define PD690XX0_I2C_ADDR 0x30
#define PD690XX1_I2C_ADDR 0x31
#define PD690XX2_I2C_ADDR 0x33
#define PD690XX3_I2C_ADDR 0x35

// Used by port_base_addr
#define PORT_CONFIG 1
#define PORT_POWER 2

// Port status monitoring
#define PORT_CLASS_BASE 0x11C2
#define PORT_POWER_BASE 0x12B4
#define PORT_PM_INDICATION 0x129C // bit per port

// Used for the port status
#define PORT_DISABLED 0
#define PORT_ENABLED 1
#define PORT_FORCED 2
#define PORT_MODE_AF 0
#define PORT_MODE_AT 1
#define PORT_PRIO_CRIT 0
#define PORT_PRIO_HIGH 1
#define PORT_PRIO_LOW 2

// Port configuration
#define PORT_ICUT_MODE 0x1160 // bit 4
#define PORT_CR_BASE 0x131A // PORT0
#define PORT_EN_CTL 0x1332 // bit per port
#define PORT_CURRENT_SENSE_BASE 0x1044

// System Status / Monitoring
#define VMAIN 0x105C
#define SYS_INIT 0x1164
#define AVG_JCT_TEMP 0x130A
#define CFGC_ICVER 0x031A
#define SYS_TOTAL_POWER 0x12E8
#define TOTAL_POWER_SLAVE_BASE 0x12EC
#define LOCAL_TOTAL_POWER 0x12AA
#define ADD_IC_STATUS 0x1314
#define UPD_POWER_MGMT_PARAMS 0x139C
#define SW_BOOT_STATE 0x1168

int pd690xx_fd(int);
unsigned int port_base_addr(int, int);
unsigned char get_pd690xx_addr(int);
int i2c_read(int, u8, u16, u16 *);
int i2c_init(void);
void i2c_close();
int pd690xx_pres_count(void);
int i2c_write(int, u8, u16, u16);
float port_power(int);
int get_power(int);
int port_type(int);
int get_temp(int);
int port_state(int);
const char * port_type_str(int);

#endif // libpd690xx