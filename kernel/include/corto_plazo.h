#ifndef KERNEL_CORTO_PLAZO_H
#define KERNEL_CORTO_PLAZO_H

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
#include "blocked.h"

// Importo largo plazo solo por find_pid
#include "largo_plazo.h"

typedef struct {
    t_PCB*              pcb_recibido;     // PCB 
    uint32_t            segment;
    uint32_t            page;
} t_arg_blocked_page_fault;

// Esta función es llamada como handler del hilo principal que se encargará
// del planificador a corto plazo.

void handler_corto_plazo();

#endif