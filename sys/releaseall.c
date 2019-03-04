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
void remote_readers(int ldesc, int pid_, int high_prio);

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
    int pid_;
    int proceed = 1, nextpid = 0, max_w_prio = 0;

    proctab[pid].locktype[lock_index] = LNONE;
    ltable[lock_index].holders[pid] = LNONE;

    if(ltable[lock_index].ltype == READ)
        if(--ltable[lock_index].nreaders)
            return OK;
        
    nextpid = get_next_process(lock_index, &max_w_prio);
    
    if(nextpid == -1){
        ltable[lock_index].ltype = LNONE;
        return OK;
    }
    
    if(proctab[nextpid].locktype[lock_index] == READ){
        pid_ = q[nextpid].qprev;
        dequeue(nextpid);
        ready(nextpid,RESCHNO);
        ltable[lock_index].holders[nextpid] = READ;
        ltable[lock_index].nreaders++;
        remote_readers(lock_index,pid_, max_w_prio);
        ltable[lock_index].ltype = READ;
    }
    else{
        ltable[lock_index].ltype = WRITE;
        dequeue(nextpid);
        ready(nextpid,RESCHNO);
        ltable[lock_index].holders[nextpid] = WRITE;
    }

    return OK;
}

void remote_readers(int lock_index, int pid_, int high_prio){
    int pid = pid_;

    while(pid != ltable[lock_index].lqhead && q[pid].qkey >= high_prio){
        if(proctab[pid].locktype[lock_index] == READ){
            ltable[lock_index].nreaders++;
            dequeue(pid);
            ready(pid,RESCHNO);
            ltable[lock_index].holders[pid] = READ;
            pid=q[pid].qprev;
        }

        else
            break;
    }
}

int get_next_process(int ldesc, int *high_prio){
    if(q[ltable[ldesc].lqtail].qprev == ltable[ldesc].lqhead)
        return SYSERR;

    int pid_1 = q[ltable[ldesc].lqtail].qprev;
    int pid_2;
    int timediff = 0,retVal = 0;

    if(proctab[pid_1].locktype[ldesc] == WRITE){
        retVal = pid_1;
        *high_prio = -1;
    }

    else{
        pid_2 = q[pid_1].qprev;
        if(q[pid_1].qkey > q[pid_2].qkey){
            retVal = pid_1;
            *high_prio = -1;
        }

        else if((q[pid_1].qkey == q[pid_2].qkey)){
            while((pid_2 != ltable[ldesc].lqhead) && (q[pid_1].qkey == q[pid_2].qkey)){
                if(proctab[pid_2].locktype[ldesc] == READ)
                    retVal = pid_1;
                else{
                    timediff = proctab[pid_2].plreqtime - proctab[pid_1].plreqtime;
                    if(timediff <= 1000 && timediff >= -1000)
                        retVal = pid_2;
                    else
                        retVal = pid_1;
                    break;
                }
                pid_2 = q[pid_2].qprev;
            }
        }
        pid_1 = q[ltable[ldesc].lqtail].qprev;
        {
            while(pid_1!=ltable[ldesc].lqhead && proctab[pid_1].locktype[ldesc]!=WRITE)
                pid_1 = q[pid_1].qprev;
            *high_prio = q[pid_1].qkey;
        }
    }
    return retVal;
}