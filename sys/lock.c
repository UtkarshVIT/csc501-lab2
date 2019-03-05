#include <kernel.h>
#include <proc.h>
#include <q.h>
#include "lock.h"

extern unsigned long ctr1000;

int get_virtual_prio(int pid){
	int i=0;
	int max_prio = proctab[pid].pprio;
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
	return max_prio;
}

void insert_in_prio_queue(int lock_index, int priority, int lock_type){
	proctab[currpid].locktype[lock_index] = lock_type;
	proctab[currpid].pstate = PRWAIT;	
	proctab[currpid].plreqtime = ctr1000;
	insert(currpid, lock_list[lock_index].lock_qhead, priority);
	resched();
}

int lock(int lock_index, int lock_type, int priority){
	STATWORD ps;
	disable(ps);

	/* If the lock lock_type in FREE */
	if(lock_list[lock_index].lock_type == FREE){
		lock_list[lock_index].lock_type = lock_type;
		proctab[currpid].locktype[lock_index] = lock_type;

		if(lock_type == READ){
			lock_list[lock_index].reader_count++;
		}
		else{
			lock_list[lock_index].writer_count++;
		}
	}

	/* If the lock lock_type for this already READ */
	else if(lock_list[lock_index].lock_type == READ){
		
		/* If the request lock_type is WRITE */
		if (lock_type == WRITE)
			insert_in_prio_queue(lock_index, priority, lock_type);
		
		/* If the request lock_type is READ */
		else if(lock_type == READ){
			int ctr, flag = 0;
			ctr = q[lock_list[lock_index].lock_lqtail].qprev;

			for (ctr = q[lock_list[lock_index].lock_lqtail].qprev; (ctr != lock_list[lock_index].lock_qhead) && (priority < q[ctr].qkey); ctr = q[ctr].qprev) {
				if (proctab[ctr].locktype[lock_index] == WRITE) {
					flag = 1;
					break;
				}
			}

			if(flag){
				insert_in_prio_queue(lock_index, priority, lock_type);
			}

			else{
				proctab[currpid].locktype[lock_index] = lock_type;
				lock_list[lock_index].reader_count += 1;
				lock_list[lock_index].lock_type = lock_type;
			}
		}
	}

	else if(lock_list[lock_index].lock_type == WRITE){
		insert_in_prio_queue(lock_index, priority, lock_type);
	} 

	restore(ps);
	return (OK);
}