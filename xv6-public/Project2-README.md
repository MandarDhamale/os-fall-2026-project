# COP 6611 - Project 2: System Calls and Schedulers in xv6

## 1. Group Members
* **Members:** Mandar Dhamale, Nadika Poudel, Sakina Rao
* **Contact Email:** mandardhamale@usf.edu
* **Group Status:** **Same group members as Project 1.**

---

## 2. System & Development Environment
* **Operating System:** Ubuntu 24.04 LTS (64-bit Linux, Kernel 6.x)
* **Processor / Architecture:** x86_64 host processor compiling 32-bit x86 (`elf_i386`) target binaries for xv6
* **Compiler & Toolchain:** GNU `gcc` with `-m32 -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall` (`gcc-multilib`)
* **Emulator:** QEMU (`qemu-system-i386`) configured with 2 virtual CPUs (`-smp 2`) and 512 MB RAM
* **Build System:** GNU Make (`make`)
* **Special Setup / Compilation Steps:**
  * Installed 32-bit cross-compilation libraries (`gcc-multilib`) and `qemu-system-x86`.
  * Updated the `Makefile` to accept the `SCHEDULER` compile-time variable (`DEFAULT`, `SJF`, or `PRIORITYRR`, supporting both `SCHEDULER=SJF` and `SCHEDULER=[SJF]` syntax) and added an automatic `.scheduler_$(SCHED_CLEAN)` dependency file so switching schedulers automatically rebuilds `proc.o`, `trap.o`, `kernel`, and `xv6.img`.

---

## 3. Implementation Approach & System Calls Overview

### Part 1: User-Space Utilities (`sort` and `grep`)
1. **`sort` (`sort.c`):**
   * Added `_sort\` to `UPROGS` in `Makefile`.
   * Supports reading from one or more file arguments or from standard input (`fd = 0`) when used in a pipeline (e.g., `cat filename | sort`).
   * Parses flags `-r` (reverse order), `-n` (numerical sort via `parse_num()` supporting optional leading whitespace and `+`/`-` signs), and `-u` (unique lines only), supporting both individual flags (`-r -n`) and combined flags (`-rnu`).
2. **`grep` (`grep.c`):**
   * Enhanced xv6's existing regular expression matcher (`^`, `.`, `*`, `$`) with flag parsing for `-i` (case-insensitive matching via `char_eq()` and `tolower_char()`), `-n` (1-indexed line number prefix `%d:`), and `-v` (invert match: prints lines when `(matched && !flag_v) || (!matched && flag_v)`).
   * Supports combined flags (e.g., `grep -in`, `grep -inv`) as well as standard input piping (`cat filename | grep pattern`).

### Part 2: Tracking Scheduled Time (`ticks_running`)
* **Kernel State (`proc.h`, `proc.c`, `trap.c`):**
  * Added `int ticks_running;` to `struct proc` in `proc.h`.
  * Initialized `p->ticks_running = 0` in `allocproc()` when a process is created (`EMBRYO`) and reset it in `wait()` when a `ZOMBIE` slot is reclaimed.
  * In `trap.c`, whenever a timer interrupt (`T_IRQ0 + IRQ_TIMER`) fires while a process is actively `RUNNING` (`myproc() && myproc()->state == RUNNING`), `myproc()->ticks_running++` is incremented before yielding.
* **System Call (`ticks_running(int pid)` — Syscall #23):**
  * Scans `ptable.proc` while holding `ptable.lock`:
    * Returns `p->ticks_running` ($\ge 0$) if an active process (`p->state != UNUSED`) matches `pid`.
    * Returns `0` if the process exists in `ptable` but has not accumulated any scheduled timer ticks yet.
    * Returns `-1` if `pid <= 0` or no process with `pid` exists in `ptable`.
* **Test Program:** `ticks_running_test.c` (`_ticks_running_test`).

### Part 3: Simple Scheduler — Shortest Job First (`SJF`)
* **Compile-Time Flag:** `make qemu-nox SCHEDULER=SJF` (or `SCHEDULER=DEFAULT` for default).
* **Global Constants & Job Length Prediction (`proc.h`, `proc.c`):**
  * Defined global constants `const int SJF_MIN_LENGTH = 10;` and `const int SJF_MAX_LENGTH = 100;` in `proc.c` (declared `extern` in `proc.h`).
  * Implemented a Linear Congruential Generator `rand()` in `proc.c` to assign a varied predicted job length `p->job_length = SJF_MIN_LENGTH + (rand() % (SJF_MAX_LENGTH - SJF_MIN_LENGTH + 1))` when a process is allocated in `allocproc()`.
* **Scheduler Logic (`scheduler()` in `proc.c`):**
  * Under `#ifdef SCHED_SJF`, the scheduler acquires `ptable.lock`, scans all `RUNNABLE` processes in `ptable.proc`, selects `shortest_p` with the minimum `p->job_length`, and context-switches to it.
