#ifndef KERNEL_BLOCKED_MODULE_H
#define KERNEL_BLOCKED_MODULE_H

// Standard library
#include <stdio.h>
#include <stdlib.h> 
#include <stdbool.h>

// Commons
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/string.h>

// Libraries
#include <semaphore.h>
#include <string.h>

// Project
#include "global.h"
#include "lists.h"
#include "estructuras.h"
#include "protocolo.h"
#include "dispatcher.h"
#include "shared_utils.h"

// Importo largo plazo solo por find_pid
#include "largo_plazo.h"

typedef struct {
    t_PCB*              pcb_recibido;     // PCB 
} t_arg_blocked_thread;

// Esta función es llamada como handler del hilo principal que se encargará
// del planificador a corto plazo.

void create_blocked_thread(t_PCB *pcb_recibido, t_package* paquete);
void atenderCola(void* args);

#endif