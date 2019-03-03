#include <kernel.h>
#include <proc.h>
#include "lock.h"

LOCAL int newldes();

int lcreate() {
    STATWORD ps;
    int ldes, i;
    disable(ps);
    for (i = 0; i < NLOCKS; i++) {
        if (ltable[i].lstate == DELETED)
        {
            ltable[i].lstate = LAVAILABLE;
            ltable[i].ltype = LNONE;
            restore(ps);
            return i;
        }
    }
    
}