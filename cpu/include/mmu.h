#ifndef MMU_H
#define MMU_H

#include <stdio.h>
#include <commons/log.h>
#include <stdbool.h>
#include <math.h>

#include "shared_utils.h"
#include "config.h"
#include "global.h"
#include "dispatch.h"
#include "interrupt.h"

typedef struct {
    uint32_t num_segmento; 
    uint32_t desplazamiento_segmento; 
    uint32_t num_pagina; 
    uint32_t desplazamiento_pagina; 
} t_datos_dir_logica;

void init_mmu();
t_datos_dir_logica* crear_dir_logica(uint32_t dir_logica_decimal);
bool es_seg_fault(uint32_t pid,t_list* segmentos, uint32_t dir_logica_decimal);




#endif