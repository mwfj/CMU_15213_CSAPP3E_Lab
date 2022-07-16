/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "Self-study",
    /* First member's full name */
    "Wufangjie Ma",
    /* First member's email address */
    "mwfj0215@gmail.com",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

#define DEBUGx

#ifdef DEBUG
#define heapchecker(verbose, lineno) mm_checkheap(verbose, lineno)
#else
#define heapchecker(verbose, lineno)
#endif

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/** $begin mallocmacro **/

#define WSIZE     (sizeof(void*))       /** Word and header/footer size(bytes) **/
#define DSIZE     (2 * WSIZE)           /** Double Word Size(bytes) **/
#define QSIZE     16                    /** Quad Word Size(bytes) */
#define CHUNKSIZE (1<<12)              /** Extend heay this amount(2048 bytes), a.k.a 2MB **/

#define MAX(x,y)                ((x) > (y) ? (x) : (y))
#define MIN(x,y)                ((x) < (y) ? (x) : (y))

/* 
   Pack a size and allocated bit into a word, 
   which returns a value that can be stored in a header or footer
*/
#define PACK(size, alloc)       ((size) | (alloc))
/* Read and write a word at address p */

/**
 * GET macro reads and returnes the word reference by arguement p,
 * where p is typically (void *) pointer, which cannot be deferenced directly
 * We need to cast (void*) pointer to (size_t*) to meet the type of address,
 * then dereferencing the value of that address *(size_t*)(p)
 */
#define GET(p)                  (*(size_t*)(p))
#define PUT(p, val)             (*(size_t*)(p) = (val))

/* Read the size and allocated fields from address p */
/**
 * As the memtion in the textbook, last three bit will also be zero.
 * Thus, we need to ignore the least three bits before calcuting the block size,
 * which from the header/footer at address p
 * ~0x7 == 111...111000(the number of 1 depends on the block size)
 */
#define GET_SIZE(p)             (GET(p) & ~0x7)
/**
 * Similarly, the last bit is to store whether this block is allocated or not.
 */
#define GET_ALLOC(p)            (GET(p) & 0x1)

/* Given block ptr bp, compute the address of its header and footer */

/**
 * Block pointer(bp) is a pointer to point the first payload byte
 * HDRP return the pointer of header, where the poiner of header is 
 * one wordsize before the pointer of payload
 */
#define HDRP(bp)                ((char*)(bp) - WSIZE)
/**
 * Similarly, to calculate the poinetr of footer by the pointer of payload,
 * it should be the size of payload away, 
 * where it should be (the whole block size) - (header size) + (footer size)
 */
#define FTRP(bp)                ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */

#define NEXT_BLKP(bp)           ((char*)(bp) + GET_SIZE((char*)(bp) - WSIZE))
#define PREV_BLKP(bp)           ((char*)(bp) - GET_SIZE((char*)(bp) - DSIZE))

/**
 * For the allocated block:
 * 
 *       31      ...           3| 2  1  0
 *       --------------------------------
 *      | 00 ... size (29 bits) | 0 0 a/f| header
 * bp -> --------------------------------
 *      |       content ...              |
 *       --------------------------------
 *      |       content ...              |
 *       --------------------------------
 *      | 00 ... size (29 bits) | 0 0 a/f| footer
 * 
 * For the free block
 *       31      ...           3| 2  1  0
 *       --------------------------------
 *      | 00 ... size (29 bits) | 0 0 a/f| header
 * bp -> --------------------------------
 *      |       Predecessor              |
 *       --------------------------------
 *      |        Successor               |
 *       --------------------------------
 *      | 00 ... size (29 bits) | 0 0 a/f| footer
 * 
 */

/** The position where the pointer of the predecessor located for free block*/
#define PRED_PTR(bp)            ((char*) bp)
/** The position where the pointer of the successor located for free block */
#define SUCC_PTR(bp)            ((char*) bp  + WSIZE)

/** Get the address of the pointer of the pointer */
#define GET_LIST_PTR(bp)        (*(char**) bp)

/** Get the address of the predecessor */
/** Equal to *(char **) bp */
#define GET_PREDECESSOR(bp)     (GET_LIST_PTR(bp))

