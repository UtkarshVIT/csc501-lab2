#include <kernel.h>
#include <proc.h>
#include <q.h>
#include "lock.h"

struct lock_data lock_list[NLOCKS];
void linit()
{
	int i=0, j;
	while(i<NLOCKS){
		lock_list[i].ltype = FREE;
		lock_list[i].reader_count = 0;
		lock_list[i].writer_count = 0;
		lock_list[i].lock_qhead = newqueue();
		lock_list[i].lock_lqtail = 1 + lock_list[i].lock_qhead;
		++i;
	}
}