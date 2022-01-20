/*
 * CS:APP Data Lab
 *
 * <Please put your name and userid here>
 *
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting if the shift amount
     is less than 0 or greater than 31.


EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implement floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants. You can use any arithmetic,
logical, or comparison operations on int or unsigned data.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operations (integer, logical,
     or comparison) that you are allowed to use for your implementation
     of the function.  The max operator count is checked by dlc.
     Note that assignment ('=') is not counted; you may use as many of
     these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */

#endif
// 1
/*
 * bitXor - x^y using only ~ and &
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 1
 */
//异或：不是同时为0情况和不是同时为1的情况进行位与 , &两端同时取反就是|
int bitXor(int x, int y) { return ~(~x & ~y) & ~(x & y); }
/*
 * tmin - return minimum two's complement integer
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
#define INTMIN (0x80u << 24)  // 1个op
int tmin(void) { return INTMIN; }
// 2
/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *     and 0 otherwise
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 1
 */
// 两个ops
#define equal(x, y) (!((x) ^ (y)))
// 两个ops
#define toBool(x) (!!(x))
int negate(int x);  // 利用一下下面的函数，这样会更好看
int isTmax(int x)
{
    x = ~x;  // 如果是 Tmax 那么 为 0111... 取反之后为 1000.. 即Tmin
    // Tmin 有一个性质， 它的相反数 等于本身
    // 当然0的相反数 也是 0， 所以我们要排除 x为 -1（~-1 == 0） 的情况
    return equal(negate(x), x) & toBool(x);
}
/*
 * allOddBits - return 1 if all odd-numbered bits in word set to 1
 *   where bits are numbered from 0 (least significant) to 31 (most significant)
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
// 奇数为全为1， 我们只需要简单的&上 一个奇数位全为1的偶数位为全0数，之后判断相等即可
int allOddBits(int x)
{
    unsigned mask = 0xAA + (0xAA << 8);  // 因为常量被限制在 0 ~ 255 所以就这么写了
    mask = (mask << 16) + mask;
    return equal((x & mask), mask);
}
/*
 * negate - return -x
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
// 负数：逐位取反并加1  2个ops
int negate(int x) { return ~x + 1; }
// 3
/*
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0' to '9')
 *   Example: isAsciiDigit(0x35) = 1.
 *            isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
// 这个less_positive 只是一个简单的 '<' 因为我们不需要考虑 符号不同
#define less_positive(x, y) (toBool((((x) + negate(y)) & INTMIN)))  // 6个ops
//我们可以看到“十”位必须是3， 并且只需要“个”位 小于A即可
// 这里的小于运算 ，用了 a < b == a + (-b) < 0  , '!!'也可以换成 '>> 31'
int isAsciiDigit(int x) { return equal(x >> 4, 0x3) & less_positive(x & 0xf, 0xA); }

/*
 * conditional - same as x ? y : z
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
// 先将x转为bool， 然后求其nagate，因为-(1)的补码 全1， -(0）的补码为全0
// 我们转换出这两个特别的补码 就可以&了
int conditional(int x, int y, int z)  // 8个ops
{
    int flag = (negate(toBool(x)));
    return (flag & y) | (~flag & z);
}
/*
 * isLessOrEqual - if x <= y  then return 1, else return 0
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
// 在这个less中我们先完善一下不同符号比较即可， 并且 再 | equal就是 <= 了
inline static int less(int x, int y)
{
#define flagx ((x & INTMIN))  // 这里没有写 >> 31 主要是想省一个ops
#define flagy ((y & INTMIN))
#define notE (flagx ^ flagy)  // 如果符号不同就直接选符号为正的，否则less_positive
    return conditional(notE, flagx >> 31, less_positive(x, y));
}
int isLessOrEqual(int x, int y) { return less(x, y) | equal(x, y); }

// 4
/*
 * logicalNeg - implement the ! operator, using all of
 *              the legal operators except !
 *   Examples: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4
 */
// 0 则return 1， 否则 return 0
// 只有0和最小数的补码 为本身，其余数为其相反数
// 且 最小数的符号位为1,0的符号位为0，并且最小数右移31 为 -1
// 所以与相反数|之后 再>>31 之后+1为1的数x 只能是0，且其余数会返回 1
int logicalNeg(int x) { return ((x | (~x + 1)) >> 31) + 1; }

