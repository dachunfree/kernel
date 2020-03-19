/* netBufLib.h - network buffers header file */

/*
 * Copyright (c) 1984-2005 Wind River Systems, Inc.
 *
 * The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement. No license to Wind River intellectual property rights
 * is granted herein. All rights not licensed by Wind River are reserved
 * by Wind River.
 */

/*
   modification history
   --------------------
   */

#ifndef __INCnetBufLibh
#define __INCnetBufLibh

#include <linux/types.h>
#include <soc/nufront/ffslib.h>

/* include cfgDefs for CFG_DATA_HDR */
typedef struct cfg_data_hdr{
	int len;
} CFG_DATA_HDR;

/* using these typedefs to void compiler error */
typedef void* SEM_ID;
typedef void* PART_ID;


/* status codes */
/* M_netBufLib is defined in vwModNum.h */
#define	M_netBufLib (115 << 16)
#define	S_netBufLib_MEMSIZE_INVALID		(M_netBufLib | 1)
#define	S_netBufLib_CLSIZE_INVALID		(M_netBufLib | 2)
#define	S_netBufLib_NO_SYSTEM_MEMORY		(M_netBufLib | 3)
#define	S_netBufLib_MEM_UNALIGNED		(M_netBufLib | 4)
#define	S_netBufLib_MEMSIZE_UNALIGNED		(M_netBufLib | 5)
#define	S_netBufLib_MEMAREA_INVALID		(M_netBufLib | 6)
#define S_netBufLib_MBLK_INVALID		(M_netBufLib | 7)
#define S_netBufLib_NETPOOL_INVALID		(M_netBufLib | 8)
#define S_netBufLib_INVALID_ARGUMENT		(M_netBufLib | 9)
#define S_netBufLib_NO_POOL_MEMORY		(M_netBufLib | 10)
#define S_netBufLib_LIB_NOT_INITIALIZED		(M_netBufLib | 11)
#define S_netBufLib_POOL_FEATURE_NOT_SUPPORTED	(M_netBufLib | 12)
#define S_netBufLib_DUPLICATE_POOL 		(M_netBufLib | 13)
#define S_netBufLib_POOL_RELEASED 		(M_netBufLib | 14)
#define S_netBufLib_POOL_RELEASE_IGNORE		(M_netBufLib | 15)

/* usage MACRO's */
/* Tuple Length related MACROS */

#define TUP_LEN_GET(pMblk)	     ((int) (pMblk)->mBlkHdr.mLen)
#define TUP_LEN_SET(pMblk,length)    ((pMblk)->mBlkHdr.mLen = (length))
#define TUP_LEN_ZERO(pMblk)          ((pMblk)->mBlkHdr.mLen = 0)
#define TUP_CL_LEN_GET(pMblk)        ((int) (pMblk)->pClBlk->clSize)
#define TUP_CL_AVIL_LEN_GET(pMblk)   ((int) (pMblk)->pClBlk->clSize - \
		((int) (pMblk)->mBlkHdr.mLen) - \
		(((int) (pMblk)->mBlkHdr.mData) - \
		 ((int)(pMblk)->pClBlk->clNode.pClBuf)))

/* Next refers to the next Tuple in a Tuple chain */

#define TUP_NEXT_GET(pMblk)		((struct mblk *) (pMblk)->mBlkHdr.mNext)
#define TUP_NEXT_SET(pMblk,pNext)	((pMblk)->mBlkHdr.mNext = \
		(struct mblk *) (pNext))
#define TUP_NEXT_NULL(pMblk)		((pMblk)->mBlkHdr.mNext = \
		(struct mblk *) NULL)

/* Next Packet refers to the next Tuple (List of Tuples) */

#define TUP_NEXT_PK_GET(pMblk)		((struct mblk *) (pMblk)->mBlkHdr.mNextPkt)
#define TUP_NEXT_PK_SET(pMblk,pNext)	((pMblk)->mBlkHdr.mNextPkt = \
		(struct mblk *) (pNext))
#define TUP_NEXT_PK_NULL(pMblk)		((pMblk)->mBlkHdr.mNextPkt = \
		(struct mblk *) NULL)


/* mblk type */

#define TUP_TYPE_GET(pMblk)		((pMblk)->mBlkHdr.mType)
#define TUP_TYPE_SET(pMblk,type)	((pMblk)->mBlkHdr.mType = type)

/* Cluster Buffer Address */

#define TUP_BUF_GET(pMblk)		((pMblk)->mBlkHdr.mData)
#define TUP_BUF_SET(pMblk,addr)		((pMblk)->mBlkHdr.mData = addr)





