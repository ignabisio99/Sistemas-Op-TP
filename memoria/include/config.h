#ifndef MEMORIA_CONFIG_H
#define MEMORIA_CONFIG_H

#include <stdint.h>

typedef struct {
    // IPs y puertos
    uint16_t PUERTO_ESCUCHA;

    // MEMORIA
    uint32_t TAM_MEMORIA;
    uint32_t TAM_PAGINA;
    uint32_t ENTRADAS_POR_TABLA;
    uint32_t RETARDO_MEMORIA;
    char* ALGORITMO_REEMPLAZO;
    uint32_t MARCOS_POR_PROCESO;
    uint32_t RETARDO_SWAP;
    char* PATH_SWAP;
    uint32_t TAMANIO_SWAP;
    
} t_config_memoria;

#endif
