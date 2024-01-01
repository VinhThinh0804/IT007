#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

sem_t sem1;
sem_t sem2;
sem_t sem3;
int x1, x2, x3, x4, x5, x6;
int w, v, y, z, ans;

void *ham_a(void *threadid) {
    w = x1 * x2;
    sem_post(&sem2);
}

void *ham_b(void *threadid) {
    v = x3 * x4;
    sem_post(&sem1);
}

void *ham_c(void *threadid) {
    sem_wait(&sem1);
    y = v * x5;
    sem_post(&sem2);
}

void *ham_d(void *threadid) {
    sem_wait(&sem1);
    z = v * x6;
    sem_post(&sem2);
}

void *ham_e(void *threadid) {
    sem_wait(&sem2);
    y = w * y;
    sem_post(&sem3);
}

void *ham_f(void *threadid) {
    sem_wait(&sem2);
    z = w * z;
    sem_post(&sem3);
}

void *ham_g(void *threadid) {
    sem_wait(&sem3);
    sem_wait(&sem3);
    ans = y + z;
}

int main() {
    pthread_t threads[7];
    sem_init(&sem1, 0, 1);
    sem_init(&sem2, 0, 0);
    sem_init(&sem3, 0, 0); 
    x1 = 1;
    x2 = 2;
    x3 = 3;
    x4 = 4;
    x5 = 5;
    x6 = 6;
    pthread_create(&threads[0], NULL, &ham_a, NULL);
    pthread_create(&threads[1], NULL, &ham_b, NULL);
    pthread_create(&threads[2], NULL, &ham_c, NULL);
    pthread_create(&threads[3], NULL, &ham_d, NULL);
    pthread_create(&threads[4], NULL, &ham_e, NULL);
    pthread_create(&threads[5], NULL, &ham_f, NULL);
    pthread_create(&threads[6], NULL, &ham_g, NULL);

    for (int i = 0; i < 7; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Result: %d\n", ans);

    return 0;
}
