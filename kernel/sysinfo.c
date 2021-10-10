#include "types.h"
#include "riscv.h"
#include "param.h"
#include "spinlock.h"
#include "defs.h"
#include "sysinfo.h"
#include "proc.h"

// Returns current system info
int
systeminfo(uint64 addr)
{
  // getting the current proc
  struct proc *p = myproc();
  struct sysinfo info;

  // getting number of free bytes of the memory
  info.freemem = freemem();
  // getting number of processes with state other than UNUSED
  info.nproc = nproc();
  
  // copying from kernel to user
  if(copyout(p->pagetable, addr, (char *)&info, sizeof(info)) < 0)
    return -1;

  return 0;
}