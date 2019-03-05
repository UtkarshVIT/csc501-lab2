#include <kernel.h>
#include <proc.h>
#include <q.h>
#include "lock.h"

int ldelete(int lock_index){
	STATWORD ps;
	int ctr;
	disable(ps);
	kprintf("Deleting a lock\n");
	lock_list[lock_index].lock_type = DELETED;
	lock_list[lock_index].reader_count = 0;
	lock_list[lock_index].reader_count = 0;
	ctr = q[lock_list[lock_index].lock_lqtail].qprev;
	while(ctr != lock_list[lock_index].lock_qhead){
		proctab[ctr].gotDeleted = 1;
		dequeue(ctr);
		ready(ctr, RESCHNO);
	}
	resched();
	restore(ps);
	return(OK);
}