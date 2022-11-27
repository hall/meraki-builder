#ifndef configd
#define configd

#define CONFIG_FILE "/etc/switch.json"

struct pd690xx_cfg;

void read_status(struct pd690xx_cfg *pd690xx);

struct json_object* read_config(struct pd690xx_cfg *pd690xx);
int write_config(struct pd690xx_cfg *pd690xx, struct json_object *json);

void poll(struct pd690xx_cfg *pd690xx);
void run_daemon(struct pd690xx_cfg *pd690xx);

#endif
