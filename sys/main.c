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

/*----------------------------------Test 4---------------------------*/
void semaphoreProc4(char *msg, int sem){
        kprintf ("  %s: to acquire sem\n", msg);
        wait(sem);
        kprintf ("  %s: acquired sem, sleep 1s\n", msg);
        sleep(1);
        kprintf ("  %s: to release sem\n", msg);
        signal(sem);
        kprintf("temp");
}
void randomSemProc4(char *msg, int lck){
        int i;
        kprintf ("starting %s\n", msg);
        for(i=0;i<100;i++)
                kprintf("%s", msg);
        kprintf ("\n %s sleeping for 3sec.\n");
        
        sleep(1);
        
        i=0;
        for(i=0;i<100;i++)
                kprintf("%s", msg);
        kprintf("\n");
    }
void writer4(char *msg, int lck){
        kprintf ("  %s: to acquire lock\n", msg);
        lock(lck, WRITE, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock, sleep 1s\n", msg);
        sleep(1);
        kprintf ("  %s: to release lock\n", msg);
        releaseall(1, lck);
}

void random4(char *msg, int lck){
        int i;
        kprintf ("starting %s\n", msg);
        for(i=0;i<100;i++)
                kprintf("%s", msg);
        kprintf ("\n %s sleeping for 3sec.\n");
        
        sleep(2);
        
        i=0;
        for(i=0;i<100;i++)
                kprintf("%s", msg);
        kprintf("\n");
    }

void testSem(){
    int semap = screate(1);
    int sem1, sem2, sem3;
    sem1 = create(semaphoreProc4, 2000, 25, "sem1", 2, "S1", semap);
    sem2 = create(randomSemProc4, 2000, 30, "sem2", 2, "R1", semap);
    sem3 = create(semaphoreProc4, 2000, 35, "sem3", 2, "S2", semap);

    resume(sem1);
    resume(sem2);
    sleep(2);
    resume(sem3);
    
}

void testCustomLocks(){
    int lock = lcreate();
    int writer1, random1, writer2;

    writer1 = create(writer4, 2000, 20, "writer1", 2, "W1", lock);
    random1 = create(random4, 2000, 25, "random1", 1, "R1", lock);
    writer2 = create(writer4, 2000, 30, "writer2", 2, "W2", lock);

    resume(writer1);
    resume(random1);
    sleep(2);
    resume(writer2);
    sleep(10);
}


int main( )
{

    kprintf("\n-----------------using custom locks-------------------\n");
    testCustomLocks();
    kprintf("\n-----------------using Xinu sem-----------------------\n");
    testSem();
    kprintf("reached here");
     shutdown();
    return 0;
}