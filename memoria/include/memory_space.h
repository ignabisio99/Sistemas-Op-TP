#ifndef MEMORIA_MEMORY_SPACE_H
#define MEMORIA_MEMORY_SPACE_H

// Standard library
#include <stdio.h>
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


void create_memory_space(uint32_t tam_memoria, uint32_t tam_pagina);

uint32_t read_data(uint32_t frame_number, uint32_t offset);
void write_data(uint32_t frame_number, uint32_t offset, uint32_t data);

void* read_page(uint32_t frame_number);
void write_page(uint32_t frame_number, void* page_data);

#endif