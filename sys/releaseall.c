#include <kernel.h>
#include <proc.h>
#include <q.h>
#include "lock.h"
#include <stdio.h>
#include <sleep.h>

int bestone(int lock)
{
	struct lentry *lptr = &locks[lock];
	int highest; 
	int item,member,i;
	//kprintf("IN best\n");

	item = q[lptr -> lqtail].qprev;
	highest = q[lptr -> lqtail].qprev;
	member = q[lptr -> lqtail].qprev;

        if(highest == lptr -> lqhead) return -1; //null
	//kprintf("before dead \n");		
        for(i=0;i<NLOCKS;i++)
        {
        	member = q[item].qprev;
        	if(q[member].qkey < 0)
        	{
        		break;       		
        	}
        	
        }
	//kprintf("calculating best\n");
        for(item = q[item].qprev;q[item].qprev != lptr -> lqhead;item=q[item].qprev)
        {
                //item = q[item].qprev;
        	if(q[item].qkey < q[highest].qkey) break;
        	else if(q[item].qkey > q[highest].qkey) kprintf("Wrong! priority inverse!\n");
        	else
        	{
                        //int wait1 = clktime - q[highest].qwait;
                        //int wait2 = clktime - q[item].qwait;
        		if( (((clktime - q[highest].qwait) - (clktime - q[item].qwait)) <= 1) && (((clktime - q[highest].qwait)- (clktime - q[item].qwait)) >= -1))
        		{
        			if((q[highest].qtype == READ) && (q[item].qtype == WRITE))
        				highest = item;
        			else highest = highest;
        			break;
        		}
                       // else if(wait1 > wait2) highest = highest;
        		else if((clktime - q[highest].qwait) < (clktime - q[item].qwait)) highest = item;
        		else highest = highest;
        		break;
        	}
        }
        return highest;
    }   

    int releaseall(int numlocks, int ldes1, ...){
    	int ldes, flag = 0;
    	unsigned long *a = (unsigned long *)(&ldes1);
    	for ( ; numlocks > 0 ; numlocks--)
    	{
    		ldes = *a++; 
    		STATWORD ps;
    		register struct lentry  *lptr;
    		kprintf("in release \n");
    		int highest, lockindex = ldes / 100;
    		int flag=0;
    		int item;
    		int pid=currpid;

    		disable(ps);
    		if ((lptr= &locks[lockindex])->lstate==LFREE || lptr -> version != ldes % 100) 
    		{
    			restore(ps);
    			return(SYSERR);

    		}

    		else if(lockholdtab[pid][lockindex] > 0) lockholdtab[pid][lockindex]--;
    		else if (lockholdtab[pid][lockindex]==0)
    		{
									//	kprintf("released a lock that you don't hold!\n");
    			restore(ps);
    			return(SYSERR);
    			flag=1;
    		}
    		else if (lockholdtab[pid][lockindex]<0)
    			{	int i;
    				for(i=0;i<NPROC;i++)
    				{
    					lockholdtab[i][lockindex]=0;
    				}

    			}	
							//kprintf("number of readers and writers sre %d %d\n",lptr->nreaders,lptr->nwriters);
    			if((locks[lockindex].nreaders > 0) && (locks[lockindex].nwriters == 0)) lptr -> nreaders --;
    			else if((lptr -> nreaders == 0) && (lptr -> nwriters == 1)) lptr -> nwriters --;
    			else kprintf("Impossible to get here. Something is wrong!!!\n");
							//kprintf("before checking queue");
    			if((lptr -> nreaders == 0) && (lptr -> nwriters == 0))
    			{
    				highest = bestone(lockindex);
									//kprintf("highest is returned\n");
    				while(highest != -1)
    				{
										//kprintf("inside while lopp\n");
    					if(q[highest].qtype == READ)
    					{
											//kprintf("highest is READ TYPE\n");
    						lptr -> nreaders ++;
    						lockholdtab[highest][lockindex] ++;
    						dequeue(highest);
    						ready(highest, RESCHNO);
											//kprintf("priority of process is %d",q[highest].qkey);
    						highest = bestone(lockindex);

    						if((highest != -1) && (q[highest].qtype == WRITE))
    						{
    							item = q[lptr -> lqtail].qprev;
    							while(item != lptr -> lqhead)
    							{
    								if(q[item].qkey < 0) 
    								{
    									flag=1;
    								}
    								item = q[item].qprev;
    							}
    							if(item == lptr -> lqhead) highest = -1;
    						}
    					}

    					else if(q[highest].qtype == WRITE)
    					{	
												//kprintf("highest is WRITE TYPE\n");
    						lptr -> nwriters ++;
    						lockholdtab[highest][lockindex] ++;
    						dequeue(highest);
    						ready(highest, RESCHNO);
												//kprintf("priority of process is %d",q[highest].qkey);
    						restore(ps);
    						break;
    					}
    				}
    			}
    		}
    		resched();
    		if(flag == 1) return(SYSERR);
    		else return(OK);
    	}

        int release(int pid, int ldes){
            STATWORD ps;
            register struct lentry  *lptr;
            int highest, lockindex = ldes / 100;

            disable(ps);
            if (isbadlock(lockindex) || (lptr= &locks[lockindex])->lstate==LFREE || lptr -> version != ldes % 100) 
            {
                restore(ps);
                return(SYSERR);
            }

            if(lockholdtab[pid][lockindex] > 0) lockholdtab[pid][lockindex]--;
            else
            {
        //kprintf("released a lock that you don't hold!\n");
                restore(ps);
                return(SYSERR);
            }

            if((lptr -> nreaders > 0) && (lptr -> nwriters == 0)) lptr -> nreaders --;
            else if((lptr -> nreaders == 0) && (lptr -> nwriters == 1)) lptr -> nwriters --;
            else kprintf("Impossible to get here. Something is wrong!!!\n");

            if((locks[lockindex].nreaders == 0) && (locks[lockindex].nwriters == 0))
            {
                highest = bestone(lockindex);
                while(highest != -1)
                {
                    if(q[highest].qtype == READ)
                    {
                        lptr -> nreaders ++;
                        lockholdtab[highest][lockindex] ++;
                        dequeue(highest);
                        ready(highest, RESCHNO);
                        highest = bestone(lockindex);

                        if((highest != -1) && (q[highest].qtype == WRITE))
                        {
                            int item = q[lptr -> lqtail].qprev;
                    //while(item != lptr -> lqhead)
                    //{
                        //if((q[item].qkey >= q[highest].qkey) && (q[item].qtype == READ)) 
                        //{
                            //highest = item;
                            //kprintf("HELLO BROTHER RAADHU SWAMI\n");
                            break;
                        //}
                        //item = q[item].qprev;
                    //}
                            if(item == lptr -> lqhead) highest = -1;
                        }
                    }
                    else if(q[highest].qtype == WRITE)
                    {
                        lptr -> nwriters ++;
                        lockholdtab[highest][lockindex] ++;
                        dequeue(highest);
                        ready(highest, RESCHNO);
                        break;
                    }
                }
            }
            restore(ps);
            return OK;
        }

