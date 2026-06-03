#include "server.h"

#include "playlist.h"
#include "sighandle.h"
#include "speedloop.h"
#include "mpvutils.h"


#include <math.h>
#include <mpv/client.h>
#include <pthread.h>
#include <sched.h>
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define PATH "/tmp/dashboard_socket.sock"




int read_data(int fd, void *buf, size_t len) {
	size_t totalRead = 0;
	char *ptr = (char *)buf;
	while (totalRead < len) {
		ssize_t n = recv(fd, ptr + totalRead, len - totalRead, 0);
		if (n <= 0)
			return -1;
		totalRead += n;
	}
	return 0;
}



typedef struct {
	int fd;
	PacketQueue *queue;
} WriterContext;

typedef struct {
	int fd;
	PacketQueue *queue;
} ReaderContext;

void queue_init(PacketQueue *q) {
	q->head = 0;
	q->tail = 0;
	q->n = 0;
	pthread_mutex_init(&q->lock, NULL);
	pthread_cond_init(&q->not_full, NULL);
	pthread_cond_init(&q->not_empty, NULL);
}

void push_packet(PacketQueue *q, uint32_t opcode, uint32_t size,
		 const void *payload) {
	if (q == NULL) {
		return;
	}
	pthread_mutex_lock(&q->lock);

	while (q->n == QUEUE_CAP) {
		pthread_cond_wait(&q->not_full, &q->lock);
	}

	q->data[q->tail].opcode = opcode;
	q->data[q->tail].size = size;

	if (payload && size) {
		q->data[q->tail].payload = malloc(size);
		memcpy(q->data[q->tail].payload, payload, size);
	}
	q->tail = (q->tail + 1) % QUEUE_CAP;
	q->n = q->n + 1;

	pthread_cond_signal(&q->not_empty);
	pthread_mutex_unlock(&q->lock);
}

Packet pop_packet(PacketQueue *q) {
	pthread_mutex_lock(&q->lock);

	while (q->n == 0) {
		pthread_cond_wait(&q->not_empty, &q->lock);
	}

	Packet p = q->data[q->head];

	q->head = (q->head + 1) % QUEUE_CAP;
	q->n--;

	pthread_cond_signal(&q->not_full);
	pthread_mutex_unlock(&q->lock);

	return p;
}

void *write_packets(void *arg) {
	WriterContext *ctx = (WriterContext *)arg;

	int fd = ctx->fd;
	PacketQueue *q = ctx->queue;

	while (running) {
		
		Packet pkt = pop_packet(q);

		if (send(fd, &pkt.opcode, sizeof(pkt.opcode), 0) < 0)
			break;
		if (send(fd, &pkt.size, sizeof(pkt.size), 0) < 0)
			break;
		
		if (pkt.payload && pkt.size) {
			printf("C writing.....\n");
			fflush(stdout);
			if (send(fd, pkt.payload, pkt.size, 0) < 0)
				break;
			free(pkt.payload);
		}
	}
	free(ctx);
	return NULL;
}

void *read_packets(void *arg) {
	ReaderContext *ctx = (ReaderContext *)arg;

	int fd = ctx->fd;
	PacketQueue *q = ctx->queue;

	while (running) {
		uint32_t opcode = 0;
		uint32_t size = 0;

		if (read_data(fd, &opcode, sizeof(opcode)) < 0)
			break;
		if (read_data(fd, &size, sizeof(size)) < 0)
			break;
		void *buf = NULL;
		if (size > 0) {
			buf = malloc(size);
			if (read_data(fd, buf, size) < 0) {
				free(buf);
				break;
			}
		}
		switch (opcode) {
		case GET_VOLTAGE: {
			double voltage = 3.3 * ((double)minVoltage / 4095.0);
			push_packet(q, SET_VOLTAGE, sizeof(voltage), &voltage);
			break;
		}
		case SET_VOLTAGE:
			if (size >= sizeof(double) && buf != NULL) {
				atomic_store(&minVoltage,
				(int)lround(((*(double *)buf) / 3.3) * 4095.0));
		}
			break;
		case GET_VOLUME: {
			double volume = get_volume(mpv);
			push_packet(q, SET_VOLUME, sizeof(volume), &volume);
			break;
		}
		case SET_VOLUME: 
			if (size >= sizeof(double) && buf != NULL) {
				set_volume(mpv, *(double *)buf);
			}
			break;
		default:
			
			break;
		}
		free(buf);
	}
	free(ctx);
	return NULL;

}
PacketQueue *pq;

void *server_init() {
	int sock_fd;
	struct sockaddr_un addr;
	sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock_fd == -1) {
		perror("failed to create socket\n");
		return NULL;
	}

	memset(&addr, 0, sizeof(struct sockaddr_un));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, PATH, sizeof(addr.sun_path) - 1);

	unlink(PATH);

	if (bind(sock_fd, (struct sockaddr *)&addr,
		 sizeof(struct sockaddr_un)) == -1) {
		perror("failed to bind\n");
		close(sock_fd);
		return NULL;
	}
	listen(sock_fd, 5);
	pid_t apiPid;
	char *args[] = {"./dashboard/dashboard", NULL};
	posix_spawnp(&apiPid, "./dashboard/dashboard", NULL, NULL, args,
		     environ);
	int go_fd = accept(sock_fd, NULL, NULL);
	
	pq = malloc(sizeof(PacketQueue));
	queue_init(pq);

	
	WriterContext *wc = malloc(sizeof(WriterContext));
	ReaderContext *rc = malloc(sizeof(ReaderContext));
	
	
	wc->fd = go_fd;
	wc->queue = pq;

	rc->fd = go_fd;
	rc->queue= pq;

	pthread_t wId;
	pthread_t rId;


	pthread_create(&wId, NULL, write_packets, wc);
	pthread_create(&rId, NULL, read_packets, rc);
	
	return NULL;
}
