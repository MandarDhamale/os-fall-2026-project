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
* **Course Lectures & Canvas Assignment Guidelines:** Clarified trap handling, user-to-kernel context switches, and file system directory table parsing. And used Gemini for understanding the code when stuck. 

---

## 6. Verification & Screenshots

### A. Source Code Implementation
<img width="1091" height="531" alt="Screenshot From 2026-09-27 22-23-40" src="https://github.com/user-attachments/assets/24c0f2da-9318-4d7a-a94e-fc80e4e67b81" />

<img width="1091" height="657" alt="Screenshot From 2026-09-27 22-24-09" src="https://github.com/user-attachments/assets/dfabb9e0-9199-44a2-8a83-e57ec1daa6d9" />

<img width="1435" height="836" alt="Screenshot From 2026-09-27 22-25-22" src="https://github.com/user-attachments/assets/42651450-8b59-4df8-b5a1-199ff477df2e" />

<img width="1435" height="836" alt="Screenshot From 2026-09-27 22-25-59" src="https://github.com/user-attachments/assets/7fe02795-6726-492d-aeb9-7f4bda857c18" />

<img width="1413" height="805" alt="Screenshot From 2026-09-27 22-26-35" src="https://github.com/user-attachments/assets/4d226f98-ebd7-4771-b0d7-3bda53b5167b" />

<img width="1413" height="851" alt="Screenshot From 2026-09-27 22-27-23" src="https://github.com/user-attachments/assets/06f96863-bf8b-487e-854b-3ffd99707cd5" />


### B. Build & Compilation Process
<img width="1913" height="1042" alt="Screenshot From 2026-09-27 22-28-34" src="https://github.com/user-attachments/assets/e23ea243-1634-4baa-9ccd-eb6bfdc29696" />

<img width="1913" height="1042" alt="Screenshot From 2026-09-27 22-29-32" src="https://github.com/user-attachments/assets/7697cf0f-5489-4c34-bac6-a209d0f2962e" />


### C. Execution & Functional Verification
hello
<img width="1913" height="1042" alt="Screenshot From 2026-09-27 22-29-49" src="https://github.com/user-attachments/assets/ea193654-7206-4187-b33c-0edde7fd21fd" />

ls
<img width="1913" height="1042" alt="Screenshot From 2026-09-27 22-30-30" src="https://github.com/user-attachments/assets/7bf4ce55-ea16-4361-8526-f076a6df457c" />

ls -a
<img width="1913" height="1042" alt="Screenshot From 2026-09-27 22-30-56" src="https://github.com/user-attachments/assets/e6647f1e-55ae-4d27-a3e9-81d409e59db6" />

sleep and sleep 100
<img width="1913" height="267" alt="Screenshot From 2026-09-27 22-31-48" src="https://github.com/user-attachments/assets/bdae44dd-3a1e-453e-bbbc-6e395cc134a1" />

