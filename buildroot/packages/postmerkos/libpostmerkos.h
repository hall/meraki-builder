#ifndef postmerkos
#define postmerkos

#define DEVICE_FILE "/etc/boardinfo"
#define PORTS_FILE "/click/switch_port_table/dump_pports"

_Bool hasPoe();
char *getTime();
_Bool startsWith(const char*, const char*);
_Bool endsWith(const char*, const char *);
char *itoa(int, char*, int);
const char *getfield(char*, int);

#endif // postmerkos