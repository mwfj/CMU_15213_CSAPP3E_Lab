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

// #define heapchecker(verbose, lineno) mm_checkheap(verbose, lineno)
#define heapchecker(verbose, lineno)

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
// #define PACK(size, alloc)  ((size) | (alloc))
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

/** The position where the pointer of the predecessor located for free block*/
#define PRED_PTR(bp)            ((char*) bp)
/** The position where the pointer of the successor located for free block */
#define SUCC_PTR(bp)            ((char*) bp  + WSIZE)

/** Get the address of the pointer of the pointer */
#define GET_LIST_PTR(bp)        (*(char**) bp)

/** Get the address of the predecessor */
/** Equal to *(char **) bp */
#define GET_PREDECESSOR(bp)     (GET_LIST_PTR(bp))
// #define GET_PREDECESSOR(bp)     (GET(PRED_PTR(bp)))

/** Get the address of the successor */
/** Equal to *(char **) SUCC_PTR(bp) */
#define GET_SUCCESSOR(bp)       (GET_LIST_PTR(SUCC_PTR(bp)))
// #define GET_SUCCESSOR(bp)       (GET(SUCC_PTR(bp)))

#define SET_SUCCESSOR(bp, vp)   (GET_SUCCESSOR(bp) = vp)
#define SET_PREDECESSOR(bp, vp) (GET_PREDECESSOR(bp) = vp)

/** $end mallocmacro **/


// #define PRED(ptr) (*(char **)(ptr))
// #define SUCC(ptr) (*(char **)(SUCC_PTR(ptr)))

/* Global variables */
static char *heap_listp = 0; // Heap List Pointer: the pointer to the first block
static char *heap_freep = 0; // Point to the first free block

/* Function prototypes for internal helper routines */
static void checkheap(int verbose, int lineno);
static void checkblock(void *bp);
static void printblock(void* bp);
static void* extend_heap(size_t words);
static void* coalesce(void* bp);
__attribute__((always_inline)) static inline void insert_node(void* bp);
__attribute__((always_inline)) static inline void delete_node(void* bp);
__attribute__((always_inline)) static inline void place(void* bp, size_t asize);
static void* find_fit(size_t asize);

static void* extend_heap(size_t words){
    char* bp;

    /** Alignment the size */
    size_t size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;

    if((long)(bp = mem_sbrk(size)) == -1)
        return NULL;

    /** Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0)); /** Free Block Header */
    PUT(FTRP(bp), PACK(size, 0)); /** Free Footer Header */
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0,1)); /** New Epilogue Header */
    return coalesce(bp);
}

static void* coalesce(void* bp){
    /** 
     * To mark whether the previous block is allocated or not *
     *  + Previous block has allocated
     *  + The current block is the first element of free list
    **/
    size_t is_prev_alloc = GET_ALLOC( FTRP(PREV_BLKP(bp)) ) || PREV_BLKP(bp) == bp;
    /** Mark whether the next block is allocatd or not */
    size_t is_next_alloc = GET_ALLOC( HDRP(NEXT_BLKP(bp)) );
    /** Get the size of the current block */
    size_t size = GET_SIZE(HDRP(bp));
    /** Case 1: Both previous block and next block are allocated */
    
    /** Just Insert the current block into the free list 
     *  Don't need anyother operation
    */

    /** Case 2: Previous block are allocated, but the next block are freed*/
    if(is_prev_alloc && !is_next_alloc){
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
        bp = PREV_BLKP(bp);
        delete_node(bp);
        /** Update the header of current block */
        PUT(HDRP(bp), PACK(size, 0));
        /** Update the footer of next block */
        PUT(FTRP(bp), PACK(size, 0));
    /** Case 4: Both previous block and next block are freed*/
    }else if(!is_next_alloc && !is_prev_alloc){
        size += GET_SIZE( HDRP( PREV_BLKP(bp) )) + GET_SIZE( HDRP( NEXT_BLKP(bp) ) );
        /** Remove previous block from freelist */
        delete_node(PREV_BLKP(bp));
        /** Remove next block from freelist */
        delete_node(NEXT_BLKP(bp));
        bp = PREV_BLKP(bp);
        /** Update the header of previous block */
        PUT(HDRP(bp), PACK(size, 0));
        /** Update the footer of footer block */
        PUT(FTRP(bp), PACK(size, 0));
    }
    /** Insert the newly coalesce block into free list */
    insert_node(bp);
    // printf("Coalesce.\n");
    return bp;
}

/** Maintain the free list in last-in-first-out(LIFO) strategy by
 *  inserting and removing the newly free block
 */
__attribute__((always_inline)) static inline void insert_node(void* bp){
    /** Insert the free block at the begin of the free list */
    SET_SUCCESSOR(bp, heap_freep),
    SET_PREDECESSOR(heap_freep, bp);
    SET_PREDECESSOR(bp, NULL);
    /** Move the heap_freep to the newest head */
    heap_freep = bp;
}

/**
 * Remove the first element of the free list
 * which is the latest inserted one
**/
__attribute__((always_inline)) static inline void delete_node(void* bp){

    /** If the current free block is the only one
     *  just insert it at the beginning
    **/
    if(GET_PREDECESSOR(bp) == NULL)
        /** heap_freep -> bp -> NUll => heap_freep -> NULL */
        heap_freep = GET_SUCCESSOR(bp);
    /* The current free list have more than one free block */
    else
        /** Remove the current block from the free list 
         * prev_node -> bp -> next_node -> ...
         * => prev_node -> next_node -> ...
        */
        SET_SUCCESSOR(GET_PREDECESSOR(bp), GET_SUCCESSOR(bp));
    SET_PREDECESSOR(GET_SUCCESSOR(bp), GET_PREDECESSOR(bp));
}