* **System Call (`sjf_job_length(int pid)` — Syscall #24):**
  * Returns `p->job_length` if a process with `pid` exists and is currently in the scheduling queue (`p->state == RUNNABLE || p->state == RUNNING`).
  * Returns `-1` if the process does not exist or is not in the scheduling queue (e.g., `SLEEPING`, `EMBRYO`, or `ZOMBIE`).
* **Test Program:** `simple_scheduler_test.c` (`_simple_scheduler_test`).

### Part 4: Advanced Scheduler — Multi-Level Priority Round-Robin (`PRIORITYRR`)
* **Compile-Time Flag:** `make qemu-nox SCHEDULER=PRIORITYRR`.
* **Priority Levels & Global Constants (`proc.h`, `proc.c`):**
  * Defined global constants for 3 priority levels (where lower numerical value denotes higher scheduling priority):
    * `PRIORITY_HIGH = 1` (Highest priority)
    * `PRIORITY_MEDIUM = 2` / `DEFAULT_PRIORITY = 2` (Medium / Default priority assigned in `allocproc()`)
    * `PRIORITY_LOW = 3` (Lowest priority)
    * `NUM_PRIORITY_LEVELS = 3`
  * Defined global time quantum constants/variables:
    * `const int DEFAULT_TIME_QUANTUM = 4;`
    * `int rr_time_quantum = 4;` (configurable at runtime via `set_sched_quantum(int quantum)`).
    * In our testing and evaluation, we tested time quantums of **4 ticks** (default) and **8 ticks** (as well as 2 ticks).
* **Scheduler & Interrupt Logic (`proc.c`, `trap.c`):**
  * In `scheduler()` under `#elif defined(SCHED_PRIORITYRR)`, a circular Round-Robin index `rr_idx` scans `ptable.proc` starting at `(rr_idx + 1) % NPROC`. It selects the first `RUNNABLE` process belonging to the highest available priority level (`p->priority < best_p->priority`) and updates `rr_idx = best_idx`. This guarantees strict priority ordering across levels and true Round-Robin fairness among processes at the same priority level.
  * In `trap.c` under `#ifdef SCHED_PRIORITYRR`, each timer interrupt increments `myproc()->quantum_ticks`. The process only calls `yield()` once `myproc()->quantum_ticks >= rr_time_quantum` (resetting `quantum_ticks = 0`), allowing it to run for its full time quantum before yielding to same-priority peers.
* **System Calls:**
  * `set_sched_priority(int priority)` (Syscall #25): Sets `myproc()->priority = priority` and resets `quantum_ticks = 0`. Returns `0` on success, `-1` if `priority < 1`.
  * `get_sched_priority(int pid)` (Syscall #26): Returns `p->priority` if `pid` exists and is in `RUNNABLE` or `RUNNING` state; returns `-1` otherwise.
  * `set_sched_quantum(int quantum)` (Syscall #27): Sets `rr_time_quantum = quantum` (for user-defined quantum testing).
* **Test Program:** `advanced_scheduler_test.c` (`_advanced_scheduler_test`).

---

## 4. Report Comparing the Schedulers (`DEFAULT` vs. `SJF` vs. `PRIORITYRR`)

### A. Tracing Process Execution with `ticks_running()` and `uptime()`
We ran our test suites across all three schedulers (`DEFAULT`, `SJF`, and `PRIORITYRR`) and recorded process start times (`uptime()`), completion times (`uptime()`), and CPU time consumed (`ticks_running(pid)`):

1. **Default Scheduler (`SCHEDULER=DEFAULT` — 1-Tick Round-Robin):**
   * All runnable child processes share the CPU equally, yielding on every single timer tick (1 tick = 10 ms).
   * When 4 CPU-bound children (`simple_scheduler_test`) are spawned at `uptime = 1460`, all 4 interleave execution every tick and finish almost simultaneously (`finish_uptime = 1664, 1665, 1665, 1666`), each accumulating ~51 `ticks_running`.
   * **Effect:** High context-switch overhead and high average turnaround time for batch jobs since no job finishes until all jobs are nearly done, but good interactive responsiveness.

2. **Shortest Job First (`SCHEDULER=SJF`):**
   * In `simple_scheduler_test`, 4 children were assigned predicted job lengths of `17`, `43`, `63`, and `73`:
     * **Child 1 (`job_length=17`):** `start_uptime=2800`, `finish_uptime=2850` (`ticks_running=50`)
     * **Child 3 (`job_length=43`):** `start_uptime=2850`, `finish_uptime=2901` (`ticks_running=51`)
     * **Child 2 (`job_length=63`):** `start_uptime=2901`, `finish_uptime=2950` (`ticks_running=49`)
     * **Child 0 (`job_length=73`):** `start_uptime=2950`, `finish_uptime=3001` (`ticks_running=51`)
   * **Effect:** Processes execute strictly in ascending order of `sjf_job_length` (`17 -> 43 -> 63 -> 73`). Shorter jobs finish much earlier (turnaround time of 50 ticks for the shortest job vs. ~204 ticks under `DEFAULT`), minimizing average waiting and turnaround time. However, long jobs (`job_length=73`) must wait until all shorter jobs complete, which can cause starvation if short jobs continuously arrive.

3. **Multi-Level Priority Round-Robin (`SCHEDULER=PRIORITYRR`):**
   * In `advanced_scheduler_test` (with `rr_time_quantum = 4` ticks), 6 CPU-bound children were created across Priorities `1` (High), `2` (Medium), and `3` (Low):
     * **Priority 1 (PIDs 8 & 9):** Started at `uptime = 156` and `160` (exactly 1 quantum of 4 ticks apart!), Round-Robined every 4 ticks, and finished first at `uptime = 236` and `237`.
     * **Priority 2 (PIDs 6 & 7):** Waited until Priority 1 completed, started at `uptime = 240` and `244` (4 ticks apart), Round-Robined every 4 ticks, and finished second at `uptime = 315` and `320`.
     * **Priority 3 (PIDs 4 & 5):** Waited until Priority 2 completed, started at `uptime = 322` and `326` (4 ticks apart), Round-Robined every 4 ticks, and finished last at `uptime = 397` and `403`.
   * When tested with **`rr_time_quantum = 8` ticks** (`advanced_scheduler_test 8`), the initial context switch between same-priority processes occurred 8 ticks apart (`607` to `615` for Priority 1; `691` to `699` for Priority 2; `775` to `782` for Priority 3), reducing context-switch frequency by half while preserving strict priority isolation.

### B. Effects on Existing xv6 Commands, `stressfs`, `fork`, and Pipes
* **Simple Interactive Commands (`ls`, `hello`, `sleep`, `wc`):**
  * For single short-lived commands run sequentially from the shell (`sh`), behavior is virtually identical across `DEFAULT`, `SJF`, and `PRIORITYRR` because only one child process is `RUNNABLE` while `sh` sleeps in `wait()`.
* **File System Stress Test (`stressfs`):**
  * `stressfs` forks 4 concurrent processes performing heavy disk I/O and CPU loops. Because disk I/O causes processes to block (`SLEEPING` in `iderw`), whenever the running process blocks on disk I/O, the scheduler immediately picks the next eligible `RUNNABLE` process. Under `SJF`, processes with lower `job_length` complete their CPU bursts first; under `PRIORITYRR` with a 4-tick quantum, fewer timer preemptions occur during memory buffers setup compared to 1-tick `DEFAULT`.
* **Pipelines (`cat filename | sort`, `cat filename | grep pattern`) & `fork()`:**
  * In a pipeline (`left | right`), `sh` forks two children connected by a kernel pipe buffer (`pipe.c`).
  * Under `DEFAULT`, the producer (`cat`) and consumer (`sort` or `grep`) alternate every tick.
  * Under `SJF`, if the consumer (`sort` or `grep`) is assigned a smaller `job_length` than `cat`, it gets scheduled first, immediately blocks on an empty pipe (`piperead` calls `sleep()`), which removes it from the `RUNNABLE` queue and allows `cat` to run and fill the pipe. Once `cat` writes to the pipe or exits, `wakeup()` makes the consumer `RUNNABLE` again. Thus, pipelines still complete accurately under all three schedulers.
  * Under `PRIORITYRR`, both piped children inherit `DEFAULT_PRIORITY (2)` from `fork()` and share the CPU in Round-Robin slices of `rr_time_quantum` ticks, resulting in fewer intermediate context switches on large inputs while maintaining smooth pipe throughput.

---

## 5. Verification & Screenshots

### A. Compiling xv6 with Scheduler Flags
*[Insert screenshot of terminal running `make clean && make qemu-nox SCHEDULER=DEFAULT`]*
*[Insert screenshot of terminal running `make clean && make qemu-nox SCHEDULER=SJF`]*
*[Insert screenshot of terminal running `make clean && make qemu-nox SCHEDULER=PRIORITYRR`]*

### B. Part 1: `sort` and `grep` Functional Verification
*[Insert screenshot of `sort`, `sort -r`, `sort -n`, `sort -u`, and `cat file | sort` in xv6]*
*[Insert screenshot of `grep`, `grep -i`, `grep -n`, `grep -v`, `grep -in`, and `cat file | grep` in xv6]*

### C. Part 2: `ticks_running_test` Execution
*[Insert screenshot of running `ticks_running_test` in xv6 shell]*

### D. Part 3: `simple_scheduler_test` (`SJF`) Execution
*[Insert screenshot of running `simple_scheduler_test` under `SCHEDULER=SJF`]*

### E. Part 4: `advanced_scheduler_test` (`PRIORITYRR`) Execution
*[Insert screenshot of running `advanced_scheduler_test` and `advanced_scheduler_test 8` under `SCHEDULER=PRIORITYRR`]*

