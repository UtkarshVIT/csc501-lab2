/* kill.c - kill */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <q.h>
#include <stdio.h>
#include "lock.h"

/*------------------------------------------------------------------------
 * kill  --  kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
void release_all_locks_for_process(int pid){
	int i = 0 ;
	while(i < NLOCKS) {
		if (proctab[pid].locktype[i] != FREE){
			kprintf("\n");
			int ctr = q[lock_list[i].lock_lqtail].qprev;
			while(ctr != lock_list[i].lock_qhead){
				kprintf("%d ", proctab[ctr].pprio);
				ctr=q[ctr].qprev;
			}
			kprintf("\nRemoving lock\n");
			if(proctab[pid].pstate == PRCURR){
				kprintf("\nyes yes\n");
			}
			release(pid, i);
			ctr = q[lock_list[i].lock_lqtail].qprev;
			while(ctr != lock_list[i].lock_qhead){
				kprintf("%d ", proctab[ctr].pprio);
				ctr=q[ctr].qprev;
			}
		}
		++i;
	}

}

SYSCALL kill(int pid) {
	STATWORD ps;
	struct pentry *pptr; /* points to proc. table for pid*/
	int i, dev;

	disable(ps);
	if (isbadpid(pid) || (pptr = &proctab[pid])->pstate == PRFREE) {
		restore(ps);
		return (SYSERR);
	}
	if (--numproc == 0)
		xdone();

	dev = pptr->pdevs[0];
	if (!isbaddev(dev))
		close(dev);
	dev = pptr->pdevs[1];
	if (!isbaddev(dev))
		close(dev);
	dev = pptr->ppagedev;
	if (!isbaddev(dev))
		close(dev);

	send(pptr->pnxtkin, pid);

	freestk(pptr->pbase, pptr->pstklen);
	switch (pptr->pstate) {

	case PRCURR:
		kprintf("in PRCURR\n");
		release_all_locks_for_process(pid);
		pptr->pstate = PRFREE; /* suicide */
		resched();

	case PRWAIT:
		semaph[pptr->psem].semcnt++;

	case PRREADY:
		kprintf("in PRREADY\n");
		release_all_locks_for_process(pid);
		dequeue(pid);
		pptr->pstate = PRFREE;
		break;

	case PRSLEEP:
	case PRTRECV:
		unsleep(pid);
		/* fall through	*/
	default:
		kprintf("in DEFAULT\n");
		release_all_locks_for_process(pid);
		pptr->pstate = PRFREE;
			}
	restore(ps);
	return (OK);
}