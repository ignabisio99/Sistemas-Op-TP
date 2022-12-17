#include "synchro.h"

// Listas

// -- READY
pthread_mutex_t MUTEX_READY_FIFO;
pthread_mutex_t MUTEX_READY_RR;
pthread_mutex_t MUTEX_EXIT;
pthread_mutex_t MUTEX_NEW;
pthread_mutex_t MUTEX_CONSOLAS;
pthread_mutex_t MUTEX_PIDS;

sem_t SEM_READY;
sem_t BLOQUEADOR;
sem_t SEM_EXIT;
sem_t SEM_NEW;
sem_t SEM_MULTIPROGRAMACION;
sem_t SEM_PAGE_FAULT;
sem_t SEM_CPU;

void initializeSemaphores(){
    pthread_mutex_init(&MUTEX_READY_FIFO, NULL);
    pthread_mutex_init(&MUTEX_READY_RR, NULL);
    pthread_mutex_init(&MUTEX_EXIT, NULL);
    pthread_mutex_init(&MUTEX_NEW,NULL);
    pthread_mutex_init(&MUTEX_CONSOLAS,NULL);
    pthread_mutex_init(&MUTEX_PIDS,NULL);
    sem_init(&SEM_READY, 0, 0);
    sem_init(&SEM_EXIT,0,0);
    sem_init(&SEM_PAGE_FAULT,0,1);
    sem_init(&SEM_CPU,0,1);
    sem_init(&BLOQUEADOR, 0, 0);
    sem_init(&SEM_NEW,0,0);
    sem_init(&SEM_MULTIPROGRAMACION,0, kernel_config->GRADO_MAX_MULTIPROGRAMACION);
}