#define NET_POOL_NAME_SZ	(4 * sizeof(long)) /* Length of pool name */


/* netBufLib Attributes required for buffer migration */

#define NB_ALIGN_OFFSET		0			/* Bits 0, 1 & 2 */
#define NB_ALIGN_MASK		(0x7 << NB_ALIGN_OFFSET)
#define NB_ALIGN_NONE		(0x0 << NB_ALIGN_OFFSET)
#define NB_ALIGN_INT		(0x1 << NB_ALIGN_OFFSET)
#define NB_ALIGN_CACHE		(0x2 << NB_ALIGN_OFFSET)
#define NB_ALIGN_PAGE		(0x3 << NB_ALIGN_OFFSET)

#define NB_SHARABLE		0x0400			/* Bit 10 */
#define NB_ISR_SAFE		0x0800			/* Bit 11 */

#define NB_MODE_OFFSET		12			/* Bits 12, 13 */
#define NB_MODE_MASK		(0x3 << NB_MODE_OFFSET)
#define NB_MODE_5_5BKWRDS	(0x0 << NB_MODE_OFFSET)

/* Flag bits used in pNetPool->flag */

#define NB_CREATE		0x01	/* netPool created via netPoolCreate */
#define NB_RELEASE		0x02	/* netPool is in a release state */
#define NB_RELEASE_START	0x04	/* started the release state */
#define NB_DESTROY_START	0x08	/* started the delete process */

/* netPoolRlease() MACROs */

#define NET_REL_IN_TASK		0x00	/* Release netPool in task context */
#define NET_REL_IN_CONTEXT	0x01	/* Release netPool in call context */
#define NET_REL_NAME		"tPoolRel"	/* Task name */
#define NET_REL_PRI		netPoolRelPri	/* see: netBufLib.c */
#define NET_REL_OPT		0		/* task options */
#define NET_REL_STACK		500		/* task stack */

/* Valid attribute combinations */

/* Int Aligned, Sharable, ISR Safe */

#define ATTR_AI_SH_ISR 		(NB_ALIGN_INT   | NB_ISR_SAFE | NB_SHARABLE | \
		NB_MODE_5_5BKWRDS)

#define ATTR_AC_SH_ISR 		(NB_ALIGN_CACHE | NB_ISR_SAFE | NB_SHARABLE | \
		NB_MODE_5_5BKWRDS)

/* Int Aligned, Private, ISR Safe */

#define ATTR_AI_ISR    		(NB_ALIGN_INT   | NB_ISR_SAFE | \
		NB_MODE_5_5BKWRDS)

#define ATTR_AC_ISR    		(NB_ALIGN_CACHE | NB_ISR_SAFE | \
		NB_MODE_5_5BKWRDS)

/* Buffer types for pMemReqRtn function */

#define NB_BUFTYPE_CLUSTER	0  /* clusters */
#define NB_BUFTYPE_M_BLK	1  /* M_BLKs */
#define NB_BUFTYPE_CL_BLK	2  /* CL_BLKs */

#define MAX_MBLK_TYPES			256	/* max number of mblk types */

/* cluster defines */

#define CL_LOG2_16			4
#define CL_LOG2_32			5
#define CL_LOG2_64			6
#define CL_LOG2_128			7
#define CL_LOG2_256			8
#define CL_LOG2_512			9
#define CL_LOG2_1024			10
#define CL_LOG2_2048			11
#define CL_LOG2_4096	 		12
#define CL_LOG2_8192 			13
#define CL_LOG2_16384			14
#define CL_LOG2_32768			15
#define CL_LOG2_65536			16

#define CL_SIZE_16			16
#define CL_SIZE_32			32
#define CL_SIZE_64			64
#define CL_SIZE_128			128
#define CL_SIZE_256			256
#define CL_SIZE_512			512
#define CL_SIZE_1024			1024
#define CL_SIZE_2048			2048
#define CL_SIZE_4096			4096
#define CL_SIZE_8192			8192
#define CL_SIZE_16384			16384
#define CL_SIZE_32768			32768
#define CL_SIZE_65536			65536

#define CL_LOG2_MIN			CL_LOG2_16
#define CL_LOG2_MAX			CL_LOG2_65536
#define CL_SIZE_MAX			(1 << CL_LOG2_MAX)
#define CL_SIZE_MIN			(1 << CL_LOG2_MIN)
#define CL_INDX_MIN			0
#define CL_INDX_MAX			(CL_LOG2_MAX - CL_LOG2_MIN)
#define CL_TBL_SIZE			(CL_INDX_MAX + 1)


