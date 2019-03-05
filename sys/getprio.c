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
	if(isbadpid(pid) || proctab[pid].pstate == PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	
	//get virtual priority;
	int virtual_prio = get_virtual_prio_prio_inversion(pid);
	kprintf("\n==> %d", virtual_prio);

	restore(ps);
	return virtual_prio;
}
