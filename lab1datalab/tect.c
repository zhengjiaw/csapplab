#include <stdio.h>
int negate(int x) { return ~x + 1; }

int main()
#define equal(x, y) (!((x) ^ (y)))
{
    int y = 0x80000000, x = 0x7fffffff;
#define flagx ((x & 0x80000000) >> 31)
#define flagy ((y & 0x80000000) >> 31)
#define notE (!equal(flagx, flagy))

    printf("%d       \n", flagx);

    return 0;
}