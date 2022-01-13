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
// cache :5,1,5 5个组，每组一行，block=32字节
char transpose_submit_desc[] = "Transpose submission";
void solve_32(int M, int N, int A[N][M], int B[M][N]);
void solve_64(int M, int N, int A[N][M], int B[M][N]);
void solve_61(int M, int N, int A[N][M], int B[M][N]);
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    switch (M) {
        case 32:
            solve_32(M, N, A, B);
            break;
        case 64:
            solve_64(M, N, A, B);
            break;
        case 61:
            solve_61(M, N, A, B);
            break;
    }
}

void solve_32(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;
    // 这里为什么是8*8分块？因为 我们的cache的Block大小为32字节，也就是8*sizeof(int)
    //，一行访存八个正好达到了cache的极限了
    for (i = 0; i < 32; i += 8) {      // 枚举每八行
        for (j = 0; j < 32; j += 8) {  // 枚举每八列
            // 这里用这些临时(寄存器)变量，如果你查看过A和B的地址的话，你会发现A和B的地址差距是64的整数倍（0x40000），
            // 那么 直接赋值的话，在对角线的时候 每一个Load A[i][i]紧跟Store B[i][i],将造成比较多的
            // eviction
            for (int cnt = 0; cnt < 8; ++cnt, ++i) {  // 枚举0~8中的每一行，一行八列
                int temp1 = A[i][j];                  // 这八个只会发生一次miss
                int temp2 = A[i][j + 1];
                int temp3 = A[i][j + 2];
                int temp4 = A[i][j + 3];
                int temp5 = A[i][j + 4];
                int temp6 = A[i][j + 5];
                int temp7 = A[i][j + 6];
                int temp8 = A[i][j + 7];

                B[j][i] = temp1;  // 第一次 这八个都会 miss,后面就会命中，当然对角线有些例外
                B[j + 1][i] = temp2;
                B[j + 2][i] = temp3;
                B[j + 3][i] = temp4;
                B[j + 4][i] = temp5;
                B[j + 5][i] = temp6;
                B[j + 6][i] = temp7;
                B[j + 7][i] = temp8;
            }
            i -= 8;
        }
    }
}
// 这是4*4的优化版本，具体思想是，尽量多地利用整个cache
void solve_64(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i += 8) {      // 枚举每八行
        for (j = 0; j < M; j += 8) {  // 枚举每八列
            // 这里用这些临时变量，如果你查看过A和B的地址的话，你会发现A和B的地址差距是64的整数倍（0x40000），
            // 那么 直接赋值的话，在对角线的时候 每一个Load A[i][i]紧跟Store B[i][i],将造成比较多的
            // eviction
            int temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8, cnt;
            // 1块8*8，我们分成4块来做，每一块4*4
            // 这是 左上，且将左下的块移动到 B中的右上，这样是为了更高效地利用cache
            for (cnt = 0; cnt < 4; ++cnt, ++i) {  // 枚举0~8中的每一行，一行八列
                temp1 = A[i][j];  // 这样我们就一次取出了8个元素，我们的A的miss就只有一次了 原始4*4
                                  // 则是两次
                temp2 = A[i][j + 1];
                temp3 = A[i][j + 2];
                temp4 = A[i][j + 3];  // 左上

                temp5 = A[i][j + 4];  // 右上
                temp6 = A[i][j + 5];
                temp7 = A[i][j + 6];
                temp8 = A[i][j + 7];

                B[j][i] = temp1;  // 左上翻转
                B[j + 1][i] = temp2;
                B[j + 2][i] = temp3;
                B[j + 3][i] = temp4;

                B[j][i + 4] = temp5;  //将A中右上 存到B中的右上，这也是全部命中的
                B[j + 1][i + 4] = temp6;
                B[j + 2][i + 4] = temp7;
                B[j + 3][i + 4] = temp8;
            }
            i -= 4;
            // 处理A中左下
            for (cnt = 0; cnt < 4; ++j, ++cnt) {  // 枚举0~8中的每一行，一行八列
                temp1 = A[i + 4][j];
                temp2 = A[i + 5][j];
                temp3 = A[i + 6][j];
                temp4 = A[i + 7][j];  // 拿到左下的元素

                // 因为我们这里本来就要处理 右上，所以这不会带来更多的miss
                temp5 = B[j][i + 4];  // 拿到我们之前赋给B右上的，也就是A右上的元素
                temp6 = B[j][i + 5];
                temp7 = B[j][i + 6];
                temp8 = B[j][i + 7];

                B[j][i + 4] = temp1;  //将左下翻转到B右上
                B[j][i + 5] = temp2;
                B[j][i + 6] = temp3;
                B[j][i + 7] = temp4;

                // 这一步，也不会带来更多的miss因为 i + 4 <= j + 4 <= i + 7 恒成立
                // 所以每次带来的 eviction 和 store B[j][i +
                // 4]只会带来一次MISS，和原来的操作是一样的

                B[j + 4][i] = temp5;  //将原B右上 赋值到 B左下， 这样A右上也就完成了翻转
                B[j + 4][i + 1] = temp6;
                B[j + 4][i + 2] = temp7;
                B[j + 4][i + 3] = temp8;
            }
            j -= 4;
            j += 4;  // 处理第四块 右下
            for (i += 4, cnt = 0; cnt < 4; ++cnt, ++i) {
                temp1 = A[i][j];  // 第四块没有任何改动， 和原来效果是一样的
                temp2 = A[i][j + 1];
                temp3 = A[i][j + 2];
                temp4 = A[i][j + 3];

                B[j][i] = temp1;
                B[j + 1][i] = temp2;
                B[j + 2][i] = temp3;
                B[j + 3][i] = temp4;
            }
            i -= 8, j -= 4;
        }
    }
}
// 这个题我没有什么好的做法，因为大小是61*67了，所以A，B相同的组索引也不那么紧密了，所以我们可以不用考虑对角线的情况
// 不断地猜测不同的分块所带来的效果。直到找到最好的
void solve_61(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;
#define ex 17  // 17是我测出来最小的了 不过像 16,18，17，21这些也都是满分的
    for (i = 0; i < N; i += ex) {      // 枚举每ex行
        for (j = 0; j < M; j += ex) {  // 枚举每ex列
                                       // 下面开始转置这个ex * ex 的块
            for (int x = i; x < N && x < i + ex; ++x)
                for (int y = j; y < M && y < j + ex; ++y) B[y][x] = A[x][y];
        }
    }
}

