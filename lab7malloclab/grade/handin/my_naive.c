/*
 * mm-implicit.c -  Simple allocator based on implicit free lists,
 *                  first fit placement, and boundary tag coalescing.
 *
 * Allocator implement by explicit free list.
 * each allocated block struct like this
 *  31      ...           3| 2  1  0
 *  --------------------------------
 * | 00 ... size (29 bits) | 0 0 a/f| header
 * |       content ...              |
 * |       content ...              |
 * | 00 ... size (29 bits) | 0 0 a/f| footer
 * where s are the meaningful size bits and a/f is set
 * iff the block is allocated. The list has the following form:
 *
 * each free block struct like this
 *  31      ...           3| 2  1  0
 *  --------------------------------
 * | 00 ... size (29 bits) | 0 0 a/f     | header
 * |      succ_addr (successor)          | succ_addr
 * |      pred_addr (predecessor)        | pred_addr
 * | 00 ... size (29 bits) | 0 0 a/f     | footer
 *  --------------------------------
 * The allocated prologue and epilogue blocks are overhead that
 * eliminate edge conditions during coalescing.
 *
 * 这个代码主要是用显式空闲链表实现了空闲块的组织，在best_fit下分数可以达到90，
 * 这个虽然是 分离链表，但是 你会发现 会单一链表的处理没有多大的区别，我用的仍然是best_fit
 * 可以发现 平均速度提高了10倍，但是利用率有所下降（2%）
 * 另外我的prev和succ是反着的，因为这样head可以省下一个WSIZE
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "memlib.h"
#include "mm.h"
#define DEBUG
/*
 * If NEXT_FIT defined use next fit search, else use first fit search
 */
#define BEST_FIT

/* Team structure */
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
/* $begin mallocmacros */
/* Basic constants and macros */
#define WSIZE 4             /* word size (bytes) */
#define DSIZE 8             /* doubleword size (bytes) */
#define CHUNKSIZE (1 << 12) /* extend heap size (bytes) */
#define OVERHEAD 8          /* overhead of header and footer (bytes) */

#define MAX(x, y) ((x) > (y) ? (x) : (y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p) (*(size_t *)(p))
#define PUT(p, val) (*(size_t *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp) ((char *)(bp)-WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp)-WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp)-GET_SIZE(((char *)(bp)-DSIZE)))
/* $end mallocmacros */

/* pointer to first block */
static char *heap_listp;  // 堆头

/* function prototypes for internal helper routines */
static void *extend_heap(size_t words);
static void place(void *bp, size_t asize);
static void *find_fit(size_t asize);
static void *coalesce(void *bp);
static void printblock(void *bp);
static void checkblock(void *bp);
#define ARRAY_SIZE 10

static unsigned int list[ARRAY_SIZE];
static int64_t t[ARRAY_SIZE];
static void *head[ARRAY_SIZE];
static void *tail[ARRAY_SIZE];
#define PREV(bp) ((char *)(bp) + WSIZE)  // prev 在第二个位置
#define SUCC(bp) ((char *)(bp))          // succ 在第一个位置，即bp的地方
#define GETP(p) (*(char **)p)
#define GET_PREV(bp) (GETP(PREV(bp)))
#define GET_SUCC(bp) (GETP(SUCC(bp)))
#define PUTP(p, val) (*(char **)(p) = (char *)(val))
#define listArray head  // 1-16, 17-32, 33-64 ... 4096-MAX

// 在pos前面插入node
#define insert_before(pos, node)                                      \
    {                                                                 \
        GET_SUCC(GET_PREV(pos)) = (char *)node; /* pos.PREV -> node*/ \
        GET_PREV(node) = GET_PREV(pos);         /*pos.PREV<-node*/    \
        GET_PREV(pos) = (char *)node;           /*node<-pos*/         \
        GET_SUCC(node) = (char *)pos;           /*node->pos*/         \
    }
