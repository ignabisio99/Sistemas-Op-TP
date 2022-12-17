#include "mock.h"

// PRINT PCB
void print_pcb(t_PCB* pcb) {

    printf("\n\nPCB\n");
    printf("---\n\n");   

    printf("PC\t%u\n", pcb->pc);
    printf("PID\t%u\n", pcb->pid);

    print_segments(pcb->segments);
    print_instruction_list(pcb->instructions);
    print_cpu_registers(pcb->cpu_registers);
}

// PRINT INSTRUCTION LIST
void print_instruction_list(t_list* instructions) {

    printf("\n\nLista de instrucciones:\n");
    printf("------------------\n\n");

    size_t size_list = list_size(instructions);
    printf("Cantidad de instrucciones:\t%ld\n\n", size_list);

    for (int i = 0; i < size_list; i++) {

        printf("\nInstrucción # %d\n", i+1);
        print_instruction(list_get(instructions, i));
    }
}

// PRINT INSTRUCTION
void print_instruction(t_instruction* instruction) {

    printf("Código instrucción:\t%d [%s]\n", instruction->instruction_code, get_string_from_instruction_code(instruction->instruction_code));
    printf("Parámetro 1:\t\t%s\n", instruction->param_1);
    printf("Parámetro 2:\t\t%s\n", instruction->param_2);

}

// PRINT SEGMENTS
void print_segments(t_list* segments) {

    printf("\n\nLista de segmentos\n");
    printf("------------------\n\n");

    size_t size_list = list_size(segments);
    printf("Cantidad de segmentos:\t%ld\n\n", size_list);

    t_segment* segment = malloc(sizeof(t_segment));

    for (int i = 0; i < size_list; i++) {

        segment = list_get(segments, i);
        printf("Segmento # %d:\n", i+1);
        printf("Segmento:\t%u\n", segment->segment);
        printf("Página:\t\t%u\n", segment->page);
        
    }

}

// PRINT CPU REGISTERS
void print_cpu_registers(t_cpu_registers* cpu_registers) {

    printf("\n\nRegistros de CPU\n");
    printf("------------------\n\n");

    printf("AX\t%u\n", cpu_registers->ax);
    printf("BX\t%u\n", cpu_registers->bx);
    printf("CX\t%u\n", cpu_registers->cx);
    printf("DX\t%u\n", cpu_registers->dx);

}

// SET SEGMENT
t_list* set_segments(int qty) {

    t_list* segments = list_create();    

    for (int i = 1; i <= qty; i++) {
        uint32_t* value = malloc(sizeof(uint32_t));
        *value = i * 2;
        list_add(segments, value);
    }

    return segments;
}


// BASE_1
t_PCB* pcb_create_base_1() {

  t_list* BASE_1_segments = set_segments(4);
  t_list* BASE_1_instructions = list_create();
  
  t_instruction* BASE_1_1 = instruction_create(SET, "AX", "1");
  t_instruction* BASE_1_2 = instruction_create(SET, "BX", "1");
  t_instruction* BASE_1_3 = instruction_create(SET, "CX", "1");
  t_instruction* BASE_1_4 = instruction_create(SET, "DX", "1");
  t_instruction* BASE_1_5 = instruction_create(ADD, "AX", "BX");
  t_instruction* BASE_1_6 = instruction_create(ADD, "AX", "CX");
  t_instruction* BASE_1_7 = instruction_create(ADD, "AX", "DX");
  t_instruction* BASE_1_8 = instruction_create(SET, "BX", "2");
  t_instruction* BASE_1_9 = instruction_create(SET, "CX", "2");
  t_instruction* BASE_1_10 = instruction_create(SET, "DX", "2");
  t_instruction* BASE_1_11 = instruction_create(ADD, "AX", "BX");
  t_instruction* BASE_1_12 = instruction_create(ADD, "AX", "CX");
  t_instruction* BASE_1_13 = instruction_create(EXIT, "", "");
    
  list_add(BASE_1_instructions, BASE_1_1);
  list_add(BASE_1_instructions, BASE_1_2);
  list_add(BASE_1_instructions, BASE_1_3);
  list_add(BASE_1_instructions, BASE_1_4);
  list_add(BASE_1_instructions, BASE_1_5);
  list_add(BASE_1_instructions, BASE_1_6);
  list_add(BASE_1_instructions, BASE_1_7);
  list_add(BASE_1_instructions, BASE_1_8);
  list_add(BASE_1_instructions, BASE_1_9);
  list_add(BASE_1_instructions, BASE_1_10);
  list_add(BASE_1_instructions, BASE_1_11);
  list_add(BASE_1_instructions, BASE_1_12);
  list_add(BASE_1_instructions, BASE_1_13);

  t_PCB* BASE_1_pcb = pcb_create(0, BASE_1_instructions, BASE_1_segments);

  return BASE_1_pcb;
    
}


