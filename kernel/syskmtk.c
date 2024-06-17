#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "syscall.h"
#include "defs.h"

int sys_kmtk(void)
{
    return 1414124;
}

int sys_myfds(void)
{
    int result_num = 0;
    struct proc *p = myproc();

    for (int i = 0; i < NOFILE; i++)
    {
        if (p->ofile[i] != 0)
        {
            result_num += 1 << i;
        }
    }
    return result_num;
}