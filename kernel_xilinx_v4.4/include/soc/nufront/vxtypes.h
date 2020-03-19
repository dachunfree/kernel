/* vxTypesOld.h - old VxWorks type definition header */

/*
 * Copyright (c) 1990-1997, 2000-2001, 2003-2005 Wind River Systems, Inc.
 *
 * The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.
 */

/*
   modification history
   --------------------
   2010/07/03 leij: adding some typedefs  macros and compiler keywords need for vxworks sources
   */

/*
   DESCRIPTION
   This header file contains a mixture of stuff.
   1) the old style typedefs (ie. POSIX now says they must end with _t).
   These will be phased out gradually.
   2) a mechanism for getting rid of const warning which are produced by the
   GNU C compiler. Hopefully, this will be removed in the future.
   3) macros that are so longer needed for vxWorks source code but maybe needed
   by some customer applications and are therefore provided for backward
   compatability.
   4) system III typedefs (used by netinet) which do not fit in anywhere else.

*/

#ifndef __INCvxTypesH
#define __INCvxTypesH

    /* vxWorks types */

    /* int types */
#ifndef INT8
    typedef	signed char	        INT8;
#endif

#ifndef INT16
    typedef	signed short	    INT16;
#endif

#ifndef INT32
    typedef	signed int	        INT32;
#endif

#ifndef INT64
    typedef	signed long long    INT64;
#endif

#ifndef UINT8
    typedef	unsigned char	    UINT8;
#endif

#ifndef UINT16
    typedef	unsigned short	    UINT16;
#endif

#ifndef UINT32
    typedef	unsigned int	    UINT32;
#endif

#ifndef UINT64
    typedef	unsigned long long  UINT64;
#endif

#ifndef UCHAR
    typedef	unsigned char	    UCHAR;
#endif

#ifndef USHORT
    typedef unsigned short	    USHORT;
#endif

#ifndef UINT
    typedef	unsigned int	    UINT;
#endif

#ifndef ULONG
    typedef unsigned long	    ULONG;
#endif

#ifndef PUCHAR
    typedef unsigned char *PUCHAR;
#endif
#ifndef PUCHAR
    typedef  char *PCHAR;
#endif

    typedef     unsigned long long  u64_t;
    typedef     unsigned int        u32_t;
    typedef     unsigned short      u16_t;
    typedef     unsigned char       u8_t;
    typedef     long long           s64_t;
    typedef     long                s32_t;
    typedef     short               s16_t;
    typedef     char                s8_t;

    typedef u8_t A_BOOL;

    typedef u8_t A_UINT8;
    typedef s8_t A_INT8;
    typedef u16_t A_UINT16;
    typedef u32_t A_UINT32;
    typedef short A_INT16;

    /*#ifndef uint8_t
      typedef	unsigned char	    uint8_t;
#endif

#ifndef uint16_t
typedef	unsigned short	    uint16_t;
#endif

#ifndef uint32_t
typedef	unsigned int	    uint32_t;
#endif

#ifndef uint64_t
typedef	unsigned long long  uint64_t;
#endif
*/

#ifndef BOOL
    typedef	int		BOOL;
#endif


    /* STATUS definition usually indicate the return status*/
#ifndef STATUS
    typedef	int		STATUS;
#endif


    /*
     * c syntax does not allow typedef void VOID ,add #ifdef
     * to void redefinition when the user include windows.h
     */

#ifndef VOID
#define VOID    void
#endif

    /* old Berkeley definitions */
#ifndef uchar_t
    typedef unsigned char	uchar_t;
#endif

#ifndef ushort_t
    typedef unsigned short	ushort_t;
#endif

#ifndef uint_t
    typedef unsigned int	uint_t;
#endif

#ifndef ulong_t
    typedef unsigned long	ulong_t;
#endif


#ifdef WIN32
    //unix/linux do not need these definition
    typedef	struct	_quad { long val[2]; } quad;
    typedef	long	daddr_t;
    typedef	char *	caddr_t;
    typedef	char *	addr_t;
    typedef	long	swblk_t;
#endif

#ifndef _FUNCPTR_DEFINED
#define _FUNCPTR_DEFINED
#ifdef __cplusplus
    typedef int (*FUNCPTR) (...);  /* ptr to function returning int */
#else
    typedef int (*FUNCPTR) (void); /* ptr to function returning int */
#endif /* __cplusplus */
#endif /* _FUNCPTR_DEFINED */

#ifndef _VOIDFUNCPTR_DEFINED
#define _VOIDFUNCPTR_DEFINED
#ifdef __cplusplus
    typedef void (*VOIDFUNCPTR) (...);  /* ptr to function returning void */
#else
    typedef void (*VOIDFUNCPTR) (void); /* ptr to function returning void */
#endif /* __cplusplus */
#endif /* _VOIDFUNCPTR_DEFINED */

#ifndef _DBLFUNCPTR_DEFINED
#define _DBLFUNCPTR_DEFINED
#ifdef __cplusplus
    typedef double (*DBLFUNCPTR) (...);  /* ptr to function returning double*/
#else
    typedef double (*DBLFUNCPTR) (void);  /* ptr to function returning double*/
#endif /* __cplusplus */
#endif /* _DBLFUNCPTR_DEFINED */

#ifndef _FLTFUNCPTR_DEFINED
#define _FLTFUNCPTR_DEFINED
#ifdef __cplusplus
    typedef float (*FLTFUNCPTR) (...);  /* ptr to function returning float */
#else
    typedef float (*FLTFUNCPTR) (void);  /* ptr to function returning float */
#endif /* __cplusplus */
#endif /* _FLTFUNCPTR_DEFINED */


    /* system III  typedefs (used by netinet) */
    /*
#ifndef u_char
typedef	unsigned char	u_char;
#endif

#ifndef u_short
typedef	unsigned short	u_short;
#endif

#ifndef u_int
typedef	unsigned int	u_int;
#endif

#ifndef u_long
typedef	unsigned long	u_long;
#endif
*/

#ifndef TBOOL
    typedef char		    TBOOL;		/* obsolete */
#endif

#endif /* __INCvxTypesh */
