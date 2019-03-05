/* getprio.c - getprio */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <q.h>
#include "lock.h"

/*------------------------------------------------------------------------
 * getprio -- return the scheduling priority of a given process
 *------------------------------------------------------------------------
 */
SYSCALL getprio(int pid)
{
	STATWORD ps;    
	struct	pentry	*pptr;

	disable(ps);
	if (isbadpid(pid) || (pptr = &proctab[pid])->pstate == PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	//get virtual priority;
	int i=0;
	int max_prio = pptr->pprio;
	while(i<NLOCKS){
		if(proctab[pid].locktype[i] == READ || proctab[pid].locktype[i] == WRITE){
			int ctr = q[lock_list[i].lock_lqtail].qprev;
			while(ctr != lock_list[i].lock_qhead){
				if(proctab[ctr].pprio > max_prio)
					max_prio = proctab[ctr].pprio;
				ctr=q[ctr].qprev;
			}
		}
		++i;
	}

	restore(ps);
	return max_prio;
}
