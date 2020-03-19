#ifndef CAP_ELL_DEBUG_H
#define CAP_ELL_DEBUG_H

#define ELL_RELOC_DBG 0x81
#define ELL_INTRA_DBG 0x82
#define ELL_AUTH_DBG  0x83
#define ELL_IFREQ_DBG 0x84
#define DRV_HWNSA_DBG 0x85

extern int cap_ell_dmesg_init(u32 module_id, u32 dmesg_len, u32 dmesg_count);
extern char* cap_ell_dmesg_alloc(u32 module_id);
extern void cap_ell_dmesg_free(u32 module_id, char *dmesg);
extern void cap_ell_dmesg_put(u32 module_id, char *dmesg);
extern char* cap_ell_dmesg_get(u32 module_id);

#endif
