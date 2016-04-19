#include "types.h"
#include "stat.h"
#include "user.h"

int is_prime(int number)
{

    if (number == 2 || number == 3)
        return 0;

    if (number % 2 == 0 || number % 3 == 0)
        return 0;

    int divisor = 6;
    while (divisor * divisor - 2 * divisor + 1 <= number)
    {

        if (number % (divisor - 1) == 0)
            return 0;

        if (number % (divisor + 1) == 0)
            return 0;

        divisor += 6;

    }

    return 1;

}


int next_prime(int number)
{

    while (!is_prime(++number)) 
    { }
    return number;

}

/*
 * Receives value from primsrv
 * if value == 0 then 
 *  print "worker <pid> exit" and exits
 * else 
 *  find first prime number larger then value and sends it to primsrv and waits for next signal
 */
void signal_handler_worker(int sender_pid, int value)
{
  if (value == 0)
    printf(1, "worker %d exit.\n", getpid());
  else
  {
    int res = next_prime(value);
    sigsend (sender_pid, res);
    sigpause();
  }
  return;
}	

/*
 * Receives value from worker
 * if value == 0 then 
 *  sends 0 to all workers wait for them to terminate and exit.
 * else 
 *  prints "worker <worker pid> returned <p> as a result for <value>"
 */

void signal_handler_primsrv(int sender_pid, int value)
{ 
  if (value == 0)
    printf(1, "\n");
  else
  {
    printf(1, "worker %d returned %d as a result for %d\n", getpid(), value, value);
  }
  return;
} 




int
main(int argc, char **argv)
{ 
  if (argc < 2)
  {
    printf(1,"Useage: primsrv integer.\n");
    exit();
  }

  int n = atoi(argv[1]);
  int counter = 0;
  int pids_arr[n];

  sigset(signal_handler_primsrv);
  printf(1,"Workers pids:\n");

  while (counter < n)
  {
    pids_arr[counter] = fork();
    if (pids_arr[counter] == 0)
    {
      sigset(signal_handler_worker);
      sigpause();
      exit();
    }

    printf(1,"%d\n",pids_arr[counter++]);
  }

  char* buf = malloc (32);
  int input;
  while(1)
  {
    printf(1,"Please enter a number: ");
    gets(buf, 32);
    input = atoi(buf);
    counter = 0;
    if (input == 0)
    {
      printf(1,"input == 0 case.\n");
    }
    else
    {
      printf(1,"input != 0 case.\n");
      while(sigsend(pids_arr[counter++], input) == -1 && counter < n)
      {
        
      }
      if (counter == n)
        printf(1,"No idle workers!.\n");
    }
  }

  exit();
}