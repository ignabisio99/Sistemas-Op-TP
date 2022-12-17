#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <semaphore.h>
#include <pthread.h>


// Instruction codes
typedef enum {
    SET,                // Registro, valor
    ADD,                // Registro destino, registro origen   
    MOV_IN,             // Registro, direción lógica
    MOV_OUT,            // Dirección lógica, registro
    I_O,                // Dispositivo, registro o unidades de trabajo
    EXIT
} t_instruction_code;


// Instruction
typedef struct {
    t_instruction_code  instruction_code;       // Instruction code  
    char*               param_1;                // Parameter 1
    char*               param_2;                // Parameter 2
} t_instruction;

// Segments
typedef struct {
    uint32_t        segment;    // Segment size
    uint32_t        page;       // Page number
} t_segment;

// CPU registers (Struct)
typedef struct {
    uint32_t            ax;
    uint32_t            bx;
    uint32_t            cx;
    uint32_t            dx;
} t_cpu_registers;

// CPU registers (Enum)
typedef enum {
    AX,
    BX,
    CX,
    DX
} t_enum_cpu_registers;

// PCB
typedef struct {
    uint32_t            pid;                // Process identifier
    t_list*             instructions;       // Lista de instrucciones a ejecutar
    uint32_t            pc;                 // Program counter
    t_cpu_registers*    cpu_registers;      // Registros de uso general de la CPU
    t_list*             segments;           // Tabla de segmentos
} t_PCB;


// socket de consola y el pid para exit
typedef struct {
    uint32_t            pid;
    int                 consola_socket;
} t_consola_pid;

// para pasar el socket como argumento a un hilo
typedef struct{
    int fd;
} t_args;

typedef struct{
    t_queue* COLA_IO;
    sem_t SEM_IO;
    pthread_mutex_t MUTEX_IO;
    uint32_t TIEMPO_IO;
    char* NOMBRE;
} t_dispositivo;

// Create & destroy structures
t_PCB* pcb_create(uint32_t pid, t_list* instructions, t_list* segments);
void pcb_destroy(t_PCB* pcb);
t_segment* segment_create();
void instruction_list_destroy(t_list* instructions);
void segment_list_destroy(t_list* segments);
t_cpu_registers* cpu_registers_create();
void cpu_registers_destroy(t_cpu_registers* cpu_registers);
t_instruction* instruction_create(t_instruction_code instruction_code, char* param_1, char* param_2);
void instruction_destroy(t_instruction* instruction);


#endif

