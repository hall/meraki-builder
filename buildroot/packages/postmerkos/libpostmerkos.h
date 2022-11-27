#ifndef postmerkos
#define postmerkos

#define DEVICE_FILE "/etc/boardinfo"
#define PORTS_FILE "/click/switch_port_table/dump_pports"
#define VLANS_FILE "/click/switch_port_table/dump_pport_vlans"

_Bool has_poe();
char *get_time();
char *get_name();
_Bool starts_with(const char *, const char *);
_Bool ends_with(const char *, const char *);
char *itoa(int, char *, int);
const char *get_field(char *, int);

#endif // postmerkos