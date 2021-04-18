#include "ch.h"
#include "pools.h"


////////////////////////////////////////////////////////////////////////////
poolStreamObject_t streamPoolObject[STREAM_TX_POOL_SIZE];
MEMORYPOOL_DECL(streamMemPool, STREAM_TX_POOL_SIZE, 0, NULL);

msg_t streamDataLetter[STREAM_TX_POOL_SIZE];
MAILBOX_DECL(streamMail, &streamDataLetter, STREAM_TX_POOL_SIZE);
/////////////////////////////////////////////////////////////////////////

void shared_pools_init(){
	chPoolObjectInit(&streamMemPool, sizeof(streamPoolObject[0]), NULL);
	chPoolLoadArray(&streamMemPool, streamPoolObject, STREAM_TX_POOL_SIZE);
}
