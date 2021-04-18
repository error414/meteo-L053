#ifndef POOLS_H
#define POOLS_H

#define STREAM_TX_POOL_SIZE 5
#define STREAM_BUFFER_SIZE 64
#define STREAM_MESSAGE_SIZE 23

//size of struct must be x/ 4
typedef struct {
	char message[STREAM_MESSAGE_SIZE];
	uint8_t size;
} poolStreamObject_t;

extern memory_pool_t streamMemPool;
extern mailbox_t streamMail;

void shared_pools_init(void);

#endif
