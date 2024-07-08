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

int sys_setenv(void)
{
    char key[MAXENV], value[MAXENV];
    struct proc *p = myproc();

    if (argstr(0, key, MAXENV) < 0 || argstr(1, value, MAXENV) < 0)
    {
        return -1;
    }

    for (int i = 0; i < p->env.size; i++)
    {
        if (strncmp(p->env.entries[i].key, key, MAXENV) == 0)
        {
            strncpy(p->env.entries[i].value, value, MAXENV);
            return 0;
        }
    }

    p->env.entries[p->env.size].key = kalloc();
    p->env.entries[p->env.size].value = kalloc();
    strncpy(p->env.entries[p->env.size].key, key, MAXENV);
    strncpy(p->env.entries[p->env.size].value, value, MAXENV);
    p->env.size++;

    return 0;
}

uint64 sys_getenv(void)
{
    char key[MAXENV];
    struct proc *p = myproc();

    if (argstr(0, key, MAXENV) < 0)
        return -1;

    for (int i = 0; i < p->env.size; i++)
    {
        if (strncmp(p->env.entries[i].key, key, MAXENV) == 0)
        {
            int size = strlen(p->env.entries[i].value);
            uint64 pointer = p->sz;
            if (growproc(size) < 0)
                return -1;

            if (copyout(p->pagetable, pointer, p->env.entries[i].value, size) < 0)
            {
                growproc(-size);
                return -1;
            }


            return pointer;
        }
    }

    // key not found
    return 0;
}