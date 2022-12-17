#ifndef DISPATCH_H
#define DISPATCH_H

#include <stdio.h>
#include <commons/string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <stdbool.h>
#include <pthread.h>
// CPU
#include "global.h"
#include "config.h"
#include "init_cpu.h"
#include "dispatch.h"
#include "interrupt.h"
#include "cpu.h"
#include "tlb.h"
#include "mmu.h"
// Shared
#include "protocolo.h"
#include "shared_utils.h"


// Execute instruction: SET, ADD, MOV IN, MOV OUT, I/O, EXIT
int exec_set(t_PCB* pcb, t_instruction* instruction);
int exec_add(t_PCB* pcb, t_instruction* instruction);
int exec_mov_in(t_PCB* pcb, t_instruction* instruction);
int exec_mov_out(t_PCB* pcb, t_instruction* instruction);
int exec_io(t_PCB* pcb);
int exec_exit(t_PCB* pcb);

// Return PCB to Kernel functions
// -- Return PCB to Kernel based on instruction code (I/O & EXIT)
int should_return_to_kernel(t_instruction_code instruction_code);
// -- Return PCB to Kernel based on an interruption
int return_pcb_interrupt(t_PCB* pcb);

// Fetch instruction
t_instruction* fetch_instruction(t_PCB* pcb);

// Fetch operands
// int fetch_operands (t_PCB* pcb, t_instruccion* instruc);

// Execute instruction 
int exec_instruction(t_PCB* pcb, t_instruction* instruction);

// Execute instruction DELAY (SET & ADD instructions)
void exec_instruction_delay(int pid);

// Dispatch server
void dispatch_server();


#endif
