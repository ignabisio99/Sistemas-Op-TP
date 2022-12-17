#ifndef INIT_CONSOLA_H
#define INIT_CONSOLA_H

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <readline/readline.h>

#include "config.h"
#include "global.h"
#include "sockets.h"
#include "shared_utils.h"
#include "shared_config.h"
#include "protocolo.h"

void initializeConsola(char* config_path,char *pseudocodigo_path);
int generar_conexiones(t_config_consola* consola_config, t_log* consola_logger, int* fd_kernel);
void close_program(t_config_consola* consola_config, t_log* consola_logger, int fd_kernel);
void crearInstrucciones(char* path_pseudocodigo);


#endif