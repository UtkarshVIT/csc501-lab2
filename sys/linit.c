#include <kernel.h>
#include <proc.h>
#include <q.h>
#include "lock.h"

struct lentry ltable[NLOCKS];
void linit()
{
	int i=0, j;
	while(i<NLOCKS){
		ltable[i].ltype = FREE;
		ltable[i].lstate = LAVAILABLE;
		ltable[i].reader_count = 0;
		ltable[i].writer_count = 0;
		ltable[i].lqhead = newqueue();
		ltable[i].lqtail = 1 + ltable[i].lqhead;
		++i;
	}
}