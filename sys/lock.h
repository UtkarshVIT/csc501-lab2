#define NLOCKS 50
#define	LAVAILABLE 1		/* this lock is available */

/* Constants for lock type */
#define FREE 0
#define READ 1  
#define WRITE 2


struct lentry{
	int lstate;
	int ltype;  
	int reader_count;
	int writer_count;
	int lqhead;  
	int lqtail;
};

extern struct lentry ltable[NLOCKS];

int lock (int ldes1, int type, int priority);
void linit();
int ldelete(int lockdescriptor);
int releaseall(int, int, ...);
int lcreate();
