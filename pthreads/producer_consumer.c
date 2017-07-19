// An implementation of the producer consumer model with pthreads
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define BUF_SIZE 3

int buffer[BUF_SIZE]; /*shared buffer */
int num = 0; /* total number of elements */
int rem = 0; /* index to start removing from */
int add = 0; /* index to add an element */

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER; /* mutex lock for buffer */
pthread_cond_t c_cons = PTHREAD_MUTEX_INITIALIZER; /* conditional variable that the consumer waits on */
pthread_cond_t c_prod = PTHREAD_MUTEX_INITIALIZER; /* conditional variable that the producer waits on */

void *producer (void *param);
void *consumer (void *param);

int main() {

	pthread_t tid1, tid2; /* thread identifiers */
	int i;
	
	/* create the threads */
	if (pthread_create(&tid1, NULL, producer, NULL) != 0) {
		fprintf(stderr, "Unable to create producer\n");
		exit(1);
	}
	if (pthread_create(&tid2, NULL, consumer, NULL) != 0) {
		fprintf(stderr, "Unable to create consumer\n");
		exit(1);
	}
	
	/* wait for the threads to exit and then join them */
	pthread_join(tid1, NULL);
	pthread_join(tid2, NULL);
	printf("Parent thread quitting...\n");
	return 0;
}

void *producer (void *param) {
	int i = 0;
	for (i=1; i<=20; i++) {
		/* insert values into the buffer */
		pthread_mutex_lock (&m);
			if (num > BUF_SIZE) {
				exit(1); /* overflow the buffer */
			}
			while (num == BUF_SIZE) {
				pthread_cond_wait(&c_prod, &m); /* buffer is full so block */
			}
			/* got out of the wait, now we can insert stuff */
			buffer[add] = i;
			add = (add+1) % BUF_SIZE;
			num++;
		pthread_mutex_unlock (&m);

		pthread_cond_signal (&c_cons);
		printf("producer: inserted %d\n", i);
		fflush(stdout);
	}
}

void *consumer (void *param) {
	int val;
	while (1) {
		pthread_mutex_lock (&m);
			if (num < 0) {
				exit(1); /* underflow */
			}
			while (num == 0) {
				pthread_cond_wait(&c_cons, &m); /* the buffer is empty so wait */	
			}
			/* got out of the wait, now we can get stuff out of the buffer */
			val = buffer[rem];
			rem = (rem+1) % BUF_SIZE;
			num--;
		pthread_mutex_unlock (&m);

		pthread_cond_signal (&c_prod);
		printf("consumer: taken out %d\n", val);
		fflush(stdout);
	}	
}
