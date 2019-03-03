#include <kernel.h>
#include <proc.h>
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

	for(i = 0; i < NLOCKS; i++)
	{
		lptr = &locks[i]
		lptr -> lstate = LFREE;
		lptr -> locknum = i;
		lptr -> lqtail = 1 + (lptr -> lqhead = newqueue());
		lptr -> nreaders = 0;
		lptr -> nwriters = 0;
		lptr -> version = 0;
	}

	for(i = 0; i < NPROC; i ++)
		for(j = 0; j < NLOCKS; j ++)
			lockholdtab[i][j] = 0;

	kprintf("\nfinished linit\n");
}