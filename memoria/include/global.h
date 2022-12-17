#ifndef MEMORIA_GLOBAL_H
#define MEMORIA_GLOBAL_H

#include <stdio.h>
#include <stdlib.h> 
#include <commons/log.h>
#include <commons/collections/list.h>
#include <stdbool.h>
#include <semaphore.h>

#include "config.h"

#define SERVER_MEMORIA "MEMORIA_SERVER"
#define MEMORIA_CPU_SERVER "MEMORIA CPU SERVER"
#define MEMORIA_KERNEL_SERVER "MEMORIA KERNEL SERVER"

//Global variables
extern t_log* memoria_logger;
extern t_config_memoria* memoria_config;

//File descriptors
extern int fd_server_memoria;
extern int fd_cpu;
extern int fd_kernel_init;
extern int fd_kernel_page_fault;
extern int fd_kernel_exit;

// Main blocking
extern sem_t BLOQUEADOR;

#endif