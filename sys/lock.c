#include <kernel.h>
#include <proc.h>
#include <q.h>
#include "lock.h"

extern unsigned long ctr1000;

void insert_in_prio_queue(int lock_index, int priority, int lock_type){
	proctab[currpid].pstate = PRWAIT;
	proctab[currpid].locktype[lock_index] = lock_type;
	proctab[currpid].plreqtime = ctr1000;
	insert(currpid, ltable[lock_index].lqhead, priority);
	resched();
}

int lock(int lock_index, int lock_type, int priority){
	STATWORD ps;
	disable(ps);
	kprintf("locking\n");

	/* If the lock lock_type in LNONE */
	if(ltable[lock_index].ltype == LNONE){
		ltable[lock_index].ltype = lock_type;
		proctab[currpid].locktype[lock_index] = lock_type;
		//ltable[lock_index].holders[currpid] = lock_type;

		if(lock_type == READ){
			ltable[lock_index].nreaders++;
		}
	}

	/* If the lock lock_type for this already READ */
	else if(ltable[lock_index].ltype == READ){
		
		/* If the request lock_type is WRITE */
		if (lock_type == WRITE)
			insert_in_prio_queue(lock_index, priority, lock_type);
		
		/* If the request lock_type is READ */
		else if(lock_type == READ){
			int ctr, flag = 0;
			ctr = q[ltable[lock_index].lqtail].qprev;

			for (ctr = q[ltable[lock_index].lqtail].qprev; (ctr != ltable[lock_index].lqhead) && (priority < q[ctr].qkey); ctr = q[ctr].qprev) {
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
				//ltable[lock_index].holders[currpid] = lock_type;
				ltable[lock_index].nreaders += 1;
				ltable[lock_index].ltype = lock_type;
			}
		}
	}

	else if(ltable[lock_index].ltype == WRITE){
		insert_in_prio_queue(lock_index, priority, lock_type);
	} 

	restore(ps);
	return (OK);
}