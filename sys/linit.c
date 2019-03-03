#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <q.h>
#include "lock.h"

struct lentry locks[NLOCKS];
int nextlock;
int lockholdtab[NPROC][NLOCKS];

void linit()
{
	struct lentry *lptr;
	int i, j;
	nextlock = NLOCKS-1;
	kprintf("nlocks:%d\n", NLOCKS);

	kprintf("%d, %d, %d\n", locks[0].version, locks[1].version, locks[2].version);
	for(i = 0; i < NLOCKS; i++)
	{
		lptr = &locks[i];
		lptr -> lstate = LFREE;
		lptr -> locknum = i;
		lptr -> lqtail = 1 + (lptr -> lqhead = newqueue());
		lptr -> nreaders = 0;
		lptr -> nwriters = 0;
		lptr -> version = 0;
		if(i>0)
			kprintf("%d, %d\n", i-1, locks[i-1].version);
	}
	/*

			*/
	kprintf("%d, %d, %d\n", locks[0].version, locks[1].version, locks[2].version);
	kprintf("%d, %d, %d\n", locks[0].nwriters, locks[1].nwriters, locks[2].nwriters);
	for(i = 0; i < NPROC; i ++)
		for(j = 0; j < NLOCKS; j ++)
			lockholdtab[i][j] = 0;
	kprintf("\nfinished linit\n");
}