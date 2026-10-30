#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "vm.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  kexit(n);
  return 0; // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return kfork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return kwait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int t;
  int n;

  argint(0, &n);
  argint(1, &t);
  addr = myproc()->sz;

  if (t == SBRK_EAGER || n < 0) {
    if (growproc(n) < 0) {
      return -1;
    }
  } else {
    // Lazily allocate memory for this process: increase its memory
    // size but don't allocate memory. If the processes uses the
    // memory, vmfault() will allocate it.
    if (addr + n < addr)
      return -1;
    if (addr + n > TRAPFRAME)
      return -1;
    myproc()->sz += n;
  }
  return addr;
}

uint64
sys_pause(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if (n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n) {
    if (killed(myproc())) {
      release(&tickslock);
      return -1;
    }
    sleep_prepare(&ticks);
    release(&tickslock);
    sleep();
    acquire(&tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kkill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// ---- PA2 system calls ----

// Q1: ticks since boot, read under tickslock (same as sys_uptime).
uint64
sys_getuptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// Q2
uint64
sys_activecount(void)
{
  return proc_activecount();
}

// Q3
uint64
sys_lineage(void)
{
  int pid;

  argint(0, &pid);
  return proc_lineage(pid);
}

// Q4
uint64
sys_getprocsize(void)
{
  int pid;

  argint(0, &pid);
  return proc_getprocsize(pid);
}

// Q5 (argument is accepted for the given prototype; the caller is used)
uint64
sys_familyheadcount(void)
{
  int unused;

  argint(0, &unused);
  return proc_familyheadcount();
}

// Bonus: getprocs(struct pinfo *buf, int max)
uint64
sys_getprocs(void)
{
  uint64 buf;
  int max;

  argaddr(0, &buf);
  argint(1, &max);
  if (max <= 0)
    return -1;
  return proc_getprocs(buf, max);
}
