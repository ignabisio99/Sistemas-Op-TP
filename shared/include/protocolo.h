#ifndef PROTOCOLO_H_
#define PROTOCOLO_H_


// Standard Library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>

// Libraries
#include <inttypes.h>

// Commons
#include <sys/socket.h>
#include <commons/collections/list.h>

// Project
#include "estructuras.h"


/*
 *  Messages: enum & structs
 */

// Message headers
typedef enum {

    // CONSOLA <-> KERNEL
    MSG_CONSOLA_KERNEL_INIT,
    MSG_KERNEL_CONSOLA_INIT,
    MSG_KERNEL_CONSOLA_PRINT,
    MSG_CONSOLA_KERNEL_PRINT, 
    MSG_KERNEL_CONSOLA_INPUT,
    MSG_CONSOLA_KERNEL_INPUT,
    MSG_KERNEL_CONSOLA_EXIT,

    // KERNEL <-> CPU
    MSG_KERNEL_CPU_INTERRUPT,
    MSG_CPU_KERNEL_INTERRUPT,
    MSG_KERNEL_CPU_EXEC,
    MSG_CPU_KERNEL_IO,
    MSG_CPU_KERNEL_EXIT,
    MSG_CPU_KERNEL_PAGE_FAULT,
    MSG_CPU_KERNEL_SIGSEGV,

    // KERNEL <-> MEMORIA
    // -- Init
    MSG_KERNEL_MEMORIA_INIT,
    MSG_MEMORIA_KERNEL_INIT,
    // -- Page fault
    MSG_KERNEL_MEMORIA_PAGE_FAULT,
    MSG_MEMORIA_KERNEL_PAGE_FAULT,
    // -- Exit
    MSG_KERNEL_MEMORIA_EXIT,
    MSG_MEMORIA_KERNEL_EXIT,
    
    // CPU <-> MEMORIA
    // -- Init
    MSG_CPU_MEMORIA_INIT,
    MSG_MEMORIA_CPU_INIT,
    // -- Page & frame
    MSG_CPU_MEMORIA_PAGE,
    MSG_MEMORIA_CPU_FRAME,
    // -- Read data
    MSG_CPU_MEMORIA_DATA_READ,
    MSG_MEMORIA_CPU_DATA_READ,
    // -- Write data
    MSG_CPU_MEMORIA_DATA_WRITE,
    MSG_MEMORIA_CPU_DATA_WRITE,        
    // -- Page fault
    MSG_MEMORIA_CPU_PAGE_FAULT,

    //
    NULL_HEADER
} t_msg_header;


// Message buffer
typedef struct {
    uint32_t            size;           // Payload size
    void*               stream;         // Payload
} t_buffer;


// Message package
typedef struct {
    t_msg_header        msg_header;     // Message header 
    t_buffer*           buffer;         // Buffer
} t_package;


/*
 *  Buffer & package functions
 */

// Buffer
t_buffer* buffer_create();
void buffer_destroy(t_buffer* buffer);
void buffer_add(t_buffer* buffer, void* stream, uint32_t size);

// Package
t_package* package_create(t_msg_header msg_header);
void package_destroy(t_package* package);
int package_send(t_package* package, int fd);
int package_recv(t_package* package, int fd);


/*
 *  Serialize & deserialize: General functions
 */

// Serialize
void serialize_segment(t_buffer* buffer, t_segment* segment);
void serialize_segment_list(t_buffer* buffer, t_list* segments);
void serialize_instruction(t_buffer* buffer, t_instruction* instruction);
void serialize_instruction_list(t_buffer* buffer, t_list* instructions);
void serialize_cpu_registers(t_buffer* buffer, t_cpu_registers* cpu_registers);
void serialize_pcb(t_buffer* buffer, t_PCB* pcb);
void serialize_list_uint32_t(t_buffer* buffer, t_list* values);
void serialize_uint32_t(t_buffer* buffer, int args_qty, ...);

// Deserialize
void deserialize_segment(t_buffer* buffer, t_segment* segment);
void deserialize_segment_list(t_buffer* buffer, t_list* segments);
void deserialize_instruction(t_buffer* buffer, t_instruction* instruction);
void deserialize_instruction_list(t_buffer* buffer, t_list* instructions);
void deserialize_cpu_registers(t_buffer* buffer, t_cpu_registers* cpu_registers);
void deserialize_pcb(t_buffer* buffer, t_PCB* pcb);
void deserialize_list_uint32_t(t_buffer* buffer, t_list* values);
void deserialize_uint32_t(t_buffer* buffer, int args_qty, ...);

/*
 *  Consola & Kernel: serialize, deserialize, send & receive messages
 */

// Serialize messages   
void serialize_msg_consola_kernel_init(t_buffer* buffer, t_list* instructions, t_list* segments);
void serialize_msg_kernel_consola_print(t_buffer* buffer, uint32_t value);
void serialize_msg_consola_kernel_input(t_buffer* buffer, uint32_t value); 

