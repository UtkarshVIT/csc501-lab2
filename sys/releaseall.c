#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <stdio.h>
#include "lock.h"
#include <sleep.h>

int release(int pid, int ldes);
int get_next_process(int ldesc, int *high_prio);

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

int release(int pid, int lock_index){
    int nextpid = 0, max_w_prio = 0;

    proctab[pid].locktype[lock_index] = LNONE;
    ltable[lock_index].holders[pid] = LNONE;

    if(ltable[lock_index].ltype == READ)
        if(--ltable[lock_index].nreaders)
            return OK;
        kprintf("entering this\n");
        nextpid = get_next_process(lock_index, &max_w_prio);
        kprintf("exiting this\n");
        if(nextpid == -1){
            kprintf("in none");
            ltable[lock_index].ltype = LNONE;
            return OK;
        }

        if(proctab[nextpid].locktype[lock_index] == READ){
            kprintf("in read\n");
            int ctr = q[nextpid].qprev;
            dequeue(nextpid);
            ready(nextpid,RESCHNO);
            ltable[lock_index].holders[nextpid] = READ;
            ltable[lock_index].nreaders++;
            ltable[lock_index].ltype = READ;
            while(ctr != ltable[lock_index].lqhead && q[ctr].qkey >= max_w_prio){
                if(proctab[ctr].locktype[lock_index] == READ){
                    ltable[lock_index].nreaders++;
                    dequeue(ctr);
                    ready(ctr,RESCHNO);
                    ltable[lock_index].holders[ctr] = READ;
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
            dequeue(nextpid);
            ready(nextpid,RESCHNO);
            ltable[lock_index].holders[nextpid] = WRITE;
        }

        return OK;
    }

int get_next_process(int lock_index, int *high_prio){

    int ctr = q[ltable[lock_index].lqtail].qprev;
    int best_reader;
    int best_reader_priority = -1;

    int best_writer;
    int best_writer_priority = -1;   
    if(ctr == ltable[lock_index].lqhead)
    {
        return -1;
    }

    while(ctr != ltable[lock_index].lqhead){
        if(proctab[ctr].locktype[lock_index] == WRITE){
            if(best_writer_priority <= q[ctr].qkey ){
                best_writer_priority = q[ctr].qkey;
                best_writer = ctr;
            }
        }
        else{
            if(best_reader_priority <= q[ctr].qkey){
                best_reader_priority = q[ctr].qkey;
                best_reader = ctr;
            }
        }
        ctr=q[ctr].qprev;
    }
    if(best_writer_priority>best_reader_priority){
        *high_prio=-1;
        return best_writer;
    }
    else if(best_writer_priority<best_reader_priority){
        *high_prio= best_reader_priority;
        return best_reader;
    }
    else{
        if((proctab[best_reader].plreqtime - proctab[best_writer].plreqtime)>600){
            *high_prio=-1;
            return best_writer;
        }
        *high_prio= best_reader_priority;
        return best_reader;
    }
}


