#define NLOCKS 50
#define	LAVAILABLE 1		/* this lock is available */

/* Constants for lock type */
#define FREE 0
#define READ 1  
#define WRITE 2


struct lock_data{
	int lstate;
	int ltype;  
	int reader_count;
	int writer_count;
	int lock_qhead;  
	int lock_lqtail;
};

extern struct lock_data lock_list[NLOCKS];

int lock (int ldes1, int type, int priority);
void linit();
int ldelete(int lockdescriptor);
int releaseall(int, int, ...);
int lcreate();
