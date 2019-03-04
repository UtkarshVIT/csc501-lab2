#include <kernel.h>
#include <proc.h>
#include <q.h>
#include "lock.h"

struct lentry ltable[NLOCKS];
void linit()
{
	int i=0, j;
	while (i<NLOCKS) {
		ltable[i].ltype = LNONE;
		ltable[i].lstate = LAVAILABLE;
		ltable[i].nreaders = 0;
		ltable[i].lqhead = newqueue();
		ltable[i].lqtail = 1 + ltable[i].lqhead;
		j=0;
		while(j<NPROC)
		{
			ltable[i].holders[j] = LNONE;
			++j;
		}
		++i;
	}
}