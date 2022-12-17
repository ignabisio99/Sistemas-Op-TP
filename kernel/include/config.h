#ifndef KERNEL_CONFIG_H
#define KERNEL_CONFIG_H

#include <stdint.h>

typedef struct {
    // IPs y puertos
    char* IP_MEMORIA;
    uint16_t PUERTO_MEMORIA;
    char* IP_CPU;
    uint16_t PUERTO_CPU_DISPATCH;
    uint16_t PUERTO_CPU_INTERRUPT;
    uint16_t PUERTO_ESCUCHA;
    
    // KERNEL
    char* ALGORITMO_PLANIFICACION;
    uint16_t GRADO_MAX_MULTIPROGRAMACION;
    char** DISPOSITIVOS_IO;
    uint32_t* TIEMPOS_IO;
    uint16_t QUANTUM_RR;
    
} t_config_kernel;

#endif
