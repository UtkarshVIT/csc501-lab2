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
	ldelete (lck);
	kprintf ("Test 1 ok\n");
}

/*----------------------------------Test 2---------------------------*/
char output2[10];
int count2;
void reader2 (char msg, int lck, int lprio)
{
        int     ret;

        kprintf ("  %c: to acquire lock\n", msg);
        lock (lck, READ, lprio);
        output2[count2++]=msg;
        kprintf ("  %c: acquired lock, sleep 3s\n", msg);
        sleep (3);
        output2[count2++]=msg;
        kprintf ("  %c: to release lock\n", msg);
	releaseall (1, lck);
}

void writer2 (char msg, int lck, int lprio)
{
	kprintf ("  %c: to acquire lock\n", msg);
        lock (lck, WRITE, lprio);
        output2[count2++]=msg;
        kprintf ("  %c: acquired lock, sleep 3s\n", msg);
        sleep (3);
        output2[count2++]=msg;
        kprintf ("  %c: to release lock\n", msg);
        releaseall (1, lck);
}

void test2 ()
{
        count2 = 0;
        int     lck;
        int     rd1, rd2, rd3, rd4;
        int     wr1;

        kprintf("\nTest 2: wait on locks with priority. Expected order of"
		" lock acquisition is: reader A, reader B, reader D, writer C & reader E\n");
        lck  = lcreate ();
        assert (lck != SYSERR, "Test 2 failed");

	rd1 = create(reader2, 2000, 20, "reader2", 3, 'A', lck, 20);
	rd2 = create(reader2, 2000, 20, "reader2", 3, 'B', lck, 30);
	rd3 = create(reader2, 2000, 20, "reader2", 3, 'D', lck, 25);
	rd4 = create(reader2, 2000, 20, "reader2", 3, 'E', lck, 20);
        wr1 = create(writer2, 2000, 20, "writer2", 3, 'C', lck, 28);
	
        kprintf("-start reader A, then sleep 1s. lock granted to reader A\n");
        resume(rd1);
        sleep (1);

        kprintf("-start writer C, then sleep 1s. writer waits for the lock\n");
        resume(wr1);
        sleep10 (1);


        kprintf("-start reader B, D, E. reader B is granted lock.\n");
        resume (rd2);
	resume (rd3);
	resume (rd4);


        sleep (15);
        kprintf("output=%s\n", output2);
        assert(mystrncmp(output2,"ABABCCDEED",10)==0,"Test 2 FAILED\n");
        kprintf ("Test 2 OK\n");
}

/*----------------------------------Test 3---------------------------*/
void reader3 (char *msg, int lck)
{
        int     ret;

        kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, READ, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock\n", msg);
        kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
}

void writer3 (char *msg, int lck)
{
        kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, WRITE, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock, sleep 10s\n", msg);
        sleep (10);
        kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
}

void test3 ()
{
        int     lck;
        int     rd1, rd2;
        int     wr1;

        kprintf("\nTest 3: test the basic priority inheritence\n");
        lck  = lcreate ();
        assert (lck != SYSERR, "Test 3 failed");

        rd1 = create(reader3, 2000, 25, "reader3", 2, "reader A", lck);
        rd2 = create(reader3, 2000, 30, "reader3", 2, "reader B", lck);
        wr1 = create(writer3, 2000, 20, "writer3", 2, "writer", lck);

        kprintf("-start writer, then sleep 1s. lock granted to write (prio 20)\n");
        resume(wr1);
        sleep (1);

        kprintf("-start reader A, then sleep 1s. reader A(prio 25) blocked on the lock\n");
        resume(rd1);
        sleep (1);
    assert (getprio(wr1) == 25, "Test 3 failed");

        kprintf("-start reader B, then sleep 1s. reader B(prio 30) blocked on the lock\n");
        resume (rd2);
    sleep (1);
    assert (getprio(wr1) == 30, "Test 3 failed");
    
    kprintf("-kill reader B, then sleep 1s\n");
    kill (rd2);
    sleep (1);
    assert (getprio(wr1) == 25, "Test 3 failed");

    kprintf("-kill reader A, then sleep 1s\n");
    kill (rd1);
    sleep(1);
    assert(getprio(wr1) == 20, "Test 3 failed");

        sleep (8);
        kprintf ("Test 3 OK\n");
}


/*----------------------------------Test 4---------------------------*/
/*
void reader4 (char *msg, int lck)
{
        int     ret;

        kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, READ, DEFAULT_LOCK_PRIO);
        int i=0;
        for(i=0;i<1000;i++)
            kprintf("%s", msg);
        kprintf ("acquired lock", msg);
        releaseall (1, lck);
}

void writer4 (char *msg, int lck)
{
        kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, WRITE, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock\n", msg);
        int i=0;
        for(i=0;i<1000;i++)
            kprintf("%s", msg);
        kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
}
*/
void lp1(int lck){
    kprintf("%s(priority = %d) is requesting to enter critical section\n", proctab[currpid].pname, getprio(currpid));    
    lock(lck, WRITE, DEFAULT_LOCK_PRIO);
        kprintf("%s(priority = %d) has entered critical section\n", proctab[currpid].pname, getprio(currpid));
        sleep(1);
        //sleep(1);
        int i = 0;
        for(i; i < 100;i++){
                kprintf("A");
            //    sleep(1);
        }
        kprintf("\n%s has completed critical section(ramped up priority = %d)\n", proctab[currpid].pname, getprio(currpid));
        releaseall(1, lck);
        kprintf("%s original priority = %d\n", proctab[currpid].pname, getprio(currpid));
    
}

void lp2(int lck){
    kprintf("%s(priority = %d) has started\n", proctab[currpid].pname, getprio(currpid));
    sleep(1);
    int i = 0;
    for(i; i < 100;i++){
            kprintf("B");
            //sleep(1);
    }
    kprintf("\n%s has completed its execution\n");
}

void lp3(int lck, int pr1){
    kprintf("%s(priority = %d) is requesting to enter critical section\n", proctab[currpid].pname, getprio(currpid));
    kprintf("Hence, ramping up the priority of %s\n", proctab[pr1].pname);
    if(lock(lck, WRITE, DEFAULT_LOCK_PRIO) == OK){
            kprintf("%s(priority = %d) has entered critical section\n", proctab[currpid].pname, getprio(currpid));
            int i = 0;
            for(i; i < 100;i++){
                    kprintf("C");
              //      sleep(1);
            }
            kprintf("\n%s has completed critical section\n", proctab[currpid].pname);
            releaseall(1, lck);
    }
}


void testCustomLocks(){
    int lck;
    lck = lcreate();

    int pr1 = create(lp1, 2000, 10, "p1", 1, lck);
    int pr2 = create(lp2, 2000, 20, "p2", 1, lck);
    int pr3 = create(lp3, 2000, 30, "p3", 2, lck, pr1);
    
    resume(pr1);
  //  sleep(1);
    resume(pr2);
    sleep(1);
    resume(pr3);
    sleep(5);

    ldelete(lck);



    //assert (getprio(wr1) == 25, "Test 3 failed"
    kprintf("done\n");
}
int main( )
{
    kprintf("in main funx\n");
	//test1();
	//test2();
	//stest3();
    testCustomLocks();

        /* The hook to shutdown QEMU for process-like execution of XINU.
         * This API call exists the QEMU process.
         */
        //shutdown();
    return 0;
}




