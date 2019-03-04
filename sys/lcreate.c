#include <kernel.h>
#include <proc.h>
#include "lock.h"

int lcreate() {
    STATWORD ps;
    int ldes, i=0;
    disable(ps);
    while (i < NLOCKS) {
        if (ltable[i].lstate == DELETED || ltable[i].lstate == LAVAILABLE){
            ltable[i].lstate = LAVAILABLE;
            ltable[i].ltype = LNONE;
            restore(ps);
            return i;
        }
        ++i;
    }
    
}