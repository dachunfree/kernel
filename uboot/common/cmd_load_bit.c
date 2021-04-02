/*
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * Misc boot support
 */
#include <common.h>
#include <command.h>

extern int zynq_move_bit(int sys_flag);
extern  int pcap_configure_pl(int boot_part);
#ifdef CONFIG_CMD_LOAD_BIT

/* Allow ports to override the default behavior */
static int do_load_bit(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	pcap_configure_pl(0);
	return 0;
}

/* -------------------------------------------------------------------- */

U_BOOT_CMD(
	load_bit, CONFIG_SYS_MAXARGS, 0,	do_load_bit,
	"Load cap_mac_fpga.bit file",
	"addr [arg ...]\n    - start application at address 'addr'\n"
	"      passing 'arg' as arguments"
);

#endif
