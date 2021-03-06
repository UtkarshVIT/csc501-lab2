#include <kernel.h>
#include <proc.h>
#include "lock.h"

int lcreate(){
    STATWORD ps;
    int i=0;
    disable(ps);
    while(i < NLOCKS){
        if(lock_list[i].lock_type == DELETED || lock_list[i].lock_type == FREE){
            lock_list[i].lock_type = FREE;
            restore(ps);
            return i;
        }
        ++i;
    }
    return SYSERR;
}