#include <kernel.h>
#include <proc.h>
#include <q.h>
#include "lock.h"

int ldelete(int lock_index){
	STATWORD ps;
	int ctr;
	disable(ps);
	
	lock_list[lock_index].lock_type = DELETED;
	lock_list[lock_index].reader_count = 0;
	lock_list[lock_index].reader_count = 0;
	//ctr = q[lock_list[lock_index].lock_lqtail].qprev;
	ctr = lock_list[lock_index].lock_lqtail;
	
	int flag = 0;		
	
	while(EMPTY != getlast(ctr)){
		flag = 1;
		kprintf("\nWaking up process: %d", ctr);
		proctab[ctr].gotDeleted = 1;
		ctr = dequeue(ctr);
		ready(ctr, RESCHNO);
	}

	if(flag){
		kprintf("\ncall reschedule");
		resched();
	}

	/*
	int pid;
	struct lock_data *lptr;
	lptr = &lock_list[lock_index];
	kprintf("\ndeleting lock %d", lock_index);

	if (nonempty(lptr->lock_qhead)) {
		kprintf("\nremoving");
		while ((pid = getfirst(lptr->lock_qhead)) != EMPTY) {
			//proctab[pid].plwaitret = DELETED;
			kprintf("\nremoving %d", pid);
			proctab[pid].lock_type[lock_index] = DELETED;
			dequeue(pid);
			ready(pid, RESCHNO);
		}
		resched();
	}*/

	kprintf("\ndone");
	restore(ps);
	return(OK);
}