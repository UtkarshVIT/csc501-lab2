#define NLOCKS 50
#define	LAVAILABLE 1		/* this lock is available */

/* Constants for lock type */
#define LNONE 0
#define READ 1  
#define WRITE 2


struct lentry{
	char lstate;
	int ltype;  
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
