#ifndef MEMORIA_PAGE_TABLES_H
#define MEMORIA_PAGE_TABLES_H

// Standard library
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h> 
#include <stdbool.h>

// Commons
#include <commons/log.h>

// Libraries
#include <semaphore.h>
#include <pthread.h>
#include <string.h>

// Project
#include "global.h"
#include "swap.h"
#include "memory_space.h"

// Estructuras memoria
// Vamos a tener una lista principal de pid_tables que nos permite asociar pid con segmentos.
// Luego en los segmentos tendremos por cada uno, una tabla de paginas y su respectivo mutex
typedef struct{
    uint32_t            pid;
    uint32_t            ultima_referencia_segmento;
    uint32_t            ultima_referencia_pagina; 
    t_list*             segmentos;                  // Lista de segmentos asociados al PID
} t_pid_tables;

typedef struct{
    t_list*             tabla_de_paginas;           // Lista de tablas de paginas asociadas al Segmento
    pthread_mutex_t     mutex_tabla_de_paginas;     // Mutex relacionado a la pag de tablas
} t_memory_segment;

typedef struct{
    uint32_t            frame;              // Frame asociado con página
    bool                presencia;          // Esta en memoria?
    bool                uso;                // Algoritmo clock y clock-M
    bool                modificado;         // Fue modificada esta pág?
    uint32_t            pos_en_swap;        // Pos en swap
} t_entrada_tabla_de_paginas;

// Handler for kernel init process request
void create_process(uint32_t pid, t_list* segments);

// Initialize internal structure of page tables
void create_page_tables_structure();

// Get a page
t_entrada_tabla_de_paginas* get_page_data(uint32_t pid, uint32_t segment_number, uint32_t page_number);

// Get a page frame
bool get_page_frame(uint32_t pid, uint32_t segment_number, uint32_t page_number,uint32_t* frame_response);

// Bring page from swap to memory and handle page tables structure
void agregar_pagina_a_memoria (uint32_t numero_pid, uint32_t segment_number, uint32_t page_number);

// Finaliza un proceso
void finalizar_proceso(uint32_t pid);

#endif