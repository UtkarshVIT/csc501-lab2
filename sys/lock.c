#include <kernel.h>
#include <q.h>
#include <proc.h>
#include "lock.h"

extern unsigned long ctr1000;

void insert_in_prio_queue(int lock_index, int priority, int lock_type){
	proctab[currpid].lock_type[lock_index] = lock_type;
	proctab[currpid].pstate = PRWAIT;	
	proctab[currpid].lock_q_wait_time[lock_index] = ctr1000;
	insert(currpid, lock_list[lock_index].lock_qhead, priority);
	resched();
}

int lock(int lock_index, int lock_type, int priority){
	STATWORD ps;
	disable(ps);
	if(lock_index<0 || lock_index>49 || lock_list[lock_index].lock_type == DELETED)
    {
    	restore(ps);
        return SYSERR;
    }

	kprintf("\n The lock value is %d", lock_list[lock_index].lock_type);

	/* If the lock lock_type in FREE */
	if(lock_list[lock_index].lock_type == FREE){
		lock_list[lock_index].lock_type = lock_type;
		proctab[currpid].lock_type[lock_index] = lock_type;

		if(lock_type == WRITE){
			lock_list[lock_index].writer_count++;
		}
		else{
			lock_list[lock_index].reader_count++;
		}
	}

	/* If the lock lock_type for this already READ */
	else if(lock_list[lock_index].lock_type == READ){
			
		/* If the request lock_type is READ */
		if(lock_type == READ){
			int ctr, flag = 0;
			ctr = q[lock_list[lock_index].lock_lqtail].qprev;

			for(ctr = q[lock_list[lock_index].lock_lqtail].qprev; (ctr!=lock_list[lock_index].lock_qhead) && (priority<q[ctr].qkey); ctr=q[ctr].qprev){
				if(proctab[ctr].lock_type[lock_index]==WRITE){
					flag = 1;
					break;
				}
			}

			if(flag){
				insert_in_prio_queue(lock_index, priority, lock_type);
			}

			else{
				proctab[currpid].lock_type[lock_index] = lock_type;
				lock_list[lock_index].reader_count += 1;
				lock_list[lock_index].lock_type = lock_type;
			}
		}
		/* If the request lock_type is WRITE */
		else if (lock_type == WRITE)
			insert_in_prio_queue(lock_index, priority, lock_type);

	}

	else if(lock_list[lock_index].lock_type == WRITE){
		insert_in_prio_queue(lock_index, priority, lock_type);
	} 

	restore(ps);
	return (OK);
}