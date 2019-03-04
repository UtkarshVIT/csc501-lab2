#include <kernel.h>
#include <proc.h>
#include "lock.h"

int lcreate() {
    STATWORD ps;
    int ldes, i=0;
    disable(ps);
    while (i < NLOCKS) {
        if (lock_list[i].lstate == DELETED || lock_list[i].lstate == LAVAILABLE){
            lock_list[i].lstate = LAVAILABLE;
            lock_list[i].ltype = FREE;
            restore(ps);
            return i;
        }
        ++i;
    }
    
}