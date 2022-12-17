#ifndef CPU_CONFIG_H
#define CPU_CONFIG_H

#include <stdint.h>

typedef struct {

    // CPU
    uint16_t ENTRADAS_TLB;
    char* REEMPLAZO_TLB;
    uint32_t RETARDO_INSTRUCCION;
    // IPs y puertos
    char* IP_MEMORIA;
    uint16_t PUERTO_MEMORIA;
    uint16_t PUERTO_ESCUCHA_DISPATCH;
    uint16_t PUERTO_ESCUCHA_INTERRUPT;
} t_config_cpu;

/*
    Ejemplo de archivo de configuracion

    ENTRADAS_TLB=4
    REEMPLAZO_TLB=LRU
    RETARDO_INSTRUCCION=1000
    IP_MEMORIA=127.0.0.1
    PUERTO_MEMORIA=8002
    PUERTO_ESCUCHA_DISPATCH=8001
    PUERTO_ESCUCHA_INTERRUPT=8005
*/

typedef struct{

    int32_t pid;
    int32_t segment;
    int32_t page;
    int32_t frame;
    int32_t cont_referencia;
} t_TLB;

#endif
