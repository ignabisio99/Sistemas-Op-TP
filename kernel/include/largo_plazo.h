#ifndef LARGO_PLAZO_H
#define LARGO_PLAZO_H

// Standard library
#include <stdio.h>
#include <stdlib.h> 
#include <stdbool.h>

// Commons
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <commons/collections/queue.h>

// Libraries
#include <semaphore.h>
#include <string.h>

// Project
#include "global.h"
#include "lists.h"
#include "estructuras.h"
#include "protocolo.h"
#include "dispatcher.h"
#include "mock.h"


void planificadorLargoPlazo();
void conexionConConsola();
void procesar_conexion(void* fd_console);
int recibirAConsola();
void enviarProcesosAReady();
void terminarProcesos();
t_consola_pid* find_pid(uint32_t pid, t_list* lista);

#endif
