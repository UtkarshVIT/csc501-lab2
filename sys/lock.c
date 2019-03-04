#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <stdio.h>
#include "lock.h"

extern unsigned long ctr1000;
int has_highprio_writer(int prio, int ldesc);
void insert_in_prio_queue(int currpid, int ldes1, int priority, int type);

int lock(int ldes1, int type, int priority) {
	STATWORD ps;

	disable(ps);

	proctab[currpid].plwaitret = OK;

	/* If the lock type in LNONE */
	if (ltable[ldes1].ltype == LNONE) {
		ltable[ldes1].ltype = type;
		proctab[currpid].locktype[ldes1] = type;
		ltable[ldes1].holders[currpid] = type;

		if (type == READ) {
			ltable[ldes1].nreaders++;
		}
	}

	/* If the lock type for this already READ */
	else if (ltable[ldes1].ltype == READ) {
		/* If the request type is WRITE */
		if (type == WRITE) {
			insert_in_prio_queue(int currpid, int ldes1, int priority, int type);
			
		}
		/* If the request type is READ */
		else if (type == READ) {
			if (has_highprio_writer(priority, ldes1) == TRUE) {
				insert_in_prio_queue(int currpid, int ldes1, int priority, int type);
			} 
			else {
				ltable[ldes1].ltype = type;
				ltable[ldes1].nreaders++;
				proctab[currpid].locktype[ldes1] = type;
				ltable[ldes1].holders[currpid] = type;
			}
		}
	}

	else if (ltable[ldes1].ltype == WRITE) {
		insert_in_prio_queue(int currpid, int ldes1, int priority, int type);
	} 

	restore(ps);
	return (proctab[currpid].plwaitret);
}

void insert_in_prio_queue(int currpid, int ldes1, int priority, int type){
	struct pentry *pptr;
	pptr = &proctab[currpid];
	pptr->pstate = PRWAIT;
	pptr->locktype[ldes1] = type;
	pptr->plreqtime = ctr1000;
	insert(currpid, ltable[ldes1].lqhead, priority);
	resched();
}
int has_highprio_writer(int prio, int ldesc) {
	int temp;
	temp = q[ltable[ldesc].lqtail].qprev;

	for (temp = q[ltable[ldesc].lqtail].qprev; (temp != ltable[ldesc].lqhead) && (prio < q[temp].qkey); temp = q[temp].qprev) {
		if (proctab[temp].locktype[ldesc] == WRITE) {
			return TRUE;
		}
	}
	return FALSE;
}