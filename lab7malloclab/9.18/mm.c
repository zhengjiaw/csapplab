/*
 * Simple, 32-bit and 64-bit clean allocator based on implicit free
 * lists, first-fit placement, and boundary tag coalescing, as described
 * in the CS:APP3e text. Blocks must be aligned to doubleword (8 byte)
 * boundaries. Minimum block size is 16 bytes.
 */
#include "mm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memlib.h"
team_t team = {
    /* Team name */
    "enjoy",
    /* First member's full name */
    "linxi",
    /* First member's email address */
    "linxi177229@gmail.com",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""};
/*
 * If NEXT_FIT defined use next fit search, else use first-fit search
 */
/*
 * mm_init - Initialize the memory manager
 */

/*
 * If NEXT_FIT defined use next fit search, else use first-fit search
 */
#define NEXT_FIT  // add 9.17

/* $begin mallocmacros */
/* Basic constants and macros */
#define WSIZE 4 /* Word and header/footer size (bytes) */             // line:vm:mm:beginconst
#define DSIZE 8                                                       /* Double word size (bytes) */
#define CHUNKSIZE (1 << 12) /* Extend heap by this amount (bytes) */  // line:vm:mm:endconst

#define MAX(x, y) ((x) > (y) ? (x) : (y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))  // line:vm:mm:pack

/* Read and write a word at address p */
#define GET(p) (*(unsigned int *)(p))               // line:vm:mm:get
#define PUT(p, val) (*(unsigned int *)(p) = (val))  // line:vm:mm:put

/* Read the size and allocated fields from address p */
#define GET_SIZE(p) (GET(p) & ~0x7)  // line:vm:mm:getsize
#define GET_ALLOC(p) (GET(p) & 0x1)  // line:vm:mm:getalloc

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp) ((char *)(bp)-WSIZE)                         // line:vm:mm:hdrp
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)  // line:vm:mm:ftrp

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp)-WSIZE)))  // line:vm:mm:nextblkp
#define PREV_BLKP(bp) ((char *)(bp)-GET_SIZE(((char *)(bp)-DSIZE)))    // line:vm:mm:prevblkp
/* $end mallocmacros */

#define GET_PREV_ALLOC(p) ((GET(p)) & 0x2)  // add 9.18
/* Global variables */
static char *heap_listp = 0; /* Pointer to first block */
#ifdef NEXT_FIT
static char *rover; /* Next fit rover */
#endif

/* Function prototypes for internal helper routines */
static void *extend_heap(size_t words);
static void place(void *bp, size_t asize);
static void *find_fit(size_t asize);
static void *coalesce(void *bp);
static void printblock(void *bp);
static void checkheap(int verbose);
static void checkblock(void *bp);

/*
 * mm_init - Initialize the memory manager
 */
/* $begin mminit */
int mm_init(void)
{
    /* Create the initial empty heap */
    // 即便我们现在是DSIZE对齐，我们也不能对此函数做修改，因为coalesce的方式需要Prologue block 和
    // Epilogue header
    // 不得不承认，这是一种很巧妙的方法！每次extend_heap会正好把结束块变成下一个空闲块的header
    if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void *)-1)  // line:vm:mm:begininit
        return -1;
    PUT(heap_listp, 0);                              /* Alignment padding 填充块 */
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 0x3)); /* Prologue header 序言块*/
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 0x3)); /* Prologue footer */
    PUT(heap_listp + (3 * WSIZE), PACK(0, 0x3));     /* Epilogue header 结束块*/
    heap_listp += (4 * WSIZE);                       //一个小小的优化，让heap_listp指向下一个块line:vm:mm:endinit
    /* $end mminit */

#ifdef NEXT_FIT
    rover = heap_listp;
#endif
    /* $begin mminit */

    /* Extend the empty heap with a free block of CHUNKSIZE bytes */
    if (extend_heap(CHUNKSIZE / WSIZE) == NULL) return -1;
    return 0;
}
/* $end mminit */

/*
 * mm_malloc - Allocate a block with at least size bytes of payload
 */