#define CL_LOG2_TO_CL_INDEX(x)		((x) - CL_LOG2_MIN)
#define CL_LOG2_TO_CL_SIZE(x)		(1 << (x))
#define SIZE_TO_LOG2(size)		(ffsMsb((size)) - 1)
#define CL_SIZE_TO_CL_INDEX(clSize) 	(ffsMsb((clSize)) - (1 + CL_LOG2_MIN))

/*
 * Minimum cluster alignment
 * Some network data structures stored in clusters
 * contain 8-byte members of types such as unsigned long long,
 * which require 8-byte alignment on some architectures (SPR 102569).
 * However, for backwards comptibilty, we only ensure 4-byte
 * alignment by default.  Clients needing the 8-byte alignment can
 * use netPoolCreate(), or replace the default pool's memory requirements
 * function (see uipc_mbuf.c for an example).
 * This value must be a power of two.
 */

#define CL_ALIGN_MIN	4

/* mblk types */
#define	MT_FREE		0	/* should be on free list */
#define	MT_DATA		1	/* dynamic (data) allocation */
#define	MT_HEADER	2	/* packet header */
#define	MT_SOCKET	3	/* socket structure */
#define	MT_PCB		4	/* protocol control block */
#define	MT_RTABLE	5	/* routing tables */
#define	MT_HTABLE	6	/* IMP host tables */
#define	MT_ATABLE	7	/* address resolution tables */
#define	MT_SONAME	8	/* socket name */
#define MT_ZOMBIE       9       /* zombie proc status */
#define	MT_SOOPTS	10	/* socket options */
#define	MT_FTABLE	11	/* fragment reassembly header */
#define	MT_RIGHTS	12	/* access rights */
#define	MT_IFADDR	13	/* interface address */
#define MT_CONTROL	14	/* extra-data protocol message */
#define MT_OOBDATA	15	/* expedited data  */
#define	MT_IPMOPTS	16	/* internet multicast options */
#define	MT_IPMADDR	17	/* internet multicast address */
#define	MT_IFMADDR	18	/* link-level multicast address */
#define	MT_MRTABLE	19	/* multicast routing tables */
#define	MT_TAG		20	/* volatile metadata associated to pkts */

#define NUM_MBLK_TYPES	21	/* number of mblk types defined */

/* mblk flags */

#define	M_EXT		0x0001	/* has an associated cluster */
#define	M_PKTHDR	0x0002	/* start of record */
#define	M_EOR		0x0004	/* end of record */

/* flags 0x0008 - 0x0080 are used in mbuf.h */

/* mblk pkthdr flags, also in mFlags */

#define	M_BCAST		0x0100	/* send/received as link-level broadcast */
#define	M_MCAST		0x0200	/* send/received as link-level multicast */

/* flags 0x0400 - 0x1000 are used in mbuf.h */

#define M_FORWARD       0x2000  /* to be fast forwarded */
#define M_PROXY         0x4000  /* broadcast forwarded through proxy */
#define M_PROMISC	0x8000	/* indicates unipromisc for multi receive */
#define M_HEADER	M_PROMISC /* NPT transmit, full header present */

/*
 * M_EOB is set when the mblk contains the last part of a large block of
 * data sent by an application using TCP (a large block of data is one
 * which causes sosend to issue multiple protocol send requests).
 * M_EOB and M_EOR can be used together since M_EOB is only used for TCP
 * and M_EOR is only used for datagram protocols (UDP, raw IP).
 */

#define M_EOB           M_EOR

/* flags to mClGet/mBlkGet */

#define	M_DONTWAIT	1	/* don't wait if buffers not available */
#define	M_WAIT		0	/* wait if buffers not available */

/* length to copy to copy all data from the mblk chain*/

#define	M_COPYALL	1000000000

/* check to see if an mblk is associated with a cluster */

#define M_HASCL(pMblk)	((pMblk)->mBlkHdr.mFlags & M_EXT)

#define	M_BLK_SZ	sizeof(struct mblk) 	/* size of an mblk */
#define M_BLK_SZ_ALIGNED	64		/* See linkBufPool */
#ifndef MSIZE
#define	MSIZE		M_BLK_SZ	/* size of an mblk */
#endif

#define CL_BLK_SZ	sizeof(struct clblk)	/* size of cluster block */
#define CL_BLK_SZ_ALIGNED	64		/* See linkBufPool */

/* macro to get to the netPool pointer from the mblk */

#define MBLK_TO_NET_POOL(pMblk) (*(NET_POOL_ID *)((int)pMblk - sizeof(int)))
#define CLBLK_TO_NET_POOL(pClBlk) 	(pClBlk->pNetPool)

