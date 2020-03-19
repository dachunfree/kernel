#ifndef  _NUFRONT_PHY_H_
#define  _NUFRONT_PHY_H_

#define PHY_REG_BASE_ADDR (MAC_REG_BASE_ADDR + 0x10000)
#define PHY_BBP_REG0_ADDR    0x0000
#define PHY_BBP_REG1_ADDR    0x0004
#define PHY_BBP_REG2_ADDR    0x0008
#define PHY_BBP_REG3_ADDR    0x000c
#define PHY_BBP_REG4_ADDR    0x0010
#define PHY_BBP_REG5_ADDR    0x0014
#define PHY_BBP_REG6_ADDR    0x0018
#define PHY_BBP_REG7_ADDR    0x001c
#define DEFAULT_PHY_ADDA_CTRL_VALUE  0x00ffffff
#define PHY_PHY_PAR_DEFAULT     0x02
#define PHY_PHY_PAR_CBW_20M     0x00
#define PHY_PHY_PAR_CBW_40M     0x01
#define PHY_PHY_PAR_CBW_80M     0x02

#define CAP_PHY_REG_LEN 	21
#define PHY_BACKUP_LEN 		21
#define IQ_BACKUP_LEN 		24
#define PHY_TOP_BACKUP_LEN 	4

#define DEFAULT_PHY_GPS_MAP     3
#define PHY_GPS_MAP_GROUP       11
#define PHY_GPS_MAP_ITEM        8

#define PHY_IQ_Calib_BASE_ADDR	(MAC_REG_BASE_ADDR + 0x10C00)
#define PHY_IQ_Calib_ADDA_sign_ctrl	0x44
#define CAP_PHY_REG_NUM	30
#define PYH_IDENT_ADDR		(MAC_REG_BASE_ADDR + (0x10FE4 << HMAC_OFFSET)) /*0x40010FE4*/
/* tx filter addr base */
#define BB_TX_FILTER_BASE (MAC_REG_BASE_ADDR + (0x10200 << HMAC_OFFSET))

#define CAP_PLUS_PHY_TYPE	0x36A2
#define CAP_TRY_PHY_TYPE	0
#define PHY_PHY_ADDA_CTRL_ADDR	((g_is_phy == CAP_PLUS_PHY_TYPE) ? 0x0fcc : 0x03cc)
#define PHY_TO_PADDE_CTRL_ADDR	((g_is_phy == CAP_PLUS_PHY_TYPE) ? 0x0fe0 : 0x03e0)
#define PHY_PHY_PAR_ADDR	((g_is_phy == CAP_PLUS_PHY_TYPE) ? 0x0ff8 : 0x03f8)
#define PHY_IQCAL_ADDR		((g_is_phy == CAP_PLUS_PHY_TYPE) ? 0x0c44 : 0x0444)
#define BB_REG_BASE_ADDR	((g_is_phy == CAP_PLUS_PHY_TYPE) ? 0x0c00 : 0x0400)
extern u16 g_is_phy;
u32 euht_phy_read(u32 phy_addr);
void euht_phy_write(u32 reg_val, u32 phy_addr);
s8 cap_phy_ident_select(u32 **pptr);
void phy_init(void);
void phy_reset(void);
#endif
