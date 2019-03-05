#include <kernel.h>
#include <proc.h>
#include <q.h>
#include "lock.h"

extern unsigned long ctr1000;

int get_next_process(int lock_index){

    unsigned long curr_time = ctr1000;
    int ctr = q[lock_list[lock_index].lock_lqtail].qprev;
    int best_reader;
    int best_reader_priority = -1;
    unsigned long best_reader_time = curr_time;

    int best_writer;
    int best_writer_priority = -1;   
    unsigned long best_writer_time = curr_time;

    if(ctr == lock_list[lock_index].lock_qhead)
    {
        return -1;
    }

    while(ctr != lock_list[lock_index].lock_qhead){
        if(proctab[ctr].locktype[lock_index] == WRITE){
            if(best_writer_priority <= q[ctr].qkey && best_writer_time > proctab[ctr].plreqtime){
                best_writer_priority = q[ctr].qkey;
                best_writer = ctr;
                best_writer_time = proctab[ctr].plreqtime;
            }
        }
        else{
            if(best_reader_priority <= q[ctr].qkey && best_reader_time > proctab[ctr].plreqtime){
                best_reader_priority = q[ctr].qkey;
                best_reader = ctr;
                best_reader_time = proctab[ctr].plreqtime;
            }
        }
        ctr=q[ctr].qprev;
    }
    if(best_writer_priority>best_reader_priority){
        return best_writer;
    }
    else if(best_writer_priority<best_reader_priority){
        return best_reader;
    }
    else{
        if(proctab[best_reader].plreqtime > proctab[best_writer].plreqtime){
            return best_writer;
        }
        return best_reader;
    }
}

int release(int pid, int lock_index){
    int nextpid = 0;

    if(lock_index<0 || lock_index>49 || lock_list[lock_index].lock_type == DELETED || proctab[pid].locktype[lock_index] == FREE)
    {
        return SYSERR;
    }

    proctab[pid].locktype[lock_index] = FREE;

    if(lock_list[lock_index].lock_type == READ)
        if(--lock_list[lock_index].reader_count)
            return OK;
    else
        --lock_list[lock_index].writer_count;

    if(proctab[pid].pstate != PRCURR && proctab[pid].pstate != PRREADY && pid != currpid){
        dequeue(pid);
        if(proctab[pid].gotDeleted)
            return DELETED;
        return OK;
    }
    
    nextpid = get_next_process(lock_index);
        
    if(nextpid == -1){
        lock_list[lock_index].lock_type = FREE;
        return OK;
    }

    if(proctab[nextpid].locktype[lock_index] == READ){
        int ctr = q[nextpid].qprev;
        dequeue(nextpid);
        ready(nextpid,RESCHNO);
        lock_list[lock_index].reader_count++;
        lock_list[lock_index].lock_type = READ;
        while(ctr != lock_list[lock_index].lock_qhead){
            if(proctab[ctr].locktype[lock_index] == READ){
                lock_list[lock_index].reader_count++;
                dequeue(ctr);
                ready(ctr,RESCHNO);
                    ctr=q[ctr].qprev;
            }
            else{
                break;
            }
        }
    }

    else{
        lock_list[lock_index].lock_type = WRITE;
        lock_list[lock_index].writer_count++;
        dequeue(nextpid);
        ready(nextpid,RESCHNO);
    }
    if(proctab[pid].gotDeleted)
        return DELETED;
    return OK;
}

int releaseall(int numlocks, int locks, ...)
{
    STATWORD ps;
    int ret = OK;
    int lock_index;
    int* base_add = &locks;
    int i;
    disable(ps);

    for(i=0;i<numlocks;i++)
    {
        lock_index = (int)*(base_add+i);
        ret = release(currpid, lock_index);
    }
    resched();
    restore(ps);
    return ret;
}