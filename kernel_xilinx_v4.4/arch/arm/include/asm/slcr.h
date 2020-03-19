#ifndef _SLCR_H_
#define _SLCR_H_

int zynq_slcr_write(u32 val, u32 offset);
int zynq_slcr_read(u32 *val, u32 offset);
#endif
