#include "memory_space.h"

bool is_memory_created = false;
pthread_mutex_t MUTEX_MEMORY_SPACE;
uint16_t largo_tipo_de_dato = sizeof(uint32_t);

void* memory_space;
uint32_t size_frame;

void create_memory_space(uint32_t tam_memoria, uint32_t tam_pagina){
    if(is_memory_created){
        log_error(memoria_logger, "Se está tratando de crear dos veces el espacio de memoria. Aborting...");
        exit(EXIT_FAILURE);
    }
    pthread_mutex_init(&MUTEX_MEMORY_SPACE, NULL);
    is_memory_created = true;
    size_frame = tam_pagina;
    memory_space = malloc(tam_memoria);
}

bool _es_operable_sobre_memoria(uint32_t offset){
    if(!is_memory_created){
        log_error(memoria_logger, "Para escribir sobre la memoria primero debe crearse. Aborting...");
        return false;
    }
    if(offset % largo_tipo_de_dato != 0 && offset != 0){
        log_error(memoria_logger, "El offset debe ser divisible por el tamaño de dato guardado. Aborting...");
        return false;
    }
    if(offset >= size_frame){
        log_error(memoria_logger, "El offset ser menor que el marco. Aborting...");
        return false;
    }
    return true;
}

void write_data(uint32_t frame_number, uint32_t offset, uint32_t data){
    if(!_es_operable_sobre_memoria(offset)){
        exit(EXIT_FAILURE);
    }

    uint32_t total_offset = frame_number * size_frame + offset;

    pthread_mutex_lock(&MUTEX_MEMORY_SPACE);
        memcpy(memory_space + total_offset, &data, largo_tipo_de_dato);
    pthread_mutex_unlock(&MUTEX_MEMORY_SPACE);
}

uint32_t read_data(uint32_t frame_number, uint32_t offset){
    if(!_es_operable_sobre_memoria(offset)){
        exit(EXIT_FAILURE);
    }

    uint32_t total_offset = frame_number * size_frame + offset;
    uint32_t data_leida = 0;

    pthread_mutex_lock(&MUTEX_MEMORY_SPACE);
        memcpy(&data_leida, memory_space + total_offset, largo_tipo_de_dato);
    pthread_mutex_unlock(&MUTEX_MEMORY_SPACE);
    
    return data_leida;
}

void* read_page(uint32_t frame_number){
    if(!_es_operable_sobre_memoria(0)){
        exit(EXIT_FAILURE);
    }

    void* page_data = malloc(size_frame);
    uint32_t total_offset = frame_number * size_frame;

    pthread_mutex_lock(&MUTEX_MEMORY_SPACE);
        memcpy(page_data, memory_space + total_offset, size_frame);
    pthread_mutex_unlock(&MUTEX_MEMORY_SPACE);
    
    return page_data;
}

void write_page(uint32_t frame_number, void* page_data){
    if(!_es_operable_sobre_memoria(0)){
        exit(EXIT_FAILURE);
    }

    uint32_t total_offset = frame_number * size_frame;

    pthread_mutex_lock(&MUTEX_MEMORY_SPACE);
        memcpy(memory_space + total_offset, page_data, size_frame);
    pthread_mutex_unlock(&MUTEX_MEMORY_SPACE);
}