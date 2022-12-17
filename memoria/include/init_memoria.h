#ifndef INIT_MEMORIA_H
#define INIT_MEMORIA_H

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
#include "shared_utils.h"
#include "sockets.h"
#include "shared_config.h"
#include "memory_space.h"
#include "page_tables.h"
#include "swap.h"

void initializeMemoria(char* config_path);
void handle_exit(int signal);
void close_program();

#endif