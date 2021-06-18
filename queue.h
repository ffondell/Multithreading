#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#ifndef QSIZE
#define QSIZE 100
#endif

typedef struct{
  char* path;
}paths;


typedef struct {
	paths data[QSIZE];
	unsigned count;
	unsigned head;
	pthread_mutex_t lock;
	pthread_cond_t read_ready;
	pthread_cond_t write_ready;
} queue_t;


int init(queue_t *Q)
{
	Q->count = 0;
	Q->head = 0;
	pthread_mutex_init(&Q->lock, NULL);
	pthread_cond_init(&Q->read_ready, NULL);
	pthread_cond_init(&Q->write_ready, NULL);

	return 0;
}

int enqueue(queue_t *Q, paths item)
{
	pthread_mutex_lock(&Q->lock);

	while (Q->count == QSIZE) {
		pthread_cond_wait(&Q->write_ready, &Q->lock);
	}

	unsigned i = Q->head + Q->count;
	if (i >= QSIZE) i -= QSIZE;

	Q->data[i] = item;
	++Q->count;

	pthread_cond_signal(&Q->read_ready);

	pthread_mutex_unlock(&Q->lock);

	return 0;
}


char* dequeue(queue_t *Q, paths *item, int activeThreads)
{
	pthread_mutex_lock(&Q->lock);

  if(Q->count == 0){
    //activeThreads--;

    if(activeThreads==0){
      
      pthread_cond_broadcast(&Q->write_ready);
    }
  }
	while (Q->count == 0 && activeThreads !=0) {
  
		pthread_cond_wait(&Q->read_ready, &Q->lock);
	}

	*item = Q->data[Q->head];
	--Q->count;
	++Q->head;
	if (Q->head == QSIZE){
    Q->head = 0;
  }

	pthread_cond_signal(&Q->write_ready);

	pthread_mutex_unlock(&Q->lock);

	return item->path;
}

int destroy(queue_t *Q)
{
	pthread_mutex_destroy(&Q->lock);
	pthread_cond_destroy(&Q->read_ready);
	pthread_cond_destroy(&Q->write_ready);

	return 0;
}
