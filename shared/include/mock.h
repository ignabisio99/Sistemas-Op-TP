#ifndef MOCK_H_
#define MOCK_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <commons/log.h>
#include <stdbool.h>
#include "protocolo.h"
#include "estructuras.h"
#include "shared_utils.h"

// Print
void print_pcb(t_PCB* pcb);
void print_instruction_list(t_list* instructions);
void print_instruction(t_instruction* instruction);
void print_segments(t_list* segments);
void print_cpu_registers(t_cpu_registers* cpu_registers);

// Set up
t_PCB* set_pcb();
t_list* set_instruction_list();
t_instruction* set_instruction();
t_list* set_segments(int qty);
t_cpu_registers* set_cpu_registers(int seed);

// PCB create
t_PCB* pcb_create_base_1();
t_PCB* pcb_create_base_2(); 
t_PCB* pcb_create_base_3();
t_PCB* pcb_create_plani_base_1();
t_PCB* pcb_create_plani_base_2(); 
t_PCB* pcb_create_plani_base_3();
t_PCB* pcb_create_plani_base_4();


#endif