// Say hello some number of times
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#define NUM_THREADS 4
void *hello (void *order) {
        int *pOrder = (int *)order;
        int myOrder = *pOrder;
        printf("Hello. This is thread %i\n", myOrder);
        return 0;
}
int main() {
        int i;
        pthread_t tid[NUM_THREADS];
        int orders[NUM_THREADS];
        for (i = 0; i < NUM_THREADS; i++) {
                orders[i] = i;
                pthread_create(&tid[i], NULL, hello, &orders[i]);
        }
        for (i = 0; i < NUM_THREADS; i++) {
                pthread_join(tid[i], NULL);
        }
        return 0;
}
