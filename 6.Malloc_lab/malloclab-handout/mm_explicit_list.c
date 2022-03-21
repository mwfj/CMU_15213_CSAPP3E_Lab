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

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/** $begin mallocmacro **/

#define WSIZE     2       /** Word and header/footer size(bytes) **/
#define DSIZE     4       /** Double Word Size(bytes) **/
#define CHUNKSIZE (1<<12) /** Extend heay this amount(2048 bytes), a.k.a 2MB **/

#define MAX(x,y)  ((x) > (y) ? (x) : (y))

/* 
   Pack a size and allocated bit into a word, 
   which returns a value that can be stored in a header or footer
*/
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
#define FTRP(bp)           ((char*)(bp) + GET_SIZE(HDRP(bp) - DSIZE))

/* Given block ptr bp, compute address of next and previous blocks */

#define NEXT_BLKP(bp)      ((char*)(bp) + GET_SIZE((char*)(bp) - WSIZE))
#define PREV_BLKP(bp)      ((char*)(bp) - GET_SIZE((char*)(bp) - DSIZE))


/** $end mallocmacro **/

/* Global variables */
static char* heap_listp = 0; // Heap List Pointer: the pointer to the first block

/* Function prototypes for internal helper routines */
static void checkheap(int verbose);
static void checkblock(void *bp);
static void printblock(void* bp);

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    // printf("Init Implicit....\n");
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    int newsize = ALIGN(size + SIZE_T_SIZE);
    void *p = mem_sbrk(newsize);
    if (p == (void *)-1)
	return NULL;
    else {
        *(size_t *)p = size;
        return (void *)((char *)p + SIZE_T_SIZE);
    }
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}

/* 
 * mm_checkheap - Check the heap for correctness
 */
void mm_checkheap(int verbose)  
{ 
    checkheap(verbose);
}

/* 
 * printblock - Print block header/footer info
**/
static void printblock(void* bp){
    /* The size stored in header, the allocation bit in header */
    size_t hsize, halloc;
    /* The size stored in footer, the allocation bit in footer */
    size_t fsize, falloc;

    checkheap(0);
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
static void checkheap(int verbose){
    /* Initialize the payload pointer of the block */
    char* bp = heap_listp;
    /* Print the infomation */
    if(verbose)
        printf("Heap: (%p):\n", heap_listp);
    
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