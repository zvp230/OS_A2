#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char **argv)
{
  int result;
  void signalhandler(int a, int b)
  {
  	printf(1,"It works!\n");
  }	
   result = (int)sigset(signalhandler);
   printf(1,"Deafult signal handler is: %d\n",result);
   result = (int)sigset(signalhandler);
   printf(1,"new signal handler is: %d\n",result);
   ((sig_handler)result)(0,0);

   exit();
}