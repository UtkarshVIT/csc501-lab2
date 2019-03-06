#include <kernel.h>
#include <q.h>
#include <proc.h>
#include "lock.h"

extern unsigned long ctr1000;

int get_next_process(int lock_index){

    unsigned long curr_time = ctr1000;
    int ctr = q[lock_list[lock_index].lock_lqtail].qprev;
    int best_reader;
    int best_reader_priority = -1;
    unsigned long best_reader_time = 4294967295;

    int best_writer;
    int best_writer_priority = -1;
    unsigned long best_writer_time = 4294967295;

    if(ctr == lock_list[lock_index].lock_qhead)
    {
        return -1;
    }
    kprintf("\n");
    while(ctr != lock_list[lock_index].lock_qhead){
        //kprintf("%d, %d", ctr, q[ctr].qkey);
        if(proctab[ctr].locktype[lock_index] == WRITE){
            //kprintf("yoyoyo");
            //kprintf("%lu, %lu", best_writer_time, proctab[ctr].lock_q_wait_time[lock_index]);
            if(best_writer_priority <= q[ctr].qkey && best_writer_time >= proctab[ctr].lock_q_wait_time[lock_index]){
                //kprintf("wwww");
                best_writer_priority = q[ctr].qkey;
                best_writer = ctr;
                best_writer_time = proctab[ctr].lock_q_wait_time[lock_index];
            }
        }
        else if(proctab[ctr].locktype[lock_index] == READ){
            //kprintf("yasas");
            if(best_reader_priority <= q[ctr].qkey && best_reader_time >= proctab[ctr].lock_q_wait_time[lock_index]){
                best_reader_priority = q[ctr].qkey;
                best_reader = ctr;
                best_reader_time = proctab[ctr].lock_q_wait_time[lock_index];
            }
        }
        ctr=q[ctr].qprev;
    }
    //kprintf("-->%d, %d\n", best_reader_priority, best_writer_priority);
    //kprintf("==>%d, %d\n", best_reader, best_writer);
    if(best_writer_priority>best_reader_priority){
        //kprintf("here1 %d, \n",best_writer);
        return best_writer;
    }

    else if(best_writer_priority<best_reader_priority){
        //kprintf("here2 %d, \n",best_reader);
        return best_reader;
    }
    else{
        if(proctab[best_reader].lock_q_wait_time[lock_index] - proctab[best_writer].lock_q_wait_time[lock_index]<600){
            return best_writer;
        }
        return best_reader;
    }
}

int release(int pid, int lock_index){
    int nextpid = 0;

    if(lock_index<0 || lock_index>49 || lock_list[lock_index].lock_type == DELETED || !(proctab[pid].locktype[lock_index] == READ || proctab[pid].locktype[lock_index] == WRITE) ||proctab[pid].locktype[lock_index] == FREE)
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
        //kprintf("in here\n");
        dequeue(pid);
        return OK;
    } 

    /////

    unsigned long curr_time = ctr1000;
    int ctr = q[lock_list[lock_index].lock_lqtail].qprev;
    int best_reader;
    int best_reader_priority = -1;
    unsigned long best_reader_time = 4294967295;

    int best_writer;
    int best_writer_priority = -1;
    unsigned long best_writer_time = 4294967295;

    if(ctr == lock_list[lock_index].lock_qhead)
    {
        return -1;
    }

    while(ctr != lock_list[lock_index].lock_qhead){
        //kprintf("%d, %d", ctr, q[ctr].qkey);
        if(proctab[ctr].locktype[lock_index] == WRITE){
            //kprintf("yoyoyo");
            //kprintf("%lu, %lu", best_writer_time, proctab[ctr].lock_q_wait_time[lock_index]);
            if(best_writer_priority <= q[ctr].qkey && best_writer_time >= proctab[ctr].lock_q_wait_time[lock_index]){
                //kprintf("wwww");
                best_writer_priority = q[ctr].qkey;
                best_writer = ctr;
                best_writer_time = proctab[ctr].lock_q_wait_time[lock_index];
            }
        }
        else if(proctab[ctr].locktype[lock_index] == READ){
            //kprintf("yasas");
            if(best_reader_priority <= q[ctr].qkey && best_reader_time >= proctab[ctr].lock_q_wait_time[lock_index]){
                best_reader_priority = q[ctr].qkey;
                best_reader = ctr;
                best_reader_time = proctab[ctr].lock_q_wait_time[lock_index];
            }
        }
        ctr=q[ctr].qprev;
    }
    //kprintf("-->%d, %d\n", best_reader_priority, best_writer_priority);
    //kprintf("==>%d, %d\n", best_reader, best_writer);
    if(best_writer_priority>best_reader_priority){
        //kprintf("here1 %d, \n",best_writer);
        nextpid = best_writer;
    }

    else if(best_writer_priority<best_reader_priority){
        //kprintf("here2 %d, \n",best_reader);
        nextpid = best_reader;
    }
    else{
        if(proctab[best_reader].lock_q_wait_time[lock_index] - proctab[best_writer].lock_q_wait_time[lock_index]<600){
            nextpid = best_writer;
        }
        else{
            nextpid = best_reader;
        }
    }

    /////
        
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
            }
            else{
                break;
            }
            ctr=q[ctr].qprev;
        }
    }

    else{
        lock_list[lock_index].lock_type = WRITE;
        //kprintf("this is now ready\n");
        lock_list[lock_index].writer_count++;
        dequeue(nextpid);
        //kprintf("new we dequed %d\n", nextpid);
        ready(nextpid,RESCHNO);
        //kprintf("put it in ready queue\n");
    }
    if(proctab[pid].gotDeleted)
        return DELETED;
    return OK;
}

int releaseall(int numlocks, int locks, ...){
    STATWORD ps;
    int check, flag=0;
    int lock_index;
    int* base_add = &locks;
    int i=0;
    disable(ps);
    while(i<numlocks)
    {
        lock_index = (int)*(base_add+i);
        check = release(currpid, lock_index);
        if(check == SYSERR)
            flag=1;
        ++i;
    }
    resched();
    restore(ps);
    if(flag)
        return SYSERR;
    return check;
}