/* macro to get to the cluster pool ptr from the cluster buffer */

#define CL_BUF_TO_CL_POOL(pClBuf) (*(CL_POOL_ID *)((int)pClBuf - sizeof(int)))

/* macro to get to the cluster reference count from the cluster buffer */

#define CL_BUF_TO_REFCNT(pClBuf)  (u8 *)((int)pClBuf - (2 * sizeof(int)))

/* macros for accessing the functions of net pool directly */

#define poolInit(poolId,pMclBlkConfig,pClDescTbl,tblNumEnt,fromKheap)	\
	(*(((NET_POOL_ID)(poolId))->pFuncTbl->pInitRtn)) 	\
((poolId), (pMclBlkConfig), (pClDescTbl), (tblNumEnt), \
 (fromKheap))

#define mBlkFree(poolId,pMblk)						\
	(*(((NET_POOL_ID)(poolId))->pFuncTbl->pMblkFreeRtn)) 	\
((poolId), (pMblk))

#define clFree(poolId,pClBuf)						\
	(*(((NET_POOL_ID)(poolId))->pFuncTbl->pClFreeRtn)) 	\
((poolId), (pClBuf))

#define clBlkFree(poolId,pClBlk)					\
	(*(((NET_POOL_ID)(poolId))->pFuncTbl->pClBlkFreeRtn)) 	\
((pClBlk))

#define mBlkClFree(poolId,pMblk)					\
	(*(((NET_POOL_ID)(poolId))->pFuncTbl->pMblkClFreeRtn)) 	\
((poolId), (pMblk))

#define mBlkGet(poolId,canWait,type)					\
	(*(((NET_POOL_ID)(poolId))->pFuncTbl->pMblkGetRtn)) 	\
((poolId), (canWait), (type))

#define clBlkGet(poolId,canWait)					\
	(*(((NET_POOL_ID)(poolId))->pFuncTbl->pClBlkGetRtn)) 	\
((poolId), (canWait))

#define clusterGet(poolId,pClPool)					\
	(*(((NET_POOL_ID)(poolId))->pFuncTbl->pClGetRtn)) 	\
((poolId), (pClPool))

#define mClGet(poolId,pMblk,bufSize,canWait,noSmaller)			\
	(*(((NET_POOL_ID)(poolId))->pFuncTbl->pMblkClGetRtn)) 	\
((poolId), (pMblk), (bufSize), (canWait), (noSmaller))

#define clPoolIdGet(poolId,bufSize,noSmaller)				\
	(*(((NET_POOL_ID)(poolId))->pFuncTbl->pClPoolIdGetRtn))	\
((poolId), (bufSize), (noSmaller))

#define netMemReq(poolId, type, num, size)				\
	(((NET_POOL_ID)(poolId))->pFuncTbl->pMemReqRtn		\
	 ((type), (num), (size)))

#define ML_SIZE \
	ROUND_UP(sizeof(struct mblk) + sizeof(struct clblk) + sizeof(NET_POOL_ID), \
			NETBUF_ALIGN)
#define CL_SIZE(x) (ROUND_UP((x), NETBUF_ALIGN))

#ifndef NB_FULL_BKWARD_MODE
#define NB_FULL_BKWARD_MODE
#endif

/* typedefs */

/*
 * This structure is used to configure the number of mBlks and cluster blocks
 * that are allocated in a netPool. Each cluster has a corresponding cluster
 * block which manages it. An mblk manages a cluster block and the cluster.
 * When a netPool is configured there should as many cluster blocks as the
 * cumulative number of clusters. The defaults are defined above, but the
 * structure is defined and filled in usrConfig.c. For TCP applications it is
 * good to have two mBlks for every cluster block.
 */

typedef struct
{
	int		mBlkNum;		/* number of mBlks */
	int		clBlkNum;		/* number of cluster Blocks */
	char * 	memArea;		/* pre allocated memory area */
	int		memSize;		/* pre allocated memory size */
} M_CL_CONFIG;

/*
 *  This structure defines the way the networking code utilizes clusters.
 *  A table of type CL_DESC is filled in usrNetwork.c.
 *  Clusters are also used to hold network data structures other than the
 *  regular data. The network data structures are routing table entries,
 *  interface address structures, multicast address structures etc.
 */

typedef struct clDesc
{
	int		clSize;			/* cluster type */
	int		clNum;			/* number of clusters */
	char *	memArea;		/* pre allocated memory area */
	int		memSize;		/* pre allocated memory size */
} CL_DESC;

