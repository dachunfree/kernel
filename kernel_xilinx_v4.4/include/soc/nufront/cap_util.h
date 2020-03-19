#ifndef _CAP_UTIL_H_
#define _CAP_UTIL_H_

#include <soc/nufront/netbuf_lib.h>

#define FILE_PRINT 5
#define MAIN_DBG  4
#define MAIN_INFO 3
#define MAIN_WARNING 2
#define MAIN_ERROR 1

#define MAIN_PRINT_LEAVLE MAIN_WARNING

#define main_printf(level,fmt,args...) \
do {\
} while (0)

#define ARQ_CAP_ACK_DROP
#define ARQ_CAP_DATA_DROP

#define ARQ_STA_ACK_DROP
#define ARQ_STA_DATA_DROP

#define DBG_INFO 1
#define ARQ_ERROR_LEVEL 3

#define	arq_printf(flag, args...) if (flag >= DBG_INFO) printk(args)

#define  ETHERTYPE_IP		0x0800	/* IP protocol */
#define  ETHERTYPE_ARP		0x0806	/* ARP protocol */
#define  PPPOE_DISCOVERY    0x8863  /* 发现阶段的以太网帧中的类型字段值 */
#define  PPPOE_SESSION      0x8864  /* PPP会话阶段的以太网帧中的类型字段值 */
#define  VLAN               0x8100  /* VLAN以太网帧中的类型字段值 */

/* its 紧急业务类型 */
#define  ETHERTYPE_EMERGENCY		0x88dc	/*  */

/* 协议头长度定义 */
#define ETH_HEADER_LEN    14
#define VLAN_HEADER_LEN   4
#define IP_HEADER_LEN     20
#define TCP_HEADER_LEN    20
#define UDP_HEADER_LEN    8
#define TCP_ACK_TOTOL_LEN 52
#define ARP_PACKET_LEN    42
#define MAC_ADDR_LEN      6

#define CAP_NTOHL(x) ((((x) & 0xFF000000) >> 24) |  \
                      (((x) & 0x00FF0000) >> 8)  |  \
                      (((x) & 0x0000FF00) << 8)  |  \
                      (((x) & 0x000000FF) << 24))

#define CAP_NTOHS(x) ((((x)& 0xFF00) >> 8) | (((x) & 0x00FF) << 8))
#define CAP_HTONL(x) CAP_NTOHL(x)
#define CAP_HTONS(x) CAP_NTOHS(x)

#define IP_DF             0x4000               /* dont fragment flag */
#define IP_MF             0x2000               /* more fragments flag */
#define IP_OFFMASK        0x1fff               /* mask for fragmenting bits */

/* TCP flags字段标识 */
#define	TCP_TH_FIN	0x01
#define	TCP_TH_SYN	0x02
#define	TCP_TH_RST	0x04
#define	TCP_TH_PUSH	0x08
#define	TCP_TH_ACK	0x10
#define	TCP_TH_URG	0x20

/* 计算IP/TCP头长度 */
#define IP_TOTAL_LENGTH(IpHdr) (CAP_NTOHS(IpHdr.TotalLength))

#define IP_HEADER_LENGTH(IpHdr)   \
        ( (unsigned long)((IpHdr.HeaderLength & 0x0F) << 2) )

#define TCP_HEADER_LENGTH(pTcpHdr) \
        ( (u16)(((*((PUCHAR)(&(pTcpHdr->th_len))) & 0xF0) >> 4) << 2) )

/* 判断是否属于分割报文 */
#define IS_IP_FRAGMENT(FlagsAndOffset) \
        ((bool)((FlagsAndOffset & IP_MF) || (FlagsAndOffset & IP_OFFMASK) != 0))

/* 以太网头定义 */
typedef struct _ETH_HEADER
{
    u8       DstAddr[MAC_ADDR_LEN];
    u8       SrcAddr[MAC_ADDR_LEN];
    u16      EthType;
}ETH_HEADER, *PETH_HEADER;


/* IPv4 Internet address,This is an 'on-wire' format structure. */
typedef struct _in_addr {
        union {
                struct { u8 s_b1,s_b2,s_b3,s_b4; } S_un_b;
                struct { u16 s_w1,s_w2; } S_un_w;
                unsigned long S_addr;
        } S_un;
#define s_addr  S_un.S_addr /* can be used for most tcp & ip code */
#define s_host  S_un.S_un_b.s_b2    // host on imp
#define s_net   S_un.S_un_b.s_b1    // network
#define s_imp   S_un.S_un_w.s_w2    // imp
#define s_impno S_un.S_un_b.s_b4    // imp #
#define s_lh    S_un.S_un_b.s_b3    // logical host
} IN_ADDR, *PIN_ADDR, *LPIN_ADDR;