/** Get the address of the successor */
/** Equal to *(char **) SUCC_PTR(bp) */
#define GET_SUCCESSOR(bp)       (GET_LIST_PTR(SUCC_PTR(bp)))

/** Segregated Free List */
#define SELECT_SEG_ENTRY(i)     (*(seg_free_listp + i))
#define MAXSEGENTRY 16

/** $end mallocmacro **/

/* Global variables */
static char *heap_listp = 0; /** Heap List Pointer: the pointer to the first block */
static char **seg_free_listp = 0; /** The pointer points to segregated free list */

/* Function prototypes for internal helper routines */
static void checkheap(int verbose, int lineno);
static void checkblock(void *bp);
static void printblock(void* bp);
static void* extend_heap(size_t words);
static void* coalesce(void* bp);
__attribute__((always_inline)) static inline void insert_node(void* bp, size_t size);
__attribute__((always_inline)) static inline void delete_node(void* bp);
__attribute__((always_inline)) static inline void *place(void* bp, size_t asize);


/**
 *
 * Segregated Free List: The block size in each free list is from the smaller to bigger
 * 
 *         Headers:Round Down Size       In each free list, blocksize:  smaller ==> bigger
 * 
 *                         Header
 * seg_free_listp     ==> +------+       +----+     +----+     +----+             +----+
 *                        | 2^0  |  ===> |    | --> |    | --> |    | --> ... --> |    | --> NULL
 *                        +------+       +----+     +----+     +----+             +----+
 * seg_free_listp + 1 ==> +------+       +----+     +----+     +----+             +----+
 *                        | 2^1  |  ===> |    | --> |    | --> |    | --> ... --> |    | --> NULL
 *                        +------+       +----+     +----+     +----+             +----+
 * seg_free_listp + 2 ==> +------+       +----+     +----+     +----+             +----+
 *                        | 2^2  |  ===> |    | --> |    | --> |    | --> ... --> |    | --> NULL
 *                        +------+       +----+     +----+     +----+             +----+
 * seg_free_listp + 3 ==> +------+       +----+     +----+     +----+             +----+
 *                        | 2^3  |  ===> |    | --> |    | --> |    | --> ... --> |    | --> NULL
 *                        +------+       +----+     +----+     +----+             +----+
 * 
 *       ...                ...           ...        ...        ...       ...      ...
 *       ...                ...           ...        ...        ...       ...      ...
 *       ...                ...           ...        ...        ...       ...      ...
 * 
 * seg_free_listp +15 ==> +------+       +----+     +----+     +----+             +----+
 *                        | 2^15 |  ===> |    | --> |    | --> |    | --> ... --> |    | --> NULL
 *                        +------+       +----+     +----+     +----+             +----+
 * seg_free_listp +16 ==> +------+       +----+     +----+     +----+             +----+
 *                        | 2^16 |  ===> |    | --> |    | --> |    | --> ... --> |    | --> NULL
 *                        +------+       +----+     +----+     +----+             +----+
**/

/****************************************************************************
 * 
 *  Malloc/Free API Area Begin
 * 
*****************************************************************************/

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    /** Initialize the Segregated free list */
    if((seg_free_listp = mem_sbrk(MAXSEGENTRY * sizeof(void*))) == (void*)(-1))
        return -1;

    int i;
    for(i=0; i<=MAXSEGENTRY; i++)
        SELECT_SEG_ENTRY(i) = NULL;
    
    /**
     * The Initial block size is 4 Word size
     * - One unused padding: 1 word
     * - One Prologue Block: One Header(1 Word) + One Footer(1 word).
     *                       It create during the initialized and never be freed
     * - Epilogure Block: the zero size allocated block that consist of only a header(1 word)
     */
    if( ( heap_listp = mem_sbrk(4 * WSIZE) ) == (void*)-1){
        printf("Fail to get allocation from mem_sbrk at line %d", __LINE__);
        return -1;
    }

    /** Alignment Padding */
    PUT(heap_listp, 0);
    /** Initialze Prologue Header */
    PUT((heap_listp + (1 * WSIZE)), PACK(DSIZE, 1));
    /** Initialze Prologue Footer */
    PUT((heap_listp + (2 * WSIZE)), PACK(DSIZE, 1));
    /** Initialze Epilogue Header */
    PUT((heap_listp + (3 * WSIZE)), PACK(0, 1));
    /** Jump the pointer between header and footer of Prologue block*/
    heap_listp += (2 * WSIZE);
    

    /** Extend the size of current dynamic heap for
     *  storing the regular block
     *  After several tests, it can bring the highest score when the initial block size is 64 bytes.
     */
    if (extend_heap((1 << 5) / WSIZE) == NULL){
        printf("Fail to extend heap at line %d", __LINE__);
        return -1;
    }
    /** For Debug */
    heapchecker(0, __LINE__);
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 * Also, we combine the process of finding fit with malloc in that function
 */
