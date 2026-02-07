#include "types.h"
#include "riscv.h"
#include "param.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
#include "sysinfo.h"

uint64
sys_sysinfo(void) {
    uint64 addr;
    struct sysinfo info;

    argaddr(0, &addr);

    info.freemem = free_list_size();
    info.nproc = unused_proc_count();

    struct proc *p = myproc();
    if (copyout(p->pagetable, addr, (char *) &info, sizeof(info)) < 0)
        return -1;

    return 0;
}
