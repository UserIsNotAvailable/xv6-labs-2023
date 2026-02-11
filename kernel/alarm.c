#include "types.h"
#include "riscv.h"
#include "param.h"
#include "spinlock.h"
#include "defs.h"
#include "proc.h"
#include "lib/pqueue.h"

#define MAX_TIMERS 64

static pqueue timer_pq;
static struct spinlock alarm_lock;

typedef struct timer {
  struct proc *proc;
  struct trapframe trapframe;
  uint64 ticks;
  uint64 handler;
  struct timer *next;
} timer;

static timer timer_pool[MAX_TIMERS];
static timer *free_list;

static inline uint
get_now(void)
{
  uint now;

  acquire(&tickslock);
  now = ticks;
  release(&tickslock);
  return now;
}

void
alarm_init(void)
{
  timer *t;

  initlock(&alarm_lock, "alarm");

  pq_init(&timer_pq);

  free_list = &timer_pool[0];
  for (t = timer_pool; timer_pool + MAX_TIMERS - 1 > t; ++t)
    t->next = t + 1;
  t->next = 0;
}

int
alarm_set(int ticks, uint64 handler)
{
  timer *t;
  uint now;

  now = get_now();
  acquire(&alarm_lock);
  if (0 == free_list) {
    release(&alarm_lock);
    return -1;
  }

  t = free_list;
  free_list = free_list->next;

  t->proc = myproc();
  t->ticks = ticks;
  t->handler = handler;

  if (0 > pq_push(&timer_pq, now + t->ticks, t))
    panic("if (0 > pq_push(&timer_pq, now + t->ticks, t))");
  release(&alarm_lock);
  return 0;
}

int
alarm_cancel()
{
  struct proc *p;
  int ret;
  timer *t;

  p = myproc();

  acquire(&alarm_lock);
  p->current_timer = 0;

  ret = -1;
  for (t = timer_pool; t < timer_pool + MAX_TIMERS; ++t)
    if (p == t->proc) {
      t->handler = MAXVA;
      ret = 0;
    }
  release(&alarm_lock);
  return ret;
}

void
alarm_process(void)
{
  struct proc *p;
  timer *t;
  uint now;
  uint64 expire;

  now = get_now();
  acquire(&alarm_lock);

  while (1) {
    if (0 > pq_peek(&timer_pq, &expire, (void *) &t))
      break;

    if (MAXVA == t->handler) {
      t->next = free_list;
      free_list = t;
      if (0 != pq_pop(&timer_pq, 0, 0))
        panic("if (MAXVA == t->handler)if (0 != pq_pop(&timer_pq, 0, 0))");
      continue;
    }

    if (expire > now)
      break;

    p = t->proc;
    if (0 != p->current_timer) {
      if (0 != pq_pop(&timer_pq, 0, 0))
        panic("if (0 != p->current_timer)if (0 != pq_pop(&timer_pq, 0, 0))");
      pq_push(&timer_pq, now + t->ticks, t);
      continue;
    }

    if (0 != pq_pop(&timer_pq, 0, 0))
      panic("if (0 != pq_pop(&timer_pq, 0, 0))");

    t->trapframe = *p->trapframe;
    p->trapframe->epc = t->handler;
    p->current_timer = t;

    pq_push(&timer_pq, now + t->ticks, t);
  }
  release(&alarm_lock);
}

uint64
alarm_return(void)
{
  struct proc *p;
  timer *t;
  uint64 ret;

  p = myproc();
  ret = p->trapframe->a0;
  t = p->current_timer;

  if (0 == t)
    return ret;

  p->current_timer = 0;
  *p->trapframe = t->trapframe;
  return ret;
}