// Deserialize messages
void deserialize_msg_consola_kernel_init(t_buffer* buffer, t_list* instructions, t_list* segments);
void deserialize_msg_kernel_consola_print(t_buffer* buffer, uint32_t* value);
void deserialize_msg_consola_kernel_input(t_buffer* buffer, uint32_t* value);

// Send messages
int send_msg_consola_kernel_init(t_list* instructions, t_list* segments, int fd);
int send_msg_kernel_consola_init(int fd);
int send_msg_kernel_consola_print(uint32_t value, int fd);
int send_msg_consola_kernel_print(int fd);
int send_msg_kernel_consola_input(int fd);
int send_msg_consola_kernel_input(uint32_t value, int fd);
int send_msg_kernel_consola_exit(int fd);

// Receive messages
int recv_msg_consola_kernel_init(t_buffer* buffer, t_list* instructions, t_list* segments);
int recv_msg_kernel_consola_print(t_buffer* buffer, uint32_t* value);
int recv_msg_consola_kernel_input(t_buffer* buffer, uint32_t* value);


/*
 *  Kernel & CPU: serialize, deserialize, send & receive messages
 */

// Serialize messages
void serialize_msg_kernel_cpu_interrupt(t_buffer* buffer, uint32_t pid);
void serialize_msg_cpu_kernel_interrupt(t_buffer* buffer, t_PCB* pcb);
void serialize_msg_kernel_cpu_exec(t_buffer* buffer, t_PCB* pcb);
void serialize_msg_cpu_kernel_io(t_buffer* buffer, t_PCB* pcb);
void serialize_msg_cpu_kernel_exit(t_buffer* buffer, t_PCB* pcb);
void serialize_msg_cpu_kernel_page_fault(t_buffer* buffer, t_PCB* pcb, uint32_t segment, uint32_t page);
void serialize_msg_cpu_kernel_sigsegv(t_buffer* buffer, t_PCB* pcb);

// Deserialize messages
void deserialize_msg_kernel_cpu_interrupt(t_buffer* buffer, uint32_t* pid);
void deserialize_msg_cpu_kernel_interrupt(t_buffer* buffer, t_PCB* pcb);
void deserialize_msg_kernel_cpu_exec(t_buffer* buffer, t_PCB* pcb);
void deserialize_msg_cpu_kernel_io(t_buffer* buffer, t_PCB* pcb);
void deserialize_msg_cpu_kernel_exit(t_buffer* buffer, t_PCB* pcb);
void deserialize_msg_cpu_kernel_page_fault(t_buffer* buffer, t_PCB* pcb, uint32_t* segment, uint32_t* page);
void deserialize_msg_cpu_kernel_sigsegv(t_buffer* buffer, t_PCB* pcb);

// Send messages
int send_msg_kernel_cpu_interrupt(uint32_t pid, int fd);
int send_msg_cpu_kernel_interrupt(t_PCB* pcb, int fd);
int send_msg_kernel_cpu_exec(t_PCB* pcb, int fd);
int send_msg_cpu_kernel_io(t_PCB* pcb, int fd);
int send_msg_cpu_kernel_exit(t_PCB* pcb, int fd);
int send_msg_cpu_kernel_page_fault(t_PCB* pcb, uint32_t segment, uint32_t page, int fd);
int send_msg_cpu_kernel_sigsegv(t_PCB* pcb, int fd);

// Receive messages
int recv_msg_kernel_cpu_interrupt(t_buffer* buffer, uint32_t* pid);
int recv_msg_cpu_kernel_interrupt(t_buffer* buffer, t_PCB* pcb);
int recv_msg_kernel_cpu_exec(t_buffer* buffer, t_PCB* pcb);
int recv_msg_cpu_kernel_io(t_buffer* buffer, t_PCB* pcb);
int recv_msg_cpu_kernel_exit(t_buffer* buffer, t_PCB* pcb);
int recv_msg_cpu_kernel_page_fault(t_buffer* buffer, t_PCB* pcb, uint32_t* segment, uint32_t* page);
int recv_msg_cpu_kernel_sigsegv(t_buffer* buffer, t_PCB* pcb);


/*
 *
 *      KERNEL & MEMORY: SEND & RECEIVE MESSAGES
 * 
 */

// -- -- SEND MESSAGES -- --

// --   INIT    --
// KERNEL -> MEMORIA :: MSG_KERNEL_MEMORIA_INIT
int send_msg_kernel_memoria_init(uint32_t pid, t_list* segments, int fd);
// MEMORIA -> KERNEL :: MSG_MEMORIA_KERNEL_INIT
int send_msg_memoria_kernel_init(int fd);

// --   PAGE FAULT    --
// KERNEL -> MEMORIA :: MSG_KERNEL_MEMORIA_PAGE_FAULT
int send_msg_kernel_memoria_page_fault(uint32_t pid, uint32_t page_table_id, uint32_t page_number, int fd);
// MEMORIA -> KERNEL :: MSG_MEMORIA_KERNEL_PAGE_FAULT_DONE
int send_msg_memoria_kernel_page_fault_done(int fd);

