#include "shared_utils.h"

/**
 * convert_string_array_to_int_array toma un array del tipo ["2","4"] (ultimo indice apunta a null)
 * y retorna [2,4] (ultimo indice apunta a 0)
 */
uint32_t* convert_string_array_to_int_array(char** array_of_strings){
    uint16_t strings_array_length = arrayLength(array_of_strings);
    uint32_t* array_of_numbers = malloc(sizeof(uint32_t) * (strings_array_length + 1));
    
    for(uint16_t i = 0; array_of_strings[i] != NULL; i++) {
        array_of_numbers[i] = atoi(array_of_strings[i]);
    }
    array_of_numbers[strings_array_length] = 0;

    return array_of_numbers;
}

uint16_t arrayLength(char** array){
    uint16_t i = 0;
    while(array[i] != NULL){
        i++;
    }
    return i;
}

uint16_t intArrayLength(uint32_t* array){
    uint16_t i = 0;
    while(array[i] != 0){
        i++;
    }
    return i;
}


/*
 *  Conversion type functions: Enum <-> String  
 */ 

// Get "instruction code" typedef enum from "instruction code" string format
t_instruction_code get_instruction_code_from_string(char* instruction_code) {

    if (strcmp(instruction_code, "SET") == 0)       { return SET; }
    if (strcmp(instruction_code, "ADD") == 0)       { return ADD; }
    if (strcmp(instruction_code, "MOV_IN") == 0)    { return MOV_IN; }
    if (strcmp(instruction_code, "MOV_OUT") == 0)   { return MOV_OUT; }
    if (strcmp(instruction_code, "I/O") == 0)       { return I_O; }
    if (strcmp(instruction_code, "EXIT") == 0)      { return EXIT; }    
    // Non-existent instruction code
    return -1;

}

// Get "instruction code" string format from "instruction code" typedef enum
char* get_string_from_instruction_code(t_instruction_code instruction_code) {

	switch(instruction_code) {

        case SET:       return "SET";
        case ADD:       return "ADD";
        case MOV_IN:    return "MOV_IN";
        case MOV_OUT:   return "MOV_OUT";
        case I_O:       return "I/O";
        case EXIT:      return "EXIT";
        // Non-existent instruction code
        default:        return "";

	}

}

// Get "CPU register" typedef enum from "CPU register" string format
t_enum_cpu_registers get_cpu_register_enum_from_string(char* cpu_register) {

    if (strcmp(cpu_register, "AX") == 0)    { return AX; }
    if (strcmp(cpu_register, "BX") == 0)    { return BX; }
    if (strcmp(cpu_register, "CX") == 0)    { return CX; }
    if (strcmp(cpu_register, "DX") == 0)    { return DX; }    
    // Non-existent CPU register
    return -1;

}

// Get "CPU register" string format from "CPU register" typedef enum
char* get_cpu_register_string_from_enum(t_enum_cpu_registers cpu_register) {

	switch(cpu_register) {

        case AX:    return "AX";
        case BX:    return "BX";
        case CX:    return "CX";
        case DX:    return "DX";
        // Non-existent CPU register
        default:        return "";

	}

}

