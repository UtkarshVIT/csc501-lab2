#include <kernel.h>
#include <proc.h>
#include <q.h>
#include "lock.h"
#include <stdio.h>

int newlock();

int lcreate(void)
{
	STATWORD ps;
    int     lock;

    disable(ps);
    int     i=0, j;

    while(i<NLOCKS) 
    {
        lock=i;
        if (locks[lock].lstate==LFREE) 
        {
            locks[lock].lstate = LUSED;
            locks[lock].version=locks[lock].version++;
            if (locks[lock].version==100)
            	locks[lock].version=0;
            locks[lock].locknum = lock*VERSION_POSS+locks[lock].version;
            locks[lock].nreaders = 0;
            locks[lock].nwriters = 0;

            for(j = NPROC; j >0; j--) lockholdtab[j][lock] = 0;
                return(locks[lock].locknum);
        }
        i++;
    }
    return(SYSERR);
}