// BASE_2
t_PCB* pcb_create_base_2() {

  t_list* BASE_2_segments = set_segments(4);
  t_list* BASE_2_instructions = list_create();

  t_instruction* BASE_2_1 = instruction_create(SET, "AX", "1");
  t_instruction* BASE_2_2 = instruction_create(SET, "BX", "1");
  t_instruction* BASE_2_3 = instruction_create(ADD, "AX", "BX");
  t_instruction* BASE_2_4 = instruction_create(EXIT, "", "");

  list_add(BASE_2_instructions, BASE_2_1);
  list_add(BASE_2_instructions, BASE_2_2);
  list_add(BASE_2_instructions, BASE_2_3);
  list_add(BASE_2_instructions, BASE_2_4);

  t_PCB* BASE_2_pcb = pcb_create(1, BASE_2_instructions, BASE_2_segments);

  return BASE_2_pcb;

}

// BASE_3
t_PCB* pcb_create_base_3() {

  t_list* BASE_3_segments = set_segments(4);
  t_list* BASE_3_instructions = list_create();
    
  t_instruction* BASE_3_1 = instruction_create(SET, "AX", "1");
  t_instruction* BASE_3_2 = instruction_create(SET, "BX", "1");
  t_instruction* BASE_3_3 = instruction_create(ADD, "AX", "BX");
  t_instruction* BASE_3_4 = instruction_create(SET, "BX", "2");
  t_instruction* BASE_3_5 = instruction_create(ADD, "AX", "BX");
  t_instruction* BASE_3_6 = instruction_create(SET, "BX", "3");
  t_instruction* BASE_3_7 = instruction_create(ADD, "AX", "BX");
  t_instruction* BASE_3_8 = instruction_create(SET, "BX", "4");
  t_instruction* BASE_3_9 = instruction_create(ADD, "AX", "BX");
  t_instruction* BASE_3_10 = instruction_create(SET, "BX", "5");
  t_instruction* BASE_3_11 = instruction_create(ADD, "AX", "BX");
  t_instruction* BASE_3_12 = instruction_create(SET, "BX", "6");
  t_instruction* BASE_3_13 = instruction_create(ADD, "AX", "BX");
  t_instruction* BASE_3_14 = instruction_create(EXIT, "", "");

  list_add(BASE_3_instructions, BASE_3_1);
  list_add(BASE_3_instructions, BASE_3_2);
  list_add(BASE_3_instructions, BASE_3_3);
  list_add(BASE_3_instructions, BASE_3_4);
  list_add(BASE_3_instructions, BASE_3_5);
  list_add(BASE_3_instructions, BASE_3_6);
  list_add(BASE_3_instructions, BASE_3_7);
  list_add(BASE_3_instructions, BASE_3_8);
  list_add(BASE_3_instructions, BASE_3_9);
  list_add(BASE_3_instructions, BASE_3_10);
  list_add(BASE_3_instructions, BASE_3_11);
  list_add(BASE_3_instructions, BASE_3_12);
  list_add(BASE_3_instructions, BASE_3_13);
  list_add(BASE_3_instructions, BASE_3_14);

  t_PCB* BASE_3_pcb = pcb_create(2, BASE_3_instructions, BASE_3_segments);

  return BASE_3_pcb;

}

