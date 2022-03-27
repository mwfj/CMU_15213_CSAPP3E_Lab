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


/*
 * If NEXT_FIT defined use next fit search, else use first-fit search 
 */
#define NEXT_FIT

// #define heapchecker(verbose, lineno) mm_checkheap(verbose, lineno)
#define heapchecker(verbose, lineno)

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/** $begin mallocmacro **/

#define WSIZE     4       /** Word and header/footer size(bytes) **/
#define DSIZE     8       /** Double Word Size(bytes) **/
#define CHUNKSIZE (1<<12) /** Extend heay this amount(2048 bytes), a.k.a 2MB **/

#define MAX(x,y)  ((x) > (y) ? (x) : (y))

/* 
   Pack a size and allocated bit into a word, 
   which returns a value that can be stored in a header or footer
*/
// #define PACK(size, alloc)  ((size) | (alloc))
#define PACK(size, alloc)  ((size) | (alloc))
/* Read and write a word at address p */

/**
 * GET macro reads and returnes the word reference by arguement p,
 * where p is typically (void *) pointer, which cannot be deferenced directly
 * We need to cast (void*) pointer to (unsigned int*) to meet the type of address,
 * then dereferencing the value of that address *(unsigned int*)(p)
 */
#define GET(p)             (*(unsigned int*)(p))
#define PUT(p, val)        (*(unsigned int*)(p) = (val))

/* Read the size and allocated fields from address p */
/**
 * As the memtion in the textbook, last three bit will also be zero.
 * Thus, we need to ignore the least three bits before calcuting the block size,
 * which from the header/footer at address p
 * ~0x7 == 111...111000(the number of 1 depends on the block size)
 */
#define GET_SIZE(p)        (GET(p) & ~0x7)
/**
 * Similarly, the last bit is to store whether this block is allocated or not.
 */
#define GET_ALLOC(p)       (GET(p) & 0x1)

/* Given block ptr bp, compute the address of its header and footer */

/**
 * Block pointer(bp) is a pointer to point the first payload byte
 * HDRP return the pointer of header, where the poiner of header is 
 * one wordsize before the pointer of payload
 */
#define HDRP(bp)           ((char*)(bp) - WSIZE)
/**
 * Similarly, to calculate the poinetr of footer by the pointer of payload,
 * it should be the size of payload away, 
 * where it should be (the whole block size) - (header size) + (footer size)
 */
// #define FTRP(bp)           ((char*)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)
#define FTRP(bp)       ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */

#define NEXT_BLKP(bp)      ((char*)(bp) + GET_SIZE((char*)(bp) - WSIZE))
#define PREV_BLKP(bp)      ((char*)(bp) - GET_SIZE((char*)(bp) - DSIZE))


/** $end mallocmacro **/

/* Global variables that always points to the prologue block */
static char* heap_listp = 0; // Heap List Pointer: the pointer to the first block
#ifdef NEXT_FIT
static char* rover; /** Next fit rover */ 
#endif

/* Function prototypes for internal helper routines */
static void checkheap(int verbose, int lineno);
static void checkblock(void *bp);
static void printblock(void* bp);
static void *extend_heap(size_t words);
static void *coalesce(void* bp);
static void *find_fit(size_t asize);
static void place(void* bp, size_t asize);

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    /* Create the initial empty heap */
    /* 
        Padding(1 Word) + 
        Prologue Block(Header(1 Word) + 
        Footer(1 Word)) + Epilogue Block(Header(1 Word)*/
    if((heap_listp = mem_sbrk(4*WSIZE)) == (void*)-1 )
        return -1;
    /** Alignment Padding */
    PUT(heap_listp, 0);
    /** Initialize Prologue Header */
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1));
    /** Initialize Prologue Footer */
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1));
    /** Initialize Epilogue Header */
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1));
    heap_listp += (2*WSIZE);

#ifdef NEXT_FIT
    rover =  heap_listp;
#endif

    /* Extend the empty heap with a free block of CHUNKSIZE bytes */
    if (extend_heap(CHUNKSIZE/WSIZE) == NULL)
        return -1;

    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