/**
 * place - Place the request block at the beginning of the free block,
 *         splitting only if the size of the remainder would equal or exceeed the minimum block size
 */
__attribute__((always_inline)) static inline void place(void *bp, size_t asize){
    /** Get the block size */
    size_t block_size = GET_SIZE(HDRP(bp));

    /** Delete pointer from the free list */
    delete_node(bp);
    

    /** The current block can be splitted
     *  if the remain block meet the requirement of the minimum block size
     *  after it allocates the space to the block we need 
    **/
    if((block_size - asize) >= (2 * DSIZE)){
        /** Mark the old block pointer as allocated */
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        /** Update the pointer to the newly free block after split */
        bp = NEXT_BLKP(bp);
        /** Update the header/footer of the newly free block */
        PUT(HDRP(bp), PACK((block_size - asize), 0));
        PUT(FTRP(bp), PACK((block_size - asize), 0));
        /** Insert it into the free list
         *  And coalesce it with adjacency free blocks
         */
        coalesce(bp);

    /** Otherwise, the current block don't need to be splited
     *  Use this block directly
     */
    }else{
        /** Mark the old block pointer as allocated */
        PUT(HDRP(bp), PACK(block_size, 1));
        PUT(FTRP(bp), PACK(block_size, 1));
    }
}

/** 
 * Place: 
 *  Iterate the whole free list to search a free block
 *  that need the size requirement
 */
static void* find_fit(size_t asize){
    void* bp = NULL;
    /** Iterate the free block list to find a fit one */
    for(bp = heap_freep; !GET_ALLOC(HDRP(bp)); bp = GET_SUCCESSOR(bp)){
        if(!GET_ALLOC(HDRP(bp)) && asize <= GET_SIZE(HDRP(bp))){
            return bp;
        }
    }
    return NULL;
}

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    // printf("Init Explicit Free List....\n");
    /** Create initial empty heap */
    /**
     * The Initial block size is 4 Word size
     * - One unused padding: 1 word
     * - One Prologue Block: One Header(1 Word) + One Footer(1 word).
     *                       It create during the initialized and never be freed
     * - Epilogure Block: the zero size allocated block that consist of only a header(1 word)
     */
    if( ( heap_listp = mem_sbrk(8 * WSIZE) ) == NULL)
        return -1;

    /** Alignment Padding */
    PUT(heap_listp, 0);
    /** Initialze Prologue Header */
    PUT((heap_listp + (1 * WSIZE)), PACK(DSIZE, 1));
    /** Initialze Prologue Footer */
    PUT((heap_listp + (2 * WSIZE)), PACK(DSIZE, 1));
    /** Initialze Epilogue Header */
    PUT((heap_listp + (3 * WSIZE)), PACK(0, 1));
    /** Jump the pointer between header and footer of Prologue block*/
    heap_freep = heap_listp + (2 * WSIZE);
    /** Extend the size of current dynamic heap for
     *  storing the regular block
     */
    if (extend_heap(CHUNKSIZE/WSIZE) == NULL)
        return -1;

    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t asize; /** Adjust block size */
    size_t extendsize; /** The size need to be extended */
    char* bp; /** Returned block pointer */

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

    /** Search for free list for a suitable block */
    if((bp = find_fit(asize)) != NULL){
        place(bp, asize);
        return bp;
    }

    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize, CHUNKSIZE);

    if((bp = extend_heap(extendsize/WSIZE)) == NULL)
        return NULL;

    /** Split block if necessary */
    place(bp,asize);
    // heapchecker(0,__LINE__);
    // printf("Malloc Finished\n");
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
    
    /** Get the block size */
    size_t block_size = (size_t)GET_SIZE(HDRP(bp));

    /** Create new free list if there have no free list here */
    if(heap_listp == 0)
        mm_init();
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
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *bp, size_t size)
{
    size_t copysize;
    void* newbp = NULL;

    /** If realloc size is zero, just free the current block*/
    if(!size){
        mm_free(bp);
        return 0;
    }
    
    /** If the bp pointer is NULL, just malloc a block with required size */
    if(bp == NULL)
        return mm_malloc(size);

    newbp = mm_malloc(size);

    /** Fail to allocate */
    if(!newbp)
        return 0;

    /** Copy the old data from the old one to the new one */
    copysize = (size_t)GET_SIZE(HDRP(bp));
    
    if(size < copysize)
        copysize = size;
        
    memcpy(newbp,bp,copysize);

    /** Free the old block */
    mm_free(bp);

    /** Return new block */
    return newbp;
}

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
    if( (size_t)bp % ALIGNMENT )
        printf("Error: %p is not Double Word(8 bytes) alignment", bp);
    /* Check size correction between header and footer */
    if( GET(HDRP(bp)) != GET(FTRP(bp)) )
        printf("Error: header and footer is not matched for storing size\n");
}

/* 
 * checkheap - Minimal check of the heap for consistency 
**/
static void checkheap(int verbose, int lineno){
    /* Initialize the payload pointer of the block */
    char* bp = heap_listp;
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
    if((GET_SIZE(HDRP(heap_listp)) != DSIZE ) || // The current block is allocated
        !(GET_ALLOC(HDRP(heap_listp)))) // The allocate bit is not set
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