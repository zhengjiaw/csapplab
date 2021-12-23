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
  3. Has unpredictable behavior when shifting an integer by more
     than the word size.

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

For the problems that require you to implent floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants.

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
  2. Each function has a maximum number of operators (! ~ & ^ | + << >>)
     that you are allowed to use for your implementation of the function. 
     The max operator count is checked by dlc. Note that '=' is not 
     counted; you may use as many of these as you want without penalty.
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

//异或：不是同时为0情况和不是同时为1的情况进行位与
int bitXor(int x, int y) { return ~(~x & ~y) & ~(x & y); }
/*
 * tmin - return minimum two's complement integer
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
// 2 ^ 31
#define INTMIN (0x80u << 24)  // 1个op
int tmin(void) { return INTMIN; }
// 2
/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *     and 0 otherwise
 *   Legalfff ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 2
 */
// 两个ops
#define equal(x, y) (!((x) ^ (y)))
// 两个ops
#define toBool(x) (!!(x))

int negate(int x);  // 利用一下下面的函数，这样会更好看
//  根据运算规则，补码最大值就是 补码最小值 - 1
int isTmax(int x)
{
    x = ~x;  // 如果是 Tmax 那么 为 0111... 取反之后为 1000.. 即Tmin
    // Tmin 有一个性质， 它的相反数 等于本身
    // 当然0的相反数 也是 0， 所以我们要排除 x为 -1（~-1 == 0） 的情况
    return equal(negate(x), x) & toBool(x);
}

/*
 * allOddBits - return 1 if all odd-numbered bits in word set to 1
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
// 奇数为全为1， 我们只需要简单的&上 一个奇数位全为1的数，之后判断相等即可
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
#define notE (flagx ^ flagy)
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

// 0 则return 1， 否则 return 0， 用conditional实现即可
int logicalNeg(int x) { return conditional(x, 0, 1); }

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
    return 0;
}
// float
/*
 * float_twice - Return bit-level equivalent of expression 2*f for
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
unsigned float_twice(unsigned uf)
{
    // 拆分出 s, exp. m
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
 * float_i2f - Return bit-level equivalent of expression (float) x
 *   Result is returned as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point values.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
// 实现 float(int x) , 并且以unsigned返回

// 这题仍然从定义出发 ： V = (-1) ^s * M * 2 ^ E, E = e - Bias(127)
// 所以我们可以先求 s，然后将其转换为 unsigned，
// 再求 E， 然后 就可以 e = E + Bias 求出e
// 再去掉最高位 ， 因为 M = frac + 1， 这里面的1 就包括了最高位
// 最后呢 x中剩下的 E位实际上就是 M
// 这个题比较难的地方在于舍入 我参考了他的写法：https://zhuanlan.zhihu.com/p/106109635
unsigned float_i2f(int x)
{
    if (!x) return 0;        // 0就直接返回
    unsigned ux = x, s = 0;  // 初始化为正数的情况
    if (x & INTMIN) {        // 考虑x 是 负数
        ux = negate(x);
        s = INTMIN;
    }
    //统计有几位
    unsigned E = 0;
    for (unsigned i = ux; i; E++) i >>= 1;
    E--;  // 因为最后一个i是0 代表是不满足的， 即多求了一位，所以减去

    ux = ux & (~(1 << (E)));  //去掉最高位
    unsigned e = E + 127;     //计算e的值
    //对尾数进行移位

    if (E <= 23) {
        ux = ux << (23 - E);  //尾数位数小于等于23的，直接将其移到顶
    } else {                  //尾数位数大于23的，要进行截断，需要考虑舍入问题
        unsigned count = 0;   // 统计>24 位的1的个数 ， 便于舍入
        while (E > 24) {      // 大于 24的都舍去
            if (ux & 0x01) count++;
            ux >>= 1;
            E--;
        }
        unsigned mask = ux & 0x01;
        ux = ux >> 1;
        if (mask) {
            if (count)  // 向最近的数舍入
                ux++;
            else if (ux & 0x01)  // 第23位是1，++之后可以得到偶数， 所以++
                ux++;
        }
        if (ux >> 23) {  //进位造成多一位
            e++;
            ux &= 0x7FFFFF;  //(~(1<<23)); //去掉最高位
        }
    }

    return s | (e << 23) | ux;
}

/*
 * float_f2i - Return bit-level equivalent of expression (int) f
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
//实现 (int)(float uf) 并且以int 形式返回
// 这题看上去就像上一题的逆运算， 仍然是从定义出发： V = (-1) ^s * M * 2 ^ E,
// uf中有 ：s e frac ，
// 再利用 E = e - Bias(127), M = 1 + frac之后运算即可返回答案
int float_f2i(unsigned uf)
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
