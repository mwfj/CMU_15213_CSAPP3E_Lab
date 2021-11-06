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
#include <stdlib.h>
#include "cachelab.h"

// The block size for matrix transpose of 32x32 matrix
#define BLOCK_SIZE 8
// Used for 64x64 matrix transposition
#define SUB_BLOCK_SIZE 4
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
void trans_64_64(int M, int N, int A[N][M], int B[M][N]);
void check_transpose_wrapper(int M, int N, int A[N][M], int B[M][N]);
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
    // trans_naive(M, N, A, B);
   
    // 32 x 32 matrix transposition
    if(M == 32 && N == 32){

		// Comment this line when run the ./test-trans
		// Only use for debug
		// printf("Do the 32x32 matrix transposition ... \n");

		// Naive 32x32 matrix transposition
        // trans_32_32(M, N, A, B);

        trans_32_32_opt(M, N, A, B);

		// Comment this line when run the ./test-trans
		// Only use for debug
        // check_transpose_wrapper(M,N,A,B);
        return ;
    }else if(M == 64 && N == 64){
		// Comment this line when run the ./test-trans
		// Only use for debug
		// printf("Do the 64x64 matrix transposition ...\n");

        trans_64_64(M,N,A,B);

		// Comment this line when run the ./test-trans
		// Only use for debug
        // check_transpose_wrapper(M,N,A,B);
        return ;
    }else if(M == 61 && N == 67){
		printf("Do the 61x67 matrix transposition ...\n");
        printf("TODO ... \n");
    }else{
        printf("The input matrix size is not required for this lab\n");
        exit(EXIT_FAILURE);
    }
	// trans_64_64(M,N,A,B);
}

/**
 * A test program to transpose 8x8 matrix
 * by simply using block strategy.
 **/
