#define NLOCKS 50
#define FREE 0
#define READ 1
#define WRITE 2

struct lock_data{
	int lock_type;
	int lock_qhead;
	int lock_lqtail;
	int reader_count;
	int writer_count;
};

extern struct lock_data lock_list[NLOCKS];