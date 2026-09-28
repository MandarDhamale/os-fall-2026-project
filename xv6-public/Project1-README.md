# COP 6611 - Project 1: Adding Programs and Implementing Shell Commands/System Calls in xv6

## 1. Group Members
* **Name:** Mandar Dhamale, Sakina Rao, Nadika Poudel 
* **Email:** mandardhamale@usf.edu

---

## 2. Development & Testing Environment
* **Operating System:** Ubuntu 24.04 LTS (x86_64)
* **Compiler / Toolchain:** `gcc` (Ubuntu standard toolchain)
* **Emulator:** QEMU emulator
* **Build Tool:** GNU Make

---

## 3. Answer to Part 3 Required Question: Instances of `getpid`
As required by the Part 3 assignment task, I tracked all instances of `getpid` across the xv6 environment to understand the necessary integration points for introducing a new system call (`hello`):

1. **`syscall.h`:** Defines the unique system call number macro (`SYS_getpid`). Updated with `#define SYS_hello 22`.
2. **`syscall.c`:** Contains the external function declaration (`extern int sys_getpid(void);`) and registers the system call in the dispatch table array (`[SYS_getpid] sys_getpid`). Updated to declare and route `sys_hello`.
3. **`user.h`:** Exposes the user-level function prototype (`int getpid(void);`) so that user programs and utilities can invoke the system call. Updated with `int hello(void);`.
4. **`usys.S`:** Uses the assembly macro `SYSCALL(getpid)` to place the system call number in register `%eax` and execute interrupt `T_SYSCALL` (trap `64`). Updated with `SYSCALL(hello)`.
5. **`sysproc.c`:** Implements the actual kernel-space execution logic (`sys_getpid(void)`). Implemented `sys_hello(void)` here using `cprintf("Hello from the Kernel!\n");`.

---

## 4. Implementation Details & Additional Notes
* **Part 1 & 3 (`hello.c`):** Implemented user program `hello.c` to print `"Hello Xv6!\n"` using user-space `printf()`, followed immediately by invoking the custom system call `hello()` which issues `"Hello from the Kernel!\n"` via kernel-level `cprintf()`. Added `_hello\` to `UPROGS` in `Makefile`.
* **Part 2 (`ls.c`):**
  * Updated command-line argument parsing in `main()` to check for the optional `-a` flag and adjust `start_idx` accordingly.
  * In `ls()`, added filter logic: if `show_hidden` is false (`0`) and the entry name starts with `.` (`de.name[0] == '.'`), the entry is skipped using `continue`.
  * Adjusted `fmtname()` to append a `/` directly to directory entries (`st.type == T_DIR`), maintaining proper string alignment and column formatting.
* **Part 4 (`sleep.c`):** Checks `argc == 2`; if invalid or missing, outputs an informative error message (`"Error: Usage: sleep <duration>\n"`) to `stderr` (fd 2) and exits. Converts the tick duration using `atoi(argv[1])`, invokes the kernel `sleep()` system call, and cleanly terminates with `exit()`. Added `_sleep\` to `UPROGS` in `Makefile`.

---

## 5. Resources Used & Concept Clarifications
* **xv6 Commentary & Source Code:** Used existing user utilities (`echo.c`, `grep.c`, `cat.c`) and system call handlers (`sys_sleep` in `sysproc.c`) to model user-space argument retrieval and trap entry mechanics.
* **Course Lectures & Canvas Assignment Guidelines:** Clarified trap handling, user-to-kernel context switches, and file system directory table parsing.

---

## 6. Verification & Screenshots

### A. Source Code Implementation
*[Replace this line with your screenshot of `hello.c`]*
*[Replace this line with your screenshot of `sleep.c`]*
*[Replace this line with your screenshot of modified `ls.c` (`fmtname`, `ls()`, and `main()`)]*
*[Replace this line with your screenshot of `sysproc.c` showing `sys_hello()`]*
*[Replace this line with your screenshot of `Makefile` showing `UPROGS` additions]*

### B. Build & Compilation Process
*[Replace this line with your screenshot of terminal running `make clean` followed by `make` and `make qemu` showing a clean build]*

### C. Execution & Functional Verification
*[Replace this line with your screenshot running the commands in the xv6 shell]*