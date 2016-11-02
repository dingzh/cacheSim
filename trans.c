/*
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

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

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
    int blockSize;
    int ii, jj, i, j;
    int tmp1,tmp2,tmp3,tmp4,tmp5,tmp6,tmp7,tmp8;
    switch(M) {        
        case 32:
            blockSize = 8;
            int diag = 0;
            int temp = 0;
            for (jj = 0; jj < N; jj += blockSize)
                for (ii = 0; ii < N; ii += blockSize)
                    for (i = ii; i < ii + blockSize; ++i) {
                        for (j = jj; j < jj + blockSize; ++j) {
                            if (i != j)
                                B[j][i] = A[i][j];
                             else {
                                temp = A[i][j];
                                diag = i;
                            }
                        }
                        if (ii == jj)
                            B[diag][diag] = temp;
                    }
            return;
        case 64:
            for (jj = 0; jj < 64; jj += 8){
                for (ii = 0; ii < 64; ii += 8){
                    for (i = ii; i < ii + 4; ++i){
                        tmp1 = A[i][jj];
                        tmp2 = A[i][jj+1];
                        tmp3 = A[i][jj+2];
                        tmp4 = A[i][jj+3];
                        tmp5 = A[i][jj+4];
                        tmp6 = A[i][jj+5];
                        tmp7 = A[i][jj+6];
                        tmp8 = A[i][jj+7];
                        B[jj][i] = tmp1;
                        B[jj+1][i] = tmp2;
                        B[jj+2][i] = tmp3;
                        B[jj+3][i] = tmp4;
                        B[jj][i+4] = tmp5;
                        B[jj+1][i+4] = tmp6;
                        B[jj+2][i+4] = tmp7;
                        B[jj+3][i+4] = tmp8;
                    }
                    for (j = jj; j < jj + 4; ++j){
                        tmp1 = B[j][ii+4];
                        tmp2 = B[j][ii+5];
                        tmp3 = B[j][ii+6];
                        tmp4 = B[j][ii+7];
                        tmp5 = A[ii+4][j];
                        tmp6 = A[ii+5][j];
                        tmp7 = A[ii+6][j];
                        tmp8 = A[ii+7][j];
                        B[j][ii+4] = tmp5;
                        B[j][ii+5] = tmp6;
                        B[j][ii+6] = tmp7;
                        B[j][ii+7] = tmp8;
                        B[j+4][ii] = tmp1;
                        B[j+4][ii+1] = tmp2;
                        B[j+4][ii+2] = tmp3;
                        B[j+4][ii+3] = tmp4;
                    }
                    /*third and fourth block*/
                    for (j = jj + 4;j < jj + 8; ++j){
                        tmp4 = A[ii+4][j];
                        tmp1 = A[ii+5][j];
                        tmp2 = A[ii+6][j];
                        tmp3 = A[ii+7][j];
                        B[j][ii+4] = tmp4;
                        B[j][ii+5] = tmp1;
                        B[j][ii+6] = tmp2;
                        B[j][ii+7] = tmp3;
                    }
                }
            }
            return;
        default:
            blockSize = 16;
            for(ii = 0; ii < N; ii += blockSize) {
                int iBound = ii + blockSize < N ? ii + blockSize : N;
                for(j = 0; j < M; ++j)
                    for(i = ii; i < iBound; ++i)
                        B[j][i] = A[i][j];
            }
            return;
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

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc);

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


