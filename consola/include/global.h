#ifndef CONSOLA_GLOBAL_H
#define CONSOLA_GLOBAL_H

#include <stdio.h>
#include <stdlib.h> 
#include <commons/log.h>
#include <commons/collections/list.h>
#include <stdbool.h>
#include <semaphore.h>

#include "config.h"

//Global variables
extern t_log* consola_logger;
extern t_config_consola* consola_config;
extern t_list* listaDeInstrucciones;

#endif