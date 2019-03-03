#include <kernel.h>
#include <proc.h>
#include <q.h>
#include "lock.h"

struct lentry ltable[NLOCKS];
void linit()
{
	int i,pid;
	struct lentry *lptr;
	for (i=0 ; i<NLOCKS ; i++) {
			(lptr = &ltable[i])->lstate = DELETED;
			lptr->ltype = LNONE;
			lptr->nreaders = 0;
			lptr->lqtail = 1 + (lptr->lqhead= newqueue());
			for(pid=0;pid<NPROC;pid++)
			{
				lptr->holders[pid] = LNONE;
			}
		}
}