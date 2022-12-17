#ifndef INIT_KERNEL_H
#define INIT_KERNEL_H

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>

#include "config.h"
#include "global.h"
#include "shared_utils.h"
#include "shared_config.h"

#include "sockets.h"
#include "synchro.h"

#include "largo_plazo.h"
#include "corto_plazo.h"
#include "blocked.h"


void initializeKernel(char* config_path);
int crear_servidor(t_config_kernel* kernel_config, t_log* kernel_logger, int* fd_server, char* server_name);
void inicializar_planificadores();

#endif