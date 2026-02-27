// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock per_cpu_lock;
  char lock_name[32];
  struct run *freelist;
  uint64 page_count;
} kmem[NCPU];

struct spinlock inter_cpu_lock;

void
kinit()
{
  int hart = cpuid();

  if (0 > snprintf(kmem[hart].lock_name, sizeof(kmem[hart].lock_name), "kmem-cpu-%d", hart))
    panic("kinit: 0 > snprintf(buf, sizeof(buf), \"kmem_cpu_%d\", hart)");

  initlock(&kmem[hart].per_cpu_lock, kmem[hart].lock_name);

  if (0 == hart) {
    initlock(&inter_cpu_lock, "kmem_inter_cpu_lock");
    freerange(end, (void *) PHYSTOP);
  }
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char *) PGROUNDUP((uint64) pa_start);
  for (; p + PGSIZE <= (char *) pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if (((uint64) pa % PGSIZE) != 0 || (char *) pa < end || (uint64) pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run *) pa;

  push_off();

  int hart = cpuid();
  acquire(&kmem[hart].per_cpu_lock);
  r->next = kmem[hart].freelist;
  kmem[hart].freelist = r;
  ++kmem[hart].page_count;
  release(&kmem[hart].per_cpu_lock);

  pop_off();
}

static
void steal(int hart) {
  for (int i = 0; i < NCPU; ++i) {
    if (hart == i) continue;
    acquire(&kmem[i].per_cpu_lock);
    if (0 == kmem[i].page_count) {
      release(&kmem[i].per_cpu_lock);
      continue;
    }

    uint64 steal_count = kmem[i].page_count >> 1;
    steal_count = steal_count == 0 ? 1 : steal_count;

    kmem[i].page_count -= steal_count;
    kmem[hart].page_count = steal_count ;

    struct run *mid = kmem[i].freelist;
    while (--steal_count)
      mid = mid->next;

    kmem[hart].freelist = kmem[i].freelist;
    kmem[i].freelist = mid->next;
    mid->next = 0;
    release(&kmem[i].per_cpu_lock);
    break;
  }
}
// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  push_off();
  int hart = cpuid();
  acquire(&kmem[hart].per_cpu_lock);

  if (0 == kmem[hart].page_count) {
    acquire(&inter_cpu_lock);
    release(&kmem[hart].per_cpu_lock);
    steal(hart);
    acquire(&kmem[hart].per_cpu_lock);
    release(&inter_cpu_lock);
  }

  struct run *r = kmem[hart].freelist;
  if (r) {
    kmem[hart].freelist = r->next;
    --kmem[hart].page_count;
  }

  release(&kmem[hart].per_cpu_lock);
  pop_off();
  if (r)
    memset((char *) r, 5, PGSIZE); // fill with junk
  return (void *) r;
}
