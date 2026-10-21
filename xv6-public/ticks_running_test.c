#include "types.h"
#include "stat.h"
#include "user.h"

void
busy_work(int iterations)
{
  volatile int i, j;
  for(i = 0; i < iterations; i++){
    for(j = 0; j < 100000; j++){
      ;
    }
  }
}

int
main(int argc, char *argv[])
{
  int my_pid = getpid();
  printf(1, "=== ticks_running() Test ===\n");

  // 1. Non-existent process
  printf(1, "1. Non-existent PID (9999): ticks_running(9999) = %d (expected -1)\n",
         ticks_running(9999));
  printf(1, "2. Invalid PID (-5): ticks_running(-5) = %d (expected -1)\n",
         ticks_running(-5));

  // 2. Calling process before and after CPU work
  int t_before = ticks_running(my_pid);
  printf(1, "3. Calling process (PID %d) initial ticks_running = %d\n", my_pid, t_before);

  busy_work(500);

  int t_after = ticks_running(my_pid);
  printf(1, "4. Calling process (PID %d) after busy work ticks_running = %d\n", my_pid, t_after);

  // 3. Other processes (init = 1, sh = 2) and child process
  printf(1, "5. Init process (PID 1) ticks_running = %d\n", ticks_running(1));
  printf(1, "6. Shell process (PID 2) ticks_running = %d\n", ticks_running(2));

  int child_pid = fork();
  if(child_pid == 0){
    busy_work(300);
    printf(1, "7. Child process (PID %d) ticks_running before exit = %d\n",
           getpid(), ticks_running(getpid()));
    exit();
  } else if(child_pid > 0){
    int child_initial = ticks_running(child_pid);
    printf(1, "6b. Child process (PID %d) immediately after fork ticks_running = %d\n",
           child_pid, child_initial);
    wait();
  }

  exit();
}

