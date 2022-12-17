#include "mmu.h"

uint32_t tam_max_segmento;

void init_mmu(){
    tam_max_segmento = page_size * page_table_entries_qty;
}

t_datos_dir_logica* crear_dir_logica(uint32_t dir_logica_decimal){

    t_datos_dir_logica* dir_logica = malloc(sizeof(t_datos_dir_logica));

    dir_logica->num_segmento = floor(dir_logica_decimal / tam_max_segmento);
    dir_logica->desplazamiento_segmento = dir_logica_decimal % tam_max_segmento;
    dir_logica->num_pagina = floor(dir_logica->desplazamiento_segmento  / page_size);
    dir_logica->desplazamiento_pagina = dir_logica->desplazamiento_segmento % page_size;
    
    return dir_logica;
}

bool es_seg_fault(uint32_t pid, t_list* segmentos, uint32_t dir_logica_decimal){

    t_datos_dir_logica* dir_logica = crear_dir_logica(dir_logica_decimal);
    t_segment* segmento = list_get(segmentos, dir_logica->num_segmento);

    bool resultado = dir_logica->desplazamiento_segmento >= segmento->segment;

    if(resultado){
        log_info(cpu_logger,"SIGSEGV - PID: %d - DESPLAZAMIENTO: %d - TAMANO SEGMENTO: %d", pid, dir_logica->desplazamiento_segmento, segmento->segment);
    }

    free(dir_logica);
    
    return resultado; 
}