/* howManyBits - return the minimum number of bits required to represent x in
 *             two's complement
 *  Examples: howManyBits(12) = 5
 *            howManyBits(298) = 10
 *            howManyBits(-5) = 4
 *            howManyBits(0)  = 1
 *            howManyBits(-1) = 1
 *            howManyBits(0x80000000) = 32
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 90
 *  Rating: 4
 */
// 2分查找 不断细分，在里面比较难的是 用 x 值的 >>bit 来 代替if的思想
int howManyBits(int x)
{
    x = conditional(x & INTMIN, ~x, x);  //如果是 负数 就让其 为 ~x ,这样负数和正数操作就统一了
    int bit16 = (toBool((x >> 16)) << 4);  // 判断高16位是否有1
    x = x >> bit16;                        // 如果有，低16位则丢弃

    int bit8 = (toBool((x >> 8)) << 3);  // 判断高8位是否有1
    x = x >> bit8;                       // 如果有，低8位则丢弃

    int bit4 = (toBool((x >> 4)) << 2);  // 判断高4位是否有1
    x = x >> bit4;                       // 如果有，低4位则丢弃

    int bit2 = (toBool((x >> 2)) << 1);  // 判断高2位是否有1
    x = x >> bit2;                       // 如果有，低2位则丢弃

    int bit1 = (toBool((x >> 1)) << 0);  // 判断高1位是否有1
    x = x >> bit1;                       // 如果有，低1位则丢弃

    int bit0 = (toBool(x));
    return bit16 + bit8 + bit4 + bit2 + bit1 + bit0 + 1;
}
// float
/*
 * floatScale2 - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
//拆分这个整数出s,exo,m即可按照定义计算
//关于非规格数的比较详细的说明：https://www.jianshu.com/p/cb377fd1a295
unsigned floatScale2(unsigned uf)
{  // 拆分出 s, exp. m
    unsigned s = uf & (1 << 31);
    unsigned exp = (uf & 0x7f800000) >> 23;
    unsigned frac = uf & (~0xff800000);

    // 全0为非规格数 因为计算方法为 m * 2 ^ -126 ,所以m乘以2即可 , | s 保证符号相同
    if (exp == 0) return frac << 1 | s;
    if (exp == 255) return uf;  // 无穷或者NaN，乘以2都不变，所以直接返回
    exp++;
    if (exp == 255) return 0x7f800000 | s;  // 乘法不可能得到NaN，所以返回无穷

    return s | (exp << 23) | frac;
}
/*
 * floatFloat2Int - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
int floatFloat2Int(unsigned uf)
{
    // 分解
    unsigned s = uf & (1 << 31);
    int exp = (uf & 0x7f800000) >> 23;
    unsigned frac = uf & (~0xff800000);

    int E = exp - 127;
    if (exp == 255 || E > 31) return INTMIN;  // 题目要求NaN and infinity 和溢出 返回这个
    //  小于0为 小数，  可以舍入为 0
    if (E < 0) return 0;

    unsigned M = frac | (1 << 23);  // 1 + frac
    //因为M 本身的值应该是小数，但是在这里是一个23位的数， 相当于被左移了23位， 因此E要和23作差
    int V = (E > 23 ? M << (E - 23) : M >> (23 - E));
    if (s) V *= -1;  // 负数
    return V;
}
/*
 * floatPower2 - Return bit-level equivalent of the expression 2.0^x
 *   (2.0 raised to the power x) for any 32-bit integer x.
 *
 *   The unsigned value that is returned should have the identical bit
 *   representation as the single-precision floating-point number 2.0^x.
 *   If the result is too small to be represented as a denorm, return
 *   0. If too large, return +INF.
 *
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. Also if, while
 *   Max ops: 30
 *   Rating: 4
 */
// E = e - 127
// V= 1*2^E
// 指数范围是 -126 ~ 127 
// 根据浮点数表示，x就是指数表示
unsigned floatPower2(int x)
{
    if (x >= 128) return 0x7f800000;  // 超过128越界了
    if (x >= -126) return (x + 127) << 23; // -126在界内，直接放进e
    if (x >= -150)          //我们还有23位小数，也可以来表示一下
        return 1 << (x + 150);
    else
        return 0;
}
