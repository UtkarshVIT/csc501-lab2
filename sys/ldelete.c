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
	ctr = lock_list[lock_index].lock_qhead;
	int flag = 0;		
	while(EMPTY != getfirst(ctr)){
		flag = 1;
		proctab[ctr].gotDeleted = 1;
		dequeue(ctr);
		ready(ctr, RESCHNO);
	}
	if(flag){
		kprintf("did something here");
		resched();
	}
	kprintf("done");
	restore(ps);
	return(OK);
}