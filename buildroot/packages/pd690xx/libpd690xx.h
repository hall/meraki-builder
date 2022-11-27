
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
int get_voltage();
const char *port_type_str(int);

#endif // libpd690xx
#ifndef pd690xx
#define pd690xx
#ifndef libpd690xx
#define libpd690xx

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





#define MAX_PD690XX_COUNT 4

struct pd690xx_cfg {
    // Array of file descriptors used to talk to the I2C bus 1 and 2
    int i2c_fds[2];
    // pd690xx addresses
    unsigned char pd690xx_addrs[4];
    // detect if the pd690xx devices are present
    int pd690xx_pres[4];
};

void i2c_init(struct pd690xx_cfg *pd690xx);
void i2c_close(struct pd690xx_cfg *pd690xx);
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
int i2c_read(int, unsigned char, unsigned int, unsigned int *);
unsigned char get_pd690xx_addr(struct pd690xx_cfg *pd690xx, int);
int pd690xx_bus(struct pd690xx_cfg *pd690xx, int);
int pd690xx_pres_count(struct pd690xx_cfg *pd690xx);
unsigned int port_base_addr(int, int);
int port_able(struct pd690xx_cfg *pd690xx, int, int);
int port_enable(struct pd690xx_cfg *pd690xx, int);
int port_disable(struct pd690xx_cfg *pd690xx, int);
int port_reset(struct pd690xx_cfg *pd690xx, int);
int port_force(struct pd690xx_cfg *pd690xx, int);
int port_state(struct pd690xx_cfg *pd690xx, int);
int port_type(struct pd690xx_cfg *pd690xx, int);
int get_power(struct pd690xx_cfg *pd690xx, int);
int get_voltage(struct pd690xx_cfg *pd690xx);
int get_temp(struct pd690xx_cfg *pd690xx);
float port_power(struct pd690xx_cfg *pd690xx, int);
int port_priority(struct pd690xx_cfg *pd690xx, int);
void enable_debug(void);

#endif
