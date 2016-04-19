#include "types.h"
#include "stat.h"
#include "user.h"


void signal_handler(int a, int b)
{
	printf(1,"Sender pid: %d\n", a);
  	printf(1,"Value: %d\n", b);
  	return;
}	

int
main(int argc, char **argv)
{
  int parent_pid = getpid();

  // set signal_handler as this process handler
  sigset(signal_handler);
  if(fork() == 0)
  {
    printf(1,"Before sleep().\n");
    sleep(100);
    printf(1,"Finished sleeping!.\n");
  	sigsend(parent_pid,1658008);
    exit();
  }
  
  printf(1,"Before sigpause().\n");
  sigpause();
  
  printf(1,"After sigpause().\n");
  wait();
  exit();
}
