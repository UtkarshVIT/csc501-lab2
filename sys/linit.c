#include <kernel.h>
#include <proc.h>
#include <q.h>
#include "lock.h"

struct lentry ltable[NLOCKS];
void linit()
{
	int i=0,j=0;
	while (i<NLOCKS) {
			ltable[i].ltype = LNONE;
			ltable[i].lstate = DELETED;
			ltable[i].nreaders = 0;
			ltable[i].lqhead = newqueue();
			kprintf("%d\n",ltable[i].lqhead);
			ltable[i].lqtail = 1 + ltable[i].lqhead;
			while(j<NPROC)
			{
				ltable[i].holders[j] = LNONE;
				++j;
			}
			++i;
		}
}