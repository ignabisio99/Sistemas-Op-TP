#ifndef TLB_H
#define TLB_H

#include <stdio.h>
#include <commons/string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <stdbool.h>
#include <pthread.h>

#include "global.h"
#include "config.h"
#include "dispatch.h"
#include "interrupt.h"

// Shared
#include "protocolo.h"
#include "shared_utils.h"

// Leer entrada de página de TLB y retornar número de frame, actualizando la ref solo si es LRU
bool obtener_marco(uint32_t pid, uint32_t segment, uint32_t page, uint32_t* frame);
// Reemplaza un dato de la TLB
void reemplazar(uint32_t pid,uint32_t segment,uint32_t page,uint32_t frame);
// Vacia los datos de un proceso que finalizo
void limpiar_proceso_TLB(uint32_t pid);
//
void inicializar_TLB();
void imprimir_tlb();

#endif