/* $begin mmmalloc */
void *mm_malloc(size_t size)
{
    size_t asize;      /* Adjusted block size */
    size_t extendsize; /* Amount to extend heap if no fit */
    char *bp;

    /* $end mmmalloc */
    if (heap_listp == 0) {
        mm_init();
    }
    /* $begin mmmalloc */
    /* Ignore spurious requests */
    if (size == 0) return NULL;

    /* Adjust block size to include overhead and alignment reqs. */

    size_t wSize = (size + (WSIZE - 1)) / WSIZE;  //取四字节的上整 add 9.18
    size_t dSize = wSize / 2 + 1;  // 求出DSIZE(8) 的个数, 加1是因为第一次进位是4
    asize = dSize * DSIZE;         // add 9.18

    if ((bp = find_fit(asize)) != NULL) {  // line:vm:mm:findfitcall
        place(bp, asize);                  // line:vm:mm:findfitplace
        return bp;
    }

    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize, CHUNKSIZE);                               // line:vm:mm:growheap1
    if ((bp = extend_heap(extendsize / WSIZE)) == NULL) return NULL;  // line:vm:mm:growheap2
    place(bp, asize);                                                 // line:vm:mm:growheap3
    return bp;
}
/* $end mmmalloc */

/*
 * mm_free - Free a block
 */
/* $begin mmfree */
void mm_free(void *bp)
{
    /* $end mmfree */
    if (bp == 0) return;

    /* $begin mmfree */
    size_t size = GET_SIZE(HDRP(bp));
    /* $end mmfree */
    if (heap_listp == 0) {
        mm_init();
    }
    /* $begin mmfree */
    // add 9.18 这里free的时候，我们要保留之前的 head 的ALLOC
    PUT(HDRP(bp), PACK(size, GET_PREV_ALLOC(HDRP(bp))));
    PUT(FTRP(bp), PACK(size, 0));
    *HDRP(NEXT_BLKP(bp)) &= ~0x2;
    coalesce(bp);
}

/* $end mmfree */
/*
 * coalesce - Boundary tag coalescing. Return ptr to coalesced block
 */
/* $begin mmfree */
static void *coalesce(void *bp)
{
    size_t prev_alloc = GET_PREV_ALLOC(HDRP(bp));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc) { /* Case 1 */
        return bp;
    }

    else if (prev_alloc && !next_alloc) { /* Case 2 prev分配了，所以ALLOC是1,即pack 0x2*/
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0x2));  // add 9.18
        PUT(FTRP(bp), PACK(size, 0));
    }

    else if (!prev_alloc && next_alloc) { /* Case 3 prev未分配， 所以ALLOC随 prev */
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        // add 9.18 我们只用处理head即可
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, GET_PREV_ALLOC(HDRP(PREV_BLKP(bp)))));
        bp = PREV_BLKP(bp);
    }

    else { /* Case 4 prev未分配， 所以ALLOC随 prev */
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
        // add 9.18 我们只用处理head即可
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, GET_PREV_ALLOC(HDRP(PREV_BLKP(bp)))));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    /* $end mmfree */
#ifdef NEXT_FIT
    /* Make sure the rover isn't pointing into the free block */
    /* that we just coalesced */
    if ((rover > (char *)bp) && (rover < NEXT_BLKP(bp))) rover = bp;
#endif
    /* $begin mmfree */
    return bp;
}
/* $end mmfree */

/*
 * mm_realloc - Naive implementation of realloc
 */
void *mm_realloc(void *ptr, size_t size)
{
    size_t oldsize;
    void *newptr;

    /* If size == 0 then this is just free, and we return NULL. */
    if (size == 0) {
        mm_free(ptr);
        return 0;
    }

    /* If oldptr is NULL, then this is just malloc. */
    if (ptr == NULL) {
        return mm_malloc(size);
    }

    newptr = mm_malloc(size);

    /* If realloc() fails the original block is left untouched  */
    if (!newptr) {
        return 0;
    }

    /* Copy the old data. */
    oldsize = GET_SIZE(HDRP(ptr));
    if (size < oldsize) oldsize = size;
    memcpy(newptr, ptr, oldsize);

    /* Free the old block. */
    mm_free(ptr);

    return newptr;
}

/*
 * mm_checkheap - Check the heap for correctness
 */
void mm_checkheap(int verbose) { checkheap(verbose); }

/*
 * The remaining routines are internal helper routines
 */

/*
 * extend_heap - Extend heap with free block and return its block pointer
 */
