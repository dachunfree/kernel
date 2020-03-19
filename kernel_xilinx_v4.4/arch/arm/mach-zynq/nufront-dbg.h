
#ifndef _NUFRONT_DBG_H_
#define _NUFRONT_DBG_H_

enum cmd {
	D_READ = 0,
	D_WRITE,
	D_BIC,
	D_BIS,
	D_MEMSET,
	D_DISPLAY,
	D_MAX,
};

struct dbg_cmd {
	int cmd;
	unsigned int addr;
	int len;
	unsigned int value;
};

#define NUFRONT_DBG_MAGIC 	('d' + 0x80)
#define NUFRONT_DBG_CMD  	_IOWR(NUFRONT_DBG_MAGIC, 8, struct dbg_cmd)
#endif