/* this structure defines the cluster */
typedef struct clBuff
{
	struct clBuff *	pClNext; 	/* pointer to the next clBuff */
} CL_BUF;
typedef CL_BUF * CL_BUF_ID;

/* This structure defines the each cluster pool */
typedef struct clPool
{
	int			clSize;		/* cluster size */
	int			clLg2;		/* cluster log 2 size */
	int			clNum; 		/* number of clusters */
	int			clNumFree; 	/* number of clusters free */
	int			clUsage;	/* number of times used */
	int			clMinAlloc;	/* smallest block size requested */
	int			clMaxAlloc;	/* largest block size requested */
	unsigned long	clTotAlloc;	/* total allocation for this pool */
	CL_BUF_ID		pClHead;	/* pointer to the cluster head */
	struct netPool *	pNetPool;	/* pointer to the netPool */
} CL_POOL;

typedef CL_POOL * CL_POOL_ID;

/* header at beginning of each mblk */
struct mhdr
{
	struct mblk	*mNext;		/* next buffer in chain */
	struct mblk	*mNextPkt;	/* next chain in queue/record */
	char 		*mData;		/* location of data */
	int		mLen;		/* amount of data in this mblk */
	u16		mType;		/* type of data in this mblk */
	u16		mFlags;		/* flags; see below */
	u16		reserved;
	u16		offset1;	/* network service offset */
};

/* record/packet header in first mblk of chain; valid if M_PKTHDR set */
struct pkthdr
{
	struct ifnet 	*rcvif;		/* rcv interface */
	struct mblk 	*aux;            /* extra data buffer */
	void 		*header;         /* pointer to packet header */
	u32		csum_flags;     /* checksum flags */
	u32		csum_data;      /* used by csum routines */
	int		len;		/* total packet length */
	u16		qnum;		/* priority queueing number */
	u16		vlan;		/* vlan control information */
};

union clblk_list
{
	struct clblk	*pClBlkNext;	/* pointer to the next clblk */
	char 		*pClBuf;		/* pointer to the buffer */
};

/* description of external storage mapped into mblk, valid if M_EXT set */

struct clblk
{
	struct netPool 		*pNetPool;	/* pointer to the netPool */
	union clblk_list	clNode;		/* union of next clblk, buffer ptr */
	u32			clSize;		/* cluster size */
	int			clRefCnt;	/* reference count of the cluster */
	int 			(*pClFreeRtn)(void);	/* pointer to cluster free routine */
	int			clFreeArg1;	/* free routine arg 1 */
	int			clFreeArg2;	/* free routine arg 2 */
	int			clFreeArg3;	/* free routine arg 3 */
};

/* mblk structure */
struct mblk
{
	struct pkthdr	mBlkPktHdr;		/* pkthdr */
	struct mhdr	mBlkHdr; 		/* header */
	struct clblk 	*pClBlk;			/* pointer to cluster blk */
};
typedef struct mblk	M_BLK;
typedef M_BLK *		M_BLK_ID;

/* mblk statistics used to show data pool by show routines */

typedef struct mbstat
{
	unsigned long mTypes[256];		/* type specific mblk allocations */
	unsigned long mNum;			/* mBlks obtained from page pool */
	unsigned long mDrops;			/* times failed to find space */
	unsigned long mWait;			/* times waited for space */
	unsigned long mDrain;			/* times drained protocols for space */
} M_STAT;

typedef struct _M_LINK
{
	struct clblk	clblk;
	struct mblk	mblk;
} M_LINK;

#define CL_BLK_TO_M_LINK(pClBlk) \
	(M_LINK *) ((char *)(pClBlk) - OFFSET (M_LINK, clblk))

#define NETBUF_ALIGN 64

typedef struct clblk *		CL_BLK_ID;	/* Cluster Block ID */
typedef struct netPool 		NET_POOL;	/* Pool ID */
typedef struct poolFunc 	POOL_FUNC;	/* Pool Function Table */
typedef NET_POOL * 		NET_POOL_ID;	/* Pool ID (pointer) */
typedef M_LINK *		M_LINK_ID;	/* mblk Cluster Block set */
typedef struct poolAttr * 	ATTR_ID;	/* Pool Attribute ID */
typedef struct iovec 	 	IOV;		/* IOV structure (See uio.h) */
typedef struct poolRel	 	POOL_REL;	/* netPoolRelease Support */


typedef struct bufVirToPhy
{
	struct mblk *	pMBlk;		/* Associated mblk */
	char *	pVAddr;		/* Virtual Address */
	char *	pPAddr;		/* Physical Address */
	int		length;		/* length of segment */
} BUF_VTOP;