/* $begin mmmalloc */
void *mm_malloc(size_t size)
{
    size_t asize; /** Adjust block size */
    size_t extendsize; /** Amount to extend heap if no fit */
    char* bp; /** block pointere */

    if (heap_listp == 0){
        mm_init();
    }
    /** Ignore spurious requests */
    if(size == 0)
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
    // int newsize = ALIGN(size + SIZE_T_SIZE);
    // void *p = mem_sbrk(newsize);
    // if (p == (void *)-1)
	// return NULL;
    // else {
    //     *(size_t *)p = size;
    //     return (void *)((char *)p + SIZE_T_SIZE);
    // }
}
/* $end mmmalloc */

/*
 * mm_free - Freeing a block does nothing.
 */
/* $begin mmfree */
void mm_free(void *bp)
{
    /* $end mmfree */
    /** The padding from the beginning cannot be freed */
    if(bp == 0)
        return ;
    
    /* $begin mmfree */
    /** Get the block size */
    size_t size = GET_SIZE(HDRP(bp));
    /* $end mmfree */

    /** Create the free list if it non-exist */
    if(heap_listp == 0)
        mm_init();

    /* $begin mmfree */
    /** Update Header */
    PUT(HDRP(bp), PACK(size, 0));

    /** Update Footer */
    PUT(FTRP(bp), PACK(size, 0));

    /** Coalesce with adjancency free block if necessary */
    coalesce(bp);
    /** Checking for the heap correction */
    heapchecker(0, __LINE__);
}
/* $end mmfree */

/*
 * coalesce - Boundary tag coalescing. Return ptr to coalesced block
 */
