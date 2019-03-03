#ifndef _LOCK_H_
#define _LOCK_H_

#ifndef NLOCKS
#define NLOCKS	50
#endif


#define VERSION_POSS 100
#define READ   1
#define WRITE    2

#define	LFREE	'\01' 
#define	LUSED	'\02'

#define isbadlock(l) (l<0 || l>=NLOCKS)

struct lentry {
	char	lstate;
	int	locknum;
	int	lqhead;
	int 	lqtail;
	int	nreaders;
	int 	nwriters;
	int version;
};

extern struct lentry locks[];
extern int nextlock;

extern int lockholdtab[][NLOCKS];

void linit();
int lcreate();
int ldelete(int lockdescriptor);
int lock(int ldes1, int type, int priority);
int releaseall(int numlocks, int ldes1, ...);
int bestinq(int lock);
int release(int pid, int ldes);
#endif