// 从空闲链表中删除pos
#define del(pos)                                                  \
    {                                                             \
        GET_SUCC(GET_PREV(pos)) = GET_SUCC(pos); /*PREV -> NEXT*/ \
        GET_PREV(GET_SUCC(pos)) = GET_PREV(pos); /*PREV<-NEXT*/   \
    }
// 将bp插入到空闲链表中
inline static size_t findHeadPos(size_t size)
{
    for (size_t i = 0; i < ARRAY_SIZE; ++i) {
        if (0 == (size >> 3 + i)) return i;
    }
    return ARRAY_SIZE - 1;
}
#define insertFreeList(bp)                                                \
    {                                                                     \
        void *p = GET_SUCC(listArray[findHeadPos((GET_SIZE(HDRP(bp))))]); \
        insert_before((p), bp);                                           \
    }

#define freeNode(bp) del(bp)

int mm_init(void)
{
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        head[i] = &list[i];
        tail[i] = &t[i];
        GET_SUCC(head[i]) = tail[i];  // 建立链表，如果你想实现next_fit，可以让
        GET_PREV(tail[i]) = head[i];  // tail->next = head -> next(即 *head) 这样循环链表方便遍历
    }
    if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void *)-1) return -1;
    PUT(heap_listp, 0);                          /* alignment padding */
    PUT(heap_listp + WSIZE, PACK(OVERHEAD, 1));  /* prologue header */
    PUT(heap_listp + DSIZE, PACK(OVERHEAD, 1));  /* prologue footer */
    PUT(heap_listp + WSIZE + DSIZE, PACK(0, 1)); /* epilogue header */
    heap_listp += DSIZE;

    /* Extend the empty heap with a free block of CHUNKSIZE bytes */
    // 经过不断地测试我发现64(70也可以)有最高的效率，可以达到90分（best_fit）了
    // 初步估计是因为后面那个数据(trace 10)里都是小块，频繁的malloc和free
    if (extend_heap((1 << 6) / DSIZE) == NULL) return -1;
    return 0;
}

void *mm_malloc(size_t size)
{
    size_t asize;      /* adjusted block size */
    size_t extendsize; /* amount to extend heap if no fit */
    char *bp;

    /* Ignore spurious requests */
    if (size <= 0) return NULL;

    /* Adjust block size to include overhead and alignment reqs. */
    if (size <= DSIZE)
        asize = DSIZE + OVERHEAD;
    else
        asize = DSIZE * ((size + (OVERHEAD) + (DSIZE - 1)) / DSIZE);

    /* Search the free list for a fit */
    if ((bp = find_fit(asize)) == NULL) {
        extendsize = MAX(asize, CHUNKSIZE);
        if ((bp = extend_heap(extendsize / DSIZE)) == NULL) return NULL;
    }
    /* No fit found. Get more memory and place the block */
    place(bp, asize);
    return bp;
}
/* $end mmmalloc */

void mm_free(void *bp)
{
    if (!bp) return;
    size_t size = GET_SIZE(HDRP(bp));

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    coalesce(bp);
}

inline static void place_alloc(void *ptr, size_t oldSize, size_t newSize)
{
    PUT(HDRP(ptr), PACK(newSize, 1));
    PUT(FTRP(ptr), PACK(newSize, 1));
    void *newFreeBlock = NEXT_BLKP(ptr);
    PUT(HDRP(newFreeBlock), PACK(oldSize - newSize, 0));
    PUT(FTRP(newFreeBlock), PACK(oldSize - newSize, 0));
    coalesce(newFreeBlock);
}