void partB(int M, int N, int A[N][M], int B[M][N]) {}
void transpose_submit3(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i += 8) {      // 枚举每八行
        for (j = 0; j < M; j += 4) {  // 枚举每八列
            int temp1, temp2, temp3, temp4, t5, t6, t7, t8;
            for (int k = i; k < i + 8; ++k) {  // 枚举0~8中的每一行，一行八列

                if (j != 0 && k == i + 3) {
                    B[j][k] = t5;
                    B[j + 1][k] = t6;
                    B[j + 2][k] = t7;
                    B[j + 3][k] = t8;
                } else {
                    temp1 = A[k][j];
                    temp2 = A[k][j + 1];
                    temp3 = A[k][j + 2];
                    temp4 = A[k][j + 3];
                    B[j][k] = temp1;
                    B[j + 1][k] = temp2;
                    B[j + 2][k] = temp3;
                    B[j + 3][k] = temp4;
                }
                if (k == i + 3) {
                    t5 = A[k][j + 4];
                    t6 = A[k][j + 5];
                    t7 = A[k][j + 6];
                    t8 = A[k][j + 7];
                }
            }
        }
    }
}

void transpose_submit2(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i += 8) {               // 枚举每八行
        for (j = 0; j < M; j += 8) {           // 枚举每八列
            for (int k = i; k < i + 8; ++k) {  // 枚举0~8中的每一行，一行八列
                int temp1 = A[k][j];
                int temp2 = A[k][j + 1];
                int temp3 = A[k][j + 2];
                int temp4 = A[k][j + 3];
                int temp5 = A[k][j + 4];
                int temp6 = A[k][j + 5];
                int temp7 = A[k][j + 6];
                int temp8 = A[k][j + 7];
                if (k & 1) {
                    B[j][k] = temp1;
                    B[j + 1][k] = temp2;
                    B[j + 2][k] = temp3;
                    B[j + 3][k] = temp4;
                    B[j + 4][k] = temp5;
                    B[j + 5][k] = temp6;
                    B[j + 6][k] = temp7;
                    B[j + 7][k] = temp8;
                } else {
                    B[j + 4][k] = temp5;
                    B[j + 5][k] = temp6;
                    B[j + 6][k] = temp7;
                    B[j + 7][k] = temp8;
                    B[j][k] = temp1;
                    B[j + 1][k] = temp2;
                    B[j + 2][k] = temp3;
                    B[j + 3][k] = temp4;
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

    /* Register any additional transpose functions */
    // registerTransFunction(transpose_submit3, trans_desc);
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