/* $begin mmextendheap */
static void *extend_heap(size_t words)
{
    char *bp;
    size_t size;

    /* Allocate an even number of words to maintain alignment */
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;  // line:vm:mm:beginextend
    if ((long)(bp = mem_sbrk(size)) == -1) return NULL;        // line:vm:mm:endextend
    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, GET_PREV_ALLOC(HDRP(bp))));               // add 9.18
    /* Free block header */                                            // line:vm:mm:freeblockhdr
    PUT(FTRP(bp), PACK(size, 0)); /* Free block footer */              // line:vm:mm:freeblockftr
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 0x1)); /* New epilogue header */  // line:vm:mm:newepihdr

    /* Coalesce if the previous block was free */
    return coalesce(bp);  // line:vm:mm:returnblock
}
/* $end mmextendheap */

/*
 * place - Place block of asize bytes at start of free block bp
 *         and split if remainder would be at least minimum block size
 */
/* $begin mmplace */
/* $begin mmplace-proto */
static void place(void *bp, size_t asize)
/* $end mmplace-proto */
{
    size_t csize = GET_SIZE(HDRP(bp));

    if ((csize - asize) >= (DSIZE)) {  // 我们的最小空闲块可以是 DSIZE
        PUT(HDRP(bp), PACK(asize, 1 | GET_PREV_ALLOC(HDRP(bp))));  // 9.18 分配位不变
        PUT(FTRP(bp), PACK(asize, 1));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(csize - asize, 0x2));  // add 9.18 前一块已经分配了
        PUT(FTRP(bp), PACK(csize - asize, 0));
    } else {
        // add 9.18 这里得记得加上前一块的head ALLOC位
        PUT(HDRP(bp), PACK(csize, 1 | GET_PREV_ALLOC(HDRP(bp))));
        PUT(FTRP(bp), PACK(csize, 1));
        *HDRP(NEXT_BLKP(bp)) |= 0x2;  // add 9.18 这里后一块要标记为已分配
    }
}
/* $end mmplace */

/*
 * find_fit - Find a fit for a block with asize bytes
 */
/* $begin mmfirstfit */
/* $begin mmfirstfit-proto */
static void *find_fit(size_t asize)
/* $end mmfirstfit-proto */
{
    /* $end mmfirstfit */

#ifdef NEXT_FIT
    /* Next fit search */
    char *oldrover = rover;

    /* Search from the rover to the end of list */
    for (; GET_SIZE(HDRP(rover)) > 0; rover = NEXT_BLKP(rover))
        if (!GET_ALLOC(HDRP(rover)) && (asize <= GET_SIZE(HDRP(rover)))) return rover;

    /* search from start of list to old rover */
    for (rover = heap_listp; rover < oldrover; rover = NEXT_BLKP(rover))
        if (!GET_ALLOC(HDRP(rover)) && (asize <= GET_SIZE(HDRP(rover)))) return rover;

    return NULL; /* no fit found */
#else
    /* $begin mmfirstfit */
    /* First-fit search */
    void *bp;

    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))) {
            return bp;
        }
    }
    return NULL; /* No fit */
#endif
}
/* $end mmfirstfit */

static void printblock(void *bp)
{
    size_t hsize, halloc, fsize, falloc;

    checkheap(0);
    hsize = GET_SIZE(HDRP(bp));
    halloc = GET_ALLOC(HDRP(bp));
    fsize = GET_SIZE(FTRP(bp));
    falloc = GET_ALLOC(FTRP(bp));

    if (hsize == 0) {
        printf("%p: EOL\n", bp);
        return;
    }

    printf("%p: header: [%ld:%c] footer: [%ld:%c]\n", bp, hsize, (halloc ? 'a' : 'f'), fsize,
           (falloc ? 'a' : 'f'));
}

static void checkblock(void *bp)
{
    if ((size_t)bp % 8) printf("Error: %p is not doubleword aligned\n", bp);
    if (GET(HDRP(bp)) != GET(FTRP(bp))) printf("Error: header does not match footer\n");
}

/*
 * checkheap - Minimal check of the heap for consistency
 */
void checkheap(int verbose)
{
    char *bp = heap_listp;

    if (verbose) printf("Heap (%p):\n", heap_listp);

    if ((GET_SIZE(HDRP(heap_listp)) != DSIZE) || !GET_ALLOC(HDRP(heap_listp)))
        printf("Bad prologue header\n");
    checkblock(heap_listp);

    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        if (verbose) printblock(bp);
        checkblock(bp);
    }

    if (verbose) printblock(bp);
    if ((GET_SIZE(HDRP(bp)) != 0) || !(GET_ALLOC(HDRP(bp)))) printf("Bad epilogue header\n");
}
