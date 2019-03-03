#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include "lock.h"
#include <stdio.h>
#include <q.h>

#define DEFAULT_LOCK_PRIO 20

#define assert(x,error) if(!(x)){ \
            kprintf(error);\
            return;\
            }
int mystrncmp(char* des,char* target,int n){
    int i;
    for (i=0;i<n;i++){
        if (target[i] == '.') continue;
        if (des[i] != target[i]) return 1;
    }
    return 0;
}

/*--------------------------------Test 1--------------------------------*/
 
void reader1 (char *msg, int lck)
{
	lock (lck, READ, DEFAULT_LOCK_PRIO);
	kprintf ("  %s: acquired lock, sleep 2s\n", msg);
	sleep (2);
	kprintf ("  %s: to release lock\n", msg);
	releaseall (1, lck);
}

void test1 ()
{
	int	lck;
	int	pid1;
	int	pid2;

	kprintf("\nTest 1: readers can share the rwlock\n");
	lck  = lcreate ();
	assert (lck != SYSERR, "Test 1 failed");

	pid1 = create(reader1, 2000, 20, "reader a", 2, "reader a", lck);
	pid2 = create(reader1, 2000, 20, "reader b", 2, "reader b", lck);

	resume(pid1);
	resume(pid2);
	
	sleep (5);
	//ldelete (lck);
	kprintf ("Test 1 ok\n");
}

struct lentry locks[NLOCKS];
int nextlock;
int lockholdtab[NPROC][NLOCKS];

void linit()
{
	struct lentry *lptr;
	int i, j;
	nextlock = NLOCKS-1;

	for(i = 0; i < NLOCKS; i++)
	{
		(lptr = &locks[i])->lstate = LFREE;
		lptr -> locknum = i;
		lptr -> lqtail = 1 + (lptr -> lqhead = newqueue());
		lptr -> nreaders = 0;
		lptr -> nwriters = 0;
		lptr -> version = 0;
	}

	for(i = 0; i < NPROC; i ++)
		for(j = 0; j < NLOCKS; j ++)
			lockholdtab[i][j] = 0;
}

int main( )
{
	linit();
    kprintf("in main funx\n");
	test1();
	//test2();
	//test3();

        /* The hook to shutdown QEMU for process-like execution of XINU.
         * This API call exists the QEMU process.
         */
        //shutdown();
    return 0;
}




