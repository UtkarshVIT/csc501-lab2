#define NLOCKS 50
#define	LAVAILABLE	1		/* this lock is available */

/* Constants for lock type */
#define READ   '\01'       /* locked by a reader */
#define WRITE  '\02'		/* locked by a writer */
#define LNONE	'\03' 		/* locked by no process */
#define	isbadlock(l)	(l<0 || l>=NLOCKS)

struct lentry{
	char lstate;
	char ltype;  
	int nreaders;
	int lqhead;  
	int lqtail;  
	int holders[NPROC];
};

extern struct lentry ltable[NLOCKS];

int lock (int ldes1, int type, int priority);
void linit();
int ldelete(int lockdescriptor);
int releaseall(int, int, ...);
int lcreate();