static void *coalesce(void* bp){
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    /** Case 1: Both previous block and next block are allocated */
    if(prev_alloc && next_alloc)
        /** Nothing to do */
        return bp;
    /** Case 2: Previous block are allocated but the next block are freed */
    else if(prev_alloc && !next_alloc){
        /** Coalesce the current block with the previous block */
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        /** Update the header of current block */
        PUT(HDRP(bp), PACK(size, 0));
        /** Update the footer of next block */
        PUT(FTRP(bp), PACK(size, 0));
        /** Clear the footer of the current block(Optional) */
        // PUT(FTRP(bp), 0);
        /** Clear the footer of the next block(Optional) */
        // PUT(HDRP(NEXT_BLKP(bp)), 0);
    }
    /** Case 3: Previous block are freed, but the next block are allocated*/
    else if(!prev_alloc && next_alloc){
        /** Coalesce the current block with the next block */
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        /** Update the footer of current block */
        PUT(FTRP(bp), PACK(size, 0));
        /** Update the header of previous block */
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        /** Clear the header of the current block(Optional) */
        // PUT(HDRP(bp), 0);
        /** Clear the footer of the previous block(Optional) */
        // PUT(FTRP(PREV_BLKP(bp)), 0);
        /** Update the block pointer */
        bp = PREV_BLKP(bp);
    }
    /** Case 4: Both previous block and next block are freed*/
    else{
        /** Coalesce the current block with both of the previous block and the next block */
        size += (GET_SIZE(FTRP(NEXT_BLKP(bp))) + GET_SIZE(HDRP(PREV_BLKP(bp))));
         /** Update the header of previous block */
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        /** Update the footer of footer block */
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        /** Clear the header of the current block(Optional) */
        // PUT(HDRP(bp), 0);
        /** Clear the footer of the current block(Optional) */
        // PUT(FTRP(bp), 0);
        /** Clear the footer of the previous block(Optional) */
        // PUT(FTRP(PREV_BLKP(bp)), 0);
        /** Clear the header of the next block(Optional) */
        // PUT(HDRP(NEXT_BLKP(bp)), 0);
        /** Update the block pointer */
        bp = PREV_BLKP(bp);
    }

#ifdef NEXT_FIT
    /** Make sure rover isn't pointing to the free block */
    /** that we just coalesce */
    /** The current block is the nearest free block */
    if((rover > (char*)bp) && (rover < NEXT_BLKP(bp)))
        rover = bp;
#endif

    return bp;
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */

void *mm_realloc(void *ptr, size_t size)
{
    size_t oldsize;
    void *newptr;

    /* If size == 0 then this is just free, and we return NULL. */
    if(size == 0) {
        mm_free(ptr);
        return 0;
    }

    /* If oldptr is NULL, then this is just malloc. */
    if(ptr == NULL) {
        return mm_malloc(size);
    }

    newptr = mm_malloc(size);

    /* If realloc() fails the original block is left untouched  */
    if(!newptr) {
        return 0;
    }

    /* Copy the old data. */
    oldsize = GET_SIZE(HDRP(ptr));
    if(size < oldsize) oldsize = size;
    memcpy(newptr, ptr, oldsize);

    /* Free the old block. */
    mm_free(ptr);

    return newptr;
}

// void *mm_realloc(void *ptr, size_t size)
// {
//     void *oldptr = ptr;
//     void *newptr;
//     size_t copySize;
    
//     newptr = mm_malloc(size);
//     if (newptr == NULL)
//       return NULL;
//     copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
//     if (size < copySize)
//       copySize = size;
//     memcpy(newptr, oldptr, copySize);
//     mm_free(oldptr);
//     return newptr;
// }

/**
 * place - Place the request block at the beginning of the free block,
 *         splitting only if the size of the remainder would equal or exceeed the minimum block size
 */
/* $begin mmplace */
/* $begin mmplace-proto */
static void place(void* bp, size_t asize){
/* $end mmplace-proto */
    /** Get the block size */
    size_t block_size = GET_SIZE(HDRP(bp));
    
    /** The current block can be splitted
     *  if the remain block meet the requirement of the minimum block size
     *  after it allocates the space to the block we need 
     */
    if( (block_size - asize) >= ( 2 * DSIZE ) ){
        /** The block with asize as allocated */
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        /** Update the pointer to the newly free block after split */
        bp = NEXT_BLKP(bp);
        /** Update the header/footer of the newly free block */
        PUT(HDRP(bp), PACK( (block_size - asize), 0));
        PUT(FTRP(bp), PACK( (block_size - asize), 0));
    }
    /** Otherwise, don't need to split */
    else{
        /** The block with asize as allocated */
        PUT(HDRP(bp), PACK(block_size, 1));
        PUT(FTRP(bp), PACK(block_size, 1));
    }
}
/* $end mmplace */


/* 
 * extend_heap - Extend heap with free block by chunk size byte and 
 * creates the initial free block
 * return its block pointer
**/
/* $begin mmextendheap */
static void *extend_heap(size_t words){

    char* bp;

    /** Request the additional heap space from the  memory system */
    /** Allocate an even numbere of words to maintain alignment, */
    /** where it round up the request size to the nearest multiple of 2 words(8-bytes) */
    size_t size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;

    if((long)(bp = mem_sbrk(size)) == -1)
        return NULL;

    /** Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0)); /** Free Block Header */
    PUT(FTRP(bp), PACK(size, 0)); /** Free Footer Header */
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0,1)); /** New Epilogue Header */
    /** Coalesce if the previous was freed */
    return coalesce(bp);
}
/* $end mmextendheap */

/* 
 * find_fit - Find a fit for a block with asize bytes 
 */
/* $begin mmfirstfit */
/* $begin mmfirstfit-proto */
static void *find_fit(size_t asize)
/* $end mmfirstfit-proto */
{
#ifdef NEXT_FIT
    char* oldrover = rover;

    /** Search from the current rover to the end of the list */
    for(; GET_SIZE(HDRP(rover)) > 0; rover = NEXT_BLKP(rover))
        if(!(GET_ALLOC(HDRP(rover))) && (asize <= GET_SIZE(HDRP(rover))))
            return rover;

    /** If not search the free block starting at the current block */
    /** Search from the beginning of the list to the oldrover */
    for(rover = heap_listp; rover < oldrover; rover = NEXT_BLKP(rover))
        if(!(GET_ALLOC(HDRP(rover))) && (asize <= GET_SIZE(HDRP(rover))))
            return rover;

#else
    /* $begin mmfirstfit */
    /* First-fit search */
    void* bp;

    for(bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)){
        /** Find the suitable free list */
        if( !GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE( HDRP(bp) )) ){
            return bp;
        }
    }
#endif
    /** Not find any fittable free list */
    return NULL;
}
/* $end mmfirstfit */

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