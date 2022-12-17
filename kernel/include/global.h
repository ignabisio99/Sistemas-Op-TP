#ifndef KERNEL_GLOBAL_H
#define KERNEL_GLOBAL_H

#include <stdio.h>
#include <stdlib.h> 
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <stdbool.h>
#include <semaphore.h>

#include "config.h"

#define SERVERNAME "KERNEL"

// Global variables
extern t_log* kernel_logger;
extern t_config_kernel* kernel_config;

// File descriptors
extern int fd_server;
extern int kernelDispatchFd;
extern int kernelInterruptFd;
extern int fd_memoria_init;
extern int fd_memoria_page_fault;
extern int fd_memoria_exit;

// -- Sincro page fault
extern sem_t SEM_PAGE_FAULT;
extern sem_t SEM_CPU;

// -- Lista READY FIFO
extern t_queue* COLA_READY_FIFO;
extern pthread_mutex_t MUTEX_READY_FIFO;

// -- Lista READY RR
extern t_queue* COLA_READY_RR;
extern pthread_mutex_t MUTEX_READY_RR;
extern sem_t SEM_READY;

// -- Cola EXIT
extern t_queue* COLA_EXIT;
extern pthread_mutex_t MUTEX_EXIT;
extern sem_t SEM_EXIT;

extern t_list* LISTA_CONSOLAS;
extern pthread_mutex_t MUTEX_CONSOLAS;

// -- SEMAFORO PARA BLOQUEAR PROCESO MAIN
extern sem_t BLOQUEADOR;

// -- Cola NEW

extern t_queue* COLA_NEW;
extern pthread_mutex_t MUTEX_NEW;
extern sem_t SEM_NEW;
extern sem_t SEM_MULTIPROGRAMACION;

extern pthread_mutex_t MUTEX_PIDS;

// 
extern t_list* LISTA_COLAS_DISPOSITIVOS; 

#endif