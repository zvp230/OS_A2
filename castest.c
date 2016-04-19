#include "types.h"
#include "stat.h"
#include "user.h"
#include "x86.h"

int
main(int argc, char **argv)
{
  int* addr = malloc(sizeof(int));
  *addr = 5;
  int goodExpected = 5;
  int badExpected = 4;
  int newValue = 3;
  int result;
  result = cas(addr,badExpected,newValue);
  printf(1,"CAS with badExpected is : %d, Result: %d\n",*addr, result);
  result = cas(addr,goodExpected,newValue);
  printf(1,"CAS with goodExpected is : %d, Result: %d\n",*addr, result);
  exit();
}