// Get "message header" string format from "message header" typedef enum
char* get_string_from_msg_header(t_msg_header msg_header) {

    switch (msg_header) {

        // CONSOLA <-> KERNEL
        case MSG_CONSOLA_KERNEL_INIT:               return "MSG_CONSOLA_KERNEL_INIT";
        case MSG_KERNEL_CONSOLA_INIT:               return "MSG_KERNEL_CONSOLA_INIT";
        case MSG_KERNEL_CONSOLA_PRINT:              return "MSG_KERNEL_CONSOLA_PRINT";
        case MSG_CONSOLA_KERNEL_PRINT:              return "MSG_CONSOLA_KERNEL_PRINT";
        case MSG_KERNEL_CONSOLA_INPUT:              return "MSG_KERNEL_CONSOLA_INPUT";
        case MSG_CONSOLA_KERNEL_INPUT:              return "MSG_CONSOLA_KERNEL_INPUT";
        case MSG_KERNEL_CONSOLA_EXIT:               return "MSG_KERNEL_CONSOLA_EXIT";

        // KERNEL <-> CPU
        case MSG_KERNEL_CPU_INTERRUPT:              return "MSG_KERNEL_CPU_INTERRUPT";
        case MSG_CPU_KERNEL_INTERRUPT:              return "MSG_CPU_KERNEL_INTERRUPT";
        case MSG_KERNEL_CPU_EXEC:                   return "MSG_KERNEL_CPU_EXEC";
        case MSG_CPU_KERNEL_IO:                     return "MSG_CPU_KERNEL_IO";
        case MSG_CPU_KERNEL_EXIT:                   return "MSG_CPU_KERNEL_EXIT";
        case MSG_CPU_KERNEL_PAGE_FAULT:             return "MSG_CPU_KERNEL_PAGE_FAULT";
        case MSG_CPU_KERNEL_SIGSEGV:                return "MSG_CPU_KERNEL_SIGSEGV";

        // KERNEL <-> MEMORIA
        case MSG_KERNEL_MEMORIA_INIT:               return "MSG_KERNEL_MEMORIA_INIT";
        case MSG_MEMORIA_KERNEL_INIT:               return "MSG_MEMORIA_KERNEL_INIT";
        case MSG_KERNEL_MEMORIA_PAGE_FAULT:         return "MSG_KERNEL_MEMORIA_PAGE_FAULT";
        case MSG_MEMORIA_KERNEL_PAGE_FAULT:         return "MSG_MEMORIA_KERNEL_PAGE_FAULT";
        case MSG_KERNEL_MEMORIA_EXIT:               return "MSG_KERNEL_MEMORIA_EXIT";
        case MSG_MEMORIA_KERNEL_EXIT:               return "MSG_MEMORIA_KERNEL_EXIT";

        // CPU <-> MEMORIA
        case MSG_CPU_MEMORIA_INIT:                  return "MSG_CPU_MEMORIA_INIT";
        case MSG_MEMORIA_CPU_INIT:                  return "MSG_MEMORIA_CPU_INIT";
        case MSG_CPU_MEMORIA_PAGE:                  return "MSG_CPU_MEMORIA_PAGE";
        case MSG_MEMORIA_CPU_FRAME:                 return "MSG_MEMORIA_CPU_FRAME";
        case MSG_CPU_MEMORIA_DATA_READ:             return "MSG_CPU_MEMORIA_DATA_READ";
        case MSG_MEMORIA_CPU_DATA_READ:             return "MSG_MEMORIA_CPU_DATA_READ";
        case MSG_CPU_MEMORIA_DATA_WRITE:            return "MSG_CPU_MEMORIA_DATA_WRITE";
        case MSG_MEMORIA_CPU_DATA_WRITE:            return "MSG_MEMORIA_CPU_DATA_WRITE";
        case MSG_MEMORIA_CPU_PAGE_FAULT:            return "MSG_MEMORIA_CPU_PAGE_FAULT";

        // Non-existent message header
        default:                                    return "";

    }

}


/*
 * CPU registers functions
 */

// Get "CPU register" pointer from an instruction parameter
uint32_t* get_cpu_register_from_instruction_parameter(t_cpu_registers* cpu_registers, char* instruction_parameter) {

    switch (get_cpu_register_enum_from_string(instruction_parameter)) {

        case AX:
            return &(cpu_registers->ax);
        case BX:
            return &(cpu_registers->bx);
        case CX:
            return &(cpu_registers->cx);
        case DX:
            return &(cpu_registers->dx);
        default:
            return NULL;

    }
        
}

// Log CPU registers status
void log_cpu_registers_status(t_log* log, int pid, t_cpu_registers* cpu_registers) {

    // PID: <PID> - Estado registros de CPU
    log_debug(log, "PID: %u - Estado registros de CPU", pid);
    
    // PID: <PID> - AX: <Value> | BX: <Value> | CX: <Value> | DX: <Value>
    log_debug(log, "PID: %u - AX: %u | BX: %u | CX: %u | DX: %u",
        pid,
        cpu_registers->ax,
        cpu_registers->bx,
        cpu_registers->cx,
        cpu_registers->dx
    );

}

// Edit CPU registers values
void edit_cpu_register_value(t_cpu_registers* cpu_registers, char* cpu_register_name, uint32_t value) {

    switch (get_cpu_register_enum_from_string(cpu_register_name)) {

        case AX:
            cpu_registers->ax = value;
            break;
        case BX:
            cpu_registers->bx= value;
            break;
        case CX:
            cpu_registers->cx= value;
            break;
        case DX:
            cpu_registers->dx= value;
            break;
        default:
            ;
    }
        
}

