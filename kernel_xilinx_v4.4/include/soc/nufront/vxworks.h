/* vxWorksCommon.h - VxWorks standard definitions header */

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

#ifndef __INCvxWorksCommonh
#define __INCvxWorksCommonh

#include <stddef.h>

#if	!defined(EOF) || (EOF!=(-1))
#define EOF		(-1)
#endif

#if	!defined(FALSE) || (FALSE!=(0))
#define FALSE		(0)
#endif

#if	!defined(TRUE) || (TRUE!=(1))
#define TRUE		(1)
#endif


#define NONE		(-1)	/* for times when NULL won't do */
#define EOS		'\0'	/* C string terminator */


/* return status values */
#if !defined(VXOK) || (VXOK!=(0))
#define VXOK		0
#endif

#if !defined(VXERROR) || (VXERROR!=(-1))
#define VXERROR		(-1)
#endif

/* timeout defines */

#define NO_WAIT		0
#define WAIT_FOREVER	(-1)

/* low-level I/O input, output, error fd's */

#define	STD_IN	0
#define	STD_OUT	1
#define	STD_ERR	2

/* modes - must match O_RDONLY/O_WRONLY/O_RDWR in ioLib.h! */

#define VX_READ		0
#define VX_WRITE	1

/* SYSTEM types */

#define V7		1	/* ATT version 7 */
#define SYS_V		2	/* ATT System 5 */
#define BSD_4_2		3	/* Berkeley BSD 4.2 */

/* network configuration parameters */

#define	INET		/* include internet protocols */
#undef  BSD             /* remove any previous definition */
#define	BSD	44	/* BSD 4.4 like OS */
#define	BSDDEBUG	/* turn on debug */
#define	GATEWAY		/* tables to be initialized for gateway routing */

/* common macros */

#define MSB(x)	(((x) >> 8) & 0xff)	  /* most signif byte of 2-byte integer */
#define LSB(x)	((x) & 0xff)		  /* least signif byte of 2-byte integer*/
#define MSW(x) (((x) >> 16) & 0xffff) /* most signif word of 2-word integer */
#define LSW(x) ((x) & 0xffff) 		  /* least signif byte of 2-word integer*/

/* swap the MSW with the LSW of a 32 bit integer */
#define WORDSWAP(x) (MSW(x) | (LSW(x) << 16))

#define LLSB(x)	((x) & 0xff)		/* 32bit word byte/word swap macros */
#define LNLSB(x) (((x) >> 8) & 0xff)
#define LNMSB(x) (((x) >> 16) & 0xff)
#define LMSB(x)	 (((x) >> 24) & 0xff)
#define LONGSWAP(x) ((LLSB(x) << 24) | \
		     (LNLSB(x) << 16)| \
		     (LNMSB(x) << 8) | \
		     (LMSB(x)))

#define OFFSET(structure, member)	/* byte offset of member in structure*/\
		((int) &(((structure *) 0) -> member))

#define MEMBER_SIZE(structure, member)	/* size of a member of a structure */\
		(sizeof (((structure *) 0) -> member))

#define NELEMENTS(array)		/* number of elements in an array */ \
		(sizeof (array) / sizeof ((array) [0]))

#define FOREVER	for (;;)

#if 0
/*
 * min & max are C++ standard functions which are provided by the user
 * side C++ libraries
 */
#if !defined(__cplusplus) && !defined(max) && !defined(min)
#define max(x, y)	(((x) < (y)) ? (y) : (x))
#define min(x, y)	(((x) < (y)) ? (x) : (y))
#endif /* _WRS_KERNEL || !__cplusplus */

#endif

/* storage class specifier definitions */

#define FAST	register
#define IMPORT	extern
#define LOCAL	static

#define ROUND_UP(x, align)	(((int) (x) + (align - 1)) & ~(align - 1))
#define ROUND_DOWN(x, align)	((int)(x) & ~(align - 1))
#define ALIGNED(x, align)	(((int)(x) & (align - 1)) == 0)

/* In the kernel, the alignment sizes are a property of the specific
 * architecture varient we build for. In user mode, the alignment sizes
 * are passed from the kernel to the RTP via the aux_v mechanism, so we
 * need to override the definitions here. This needs to be done now,
 * rather than in types/vxArch.h because we're referencing variables of
 * type size_t, which aren't defined until vxTypes.h
 */

#ifndef _WRS_KERNEL
#ifndef _ASMLANGUAGE

/* XXX Note - there is probably an issue with Assembly builds with this
 * model. This needs to be fixed.
 */

extern size_t allocAlignSize;
extern size_t stackAlignSize;
extern size_t cacheAlignSize;
extern size_t vmPageSize;

#ifdef _ALLOC_ALIGN_SIZE
#undef _ALLOC_ALIGN_SIZE
#endif /* _ALLOC_ALIGN_SIZE */
#define _ALLOC_ALIGN_SIZE       allocAlignSize

#ifdef _STACK_ALIGN_SIZE
#undef _STACK_ALIGN_SIZE
#endif /* _STACK_ALIGN_SIZE */
#define _STACK_ALIGN_SIZE       stackAlignSize

#ifdef _CACHE_ALIGN_SIZE
#undef _CACHE_ALIGN_SIZE
#endif /* _CACHE_ALIGN_SIZE */
#define _CACHE_ALIGN_SIZE       cacheAlignSize

#define VM_PAGE_SIZE_GET()      vmPageSize

#endif /* _ASMLANGUAGE */
#endif /* _WRS_KERNEL */

#if defined(_ASMLANGUAGE) && !defined(_WRS_KERNEL)

/* XXX only exclude these macros if user mode & assembly build */

#else

#define MEM_ROUND_UP(x)		ROUND_UP(x, _ALLOC_ALIGN_SIZE)
#define MEM_ROUND_DOWN(x)	ROUND_DOWN(x, _ALLOC_ALIGN_SIZE)
#define STACK_ROUND_UP(x)	ROUND_UP(x, _STACK_ALIGN_SIZE)
#define STACK_ROUND_DOWN(x)	ROUND_DOWN(x, _STACK_ALIGN_SIZE)
#define MEM_ALIGNED(x)		ALIGNED(x, _ALLOC_ALIGN_SIZE)

#endif /* defined(_ASMLANGUAGE) && !defined(_WRS_KERNEL) */

/*
 * In case TOOL_FAMILY isn't defined, we shal assume gnu for
 * backward compatibility.  All this is so we can include
 * the toolMacros.h file from the h/tool/$(TOOL_FAMILY) directory
 * path.  This is needed because for host tool building we can't
 * always specify additional include paths for compiler tool
 * builds.
 */

#ifndef TOOL_FAMILY
#   define TOOL_FAMILY gnu
#endif

/* All this to generate a string we can #include */

#define TOOL_HDR_STRINGIFY(x)  #x
#define TOOL_HDR(tc, file) TOOL_HDR_STRINGIFY(tool/tc/file)
#define TOOL_HEADER(file) TOOL_HDR(TOOL_FAMILY,file)

/* Now include the correct header file */
#if 0
#include TOOL_HEADER(toolMacros.h)

#define _WRS_VXWORKS_5_X

#ifdef _WRS_KERNEL
#include <version.h>	/* kernel version numbers */
#endif

/*
 * Before we insist on defining __P here make sure there
 * aren't any previous occurrences! __P may clash with
 * the compiler's definition of it.
 */

#ifndef __P
#define	__P(protos)	protos		/* Use ANSI C proto */
#endif
#endif
#endif /* __INCvxWorksCommonh */