/*
// IPV4_HEADER
// Define the structure of an IPv4 header.
// The field names match those in section 3.1 of RFC 791.
// RFC 2474 redefines type of service to the 6 bit DSCP value. RFC 2780 and
// 3168 redefine the unused 2 bits in the traffic class octet as used by
// ECN.
*/
typedef struct _IPV4_HEADER {
    //union {
        //u8 VersionAndHeaderLength;  /* Version and header length. */
        //struct {
            u8 HeaderLength : 4;
            u8 Version : 4;
        //}S_un_a;
    //}S_un_a;

    //union {
        //u8 TypeOfServiceAndEcnField;  /* Type of service & ECN (RFC 3168). */
        //struct {
            u8 EcnField : 2;
            u8 TypeOfService : 6;
        //}S_un_a;
    //}S_un_b;

    u16 TotalLength;                  /* Total length of datagram. */
    u16 Identification;
    //union {
        u16 FlagsAndOffset;           /*  Flags and fragment offset. */
        //struct {
        //    u16 Reserved : 1;
        //    u16 DontFragment : 1;
        //    u16 MoreFragments : 1;
        //    u16 DontUse1 : 5;         /*  High bits of fragment offset. */
        //    u16 DontUse2 : 8;         /* Low bits of fragment offset. */
        //}S_un_a;
    //}S_un_c;

    u8 TimeToLive;
    u8 Protocol;
    u16 HeaderChecksum;
    IN_ADDR SourceAddress;
    IN_ADDR DestinationAddress;
} IPV4_HEADER, *PIPV4_HEADER;


/* ICMP头结构体定义 */
typedef struct _ICMP_HEADER {
    u8 Type;         // Type of message (high bit zero for UWB_ERROR messages).
    u8 Code;         // Type-specific differentiater.
    u16 Checksum;    // Calculated over ICMP message and IPvx pseudo-header.
} ICMP_HEADER, *PICMP_HEADER;

/* TCP头结构体定义 */
/* Define a type for TCP sequence numbers. */
typedef u32 SEQ_NUM, *PSEQ_NUM;

/*
// TCP_HDR
// Define the structure of a TCP header.
// $REVIEW: We shouldn't have two different structures for representing
// a TCP header.  Can we get rid of this one?
*/
typedef struct _tcp_header {
        u16 th_sport;              /* 源端口号 */
        u16 th_dport;              /* 目的端口号 */
        SEQ_NUM th_seq;               /* 序列号 */
        SEQ_NUM th_ack;               /* 确认号 */
        u8 th_x2 : 4;              /* 高4位表示数据偏移 */
        u8 th_len : 4;
        u8 th_flags;               /* 标志位 */
        u16 th_win;                /* 窗口大小 */
        u16 th_sum;                /* 校验和 */
        u16 th_urp;                /* 紧急数据偏移量 */
} TCP_HEADER, *PTCP_HEADER;

/* UDP头结构体定义 */
typedef struct _udp_header
{
    u16    srcport;            /* 源端口号 */
    u16    dstport;            /* 目的端口号 */
    u16    len;                /* 封包长度 */
    u16    checksum;           /* 校验和 */
}UDP_HEADER, *PUDP_HEADER;

/* cap端的station id */
#define CAP_ID 0xfff
/* cap端的ERROR station id */
#define CAP_ERROR_ID 0xffe

#define ERROR_ID 0xff

/* 广播的station id */
#define BROADCAST_CTR_STAID 0x000

#define BROADCAST_DATA_STAID 0x001

#define CHAR_BIT (8)
#define BITMASK(b) (1 << ((b) % CHAR_BIT))
//#define BITSLOT(b) ((b) / CHAR_BIT)
#define BITSLOT(b) ((b) >> 3)
#define BITSET(a,b) ((a)[BITSLOT(b)] |= BITMASK(b))
#define BITCLEAR(a,b) ((a)[BITSLOT(b)] &= ~BITMASK(b))
#define BITTEST(a,b) ((a)[BITSLOT(b)] & BITMASK(b))
//#define BITNSLOTS(nb) ((nb + CHAR_BIT - 1) / CHAR_BIT)
#define BITNSLOTS(nb) ((nb + CHAR_BIT - 1) >> 3)
//add by shenfa.liu
#define BIT_TEST(a,b) ( ((a)[BITSLOT(b)] & BITMASK(b))>> ((b) % CHAR_BIT))

void bufferPushTail(struct mblk *pMblk, u8 *pSrcAddr, u32 len);
#endif
