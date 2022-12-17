#ifndef CONSOLA_CONFIG_H
#define CONSOLA_CONFIG_H

#include <stdint.h>

typedef struct {
    // IPs y puertos
    char* IP_KERNEL;
    uint16_t PUERTO_KERNEL;

    // CONSOLA
    uint32_t* SEGMENTOS;
    uint32_t TIEMPO_PANTALLA;
} t_config_consola;

#endif
