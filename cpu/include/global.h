#ifndef CPU_GLOBAL_H
#define CPU_GLOBAL_H

#include <stdio.h>
#include <stdlib.h> 
#include <commons/log.h>
#include <commons/collections/list.h>
#include <stdbool.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>

#include "config.h"

#define SERVER_DISPATCH "CPU_DISPATCH"
#define SERVER_INTERRUPT "CPU_INTERRUPT"

// Config & logger
extern t_config_cpu* cpu_config;
extern t_log* cpu_logger;

// Sockets
extern int fd_server_dispatch;
extern int fd_server_interrupt;
extern int fd_kernel_dispatch;
extern int fd_kernel_interrupt;
extern int fd_memoria;

// Mutex interrupt
extern pthread_mutex_t mutex_interrupt;
extern bool interrupt;
extern uint32_t pid_interrupted;

extern sem_t SOCKET_KERNEL;

// Main blocking
extern sem_t BLOQUEADOR;

// Memory data
extern uint32_t page_table_entries_qty; 
extern uint32_t page_size;

#endif