// PLANI_BASE_1
t_PCB* pcb_create_plani_base_1() {  

  t_list* PLANI_BASE_1_segments = set_segments(4);
  t_list* PLANI_BASE_1_instructions = list_create();

  t_instruction* PLANI_BASE_1_1 = instruction_create(SET, "AX", "1");
  t_instruction* PLANI_BASE_1_2 = instruction_create(I_O, "TECLADO", "BX");
  t_instruction* PLANI_BASE_1_3 = instruction_create(ADD, "AX", "BX");
  t_instruction* PLANI_BASE_1_4 = instruction_create(I_O, "PANTALLA", "AX");
  t_instruction* PLANI_BASE_1_5 = instruction_create(SET, "CX", "1");
  t_instruction* PLANI_BASE_1_6 = instruction_create(EXIT, "", "");

  list_add(PLANI_BASE_1_instructions, PLANI_BASE_1_1);
  list_add(PLANI_BASE_1_instructions, PLANI_BASE_1_2);
  list_add(PLANI_BASE_1_instructions, PLANI_BASE_1_3);
  list_add(PLANI_BASE_1_instructions, PLANI_BASE_1_4);
  list_add(PLANI_BASE_1_instructions, PLANI_BASE_1_5);
  list_add(PLANI_BASE_1_instructions, PLANI_BASE_1_6);

  t_PCB* PLANI_BASE_1_pcb = pcb_create(3, PLANI_BASE_1_instructions, PLANI_BASE_1_segments);

  return PLANI_BASE_1_pcb;

}

// PLANI_BASE_2
t_PCB* pcb_create_plani_base_2() {

  t_list* PLANI_BASE_2_segments = set_segments(4);
  t_list* PLANI_BASE_2_instructions = list_create();

  t_instruction* PLANI_BASE_2_1 = instruction_create(SET, "AX", "1");
  t_instruction* PLANI_BASE_2_2 = instruction_create(SET, "CX", "1");
  t_instruction* PLANI_BASE_2_3 = instruction_create(SET, "DX", "1");
  t_instruction* PLANI_BASE_2_4 = instruction_create(ADD, "CX", "DX");
  t_instruction* PLANI_BASE_2_5 = instruction_create(ADD, "DX", "CX");
  t_instruction* PLANI_BASE_2_6 = instruction_create(ADD, "AX", "CX");
  t_instruction* PLANI_BASE_2_7 = instruction_create(I_O, "PANTALLA", "AX");
  t_instruction* PLANI_BASE_2_8 = instruction_create(SET, "BX", "1");
  t_instruction* PLANI_BASE_2_9 = instruction_create(I_O, "PANTALLA", "BX");
  t_instruction* PLANI_BASE_2_10 = instruction_create(ADD, "AX", "BX");
  t_instruction* PLANI_BASE_2_11 = instruction_create(EXIT, "", "");

  list_add(PLANI_BASE_2_instructions, PLANI_BASE_2_1);
  list_add(PLANI_BASE_2_instructions, PLANI_BASE_2_2);
  list_add(PLANI_BASE_2_instructions, PLANI_BASE_2_3);
  list_add(PLANI_BASE_2_instructions, PLANI_BASE_2_4);
  list_add(PLANI_BASE_2_instructions, PLANI_BASE_2_5);
  list_add(PLANI_BASE_2_instructions, PLANI_BASE_2_6);
  list_add(PLANI_BASE_2_instructions, PLANI_BASE_2_7);
  list_add(PLANI_BASE_2_instructions, PLANI_BASE_2_8);
  list_add(PLANI_BASE_2_instructions, PLANI_BASE_2_9);
  list_add(PLANI_BASE_2_instructions, PLANI_BASE_2_10);
  list_add(PLANI_BASE_2_instructions, PLANI_BASE_2_11);

  t_PCB* PLANI_BASE_2_pcb = pcb_create(4, PLANI_BASE_2_instructions,  PLANI_BASE_2_segments);

  return PLANI_BASE_2_pcb;

}