struct	poolFunc			/* POOL_FUNC */
{
	/* pointer to the pool initialization routine */
	int	(*pInitRtn) (NET_POOL_ID pNetPool, M_CL_CONFIG * pMclBlkConfig,
			CL_DESC * pClDescTbl, int clDescTblNumEnt,
			int fromKheap);

	/* pointer to mblk free routine */
	void	(*pMblkFreeRtn) (NET_POOL_ID pNetPool, struct mblk * pMblk);

	/* pointer to cluster Blk free routine */
	void	(*pClBlkFreeRtn) (CL_BLK_ID pClBlk);

	/* pointer to cluster free routine */
	void	(*pClFreeRtn) (NET_POOL_ID pNetPool, char * pClBuf);

	/* pointer to mblk/cluster pair free routine */
	struct mblk * 	(*pMblkClFreeRtn) (NET_POOL_ID pNetPool, struct mblk * pMblk);

	/* pointer to mblk get routine */
	struct mblk *	(*pMblkGetRtn) (NET_POOL_ID pNetPool, int canWait, u8 type);

	/* pointer to cluster Blk get routine */
	CL_BLK_ID	(*pClBlkGetRtn) (NET_POOL_ID pNetPool, int canWait);

	/* pointer to a cluster buffer get routine */
	char *	(*pClGetRtn) (NET_POOL_ID pNetPool, CL_POOL_ID pClPool);

	/* pointer to mblk/cluster pair get routine */
	int	(*pMblkClGetRtn) (NET_POOL_ID pNetPool, struct mblk * pMblk,
			int bufSize, int canWait, int noSmaller);

	/* pointer to cluster pool Id get routine */
	CL_POOL_ID	(*pClPoolIdGetRtn) (NET_POOL_ID pNetPool, int	bufSize,
			int noSmaller);

	/* netBuf 6.0 Pluggable API */

	/* pointer to pool delete routine */
	int (*pNetPoolDeleteRtn)       (NET_POOL_ID pNetPool);

	/* pointer to pool destroy routine */
	int (*pNetPoolDestroyRtn)      (NET_POOL_ID pNetPool);


	/* pointer to pool release routine */
	int (*pNetPoolReleaseRtn)      (NET_POOL_ID pNetPool);


	/* pointer to Tuple get routine */
	struct mblk * (*pNetTupleGetRtn)       (NET_POOL_ID pNetPool,
			int	bufSize,
			int	canWait,
			u8 type,
			int noSmaller);

	/* pointer to Tuple get routine */
	struct mblk * (*pNetTupleFreeRtn)      (struct mblk * pMblk);

	/* pointer to Tuple List Get routine */
	int (*pNetTupleListGetRtn)        (NET_POOL_ID pNetPool,
			CL_POOL_ID pClPool,
			int count,
			int type,
			struct mblk * * pMblk);

	/* pointer to Tuple List Free routine */
	int (*pNetTupleListFreeRtn)    (struct mblk * pMblk);


	/* pointer to Tuple Migrate routine */
	struct mblk * (*pNetTupleMigrateRtn)   (NET_POOL_ID pDstNetPool,
			struct mblk * pMblk);

	/* pointer to memory requirement function */
	int (*pMemReqRtn) (int type, int num, int size);
};

struct	poolRel				/* POOL_REL */
{
	SEM_ID	releaseSemId;               /* Release Semaphore ID */
	int		releaseTaskId;              /* Release task ID */

	/* pointer to Tuple netPoolReleaseStart routine */
	int (*pNetPoolReleaseStartRtn) (NET_POOL_ID pNetPool,
			POOL_FUNC * pReleaseFunc);

	/* pointer to Tuple netPoolReleaseEnd routine */
	int (*pNetPoolReleaseEndRtn)   (NET_POOL_ID pNetPool);

};

/* Attribute Set */

typedef struct poolAttr
{
	u32	attribute;	/* Attribute Bits */
	int		refCount;	/* Registered objects sharing this set */
	PART_ID     ctrlPartId;	/* Memory partition for control structures */
	PART_ID 	bMemPartId;	/* Memory partition for clusters */
	void *	pDomain;	/* NULL = Kernel */
	int 	ctrlAlign;	/* Alignment Control Structures */
	int		clusterAlign;	/* Alignment Clusters (Buffers) */
	struct poolAttr * pNext;	/* Next in Link List */
} POOL_ATTR;