// --   EXIT    --
// KERNEL -> MEMORIA :: MSG_KERNEL_MEMORIA_EXIT
int send_msg_kernel_memoria_exit(uint32_t pid, int fd);
// KERNEL -> MEMORIA :: MSG_KERNEL_MEMORIA_EXIT ALT. VERSION
// int send_msg_kernel_memoria_exit(t_list* page_table_ids, int fd);
// MEMORIA -> KERNEL :: MSG_MEMORIA_KERNEL_EXIT
int send_msg_memoria_kernel_exit(int fd);


// -- -- RECEIVE MESSAGES -- --

// --   INIT    --
// KERNEL -> MEMORIA :: MSG_KERNEL_MEMORIA_INIT 
int recv_msg_kernel_memoria_init(t_buffer* buffer, uint32_t* pid, t_list* segments);

// --   PAGE FAULT    --
// MSG_KERNEL_MEMORIA_PAGE_FAULT
int recv_msg_kernel_memoria_page_fault(t_buffer* buffer, uint32_t* pid, uint32_t* page_table_id, uint32_t* page_number);

// --   EXIT    --
// KERNEL -> MEMORIA :: MSG_KERNEL_MEMORIA_EXIT
int recv_msg_kernel_memoria_exit(t_buffer* buffer, uint32_t* pid);
// KERNEL -> MEMORIA :: MSG_KERNEL_MEMORIA_EXIT ALT. VERSION
// int recv_msg_kernel_memoria_exit(t_buffer* buffer, t_list* page_table_ids);


/*
 *
 *      CPU & MEMORY: SEND & RECEIVE MESSAGES
 *
 */

// -- -- SEND MESSAGES -- --

// --   INIT    --
// CPU -> MEMORIA :: MSG_CPU_MEMORIA_INIT
int send_msg_cpu_memoria_init(int fd);
// MEMORIA -> CPU :: MSG_MEMORIA_CPU_INIT
int send_msg_memoria_cpu_init(uint32_t page_table_entries_qty, uint32_t page_size, int fd);

// --   PAGE & FRAME    --
// CPU -> MEMORIA :: MSG_CPU_MEMORIA_PAGE
int send_msg_cpu_memoria_page(uint32_t pid, uint32_t page_table_id, uint32_t page, int fd);
// MEMORIA -> CPU :: MSG_MEMORIA_CPU_FRAME
int send_msg_memoria_cpu_frame(uint32_t frame, int fd);

// --   READ DATA   --
// CPU -> MEMORIA :: MSG_CPU_MEMORIA_DATA_READ
int send_msg_cpu_memoria_data_read(uint32_t pid, uint32_t segment, uint32_t page, uint32_t frame, uint32_t offset, int fd);
// MEMORIA -> CPU :: MSG_MEMORIA_CPU_DATA_READ
int send_msg_memoria_cpu_data_read(uint32_t value, int fd);

// --   WRITE DATA   --
// CPU -> MEMORIA :: MSG_CPU_MEMORIA_DATA_WRITE
int send_msg_cpu_memoria_data_write(uint32_t pid, uint32_t segment, uint32_t page, uint32_t frame, uint32_t offset, uint32_t value, int fd);
// MEMORIA -> CPU :: MSG_MEMORIA_CPU_DATA_WRITE
int send_msg_memoria_cpu_data_write(int fd);

// --   PAGE FAULT    --
// MSG_MEMORIA_CPU_PAGE_FAULT
int send_msg_memoria_cpu_page_fault(int fd);


// -- -- RECEIVE MESSAGES -- --

// --   INIT   --
// MEMORIA -> CPU :: MSG_MEMORIA_CPU_INIT
int recv_msg_memoria_cpu_init(t_buffer* buffer, uint32_t* page_table_entries_qty, uint32_t* page_size);

// --   PAGE & FRAME   --
// CPU -> MEMORIA :: MSG_CPU_MEMORIA_PAGE
int recv_msg_cpu_memoria_page(t_buffer* buffer, uint32_t* pid, uint32_t* page_table_id, uint32_t* page);
// MEMORIA -> CPU :: MSG_MEMORIA_CPU_FRAME
int recv_msg_memoria_cpu_frame(t_buffer* buffer, uint32_t* frame);

// --   READ DATA    --
// CPU -> MEMORIA :: MSG_CPU_MEMORIA_DATA_READ
int recv_msg_cpu_memoria_data_read(t_buffer* buffer, uint32_t* pid, uint32_t* segment, uint32_t* page, uint32_t* frame, uint32_t* offset);
// MEMORIA -> CPU :: MSG_MEMORIA_CPU_DATA_READ
int recv_msg_memoria_cpu_data(t_buffer* buffer, uint32_t* value);

// --   WRITE DATA    --
// CPU -> MEMORIA :: MSG_MEMORIA_CPU_DATA_WRITE
int recv_msg_cpu_memoria_data_write(t_buffer* buffer, uint32_t* pid, uint32_t* segment, uint32_t* page, uint32_t* frame, uint32_t* offset, uint32_t* value);


#endif