#include "types.h"
#include "stat.h"
#include "user.h"

#define NUM_CHILDREN 6

struct child_result {
  int child_idx;
  int pid;
  int priority;
  int ticks;
  int start_up;
  int end_up;
};

void
cpu_work(int loops)
{
  volatile int i, j;
  for(i = 0; i < loops; i++){
    for(j = 0; j < 250000; j++){
      ;
    }
  }
}

int
main(int argc, char *argv[])
{
  int i;
  int my_pid = getpid();
  int quantum = 4;
  int res_pipe[2];
  int ready_pipe[2];
  int start_pipe[2];
  char token = 'R';

  // Optional command-line argument to set custom RR time quantum
  if(argc > 1){
    int q = atoi(argv[1]);
    if(q > 0)
      quantum = q;
  }
  set_sched_quantum(quantum);

  printf(1, "=== Advanced Scheduler (PRIORITYRR) Test (Quantum = %d ticks) ===\n", quantum);
  printf(1, "1. Non-existent PID (9999): get_sched_priority(9999) = %d (expected -1)\n",
         get_sched_priority(9999));
  printf(1, "2. Sleeping process (init PID 1): get_sched_priority(1) = %d (expected -1)\n",
         get_sched_priority(1));
  printf(1, "3. Calling process (PID %d) default priority: get_sched_priority(%d) = %d (expected 2)\n",
         my_pid, my_pid, get_sched_priority(my_pid));

  set_sched_priority(1);
  printf(1, "4. Calling process (PID %d) after set_sched_priority(1): get_sched_priority(%d) = %d (expected 1)\n",
         my_pid, my_pid, get_sched_priority(my_pid));

  if(pipe(res_pipe) < 0 || pipe(ready_pipe) < 0 || pipe(start_pipe) < 0){
    printf(2, "pipe failed\n");
    exit();
  }

  // Assign priorities across 3 levels (1 = High, 2 = Medium, 3 = Low)
  // Two processes per priority level to demonstrate RR within the same priority level
  int priorities[NUM_CHILDREN] = {3, 3, 2, 2, 1, 1};

  printf(1, "\nForking %d children across 3 priority levels (1=High, 2=Med, 3=Low):\n", NUM_CHILDREN);

  for(i = 0; i < NUM_CHILDREN; i++){
    int pid = fork();
    if(pid == 0){
      close(res_pipe[0]);
      close(ready_pipe[0]);
      close(start_pipe[1]);

      set_sched_priority(priorities[i]);

      // Notify parent that priority is set and child is ready
      write(ready_pipe[1], &token, 1);
      close(ready_pipe[1]);

      // Wait for parent's simultaneous start signal
      char go;
      read(start_pipe[0], &go, 1);
      close(start_pipe[0]);

      int cpid = getpid();
      int prio = get_sched_priority(cpid);
      int start_up = uptime();

      cpu_work(400);

      int end_up = uptime();
      int ticks = ticks_running(cpid);

      struct child_result r;
      r.child_idx = i;
      r.pid = cpid;
      r.priority = prio;
      r.ticks = ticks;
      r.start_up = start_up;
      r.end_up = end_up;

      write(res_pipe[1], &r, sizeof(r));
      close(res_pipe[1]);
      exit();
    }
  }

  close(res_pipe[1]);
  close(ready_pipe[1]);
  close(start_pipe[0]);

  // Wait until all children have set their priorities
  for(i = 0; i < NUM_CHILDREN; i++){
    char c;
    read(ready_pipe[0], &c, 1);
  }
  close(ready_pipe[0]);

  // Release all children into the RUNNABLE queue
  for(i = 0; i < NUM_CHILDREN; i++){
    write(start_pipe[1], &token, 1);
  }
  close(start_pipe[1]);

  // Read and print results in order of completion
  for(i = 0; i < NUM_CHILDREN; i++){
    struct child_result r;
    if(read(res_pipe[0], &r, sizeof(r)) == sizeof(r)){
      printf(1, "  [Child %d | PID %d] priority=%d | ticks_running=%d | start_uptime=%d | finish_uptime=%d\n",
             r.child_idx, r.pid, r.priority, r.ticks, r.start_up, r.end_up);
    }
  }
  close(res_pipe[0]);

  for(i = 0; i < NUM_CHILDREN; i++){
    wait();
  }

  printf(1, "=== PRIORITYRR Test Complete ===\n");
  exit();
}

