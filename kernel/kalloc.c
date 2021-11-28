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

struct kmem{
  struct spinlock lock;
  struct run *freelist;
} kmem;

struct kmem kemArr[NCPU];

void
kinit()
{
  // initializing the ncpu kmem structure
  for (int i=0; i<NCPU; i++){
    initlock(&kemArr[i].lock, "kmem");
  }
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  // obtaining the current cpuid and releasing the corresponding freelist
  push_off();
  int cpuId = cpuid();
  acquire(&kemArr[cpuId].lock);
  r->next = kemArr[cpuId].freelist;
  kemArr[cpuId].freelist = r;
  release(&kemArr[cpuId].lock);
  pop_off();
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  push_off();
  int id = cpuid();
  acquire(&kemArr[id].lock);
  r = kemArr[id].freelist;

  // when the current cpu freelist is empty, get it from the other cpu freelist
  if(r) {
    kemArr[id].freelist = r->next;
    release(&kemArr[id].lock);
  } else {
    release(&kemArr[id].lock);
    int current = id + 1;
    do {
      acquire(&kemArr[current].lock);
      r = kemArr[current].freelist;
      if(r) {
        kemArr[current].freelist = r->next;
        release(&kemArr[current].lock);
        break;
      }
      release(&kemArr[current].lock);
      current++;
      if (current == NCPU)
        current = 0;
    } while (current != id);
  }
  pop_off();

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}
