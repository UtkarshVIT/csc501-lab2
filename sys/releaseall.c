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

    proctab[pid].locktype[lock_index] = FREE;

    if(lock_list[lock_index].ltype == READ)
        if(--lock_list[lock_index].reader_count)
            return OK;
    else
        --lock_list[lock_index].writer_count;

    kprintf("entering this\n");
    nextpid = get_next_process(lock_index);
    kprintf("exiting this, returned: %d\n", nextpid);
        
    if(nextpid == -1){
        kprintf("in none");
        lock_list[lock_index].ltype = FREE;
        return OK;
    }

    if(proctab[nextpid].locktype[lock_index] == READ){
        kprintf("in read\n");
        int ctr = q[nextpid].qprev;
        dequeue(nextpid);
        ready(nextpid,RESCHNO);
        lock_list[lock_index].reader_count++;
        lock_list[lock_index].ltype = READ;
        while(ctr != lock_list[lock_index].lock_qhead){
            if(proctab[ctr].locktype[lock_index] == READ){
                kprintf("unlocking\n");
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
        kprintf("in write\n");
        lock_list[lock_index].ltype = WRITE;
        lock_list[lock_index].writer_count++;
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