#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->usyscall->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;


  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}


#ifdef LAB_PGTBL
int
sys_pgaccess(void) {
  uint64 base;
  int len;
  uint32 mask;

  argaddr(0, &base);
  argint(1, &len);
  argint(2, (int *) &mask);

  if (0 >= len||32<len)
    return -1;

  if (MAXVA < PGROUNDUP(base + PGSIZE * (len - 1)))
    return -2;

  pagetable_t pagetable = myproc()->pagetable;
  uint32 ret = 0;

  for (int i = 0; i < len; ++i) {
    uint64 p = base + PGSIZE * i;
    pte_t *pte = walk(pagetable, p, 0);

    if (0 == pte)
      continue;

    if (PTE_A & *pte)
      ret |= (1l << i);

    *pte &= ~PTE_A;
  }

  if (0 > copyout(pagetable, mask, (char *) &ret, sizeof(ret)))
    return -3;

  return 0;
}
#endif

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
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
