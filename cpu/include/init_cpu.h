#ifndef INIT_CPU_H
#define INIT_CPU_H


// Standard Library
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

// Libraries
#include <string.h>
#include <readline/readline.h>
#include <signal.h>

// Commons
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>

// Project
#include "config.h"
#include "global.h"
#include "sockets.h"
#include "shared_utils.h"
#include "shared_config.h"
#include "tlb.h"
#include "mmu.h"

void initializeSemaphores();
uint8_t cargar_configuracion(char* config_path);
void initializeCpu(char* config_path);
int server_create(t_config_cpu* cpu_config, t_log* cpu_logger, int* fd_server, char* server_name);
void close_program();
void handle_exit(int signal);


#endif