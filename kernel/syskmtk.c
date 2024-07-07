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
        printf("argstr failed\n");
        return -1;
    }

    for(int i = 0; i < p->env.size; i++) {
        printf("check key: %s\n", p->env.entries[i].key);
        if(strncmp(p->env.entries[i].key, key, MAXENV) == 0) {
            strncpy(p->env.entries[i].value, value, MAXENV);
            return 0;
        }
    }
 
    p->env.entries[p->env.size].key = kalloc();
    p->env.entries[p->env.size].value = kalloc();
    strncpy(p->env.entries[p->env.size].key, key, sizeof(key));
    strncpy(p->env.entries[p->env.size].value, value, sizeof(value));
    p->env.size++;

    return 0;
}

int sys_getenv(void)
{
    char key[MAXENV], value[MAXENV];
    struct proc *p = myproc();

    if (argstr(0, key, MAXENV) < 0)
    {
        return -1;
    }

    for(int i = 0; i < p->env.size; i++) {
        if(strncmp(p->env.entries[i].key, key, MAXENV) == 0) {
            if(copyout(p->pagetable, (uint64)value, p->env.entries[i].value, sizeof(value)) < 0)
                return -1;
            return 0;
        }
    }

    return -1;
}