void *mm_realloc(void *ptr, size_t newSize)
{
    if (newSize == 0) {
        mm_free(ptr);
        return NULL;
    }
    if (ptr == NULL) return mm_malloc(newSize);
    // 先将newSize对齐

    if (newSize <= DSIZE)
        newSize = DSIZE + OVERHEAD;
    else
        newSize = DSIZE * ((newSize + (OVERHEAD) + (DSIZE - 1)) / DSIZE);

    size_t oldSize = GET_SIZE(HDRP(ptr));
    if (oldSize == newSize) return ptr;
    // 缩减，并且缩减之后能多出空闲块，就分割
    else if (oldSize >= newSize) {
        if (oldSize >= newSize + (DSIZE + OVERHEAD)) place_alloc(ptr, oldSize, newSize);
        return ptr;
    }
    // 扩大
    void *next_block = NEXT_BLKP(ptr);
    size_t next_alloc = GET_ALLOC(HDRP(next_block));
    size_t oldNextSize = oldSize;
    //第一种情况， next_alloc 是free的， 所以我们可以尝试合并，这样利用率更高
    if (!next_alloc) {
        oldNextSize += GET_SIZE(HDRP(next_block));
        /*考虑这个情况不会让效率更高, 并且速度可能更慢*/
        // if (oldNextSize < newSize && !GET_SIZE(HDRP(next_block)))
        //     extend_heap(MAX(newSize - oldNextSize, CHUNKSIZE));
        freeNode(next_block);  // 在空闲链表中删除next_block
        // 接下来修改一下块中的oldSize ， 和Foot
        PUT(HDRP(ptr), PACK(oldNextSize, 1));
        PUT(FTRP(ptr), PACK(oldNextSize, 1));
        if (oldNextSize - newSize >= (DSIZE + OVERHEAD)) place_alloc(ptr, oldNextSize, newSize);
        if (oldNextSize - newSize >= 0) return ptr;
    }
    /*加了下面的情况分就只有88了, 说实话我不是很明白为什么会更低,如果要使用这个和extend_heap
    最好对上面的extend_heap做修改,不然没意义*/
    // // 第二种情况，prev_alloc 是free的, 所以我们也可以尝试合并
    // void *prev_block = PREV_BLKP(ptr);
    // size_t prev_alloc = GET_ALLOC(HDRP(prev_block));
    // size_t oldPrevSize = GET_SIZE(HDRP(prev_block)) + oldNextSize;
    // if (!prev_alloc) {
    //     // place(next_block, newSize - oldSize);
    //     del(prev_block);  // 在空闲链表中删除prev_block
    //     // 接下来修前一块中的oldSize ， 和Foot
    //     PUT(HDRP(prev_block), PACK(oldPrevSize, 1));
    //     PUT(FTRP(prev_block), PACK(oldPrevSize, 1));
    //     memcpy(prev_block, ptr, oldSize);
    //     if (oldPrevSize - newSize >= (DSIZE + OVERHEAD))
    //         place_alloc(prev_block, oldPrevSize, newSize);
    //     if (oldPrevSize - newSize >= 0) return prev_block;
    // }
    void *newp;
    if ((newp = mm_malloc(newSize)) == NULL) {
        printf("ERROR: mm_malloc failed in mm_realloc\n");
        exit(1);
    }
    memcpy(newp, ptr, oldSize);
    mm_free(ptr);
    return newp;
}

/*
 * mm_checkheap - Check the heap for consistency
 */
void mm_checkheap(int verbose)
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

/* The remaining routines are internal helper routines */

/*
 * extend_heap - Extend heap with free block and return its block pointer
 */
/* $begin mmextendheap */
static void *extend_heap(size_t words)
{
    char *bp;
    size_t size;

    /* 分配偶数单词以保持对齐方式 最小size是 16字节 即 2 * DSIZE*/
    size = (words % 2) ? (words + 1) * DSIZE : words * DSIZE;
    if ((bp = mem_sbrk(size)) == (void *)-1) return NULL;

    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0));         /* free block header */
    PUT(FTRP(bp), PACK(size, 0));         /* free block footer */
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); /* new epilogue header */

    /* Coalesce if the previous block was free */
    return coalesce(bp);
}
/* $end mmextendheap */