struct netPool				/* NET_POOL */
{
	struct mblk *	pmBlkHead;		    /* head of mBlks */
	CL_BLK_ID	pClBlkHead;	    	    /* head of cluster Blocks */
	int		mBlkCnt;        	    /* number of mblks */
	int		mBlkFree;	            /* number of free mblks -
						       deprecated, use
						       pPoolStat->mTypes[MT_FREE]
						       instead */
	int		clBlkOutstanding;	    /* Number of clBlks in use */
	int		clMask;			    /* cluster availability mask */
	int		clLg2Max;		    /* cluster log2 maximum size */
	int		clSizeMax;		    /* maximum cluster size */
	int		clLg2Min;		    /* cluster log2 minimum size */
	int		clSizeMin;		    /* minimum cluster size */
	CL_POOL * 	clTbl [CL_TBL_SIZE];	    /* pool table */
	M_STAT *	pPoolStat;		    /* pool statistics */
	POOL_FUNC *	pFuncTbl;		    /* ptr to function ptr table */
	int 	flag;			    /* Pool Flag */
	ATTR_ID  	pAttrSet;		    /* Attribute Reference */
	PART_ID     poolPartId;		    /* Pool Struct Memory Partition */
	int		attachRefCount;		    /* Attach Ref Count */
	int		bindRefCount;		    /* Attach Ref Count */
	char *	pControlMemArea;	    /* Memory Area for mClBlk */
	PART_ID	controlMemPartId;	    /* memory partition for mClBlk */
	char *	pClusterMemArea;	    /* Memory Area for clusters */
	PART_ID	clusterMemPartId;	    /* memory partition for clusters */
	char        poolName[NET_POOL_NAME_SZ]; /* Pool name */
	POOL_REL    poolRelease;                /* netPoolRelease Support */
	struct netPool * pParent;		    /* Parent netPool */
	struct netPool * pNext;		    /* next in link list */
};

/* Cluster description structure for a buffer pool see netPoolCreate () */

typedef struct netBufClDesc
{
	int clSize;				/* Cluster Size */
	int clNum;				/* Number of clusters in pool */
} NETBUF_CL_DESC;

/* Pool configuration structure see netPoolCreate () */

typedef struct netBufCfg
{
	char *          pName;	     /* Pool Name */
	u32          attributes;	     /* pool attributes */
	void *          pDomain;         /* RTP ID or NULL for kernel */
	int             ctrlNumber;	     /* # of ctrl structures to pre-allocate */
	PART_ID         ctrlPartId;	     /* Mem Partition for Control structures */
	/*     NULL = use Kernel partition */
	int             bMemExtraSize;   /* Additional memory for runtime buffers */
	PART_ID         bMemPartId;	     /* Mem Partition for buffers */
	/*     NULL = default for kernel or RTP */
	NETBUF_CL_DESC * pClDescTbl;     /* desired cluster sizes and count */
	int             clDescTblNumEnt; /* num of entries in cluster table */
} NETBUF_CFG;

/*
 * NET_POOL_CFG:
 * Optimized allocation and freeing for netPool and function table
 */

typedef struct netPoolCfg
{
	NET_POOL	netPool;	/* netPool - Must be 1st Entry */
	POOL_FUNC 	funcTbl;	/* Function ptr table */
} NET_POOL_CFG, *pNET_POOL_CFG;

/* CLTAG:
 *    Cluster Prefix and appended tags
 *    NB_BUF_SANITY - Optional buffer corruption check
 *    NB_BUF_USRTAG - Used by drivers to associate a cluster
 *                    address with an Tuple.
 *
 */

typedef struct clPreTag
{
#ifdef NB_BUF_SANITY
	uint32_t 	tag;		/* Used to validate cluster */
#endif		/* NB_BUF_SANITY */
#ifdef NB_BUF_USRTAG
	uint32_t 	usrTag;		/* User tag */
#endif		/* NB_BUF_USRTAG */
#ifdef NB_FULL_BKWARD_MODE
	NET_POOL_ID	poolId;		/* Pool ID. Req for 5.5 compatibility */
#endif		/* NB_BUF_USRTAG */
} CL_PRE_TAG;

/* external declarations */

//_WRS_INITTEXT
int 		netBufLibInit (void);
//_WRS_INITTEXT
int 		netBufPoolInit (void);
//_WRS_INITTEXT
int 		linkBufPoolInit (void);
int 		netPoolInit (NET_POOL_ID pNetPool,
		M_CL_CONFIG * pMclBlkConfig,
		CL_DESC * pClDescTbl, int clDescTblNumEnt,
		POOL_FUNC * pFuncTbl);
