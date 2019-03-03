#include <kernel.h>
#include <proc.h>
#include <q.h>
#include "lock.h"

struct lentry ltable[NLOCKS];
void linit()
{
	struct lentry *lptr;
	int i=0,pid;
	while (i<NLOCKS) {
		/*
			lptr = &ltable[i];
			lptr->lstate = DELETED;
			lptr->ltype = LNONE;
			lptr->nreaders = 0;
			lptr->lqtail = 1 + (lptr->lqhead= newqueue());
			for(pid=0;pid<NPROC;pid++)
			{
				lptr->holders[pid] = LNONE;
			}
			*/
			//lptr = &ltable[i];
			ltable[i].lstate = DELETED;
			ltable[i].ltype = LNONE;
			ltable[i].nreaders = 0;
			ltable[i].lqhead= newqueue();
			ltable[i].lqtail = 1 + ltable[i].lqhead;
			for(pid=0;pid<NPROC;pid++)
			{
				ltable[i].holders[pid] = LNONE;
			}
			i++;
		}
}