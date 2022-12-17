#ifndef SHARED_UTILS_H
#define SHARED_UTILS_H

// Standard Library
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

// Commons
#include <commons/log.h>

// Project
#include "protocolo.h"
#include "estructuras.h"


/*
 * 
 */

uint32_t* convert_string_array_to_int_array(char** array_of_strings);
uint16_t arrayLength(char** array);
uint16_t intArrayLength(uint32_t* array);


/*
 * Conversion type functions: Enum <-> String
 */

// Instruction codes
// -- Get "instruction code" typedef enum from "instruction code" string format
t_instruction_code get_instruction_code_from_string(char* instruction_code);
// -- Get "instruction code" string format from "instruction code" typedef enum
char* get_string_from_instruction_code(t_instruction_code instruction_code);

// CPU registers
// -- Get "CPU register" typedef enum from "CPU register" string format
t_enum_cpu_registers get_cpu_register_enum_from_string(char* cpu_register);
// -- Get "CPU register" string format from "CPU register" typedef enum
char* get_cpu_register_string_from_enum(t_enum_cpu_registers cpu_register);

// Message headers
// -- Get "message header" string format from "message header" typedef enum
char* get_string_from_msg_header(t_msg_header msg_header);


/*
 * CPU registers functions
 */

// Get CPU register pointer from an instruction parameter
uint32_t* get_cpu_register_from_instruction_parameter(t_cpu_registers* cpu_registers, char* instruction_parameter);
// Log CPU registers status
void log_cpu_registers_status(t_log* log, int pid, t_cpu_registers* cpu_registers);

void edit_cpu_register_value(t_cpu_registers* cpu_registers, char* cpu_register_name, uint32_t value);

#endif