/* 
 * @author : Wufangjie Ma
 *
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

// The block size for matrix transpose of 32x32 matrix
#define BLOCK_SIZE_32 8
/**
 * 
 * Structure for block in Cache line:
 *      +-----------+----------+---------------+
 *      + Valid Bit +  Tag Bit +  Cache Block  + 
 *      +-----------+----------+---------------+    
 *
 *
 * Address of word that CPU send to Cache: 64bit 
 *      +-----------+------------+---------------+
 *      + Tag Bit   +  Set Index +  Block Offset +
 *      +-----------+------------+---------------+
 * 
 * The cache memory layout for this program: 
 * 
 *                                                2^5 = 32 bytes per cache block
 *                                                       4 bytes per block
 *                                                               /\
 *                                       /-----------------------  ----------------------\
 *              / +-----------+----------+-----------------------------------------------+
 *             |  + Valid Bit + Tag Bit  +  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  | 
 *             |  +-----------+----------+-----------------------------------------------+ 
 *             |  + Valid Bit + Tag Bit  +  8  |  9  |  10 |  11 |  12 |  13 |  14 |  15 |
 *             |  +-----------+----------+-----------------------------------------------+
 *             |  + Valid Bit + Tag Bit  +  16 |  17 |  18 |  19 |  20 |  21 |  22 |  23 |
 *             |  +-----------+----------+-----------------------------------------------+
 * 2^5 = 32   /   | ...         ...      26 lines omitted here     ...      ...      ... | 
 *   sets     \   +-----------+----------+-----------------------------------------------+
 *             |  + Valid Bit + Tag Bit  +  232|  233|  234|  235|  236|  237|  238|  239| 
 *             |  +-----------+----------+-----------------------------------------------+
 *             |  + Valid Bit + Tag Bit  +  240|  241|  242|  243|  244|  245|  246|  247|
 *             |  +-----------+----------+-----------------------------------------------+
 *             |  + Valid Bit + Tag Bit  +  248|  249|  250|  251|  252|  253|  254|  255|
 *              \ +-----------+----------+-----------------------------------------------+
 *                 \----------------------------------  ---------------------------------/
 *                                                    \/
 *                                             1 cache line per set 
 * 
**/


static inline int min(int a, int b){
    return (a<b) ? a : b;
}

int is_transpose(int M, int N, int A[N][M], int B[M][N]);
void trans_naive(int M, int N , int A[N][M], int B[M][N]);
void trans_32_32(int M, int N, int A[N][M], int B[M][N]);
void trans_32_32_opt(int M, int N, int A[N][M], int B[M][N]);
/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    // 32 x 32 matrix transposition
    // trans_naive(M, N, A, B);
    // trans_32_32(M, N, A, B);
    trans_32_32_opt(M, N, A, B);

    // Check the correctness of the answer
    if(is_transpose(M, N, A, B))
        printf("Transpose Succeed!!\n");
    else{
        printf("Nah, wrong matrix transpose answer.\n.");
        printf("Double check your function.\n");
    }
}

/**
 * A test program to transpose 8x8 matrix
 * by simply using block strategy.
 **/
char trans_naive_desc[] = "Matrix Transpose with block";
void trans_naive(int M, int N , int A[N][M], int B[M][N]){
    int i, j, bi, bj, tmp;
    for(i=0; i<N; i += BLOCK_SIZE_32)
        for(j=0; j<M; j+= BLOCK_SIZE_32)
            // Iterate inside of block
            for(bi = i; bi < min(i + 8, N); bi++)
                for(bj = j; bj < min(j+8, M); bj++){
                    tmp = A[bi][bj];
                    B[bj][bi] = tmp;
                }

}

/**
 * Reduce the cache miss by using the local variable
**/
char trans_32_32_desc[] = "32X32 matrix transposition using local variable";
void trans_32_32(int M, int N, int A[N][M], int B[M][N]){
    int i,j,k;
    for(i=0; i<N; i+=BLOCK_SIZE_32)
        for(j=0; j<M; j+=BLOCK_SIZE_32)
            // Iterate inside of block
            for(k=i; k<i+BLOCK_SIZE_32; k++){
                // Save the elements from the current matrix row
                // to register
                int tmp0 = A[k][j];
                int tmp1 = A[k][j+1];
                int tmp2 = A[k][j+2];
                int tmp3 = A[k][j+3];
                int tmp4 = A[k][j+4];
                int tmp5 = A[k][j+5];
                int tmp6 = A[k][j+6];
                int tmp7 = A[k][j+7];

                // Get the element from A[k][*] from register
                // and copy it to Matrix B
                B[j][k] = tmp0;
                B[j+1][k] = tmp1;
                B[j+2][k] = tmp2;
                B[j+3][k] = tmp3;
                B[j+4][k] = tmp4;
                B[j+5][k] = tmp5;
                B[j+6][k] = tmp6;
                B[j+7][k] = tmp7;
            }
}

/**
 * The optimize way to reduce the cache miss in 32 x 32 matrix
**/
char trans_32_32_opt_desc[] = "The optimize way to do the 32x32 matrix transposition";
void trans_32_32_opt(int M, int N, int A[N][M], int B[M][N]){

    int i,j,bi,bj;
    int tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
    for(i=0; i<N; i += BLOCK_SIZE_32)
        for(j=0; j<M;  j+= BLOCK_SIZE_32){

            // Iterate inside of block
            
            // Firstlt, copy the transpose elements from Matrix A to Matrix B
            for(bi = i, bj = j; bi < i + BLOCK_SIZE_32; bi++, bj++){
                // Save the elements from the current matrix row
                // to register
                tmp0 = A[bi][j];
                tmp1 = A[bi][j+1];
                tmp2 = A[bi][j+2];
                tmp3 = A[bi][j+3];
                tmp4 = A[bi][j+4];
                tmp5 = A[bi][j+5];
                tmp6 = A[bi][j+6];
                tmp7 = A[bi][j+7];
                // Copy the element to matrix by row
                // which is the access order of matrix A
                B[bj][i] = tmp0;
                B[bj][i+1] = tmp1;
                B[bj][i+2] = tmp2;
                B[bj][i+3] = tmp3;
                B[bj][i+4] = tmp4;
                B[bj][i+5] = tmp5;
                B[bj][i+6] = tmp6;
                B[bj][i+7] = tmp7;
            }

            // Then do the transpose operation inside of matrix B
            for(bi = 0; bi < BLOCK_SIZE_32; bi++)
                for(bj = bi+1; bj < BLOCK_SIZE_32; bj++){
                    tmp0 = B[bi + j][bj + i];
                    B[bi + j][bj + i] = B[bj + j][bi + i];
                    B[bj + j][bi + i] = tmp0;
                }
            
        }

}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register the naive block matrix transpose */
    // registerTransFunction(trans_naive, trans_naive_desc);
    /* Solution for 32x32 matrix */
    // registerTransFunction(trans_32_32, trans_32_32_desc);

    /* Register any additional transpose functions */
    // registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

