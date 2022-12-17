#include <estructuras.h>

/*
 *  Create & destroy structures
 */

// PCB
t_PCB* pcb_create(uint32_t pid, t_list* instructions, t_list* segments) {

    t_PCB* pcb = malloc(sizeof(t_PCB));

    pcb->pid = pid;
    pcb->instructions = instructions;
    pcb->pc = 0;
    pcb->cpu_registers = cpu_registers_create();
    pcb->segments = segments;

    return pcb;

}

void pcb_destroy(t_PCB* pcb) {

    instruction_list_destroy(pcb->instructions);
    segment_list_destroy(pcb->segments);
    cpu_registers_destroy(pcb->cpu_registers);
    free(pcb);

}

// Segments
t_segment* segment_create() {

    t_segment* segment = malloc(sizeof(t_segment));

    segment->segment = 0;
    segment->page = 0;

    return segment;
}

void instruction_list_destroy(t_list* instructions) {

    size_t size_list = list_size(instructions);

    for (int i = 0; i < size_list; i++) {
        t_instruction* instruction = list_get(instructions, i);
        free(instruction->param_1);
        free(instruction->param_2);
        free(instruction);
    }

    list_destroy(instructions);
}

void segment_list_destroy(t_list* segments) {

    size_t size_list = list_size(segments);

    for (int i = 0; i < size_list; i++) {
        free(list_get(segments, i));
    }

    list_destroy(segments);

}

// CPU registers
t_cpu_registers* cpu_registers_create() {

    t_cpu_registers* cpu_registers = malloc(sizeof(t_cpu_registers));

    cpu_registers->ax = 0;
    cpu_registers->bx = 0;
    cpu_registers->cx = 0;
    cpu_registers->dx = 0;

    return cpu_registers;

}

void cpu_registers_destroy(t_cpu_registers* cpu_registers) {

    free(cpu_registers);

}

// Instruction
t_instruction* instruction_create(t_instruction_code instruction_code, char* param_1, char* param_2) {

    t_instruction* instruction = malloc(sizeof(instruction));

    instruction->instruction_code = instruction_code;
    instruction->param_1 = param_1;
    instruction->param_2 = param_2;

    return instruction;

}

void instruction_destroy(t_instruction* instruction) {

    free(instruction->param_1);
    free(instruction->param_2);
    free(instruction);

}