void *mm_malloc(size_t size)
{
    size_t asize; /** Adjust block size */
    size_t extendsize; /** The size need to be extended */
    char* bp = NULL; /** Returned block pointer */

    if(heap_listp == 0)
        mm_init();

    if(size <= 0)
        return NULL;
    /** Adjust block size to include overhead and alignment reqs */
    /** 
     * Enforce minimum block size of 16 bytes 
     *     1. 8 bytes to satisfy the alignment requirement;
     *     2. 8 bytes for the overhead of header and footer. 
    **/
    if(size <= DSIZE)        
        asize = 2 * DSIZE; /** 2*DSIZE is the minimum block size */
    /**
     * For requests over 8 bytes:
     * The general rule is to add in the overhead bytes
     * and then round up to the nearest multiple of 8
     */
    else
        asize = DSIZE * ( (size + (DSIZE) + (DSIZE - 1)) / DSIZE );

    int list_entry_num = 0; /** Keep the track the free list entry number */
    size_t search_size = asize; /** Keep track of the current size */
    
    /** 
     *  Find the best fit for
     *  searching the suitable free list entry and block in that free list
    **/
    while(list_entry_num < MAXSEGENTRY){
        if(( (list_entry_num == MAXSEGENTRY - 1) || (search_size <= 1) ) &&
             (SELECT_SEG_ENTRY(list_entry_num) != NULL)){
            bp = SELECT_SEG_ENTRY(list_entry_num);

            /** Search the current free list to find the suitable free block */
            while((bp != NULL) && (GET_SIZE(HDRP(bp))) < asize)
                bp = GET_SUCCESSOR(bp);
            
            /** Found the fit free block */
            if(bp != NULL){
                bp = place(bp,asize);
                return bp;
            }
        }
        search_size = search_size >> 1;
        list_entry_num++;
    }

    /** None of the available free block is suitable for the requirement */
    extendsize = MAX(asize, CHUNKSIZE);
    if((bp = extend_heap(extendsize / WSIZE)) == NULL){
        printf("Fail to extend heap at line %d", __LINE__);
        return NULL;
    }

    bp = place(bp,asize);
    /** For Debug */
    heapchecker(0,__LINE__);
    return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *bp)
{
    /** The padding from the beginning cannot be freed */
    if(bp == 0)
        return ;

    /** Create new free list if there have no free list here */
    if(heap_listp == 0)
        mm_init();
    
    /** Get the block size */
    size_t block_size = (size_t)GET_SIZE(HDRP(bp));

    /** Clear the payload of the current block*/
    /** Update the header */
    PUT(HDRP(bp), PACK(block_size, 0));
    /** Update the footer */
    PUT(FTRP(bp), PACK(block_size, 0));
    /** 
     * Insert that node into the free list and 
     * coalesce it with its adjacency free block 
    **/
   coalesce(bp);
   /** For Debug */
   heapchecker(0, __LINE__);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *bp, size_t size)
{   
    void *new_block = bp;

    /** If the bp pointer is NULL, just malloc a block with required size */
    if (bp == NULL) { 
        if( (bp = mm_malloc(size)) == NULL){
            printf("Fail to call mm_malloc at line : %d\n", __LINE__);
            return NULL;
        }
        return bp;
    }
    /** If realloc size is zero, just free the current block*/
    if (size == 0) { 
        mm_free(bp);
        return NULL;
    }

    /** Add the size of Header/Footer for the required payload size */
    if (size <= DSIZE)
        size = 2 * DSIZE;
    /**
     * For requests over 8 bytes:
     * The general rule is to add in the overhead bytes
     * and then round up to the nearest multiple of 8
     */
    else
        size = DSIZE * ( (size + (DSIZE) + (DSIZE - 1)) / DSIZE );

    /** Copy the old data from the old one to the new one */
    size_t old_size = (size_t)GET_SIZE(HDRP(bp));
    if(old_size >= size)
        return bp;

    /** 
     * Otherwise, to check whether 
     * the size of combining the current old block the next adjacency block
     * can meet the requirment if the next adjacency is freed.
     * in order to increase the usage of free block
     */

    /** Get the allocate condition for the next block */
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t next_blk_size = GET_SIZE(HDRP(NEXT_BLKP(bp)));
    

    /**
     * When next block is free and
     * the size of two block is greater than or equal the required size,
     * then just combine this two block
     */    
    if ((!next_alloc || !next_blk_size) && ((old_size + next_blk_size) >= size)){

        delete_node(NEXT_BLKP(bp));
        PUT(HDRP(bp), PACK(old_size + next_blk_size, 1));
        PUT(FTRP(bp), PACK(old_size + next_blk_size, 1));
        return bp;
    } 
    
    /** Otherwise, allocate newly one */
    if((new_block = mm_malloc(size)) == NULL){
        /** Fail to allocate */ 
        printf("Fail to call mm_malloc at line : %d\n", __LINE__);
        return NULL;
    }

     /** Copy the contend of the old block */
    memcpy(new_block, bp, MIN(size,old_size));
    /** Free the old block */
    mm_free(bp);
    /** For Debug */
    heapchecker(0, __LINE__);
    /** Return new block */
    return new_block;
    
}
/****************************************************************************
 * 
 *  Malloc/Free API Area End
 * 
*****************************************************************************/


