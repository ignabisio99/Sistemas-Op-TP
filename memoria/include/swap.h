#ifndef MEMORY_SWAP_H
#define MEMORY_SWAP_H

// Standard library
#include <stdio.h>
#include <stdlib.h> 
#include <stdbool.h>
#include <unistd.h>

// Commons
#include <commons/log.h>

// Libraries
#include <semaphore.h>
#include <pthread.h>
#include <string.h>

// Project
#include "global.h"


void create_swap_file();
void close_swap_file();
void write_swap_file(void* page_data, uint32_t posicion_swap);
void* read_swap_file(uint32_t posicion_swap);

// Busca un marco libre en swap y escribe la página
uint32_t create_page_in_swap();

// Marcamos al marco como libre y listop
void remove_page_from_swap(uint32_t pos_en_swap);

#endif