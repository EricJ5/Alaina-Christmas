

#ifndef SERVER_HEADER

#define SERVER_HEADER


#include <threads.h>
#include <pthread.h>
#include <inttypes.h>
#define QUEUE_CAP 128
enum Opcode {
	GET_VOLTAGE = 1,
	SET_VOLTAGE = 2,
	GET_VOLUME = 3,
	SET_VOLUME = 4,
};

typedef struct {
	uint32_t opcode;
	uint32_t size;
	void *payload;
} Packet;

typedef struct {
	Packet data[QUEUE_CAP];
	int head;
	int tail;
	int n;
	pthread_mutex_t lock;
	pthread_cond_t not_full;
	pthread_cond_t not_empty;

} PacketQueue;
void push_packet(PacketQueue *q, uint32_t opcode, uint32_t size,
		 const void *payload);
void *server_init();
extern PacketQueue *pq;


#endif //SERVER_HEADER
