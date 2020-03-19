#ifndef __INCrngLibh
#define __INCrngLibh

#include <soc/nufront/vxtypes.h>

#ifndef min
#define min(a,b) (((a)<(b)) ? (a):(b))
#endif

/* HIDDEN */

/* typedefs */

/* RING - ring buffer */
typedef struct ring
{
	/* offset from start of buffer where to write next */
	int pToBuf;
	/* offset from start of buffer where to read next */
	int pFromBuf;
	/* size of ring in bytes */
	int bufSize;
	/* pointer to start of buffer */
	char *buf;
} RING;

/* END_HIDDEN */

typedef RING *RING_ID;


/*
 * The following macros are designed to do various operations on
 * the RING object.  By using them, users can avoid having to know
 * the structure of RING.  However they are intended to be very
 * efficient and this is why they are macros in the first place.
 * In general the parameters to them should be register variables
 * for maximum efficiency.
 */

/*******************************************************************************
*
* RNG_ELEM_GET - get one character from a ring buffer
*
* This macro gets a single character from the specified ring buffer.
* Must supply temporary variable (register int) 'fromP'.
*
* RETURNS: 1 if there was a char in the buffer to return, 0 otherwise
*
* NOMANUAL
*/

#define RNG_ELEM_GET(ringId, pCh, fromP)		\
    (						\
    fromP = (ringId)->pFromBuf,			\
    ((ringId)->pToBuf == fromP) ?		\
	0 					\
    :						\
	(					\
	*pCh = (ringId)->buf[fromP],		\
	(ringId)->pFromBuf = ((++fromP == (ringId)->bufSize) ? 0 : fromP), \
	1					\
	)					\
    )

/*******************************************************************************
*
* RNG_ELEM_PUT - put one character into a ring buffer
*
* This macro puts a single character into the specified ring buffer.
* Must supply temporary variable (register int) 'toP'.
*
* RETURNS: 1 if there was room in the buffer for the char, 0 otherwise
*
* NOMANUAL
*/

#define RNG_ELEM_PUT(ringId, ch, toP)		\
    (						\
    toP = (ringId)->pToBuf,			\
    (toP == (ringId)->pFromBuf - 1) ?		\
	0 					\
    :						\
	(					\
    	(toP == (ringId)->bufSize - 1) ?	\
	    (					\
	    ((ringId)->pFromBuf == 0) ?		\
		0				\
	    :					\
		(				\
		(ringId)->buf[toP] = ch,	\
		(ringId)->pToBuf = 0,		\
		1				\
		)				\
	    )					\
	:					\
	    (					\
	    (ringId)->buf[toP] = ch,		\
	    (ringId)->pToBuf++,			\
	    1					\
	    )					\
	)					\
    )

/* function declarations */

#define HO_SUPPORT
#if defined(__STDC__) || defined(__cplusplus)

extern BOOL 	rngIsEmpty (RING_ID ringId);
extern BOOL 	rngIsFull (RING_ID ringId);
extern RING_ID 	rngCreate (int nbytes);
extern int 	rngBufGet (RING_ID rngId, char *buffer, int maxbytes);
extern int 	rngBufPeek (RING_ID rngId, char *buffer,int skipbytes, int maxbytes);
extern int 	rngBufPut (RING_ID rngId, char *buffer, int nbytes);
extern int 	rngFreeBytes (RING_ID ringId);
extern int 	rngNBytes (RING_ID ringId);
extern void 	rngDelete (RING_ID ringId);
extern void 	rngFlush (RING_ID ringId);
extern void 	rngMoveAhead (RING_ID ringId, int n);
extern void 	rngPutAhead (RING_ID ringId, char byte, int offset);

#else	/* __STDC__ */

extern BOOL 	rngIsEmpty ();
extern BOOL 	rngIsFull ();
extern RING_ID 	rngCreate ();
extern int 	rngBufGet ();
extern int 	rngBufPeek ();
extern int 	rngBufPut ();
extern int 	rngFreeBytes ();
extern int 	rngNBytes ();
extern void 	rngDelete ();
extern void 	rngFlush ();
extern void 	rngMoveAhead ();
extern void 	rngPutAhead ();

#endif	/* __STDC__ */

#endif /* __INCrngLibh */
