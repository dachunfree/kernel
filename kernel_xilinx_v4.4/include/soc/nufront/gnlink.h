#ifndef _GNLINK_H_
#define _GNLINK_H_

enum {
	CTL_CMD_UNSPEC,
	CTL_CMD_REPORT,
	CTL_CMD_GET,
	CTL_CMD_SET,
	__CTL_CMD_MAX,
};

int euht_snd_genl_msg(void *data, int len, int cmd_idx);
int euht_genl_kernel_init(void);
void euht_genl_kernel_release(void);

#endif
