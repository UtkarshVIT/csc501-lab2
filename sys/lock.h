#define NLOCKS 50

/* Constants for lock type */
#define FREE 0
#define READ 1  
#define WRITE 2


struct lock_data{
	int lock_type;
	int lock_qhead;  
	int lock_lqtail;		// check state of the lock
	int reader_count;
	int writer_count;

};

extern struct lock_data lock_list[NLOCKS];
/*
int lock (int ldes1, int type, int priority);
void linit();
int ldelete(int lockdescriptor);
int releaseall(int, int, ...);
int lcreate();
*/