#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include <sleep.h>
#include "lock.h"

int lock(int ldes1, int type, int priority)
{
	STATWORD ps;
	struct  lentry  *lptr;
	struct  pentry  *pptr;
	int item, wait = 0, lockindex = ldes1 /100;
	int i;
	disable(ps);
	if ((lptr= &locks[lockindex])->lstate==LFREE || lptr -> version != ldes1 % 100) 
	{	
		//kprintf("should not be here nth lock %d with version %d\n",lockindex,lptr->version);
		restore(ps);
		return(SYSERR);
	}
	for (i=0;i<NLOCKS;i++)
	{
		if (locks[i].locknum<0 && locks[i].nwriters>2)
		{
			restore(ps);
			return(SYSERR);
		}
		
	}
	if((locks[lockindex].nreaders == 0) && (locks[lockindex].nwriters == 0)) 
	//wait = 0;
	{		
			//kprintf("in here\n");
		lockholdtab[currpid][lockindex] ++;	
		if(type == READ) lptr -> nreaders ++;
		else if(type == WRITE) lptr -> nwriters ++;
		else kprintf("Type is wrong!!!");
		restore(ps);
		return(OK);
	}
	else if((locks[lockindex].nreaders > 0) && (locks[lockindex].nwriters == 0))
	{	
		//kprintf("In the 2nd condition\n");
		wait = 0;
		if(type == READ)
		{
			for(item = q[lptr -> lqtail].qprev; (item != lptr -> lqhead) && (priority < q[item].qkey); item = q[item].qprev)
			{
				if(q[item].qtype == WRITE)
				{
					wait = 1;
					(pptr = &proctab[currpid]) -> pstate = PRLWAIT;
					pptr -> plock = ldes1;
					pptr -> plwaitret = OK;
					insert(currpid, lptr -> lqhead, priority);
					q[currpid].qtype = type;
					q[currpid].qwait = clktime;
					resched();
					restore(ps);
					return pptr -> plwaitret;					
					//break;
				}
				if((q[item].qtype!= READ) && (q[item].qtype!=WRITE))
				{
					restore (ps);
					return (OK);
				}
			}	
			if (wait==0)
			{
				lockholdtab[currpid][lockindex] ++;	
						//if(type == READ) 
				lptr -> nreaders ++;
						//else if(type == WRITE) lptr -> nwriters ++;
						//else kprintf("Type is wrong!!!");
				restore(ps);
				return(OK);
			}
	//	}
		}
		if(type == WRITE) 
		{
			(pptr = &proctab[currpid]) -> pstate = PRLWAIT;
			pptr -> plock = ldes1;
			pptr -> plwaitret = OK;
			insert(currpid, lptr -> lqhead, priority);
			q[currpid].qtype = type;
			q[currpid].qwait = clktime;
			resched();
			restore(ps);
			return pptr -> plwaitret;	
		}
		//wait = 1;
	}
	if((locks[lockindex].nreaders == 0) && (locks[lockindex].nwriters == 1))
	//else kprintf("Impossible! How could I get here?");
	//if(wait)
	{
		(pptr = &proctab[currpid]) -> pstate = PRLWAIT;
		pptr -> plock = ldes1;
		pptr -> plwaitret = OK;
		insert(currpid, lptr -> lqhead, priority);
		q[currpid].qtype = type;
		q[currpid].qwait = clktime;
		resched();
		restore(ps);
		return pptr -> plwaitret;
	}
	if((locks[lockindex].nwriters < 0) && (locks[lockindex].nwriters<0))
	{
		restore(ps);
		return SYSERR;
	}
}