// PLANI_BASE_3
t_PCB* pcb_create_plani_base_3() {

  t_list* PLANI_BASE_3_segments = set_segments(4);
  t_list* PLANI_BASE_3_instructions = list_create();

  t_instruction* PLANI_BASE_3_1 = instruction_create(SET, "AX", "1");
  t_instruction* PLANI_BASE_3_2 = instruction_create(SET, "BX", "1");
  t_instruction* PLANI_BASE_3_3 = instruction_create(I_O, "DISCO", "7");
  t_instruction* PLANI_BASE_3_4 = instruction_create(SET, "CX", "1");
  t_instruction* PLANI_BASE_3_5 = instruction_create(SET, "DX", "1");
  t_instruction* PLANI_BASE_3_6 = instruction_create(I_O, "IMPRESORA", "2");
  t_instruction* PLANI_BASE_3_7 = instruction_create(SET, "AX", "1");
  t_instruction* PLANI_BASE_3_8 = instruction_create(EXIT, "", "");

  list_add(PLANI_BASE_3_instructions, PLANI_BASE_3_1);
  list_add(PLANI_BASE_3_instructions, PLANI_BASE_3_2);
  list_add(PLANI_BASE_3_instructions, PLANI_BASE_3_3);
  list_add(PLANI_BASE_3_instructions, PLANI_BASE_3_4);
  list_add(PLANI_BASE_3_instructions, PLANI_BASE_3_5);
  list_add(PLANI_BASE_3_instructions, PLANI_BASE_3_6);
  list_add(PLANI_BASE_3_instructions, PLANI_BASE_3_7);
  list_add(PLANI_BASE_3_instructions, PLANI_BASE_3_8);

  t_PCB* PLANI_BASE_3_pcb = pcb_create(5, PLANI_BASE_3_instructions,  PLANI_BASE_3_segments);

  return PLANI_BASE_3_pcb;

}
    
// PLANI_BASE_4
t_PCB* pcb_create_plani_base_4() {

  t_list* PLANI_BASE_4_segments = set_segments(4);
  t_list* PLANI_BASE_4_instructions = list_create();

  t_instruction* PLANI_BASE_4_1 = instruction_create(SET, "AX", "1");
  t_instruction* PLANI_BASE_4_2 = instruction_create(I_O, "DISCO", "3");
  t_instruction* PLANI_BASE_4_3 = instruction_create(SET, "BX", "1");
  t_instruction* PLANI_BASE_4_4 = instruction_create(I_O, "IMPRESORA", "3");
  t_instruction* PLANI_BASE_4_5 = instruction_create(SET, "CX", "1");
  t_instruction* PLANI_BASE_4_6 = instruction_create(SET, "DX", "1");
  t_instruction* PLANI_BASE_4_7 = instruction_create(EXIT, "", "");

  list_add(PLANI_BASE_4_instructions, PLANI_BASE_4_1);
  list_add(PLANI_BASE_4_instructions, PLANI_BASE_4_2);
  list_add(PLANI_BASE_4_instructions, PLANI_BASE_4_3);
  list_add(PLANI_BASE_4_instructions, PLANI_BASE_4_4);
  list_add(PLANI_BASE_4_instructions, PLANI_BASE_4_5);
  list_add(PLANI_BASE_4_instructions, PLANI_BASE_4_6);
  list_add(PLANI_BASE_4_instructions, PLANI_BASE_4_7);

  t_PCB* PLANI_BASE_4_pcb = pcb_create(6, PLANI_BASE_4_instructions,  PLANI_BASE_4_segments);

  return PLANI_BASE_4_pcb;

}