char trans_naive_desc[] = "Matrix Transpose with block";
void trans_naive(int M, int N , int A[N][M], int B[M][N]){
    int i, j, bi, bj, tmp;
    for(i=0; i<N; i += BLOCK_SIZE)
        for(j=0; j<M; j+= BLOCK_SIZE)
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
    for(i=0; i<N; i+=BLOCK_SIZE)
        for(j=0; j<M; j+=BLOCK_SIZE)
            // Iterate inside of block
            for(k=i; k<i+BLOCK_SIZE; k++){
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
    for(i=0; i<N; i += BLOCK_SIZE)
        for(j=0; j<M;  j+= BLOCK_SIZE){

            // Iterate inside of block
            
            // Firstlt, copy the transpose elements from Matrix A to Matrix B
            for(bi = i, bj = j; bi < i + BLOCK_SIZE; bi++, bj++){
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
            for(bi = 0; bi < BLOCK_SIZE; bi++)
                for(bj = bi+1; bj < BLOCK_SIZE; bj++){
                    tmp0 = B[bi + j][bj + i];
                    B[bi + j][bj + i] = B[bj + j][bi + i];
                    B[bj + j][bi + i] = tmp0;
                }
            
        }

}

// without blocks-shifting and lazy-transposing, the expected cache miss is: 1176 = 35 * 8 + 16 * 56
// with block-shifting and lazy-transposing, it reaches theoratical limit: 1024 = 16 * 64
char trans_64_64_desc[] = "The 64x64 matrix transposition"; 
void trans_64_64(int M, int N, int A[N][M], int B[M][N]){
	// i = jj, j = ii, bi = i, bj = j
	// i and j are the outside loop index
	// whereas bi, bj are the inside loop index
    int i, j, bi, bj;
    int tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
    for(i = 0; i < N; i+= BLOCK_SIZE){
		// 1. Processing the block contains the diagonal element first

		// User the upper half to [ii,jj] to transpose the diagonal element [ii,ii]
		if(i == 0)
			j = BLOCK_SIZE;
		else
			j = 0;
		
		// Move the downside half 4x8 block from Matrix A to Matrix B
		// with the block-shifting
		for(bi = i; bi < i + SUB_BLOCK_SIZE; bi++){
			// Copy from Matrix A
			tmp0 = A[bi + SUB_BLOCK_SIZE][i+0];
			tmp1 = A[bi + SUB_BLOCK_SIZE][i+1];
			tmp2 = A[bi + SUB_BLOCK_SIZE][i+2];
			tmp3 = A[bi + SUB_BLOCK_SIZE][i+3];
			tmp4 = A[bi + SUB_BLOCK_SIZE][i+4];
			tmp5 = A[bi + SUB_BLOCK_SIZE][i+5];
			tmp6 = A[bi + SUB_BLOCK_SIZE][i+6];
			tmp7 = A[bi + SUB_BLOCK_SIZE][i+7];

			// Move to the matrix B from the local variables
			B[bi][j+0] = tmp0;
			B[bi][j+1] = tmp1;
			B[bi][j+2] = tmp2;
			B[bi][j+3] = tmp3;
			B[bi][j+4] = tmp4;
			B[bi][j+5] = tmp5;
			B[bi][j+6] = tmp6;
			B[bi][j+7] = tmp7;
		}

		
		// taking the transposition operation in the same position
		// for the 4x4 lower-left and lower-right blocks
		for(bi = 0; bi < SUB_BLOCK_SIZE; bi++)
			for(bj = bi+1; bj < SUB_BLOCK_SIZE; bj++){
				tmp0 = B[ i + bi ][ j + bj ];
				B[ i + bi ][ j + bj ] = B[ i + bj][ j + bi];
				B[ i + bj ][ j + bi ] = tmp0;

				tmp0 = B[ i + bi ][ j + bj + SUB_BLOCK_SIZE ];
				B[ i + bi ][ j + bj + SUB_BLOCK_SIZE ] = B[ i + bj ][ j + bi + SUB_BLOCK_SIZE];
				B[ i + bj ][ j + bi + SUB_BLOCK_SIZE] = tmp0;
			}

		// Move the upper-half 4x8 blocks from Matrix A to Matrix B
		for(bi = i; bi < i + SUB_BLOCK_SIZE; bi++){
			// Copy from the Matrix A 
			tmp0 = A[bi][ i + 0 ];
			tmp1 = A[bi][ i + 1 ];
			tmp2 = A[bi][ i + 2 ];
			tmp3 = A[bi][ i + 3 ];
			tmp4 = A[bi][ i + 4 ];
			tmp5 = A[bi][ i + 5 ];
			tmp6 = A[bi][ i + 6 ];
			tmp7 = A[bi][ i + 7 ];

			// Move to the Matrix B from the local variables
			B[bi][ i + 0 ] = tmp0;
			B[bi][ i + 1 ] = tmp1;
			B[bi][ i + 2 ] = tmp2;
			B[bi][ i + 3 ] = tmp3;
			B[bi][ i + 4 ] = tmp4;
			B[bi][ i + 5 ] = tmp5;
			B[bi][ i + 6 ] = tmp6;
			B[bi][ i + 7 ] = tmp7;
		}

		// taking the transposition operation in the same position
		// for the 4x4 upper-left and upper-right blocks
		for(bi = i; bi < i + SUB_BLOCK_SIZE; bi++)
			for(bj = bi+1; bj < i + SUB_BLOCK_SIZE; bj++){
				tmp0 = B[bi][bj];
				B[bi][bj] = B[bj][bi];
				B[bj][bi] =  tmp0;

				tmp0 = B[bi][ bj + SUB_BLOCK_SIZE ];
				B[bi][ bj + SUB_BLOCK_SIZE ] = B[bj][ bi + SUB_BLOCK_SIZE ];
				B[bj][ bi + SUB_BLOCK_SIZE ] = tmp0;
			}

		// swapping the lower-left block and the upper-right block
		for(bi = 0; bi < SUB_BLOCK_SIZE; bi++){
			// Lower-left
			tmp0 = B[i+bi][i+4];
			tmp1 = B[i+bi][i+5];
			tmp2 = B[i+bi][i+6];
			tmp3 = B[i+bi][i+7];

			// Transfer from Upper-right to Lower-left
			B[i+bi][i+4] = B[i+bi][j+0];
			B[i+bi][i+5] = B[i+bi][j+1];
			B[i+bi][i+6] = B[i+bi][j+2];
			B[i+bi][i+7] = B[i+bi][j+3];

			// Upper-right
			B[i+bi][j+0] = tmp0;
			B[i+bi][j+1] = tmp1;
			B[i+bi][j+2] = tmp2;
			B[i+bi][j+3] = tmp3;

		}

		// Filling the lower-part 4x8 block into its position in Matrix B
		for(bi = 0; bi < SUB_BLOCK_SIZE; bi++){
			B[i + bi + SUB_BLOCK_SIZE][i  + 0] = B[i + bi][j + 0];
			B[i + bi + SUB_BLOCK_SIZE][i  + 1] = B[i + bi][j + 1];
			B[i + bi + SUB_BLOCK_SIZE][i  + 2] = B[i + bi][j + 2];
			B[i + bi + SUB_BLOCK_SIZE][i  + 3] = B[i + bi][j + 3];
			B[i + bi + SUB_BLOCK_SIZE][i  + 4] = B[i + bi][j + 4];
			B[i + bi + SUB_BLOCK_SIZE][i  + 5] = B[i + bi][j + 5];
			B[i + bi + SUB_BLOCK_SIZE][i  + 6] = B[i + bi][j + 6];
			B[i + bi + SUB_BLOCK_SIZE][i  + 7] = B[i + bi][j + 7];
		}
		
		// 2.  we process the non-diagonal block

		for(j = 0; j < M; j+=BLOCK_SIZE){
		// iterator inside of the 8x8 blocks

			// skip the diagonal element
			if(i == j)
				continue;
			else{
				// Only process the non-diagonal one
				
				// Do the transpose operation at the same sub-block(4x4 block)
				// for upper-part(both of upper-left and upper-right)
				for(bi = j; bi < j + SUB_BLOCK_SIZE; bi++){
					tmp0 = A[bi][i + 0];
					tmp1 = A[bi][i + 1];
					tmp2 = A[bi][i + 2];
					tmp3 = A[bi][i + 3];

					tmp4 = A[bi][i + 4];
					tmp5 = A[bi][i + 5];
					tmp6 = A[bi][i + 6];
					tmp7 = A[bi][i + 7];

					B[i + 0][bi] = tmp0;
					B[i + 1][bi] = tmp1;
					B[i + 2][bi] = tmp2;
					B[i + 3][bi] = tmp3;

					B[i + 0][bi + SUB_BLOCK_SIZE ] = tmp4;
					B[i + 1][bi + SUB_BLOCK_SIZE ] = tmp5;
					B[i + 2][bi + SUB_BLOCK_SIZE ] = tmp6;
					B[i + 3][bi + SUB_BLOCK_SIZE ] = tmp7;
				}		

				// Do the transpose operation at the same sub-block(4x4 block)
				// for lower-part(both of lower-left and lower-right),
				// and also upper-right to the lower-left
				for(bj = i; bj < i + SUB_BLOCK_SIZE; bj++){
					// Lower-left in the Matrix A
					tmp0 = A[j+4][bj];
					tmp1 = A[j+5][bj];
					tmp2 = A[j+6][bj];
					tmp3 = A[j+7][bj];
					// Uppper-right block in the Matrix B
					tmp4 = B[bj][j+4];
					tmp5 = B[bj][j+5];
					tmp6 = B[bj][j+6];
					tmp7 = B[bj][j+7];

					B[bj][j+4] = tmp0;
					B[bj][j+5] = tmp1;
					B[bj][j+6] = tmp2;
					B[bj][j+7] = tmp3;

					B[bj + SUB_BLOCK_SIZE][j+0] = tmp4;
					B[bj + SUB_BLOCK_SIZE][j+1] = tmp5;
					B[bj + SUB_BLOCK_SIZE][j+2] = tmp6;
					B[bj + SUB_BLOCK_SIZE][j+3] = tmp7;
				}

				// Do the lazy-transposing operation in lower-right sub-block of the Matix B.
				for(bi = j + SUB_BLOCK_SIZE; bi < j + BLOCK_SIZE; bi++){
					tmp4 = A[bi][i + 4];
					tmp5 = A[bi][i + 5];
					tmp6 = A[bi][i + 6];
					tmp7 = A[bi][i + 7];

					B[i + 4][bi] = tmp4;
					B[i + 5][bi] = tmp5;
					B[i + 6][bi] = tmp6;
					B[i + 7][bi] = tmp7;
				}
			}
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

    /* Solution for 64x64 matrix */
    // registerTransFunction(trans_64_64, trans_64_64_desc);

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

// Debug information
// Check the correction of the matrix transpose
// by calling ./tracegen in shell
void check_transpose_wrapper(int M, int N, int A[N][M], int B[M][N]){
    // Check the correctness of the answer
    if(is_transpose(M, N, A, B))
        printf("Transpose Succeed!!\n");
    else{
        printf("Nah, wrong matrix transpose answer.\n.");
        printf("Double check your function.\n");
    }
}