#include "types.h"
#include "stat.h"
#include "user.h"

#define NUM_CHILDREN 4

struct child_result {
  int child_idx;
  int pid;
  int job_len;
  int ticks;
  int start_up;
  int end_up;
};

void
cpu_work(int loops)
{
  volatile int i, j;
  for(i = 0; i < loops; i++){
    for(j = 0; j < 300000; j++){
      ;
    }
  }
}

int
main(int argc, char *argv[])
{
  int i;
  int my_pid = getpid();
  int res_pipe[2];
  int start_pipe[2];
  char go = 'G';

  if(pipe(res_pipe) < 0 || pipe(start_pipe) < 0){
    printf(2, "pipe failed\n");
    exit();
  }

  printf(1, "=== Simple Scheduler (SJF) Test ===\n");
  printf(1, "1. Non-existent PID (9999): sjf_job_length(9999) = %d (expected -1)\n",
         sjf_job_length(9999));
  printf(1, "2. Sleeping process (init PID 1): sjf_job_length(1) = %d (expected -1)\n",
         sjf_job_length(1));
  printf(1, "3. Calling process (PID %d, RUNNING): sjf_job_length(%d) = %d\n",
         my_pid, my_pid, sjf_job_length(my_pid));

  printf(1, "\nForking %d child processes to observe SJF scheduling:\n", NUM_CHILDREN);

  for(i = 0; i < NUM_CHILDREN; i++){
    int pid = fork();
    if(pid == 0){
      close(res_pipe[0]);
      close(start_pipe[1]);

      // Wait until parent has forked all children
      char ch;
      read(start_pipe[0], &ch, 1);
      close(start_pipe[0]);

      int cpid = getpid();
      int len = sjf_job_length(cpid);
      int start_up = uptime();

      cpu_work(500);

      int end_up = uptime();
      int ticks = ticks_running(cpid);

      struct child_result r;
      r.child_idx = i;
      r.pid = cpid;
      r.job_len = len;
      r.ticks = ticks;
      r.start_up = start_up;
      r.end_up = end_up;

      write(res_pipe[1], &r, sizeof(r));
      close(res_pipe[1]);
      exit();
    }
  }

  close(res_pipe[1]);
  close(start_pipe[0]);

  // Release all children simultaneously into the RUNNABLE queue
  for(i = 0; i < NUM_CHILDREN; i++){
    write(start_pipe[1], &go, 1);
  }
  close(start_pipe[1]);

  // Read results in the exact order children finish and print cleanly from parent
  for(i = 0; i < NUM_CHILDREN; i++){
    struct child_result r;
    if(read(res_pipe[0], &r, sizeof(r)) == sizeof(r)){
      printf(1, "  [Child %d | PID %d] sjf_job_length=%d | ticks_running=%d | start_uptime=%d | finish_uptime=%d\n",
             r.child_idx, r.pid, r.job_len, r.ticks, r.start_up, r.end_up);
    }
  }
  close(res_pipe[0]);

  for(i = 0; i < NUM_CHILDREN; i++){
    wait();
  }

  printf(1, "=== SJF Test Complete ===\n");
  exit();
}
