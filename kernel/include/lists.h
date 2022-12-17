#ifndef KERNEL_LISTS_H
#define KERNEL_LISTS_H

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
#include <pthread.h>

// Project
#include "global.h"
#include "estructuras.h"

// Wrappers para agregar/mover distintos elementos de las listas de nuestro
// dominio

void agregar_a_cola_exit(t_PCB* pcb);
void agregar_a_cola_ready_fifo(t_PCB* pcb);
void agregar_a_cola_ready_rr(t_PCB* pcb);


// Helpers que deben ser la única forma para realizar distintas operaciones
// relacionadas con la cola ready. Ya que implementará la lógica necesaria
// para decidir si agregar los procesos a la cola RR o a la cola FIFO

void agregar_de_new_a_ready(t_PCB *pcb);
void agregar_a_ready_fin_quantum(t_PCB *pcb);
void agregar_de_blocked_a_ready(t_PCB *pcb);

#endif