/****************************************************************************
 * 
 *  Internal Utility Function Area Begin
 * 
*****************************************************************************/
static void* extend_heap(size_t words){
    char* bp;

    /** Alignment the size */
    size_t size;
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;

    if((bp = mem_sbrk(size)) == (void*)-1){
        printf("Fail to get allocation from mem_sbrk at line %d", __LINE__);
        return NULL;
    }

    /** Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0)); /** Free Block Header */
    GET_PREDECESSOR(bp) = NULL; /** Predecessor */
    GET_SUCCESSOR(bp) = NULL; /** Successor */
    PUT(FTRP(bp), PACK(size, 0)); /** Free Footer Header */
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0,1)); /** New Epilogue Header */

    return coalesce(bp);
}

static void* coalesce(void* bp){

#ifdef DEBUG
    assert(bp != NULL);
#endif
    /** 
     * To mark whether the previous block is allocated or not *
     *  + Previous block has allocated
     *  + The current block is the first element of free list
    **/
    size_t is_prev_alloc = GET_ALLOC( FTRP(PREV_BLKP(bp)) );
    /** Mark whether the next block is allocatd or not */
    size_t is_next_alloc = GET_ALLOC( HDRP(NEXT_BLKP(bp)) );
    /** Get the size of the current block */
    size_t size = GET_SIZE(HDRP(bp));
    /** Case 1: Both previous block and next block are allocated */
    
    /** Just Insert the current block into the free list 
     *  Don't need any other operations.
    */
    if(is_prev_alloc && is_next_alloc){
        insert_node(bp, size);
        return bp;
    }
    /** Case 2: Previous block are allocated, but the next block are freed*/
    else if(is_prev_alloc && !is_next_alloc){
        /** Coalesce the current block and the previous block */
        size += GET_SIZE( HDRP( NEXT_BLKP(bp) ) );
        delete_node(NEXT_BLKP(bp));
        /** Update the header of previous block */
        PUT(HDRP(bp), PACK(size, 0));
        /** Update the footer of current block */
        PUT(FTRP(bp), PACK(size, 0));
    /** Case 3: Previous block are freed but the next block are allocated */
    }else if(!is_prev_alloc && is_next_alloc){
        /** Coalesce the current block and the next block */
        size += GET_SIZE( HDRP( PREV_BLKP(bp) ));
        delete_node(PREV_BLKP(bp));
        /** Update the footer of next block */
        PUT(FTRP(bp), PACK(size, 0));
        /** Update the header of current block */
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    /** Case 4: Both previous block and next block are freed*/
    }else if(!is_next_alloc && !is_prev_alloc){
        size += GET_SIZE( HDRP( PREV_BLKP(bp) )) + GET_SIZE( FTRP( NEXT_BLKP(bp) ) );
        /** Remove previous block from freelist */
        delete_node(PREV_BLKP(bp));
        /** Remove next block from freelist */
        delete_node(NEXT_BLKP(bp));
        /** Update the header of previous block */
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        /** Update the footer of footer block */
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    /** Insert the newly coalesce block into free list */
    insert_node(bp, size);
    return bp;
}

/** 
 *  For every insertion, it maintains the free list in the size order
 *  from smaller size to the larger size.
 */
__attribute__((always_inline)) static inline void insert_node(void* bp, size_t size){

#ifdef DEBUG
    assert(bp != NULL);
#endif
    int list_entry_num = 0; /** Keep the track the free list entry number */
    void* insert_ptr = bp; /* The new block will be inserted after this pointer */
    void* next_ptr = NULL; /** Keep track of the next pointer the insert_prt */

    /** Select the suitable free list */
    while((list_entry_num < (MAXSEGENTRY - 1) ) && (size > 1)){
        size = size >> 1;
        list_entry_num  ++;
    }

    insert_ptr = SELECT_SEG_ENTRY(list_entry_num);
    if(insert_ptr != NULL)
        next_ptr = GET_SUCCESSOR(insert_ptr);
    else
        next_ptr = NULL;
    /* We keep the size order for each free list from the smaller size to the bigger size */
    while((insert_ptr != NULL) && (GET_SIZE(HDRP(insert_ptr)) < size)){
        insert_ptr = next_ptr;
        next_ptr = GET_SUCCESSOR(next_ptr);
    }

    if(insert_ptr != NULL){
        /** 
         * Insert the new block at the middle of the current free list:
         *      seg_free_listp[list_entry_num] -> ... -> insert_ptr -> bp -> next_ptr -> ... -> NULL
        */
        if(next_ptr != NULL){
            GET_PREDECESSOR(bp) = insert_ptr;
            GET_SUCCESSOR(insert_ptr) = bp;
            GET_SUCCESSOR(bp) = next_ptr;
            GET_PREDECESSOR(next_ptr) = bp;
        }
        /**
         * Insert the new block at the end of the current free list:
         *       seg_free_listp[list_entry_num] -> ... -> insert_ptr -> bp -> NULL(next_ptr)
         */
        else{
            GET_PREDECESSOR(bp) = insert_ptr;
            GET_SUCCESSOR(insert_ptr) = bp;
            GET_SUCCESSOR(bp) = NULL;
        }
    }else{
        /**
         * Insert the new block at beginning of the free list;
         * And also there still have other free blocks:
         *      seg_free_listp[list_entry_num](insert_ptr(NULL)) -> bp -> next_ptr -> ... -> NULL
         */
        if(next_ptr != NULL){
            GET_PREDECESSOR(bp) = NULL;
            GET_SUCCESSOR(bp) = next_ptr;
            GET_PREDECESSOR(next_ptr) = bp;
            SELECT_SEG_ENTRY(list_entry_num) = bp;
        }
        /**
         * The current free list is empty:
         *      seg_free_listp[list_entry_num](insert_ptr(NULL)) -> bp -> NULL(next_ptr)
         */
        else{
            GET_PREDECESSOR(bp) = NULL;
            GET_SUCCESSOR(bp) = NULL;
            SELECT_SEG_ENTRY(list_entry_num) = bp;
        }
    }
}


/**
 * Remove the first element of the free list
 * which is the latest inserted one
**/
__attribute__((always_inline)) static inline void delete_node(void* bp){

#ifdef DEBUG
    assert(bp != NULL);
#endif

    int list_entry_num = 0; /** Keep the track the free list entry number */
    size_t size = GET_SIZE(HDRP(bp));

    /** Select the suitable free list */
    while((list_entry_num < (MAXSEGENTRY - 1) ) && (size > 1)){
        size = size >> 1;
        list_entry_num  ++;
    }

    /** Just like insertion, we also need to separate deletion in separate situation */
    if(GET_PREDECESSOR(bp) != NULL){
        /**
         * Before: 
         *  seg_free_listp[list_entry_num] -> ... -> predecessor -> bp -> successor -> ... -> NULL
         * After: 
         *  seg_free_listp[list_entry_num] -> ... -> predecessor -> successor -> ... -> NULL
         */
        if(GET_SUCCESSOR(bp) != NULL){
            GET_SUCCESSOR(GET_PREDECESSOR(bp)) = GET_SUCCESSOR(bp);
            GET_PREDECESSOR(GET_SUCCESSOR(bp)) = GET_PREDECESSOR(bp);
        }
        /**
         * Before: 
         *  seg_free_listp[list_entry_num] -> ... -> predecessor -> bp -> NULL
         * After: 
         *  seg_free_listp[list_entry_num] -> ... -> predecessor -> NULL
         */
        else{
            GET_SUCCESSOR(GET_PREDECESSOR(bp)) = NULL;
        }
    }else{
        /**
         * Before: 
         *  seg_free_listp[list_entry_num] -> bp -> successor -> ... -> NULL
         * After: 
         *  seg_free_listp[list_entry_num] -> successor -> ... -> NULL
         */
        if(GET_SUCCESSOR(bp) != NULL){
            SELECT_SEG_ENTRY(list_entry_num) = GET_SUCCESSOR(bp);
            GET_PREDECESSOR(GET_SUCCESSOR(bp)) = NULL;
        }
        /**
         * Before: 
         *  seg_free_listp[list_entry_num] -> bp -> NULL
         * After: 
         *  seg_free_listp[list_entry_num] -> NULL
         */
        else{
            SELECT_SEG_ENTRY(list_entry_num) = NULL;
        }
    }
}

/**
 * place - Place the request block at the beginning of the free block,
 *         splitting only if the size of the remainder would equal or exceeed the minimum block size
 */
__attribute__((always_inline)) static inline void *place(void *bp, size_t asize){
#ifdef DEBUG
    assert(bp != NULL);
#endif
    /** Get the block size */
    size_t block_size = GET_SIZE(HDRP(bp));
    size_t remaining_size = block_size - asize;
    /** Delete pointer from the free list */
    delete_node(bp);
    
    /** The current block don't need to be splited
     *  Use this block directly
     */
    if(remaining_size <= 2 * DSIZE){
        /** 
         * Mark the old block pointer as allocated 
         * No need to allocate
        **/
        PUT(HDRP(bp), PACK(block_size, 1));
        PUT(FTRP(bp), PACK(block_size, 1));
    }
    /** The current block can be splitted
     *  if the remain block meet the requirement of the minimum block size
     *  after it allocates the space to the block we need 
    **/
    else if(asize >= 96){
    /**
     * 96 is empirical size. 
     *
     * This part of code inspired by this link: 
     * https://github.com/sally20921/malloclab/blob/master/malloclab-handout/src/mm.c#:~:text=else%20if%20(-,asize%20%3E%3D%2096,-)%20%7B
     * 
     * In binary-bal.rep, binary2-bal.rep, if the free block follow the order of
     * small - big(Freed) - small - big(Freed) - small ... the external fragmentation may occured.
     * Thus, in here, to avoid such circumstance, 
     * I'm trying to make the smaller blocks to be in a relatively forward position 
     * and the larger blocks to be in a relatively backward position
     * in order to minimize the external fragmentation:
     * small - small - small - big(Freed) - big(Free)
     * 
     */
        PUT(HDRP(bp), PACK(remaining_size, 0));
        PUT(FTRP(bp), PACK(remaining_size, 0));
        // PUT(HDRP(bp), PACK(asize, 1));
        // PUT(FTRP(bp), PACK(asize, 1));
        PUT(HDRP(NEXT_BLKP(bp)), PACK(asize, 1));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(asize, 1));
        /** 
         *  Insert it into the free list
         *  And coalesce it with adjacency free blocks
         */
        coalesce(bp);
        return NEXT_BLKP(bp);
    }
    /** Do the normal split */
    else{
        /** Mark the old block pointer as allocated */
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        /** Update the pointer to the newly free block after split */
        /** Update the header/footer of the newly free block */
        PUT(HDRP(NEXT_BLKP(bp)), PACK(remaining_size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(remaining_size, 0));
        /** 
         *  Insert it into the free list
         *  And coalesce it with adjacency free blocks
         */
        // insert_node(NEXT_BLKP(bp), remaining_size);
        coalesce(NEXT_BLKP(bp));
    }
    return bp;
}
/****************************************************************************
 * 
 *  Internal Utility Function Area End
 * 
*****************************************************************************/


/****************************************************************************
 * 
 *  Debug Function Begin
 * 
*****************************************************************************/

/* 
 * mm_checkheap - Check the heap for correctness
 */
void mm_checkheap(int verbose, int lineno)  
{ 
    checkheap(verbose, lineno);
}

/* 
 * printblock - Print block header/footer info
**/
static void printblock(void* bp){
    /* The size stored in header, the allocation bit in header */
    size_t hsize, halloc;
    /* The size stored in footer, the allocation bit in footer */
    size_t fsize, falloc;

    checkheap(0,__LINE__);
    hsize = GET_SIZE(HDRP(bp));
    halloc = GET_ALLOC(HDRP(bp));
    fsize = GET_SIZE(FTRP(bp));
    falloc = GET_ALLOC(FTRP(bp));

    if(hsize == 0){
        printf("%p: EOL\n", bp);
        return;
    }
    
    printf("%p: header: [%zu:%c] footer: [%zu:%c]\n", bp, 
        hsize, (halloc ? 'a' : 'f'), 
        fsize, (falloc ? 'a' : 'f')); 
}

/* 
 * checkblock - Check the header/footer correction 
**/
static void checkblock(void* bp){
    /* Check alignment correction */
    if( (size_t)bp % 8 )
        printf("Error: %p is not Double Word(8 bytes) alignment", bp);
    /* Check size correction between header and footer */
    if( GET(HDRP(bp)) != GET(FTRP(bp)) )
        printf("Error: header and footer is not matched for storing size\n");
}

/* 
 * checkheap - Minimal check of the heap for consistency 
**/
static void checkheap(int verbose, int lineno){

    int list_entry_num = 0; /** Keep the track the free list entry number */

    void *bp = NULL;

    /** List Level Checking */
    while(list_entry_num < MAXSEGENTRY){
        /** Get the head pointer of each free list */
        bp = SELECT_SEG_ENTRY(list_entry_num);
        /** Iterate each block of the current free list */
        while(bp != NULL){
            /** Make sure every block in the free list is free block */
            if (GET_ALLOC(HDRP(bp))) {
                printf("An Allocated Block exist in the free list, addr: %p\n", (char*)bp);
            }
            /** check correction of that block */
            checkblock(bp);
            /** Go to the next block */
            bp = GET_SUCCESSOR(bp);
        }
        /** Move to the next free list */
        list_entry_num ++;
    }

    /** Block Level Checking */
    /* Initialize the payload pointer of the block */
    bp = heap_listp;

    /* Print the infomation */
    if(verbose){
        printf("mm_checkheap called from %d\n",lineno);
        printf("Heap: (%p):\n", heap_listp);
    }

    /* Check the correction of Prologue Header correction */
    /* 
        The prologue block is an 8-byte allocated block consisting of only a header and footoer,
        where it created during initialization and never freed
    **/
    if((GET_SIZE(HDRP(heap_listp)) != DSIZE ) || /** The current block is allocated */
        !(GET_ALLOC(HDRP(heap_listp)))) /** The allocate bit is not set*/
        printf("Bad Prologue Header\n");
    checkblock(heap_listp);
    
    /* 
     * Iterate all the block,
     * print block and check its correctness.
    **/
    for(bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)){
        if(verbose)
            printblock(bp);
        checkblock(bp);
    }

    if(verbose)
        printblock(bp);

    /* Check the correction of Epilogue Header correction */
    /*
        The heap always end with epilogue block, which is a zero-size allocated block
        that consist of only a header. 
    **/
    if( (GET_SIZE(HDRP(bp)) != 0) || !GET_ALLOC(HDRP(bp)) )
        printf("Bad Epilogue Header\n");
}
/****************************************************************************
 * 
 *  Debug Function End
 * 
*****************************************************************************/