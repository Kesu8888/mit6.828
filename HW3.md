# HW3 report

## Part One: System call tracing

print out a line for each system call invocation. The method I am doing is quite straight forward.
{
  int num;
  struct proc *curproc = myproc();

  num = curproc->tf->eax;
  if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
    int returnID = syscalls[num](); // <- returnValue of the system call
    curproc->tf->eax = returnID;
      callProc(num, returnID); // <- A function that calls the name of the system call with returnID
  } else {
    cprintf("%d %s: unknown sys call %d\n",
            curproc->pid, curproc->name, num);
    curproc->tf->eax = -1;
  }
}
This is the quite stupid callProc function I write
void
callProc(int num, int returnID) {
    char const *procName[21];
    procName[1] = "fork";
    procName[2] = "exit";
    procName[3] = "wait";
    procName[4] = "pipe";
    procName[5] = "read";
    procName[6] = "kill";
    procName[7] = "exec";
    procName[8] = "fstat";
    procName[9] = "chdir";
    procName[10] = "getpid";
    procName[11] = "sbrk";
    procName[12] = "sleep";
    procName[13] = "duptime";
    procName[14] = "open";
    procName[15] = "write";
    procName[16] = "mknod";
    procName[17] = "unlink";
    procName[18] = "link";
    procName[19] = "mkdir";
    procName[20] = "close";
    cprintf("%s -> %d\n", procName[num - 1], returnID);
}


## Part Two: Date system call

Firstly we create a file(date.c) and copy the code displayed in the question into this file.
#include "types.h"
#include "user.h"
#include "date.h"

int
main(int argc, char *argv[])
{
  struct rtcdate r;

  if (date(&r)) {
    printf(2, "date failed\n");
    exit();
  }

  // your code to print the time in any format you like...

  exit();
}
step 1: the function date suppose to change the value of struct variable r. Since the date function should use lapic.c/cmostime() to change the value of r, I first try #include lapic.c and write the date function in date.c. Then I encounter the problem of multiple declaration of some functions (e.g., exit()) in the file "user.h" and "defs.h". This is because lapic.c has included defs.h and some functions with the same name are declared differently in "user.h" and "defs.h". Hence we should write the date function indirectly.

I add the function date() inside "user.h" under //system call.
int date(struct rtcdate*);

step 2: When we call the function date() in date.c, the function calling sequence is as shown below.
date() in date.c -> "user.h" -> syscall.c/syscall() -> syscall.c/static int (*syscalls[])(void) -> syscall.h -> extern int sys_date(void) -> sysproc.c
Hence we should add the related code to these places.
- Add "[SYS_date]    sys_date," to static int (*syscalls[])(void)
- Add "extern int sys_date(void)" under those extern functions in syscall.C
- Add "#define SYS_date   22" to syscall.h
- Add out function body int sys_date(void) to sysproc.c.

step 3: Write the function body. Below is my answer.
int
sys_date(void)
{
    struct rtcdate *r;
    if (argptr(0, (char **)&r, sizeof(struct rtcdate)) < 0) //<- Here we fetch the passed in pointer
        return 1;
    cmostime(r);
    return 0;
}
Note. From syscall.c/syscall(void), we noticed that when syscall calls corresponding functions it cannot include any arguments. That's why we need to use syscall.c/argptr to fetch the pointer we passed into date in date.c/main().
void
syscall(void)
{
  int num;
  struct proc *curproc = myproc();

  num = curproc->tf->eax;
  if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
    int returnID = syscalls[num](); // <- Here it calls the function date without any argument.
    curproc->tf->eax = returnID;

step 4: Edit the main function in date.C and include _date in makefile/UPROGS. Below is my main code.
#include "types.h"
#include "date.h"
#include "user.h"

int
main(int argc, char *argv[])
{
    struct rtcdate r;
    if (date(&r)) {
        printf(2, "date failed\n");
        exit();
    }

    // your code to print the time in any format you like...
    printf(2, "y: %d m: %d d: %d %d:%d:%d\n", r.year, r.month, r.day, r.hour, r.minute, r.second);
    exit();
}























