#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char **argv)
{
  int parent_pid = getpid();
  if (fork() == 0)
  {
  	sigsend(parent_pid,2);
  	exit();
  }
  else 
  	wait();

  if (fork() == 0)
  {
  	sigsend(parent_pid,10000002);
  	exit();
  }
  else
  	wait();
  exit();
}