int		netPoolDelete (NET_POOL_ID);
void		netMblkFree (NET_POOL_ID pNetPool, struct mblk * pMblk);
void		netClBlkFree (NET_POOL_ID pNetPool, CL_BLK_ID pClBlk);
void 		netClFree (NET_POOL_ID pNetPool, u8 * pClBuf);
//_WRS_FASTTEXT
struct mblk * 	netMblkClFree (struct mblk * pMblk);
//_WRS_FASTTEXT
void		netMblkClChainFree (struct mblk * pMblk);
struct mblk * 	netMblkGet (NET_POOL_ID pNetPool, int canWait,
		u8 type);
CL_BLK_ID	netClBlkGet (NET_POOL_ID pNetPool, int canWait);
char *	 	netClusterGet (NET_POOL_ID pNetPool,
		CL_POOL_ID pClPool);
int 	 	netMblkClGet (NET_POOL_ID pNetPool, struct mblk * pMblk,
		int bufSize, int canWait,
		int noSmaller);
//_WRS_FASTTEXT
struct mblk *		netTupleGet (NET_POOL_ID pNetPool, int bufSize,
		int canWait, u8 type, int noSmaller);

CL_BLK_ID  	netClBlkJoin (CL_BLK_ID pClBlk, char * pClBuf,
		int size, int (*pFreeRtn)(void), int arg1,
		int arg2, int arg3);
struct mblk *  	netMblkClJoin (struct mblk * pMblk, CL_BLK_ID pClBlk);
CL_POOL_ID 	netClPoolIdGet (NET_POOL_ID pNetPool, int bufSize,
		int noSmaller);

int 		netMblkToBufCopy (struct mblk * pMblk, char * pBuf);
int		netMblkOffsetToBufCopy (struct mblk * pMblk, int offset,
		char * pBuf, int len);
struct mblk * 	netMblkDup (struct mblk * pSrcMblk, struct mblk *	pDestMblk);
struct mblk * 	netMblkChainDup (NET_POOL_ID pNetPool, struct mblk *	pMblk,
		int offset, int len, int canWait);
u32           netMblkFromBufCopy(struct mblk * pMblk, char* pBuf,
		u32 offset, u32 len,
		int canWait);
void            netPoolShow (NET_POOL_ID pNetPool);
void 	        mbufShow (void);
void             netStackSysPoolShow (void);
void             netStackDataPoolShow (void);

/* NetBuf enhanced functions */
NET_POOL_ID netPoolCreate          (NETBUF_CFG * pnetBufCfg,
		POOL_FUNC * pFuncTbl);
int	   netBufAdvLibInit	  (void);
NET_POOL_ID netPoolAttach	  (const char * pName);
int      netPoolDetach	  (NET_POOL_ID pNetPool);
int      netPoolBind            (NET_POOL_ID pNetPoolChild,
		const char *pParentName);
int      netPoolUnbind          (NET_POOL_ID pNetPool);
int      netPoolRelease	  (NET_POOL_ID pNetPool,
		int destroyContext);
struct mblk *    netTupleFree           (struct mblk * pMblk);
int         netTupleListGet	  (NET_POOL_ID pNetPool,
		CL_POOL_ID pClPool,
		int count,
		int type,
		struct mblk * * pMblk);
int      netTupleListFree	  (struct mblk *);
struct mblk *    netTupleMigrate        (NET_POOL_ID pDstNetPool,
		struct mblk * pMblk);
int      netTuplePhysAddrMap	  (struct mblk * pMblk,
		BUF_VTOP * pVtopTbl,
		int * pElementCount);
NET_POOL_ID netPoolIdGet		  (const char * pPoolName);
char *      netPoolNameGet	  (NET_POOL_ID pNetPool);
int	   _netMemReqDefault	  (int type, int num, int size);

extern POOL_FUNC * _pNetPoolFuncTbl;
extern POOL_FUNC * _pLinkPoolFuncTbl;

extern void (*_func_netPoolShow) (NET_POOL_ID pNetPool);

/*
 * The following routines and variables are considered private and should
 * not be used outside of the netBufLib/netBufAdvLib implementation.
 */
extern int      netPoolCreateValidate (NET_POOL_ID pNetPool);
extern POOL_ATTR * (*_func_netAttrReg) (u32 attribute, PART_ID ctrlPartId,
		PART_ID bMemPartId, void * pDomain);
extern int      (*_func_netAttrDeReg) (POOL_ATTR * pBufAttr);
extern SEM_ID      netBufLock;
extern NET_POOL_ID netPoolIdGetWork (const char * pPoolName);

/* End private routines/variables */

//#if CPU_FAMILY==I960
//#pragma align 0			/* turn off alignment requirement */
//#endif	/* CPU_FAMILY==I960 */

#endif /* __INCnetBufLibh */

