#include <kernel.h>
#include <proc.h>
#include <q.h>
#include "lock.h"

extern unsigned long ctr1000;

int get_next_process(int lock_index){

    unsigned long curr_time = ctr1000;
    int ctr = q[ltable[lock_index].lqtail].qprev;
    int best_reader;
    int best_reader_priority = -1;
    unsigned long best_reader_time = curr_time;

    int best_writer;
    int best_writer_priority = -1;   
    unsigned long best_writer_time = curr_time;

    if(ctr == ltable[lock_index].lqhead)
    {
        return -1;
    }

    while(ctr != ltable[lock_index].lqhead){
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

    proctab[pid].locktype[lock_index] = FREE;

    if(ltable[lock_index].ltype == READ)
        if(--ltable[lock_index].reader_count)
            return OK;
    else
        --ltable[lock_index].writer_count;

    kprintf("entering this\n");
    nextpid = get_next_process(lock_index);
    kprintf("exiting this, returned: %d\n", nextpid);
        
    if(nextpid == -1){
        kprintf("in none");
        ltable[lock_index].ltype = FREE;
        return OK;
    }

    if(proctab[nextpid].locktype[lock_index] == READ){
        kprintf("in read\n");
        int ctr = q[nextpid].qprev;
        dequeue(nextpid);
        ready(nextpid,RESCHNO);
        ltable[lock_index].reader_count++;
        ltable[lock_index].ltype = READ;
        while(ctr != ltable[lock_index].lqhead){
            if(proctab[ctr].locktype[lock_index] == READ){
                kprintf("unlocking\n");
                ltable[lock_index].reader_count++;
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
        kprintf("in write\n");
        ltable[lock_index].ltype = WRITE;
        ltable[lock_index].writer_count++;
        dequeue(nextpid);
        ready(nextpid,RESCHNO);
    }

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