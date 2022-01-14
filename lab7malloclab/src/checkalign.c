/* 
 * checkalign.c - determines the alignment of libc malloc 
 */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1<<16
#define min(x,y) (x < y ? x : y) 

int main() 
{
  int i, minalign = 16;
  char *p;

  for (i=0; i<SIZE; i++) {
    p = (char *)malloc(i);
    if ((int)p % 8 == 0)
      minalign = min(8, minalign);
    else if ((int)p % 4 == 0)
      minalign = min(4, minalign);
    else if ((int)p % 2 == 0)
      minalign = min(2, minalign);
    free(p);
  }
  printf("libc malloc uses %d-byte alignment\n", minalign);
  exit(0);
}
