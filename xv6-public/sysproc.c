#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// Print the current date in UTC 24 hours time
int
sys_date(void)
{
    /*struct rtcdate r;
    int i = 0;
    cmostime(&r);
    if (r.year < 2000) {
        return 0;
    }
    //cprintf("y: %d m: %d d: %d %d:%d:%d\n", r.year, r.month, r.day, r.hour, r.minute, r.second);
    i = i + r.second;
    i = i + (r.minute << 6);
    i = i + (r.hour << 12);
    i = i + (r.day << 17);
    i = i + (r.month << 22);
    i = i + ((r.year - 2000) << 26);
    return i;*/
    struct rtcdate *r;
    if (argptr(0, (char **)&r, sizeof(struct rtcdate)) < 0)
        return 1;
    cmostime(r);
    return 0;
}