// 分片， 如果bp指向的块的大小 - asize >= 最小空闲块大小就分块
static void place(void *bp, size_t asize)
{
    size_t csize = GET_SIZE(HDRP(bp));
    freeNode(bp);  // 删去分配块
    if ((csize - asize) >= (DSIZE + OVERHEAD)) {
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(csize - asize, 0));
        PUT(FTRP(bp), PACK(csize - asize, 0));
        insertFreeList(bp);  // 加入新的空闲块
    } else {
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
    }
}
/* $end mmplace */

/*
 * find_fit - Find a fit for a block with asize bytes
 */
static void *find_first_fit(size_t size);
static void *find_best_fit(size_t size);
static inline void *find_fit(size_t asize)
{
#ifdef NEXT_FIT
    return find_next_fit(size);  // note ！没有实现这个函数
#elif BEST_FIT 1
    void *bp = find_best_fit(asize);
    return bp;
#else
    /* first fit search */

    return find_next_fit(asize); /* find_next_fit */
#endif
}

static void *find_first_fit(size_t size)
{
    for (void *bp = GET_SUCC(head); bp != tail; bp = GET_SUCC(bp)) {
        if (GET_SIZE(HDRP(bp)) >= size) {
            return bp;
        }
    }
    return NULL;
}

static void *find_best_fit(size_t size)
{
    size_t size_gap = 1 << 30;
    void *best_addr = NULL, *temp;
    size_t pos = findHeadPos(size);

    while (best_addr == NULL && pos != ARRAY_SIZE) {
        for (void *bp = GET_SUCC(listArray[pos]); bp != tail[pos]; bp = GET_SUCC(bp)) {
            temp = HDRP(bp);
            if (GET_SIZE(temp) - size < size_gap) {
                size_gap = GET_SIZE(temp) - size;
                best_addr = bp;
                if (GET_SIZE(temp) == size) return best_addr;
            }
        }
        pos++;
    }

    return best_addr;
}
// note !! coalesce 的语义已经发生了改变, 具体原因是因为, 我不想插入了又删除,这浪费时间
// 这个函数负责加入 新出来的空闲块到显式空闲链表并且合并相邻的空闲块
// 这里实现时需要注意的是，除了bp，其他空闲块都已经在空闲链表中了，注意重复！和漏删
static void *coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));
    // alloc, alloc
    if (prev_alloc && next_alloc) { /* Case 1 */
        insertFreeList(bp);
        return bp;
    }
    // alloc, unallocated
    else if (prev_alloc && !next_alloc) { /* Case 2 */
        freeNode(NEXT_BLKP(bp));
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
        insertFreeList(bp);
    }
    // unallocated, alloc，空闲链表无变化，不需要修改
    else if (!prev_alloc && next_alloc) { /* Case 3 */
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    // unallocated, unallocated
    else { /* Case 4 */
        freeNode(NEXT_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }

#ifdef NEXT_FIT
    /* Make sure the rover isn't pointing into the free block */
    /* that we just coalesced */
    if ((rover > (char *)bp) && (rover < NEXT_BLKP(bp))) rover = bp;
#endif

    return bp;
}

static void printblock(void *bp)
{
    size_t hsize, halloc, fsize, falloc;

    hsize = GET_SIZE(HDRP(bp));
    halloc = GET_ALLOC(HDRP(bp));
    fsize = GET_SIZE(FTRP(bp));
    falloc = GET_ALLOC(FTRP(bp));

    if (hsize == 0) {
        printf("%p: EOL\n", bp);
        return;
    }

    printf("%p: header: [%d:%c] footer: [%d:%c]\n", bp, hsize, (halloc ? 'a' : 'f'), fsize,
           (falloc ? 'a' : 'f'));
}

static void checkblock(void *bp)
{
    if ((size_t)bp % 8) printf("Error: %p is not doubleword aligned\n", bp);
    if (GET(HDRP(bp)) != GET(FTRP(bp))) printf("Error: header does not match footer\n");
}
