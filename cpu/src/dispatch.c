#include "dispatch.h"


int fd_server_dispatch;
int fd_kernel_dispatch;


void _data_read(uint32_t pid, t_datos_dir_logica* dir_logica, uint32_t frame, uint32_t* memory_value){
    send_msg_cpu_memoria_data_read(pid, dir_logica->num_segmento, dir_logica->num_pagina, frame, dir_logica->desplazamiento_pagina, fd_memoria);

    t_package* paquete = package_create(NULL_HEADER);
    package_recv(paquete, fd_memoria);

    if(paquete->msg_header != MSG_MEMORIA_CPU_DATA_READ){
        log_debug(cpu_logger, "Se espera recibir mensaje desde memoria de data read");
        exit(EXIT_FAILURE);
    }

    recv_msg_memoria_cpu_data(paquete->buffer, memory_value);
    package_destroy(paquete);
}

int read_from_memory(t_PCB* pcb, uint32_t pid, uint32_t logical_address, uint32_t* memory_value) {
    uint32_t frame;
    t_datos_dir_logica* dir_logica = crear_dir_logica(logical_address);

    if(obtener_marco(pid, dir_logica->num_segmento, dir_logica->num_pagina, &frame)){
        //TLB HIT
        _data_read(pid, dir_logica, frame, memory_value);

        free(dir_logica);
        // Retorno que no fue page fault
        return 1;
    }

    // TLB MISS
    send_msg_cpu_memoria_page(pid, dir_logica->num_segmento, dir_logica->num_pagina, fd_memoria);

    t_package* paquete = package_create(NULL_HEADER);
    package_recv(paquete, fd_memoria);

    if(paquete->msg_header == MSG_MEMORIA_CPU_FRAME){
        
        recv_msg_memoria_cpu_frame(paquete->buffer, &frame);
        reemplazar(pid, dir_logica->num_segmento, dir_logica->num_pagina, frame); //Reemplazamos en la TLB
        _data_read(pid, dir_logica, frame, memory_value);
        package_destroy(paquete);

        free(dir_logica);
        return 1;
    } else if(paquete->msg_header == MSG_MEMORIA_CPU_PAGE_FAULT){
        pcb->pc = pcb->pc - 1;
        send_msg_cpu_kernel_page_fault(pcb, dir_logica->num_segmento, dir_logica->num_pagina, fd_kernel_dispatch);
        sem_post(&SOCKET_KERNEL);

        // Retornamos 0 por page fault
        package_destroy(paquete);
        free(dir_logica);
        return 0;
    } else {
        log_debug(cpu_logger, "Se espera recibir mensaje desde memoria de page");
        exit(EXIT_FAILURE);
    }
}

void _write_data(uint32_t pid, t_datos_dir_logica* dir_logica, uint32_t frame, uint32_t write_value){
    send_msg_cpu_memoria_data_write(pid, dir_logica->num_segmento, dir_logica->num_pagina, frame, dir_logica->desplazamiento_pagina, write_value, fd_memoria);
}

int write_into_memory(t_PCB* pcb, uint32_t pid, uint32_t logical_address, uint32_t cpu_register) {
    uint32_t frame;
    t_datos_dir_logica* dir_logica = crear_dir_logica(logical_address);

    if(obtener_marco(pid, dir_logica->num_segmento, dir_logica->num_pagina, &frame)){
        //TLB HIT
        _write_data(pid, dir_logica, frame, cpu_register);

        free(dir_logica);
        // Retorno que no fue page fault
        return 1;
    }

    // TLB MISS
    send_msg_cpu_memoria_page(pid, dir_logica->num_segmento, dir_logica->num_pagina, fd_memoria);

    t_package* paquete = package_create(NULL_HEADER);
    package_recv(paquete, fd_memoria);

    if(paquete->msg_header == MSG_MEMORIA_CPU_FRAME){
        recv_msg_memoria_cpu_frame(paquete->buffer, &frame);
        reemplazar(pid, dir_logica->num_segmento, dir_logica->num_pagina, frame); //Reemplazamos en la TLB
        _write_data(pid, dir_logica, frame, cpu_register);
        package_destroy(paquete);

        free(dir_logica);
        return 1;
    } else if(paquete->msg_header == MSG_MEMORIA_CPU_PAGE_FAULT){
        pcb->pc = pcb->pc - 1;
        send_msg_cpu_kernel_page_fault(pcb, dir_logica->num_segmento, dir_logica->num_pagina, fd_kernel_dispatch);
        sem_post(&SOCKET_KERNEL);

        // Retornamos 0 por page fault
        package_destroy(paquete);
        free(dir_logica);
        return 0;
    } else {
        log_debug(cpu_logger, "Se espera recibir mensaje desde memoria de page");
        exit(EXIT_FAILURE);
    }
}

// Execute instruction DELAY (SET & ADD instructions)
void exec_instruction_delay(int pid) {
    
    log_debug(cpu_logger, "PID: %d - Retardo de instrucción: Inicio", pid);
    usleep(cpu_config->RETARDO_INSTRUCCION * 1000);
    log_debug(cpu_logger, "PID: %d - Retardo de instrucción: Fin", pid);

}

// Return PCB to Kernel based on instruction code (I/O & EXIT)
int should_return_to_kernel(t_instruction_code instruction_code) {

    switch(instruction_code) {

        case I_O:
        case EXIT:
            return 1;

        default:
            return 0;

    }
}


/*
 *  Execute SET
 *
 *  SET (Registro, Valor)
 *  Asigna al <Registro> el <Valor> pasado como parámetro.
 * 
 */
int exec_set(t_PCB* pcb, t_instruction* instruction) {
    // Registro CPU
   
    uint32_t* cpu_register = get_cpu_register_from_instruction_parameter(pcb->cpu_registers, instruction->param_1);
    
    if (cpu_register == NULL) {
        log_error(cpu_logger, "Registro de CPU inválido");
        return EXIT_FAILURE;        
    }

    // SET (Registro, Valor)
    *cpu_register = atoi(instruction->param_2);

    log_cpu_registers_status(cpu_logger, pcb->pid, pcb->cpu_registers);

    exec_instruction_delay(pcb->pid);

    return EXIT_SUCCESS;

}


/*
 *  Execute ADD
 *  
 *  ADD (Registro Destino, Registro Origen)
 *  Sumar ambos registros y dejar el resultado en <Registro Destino>.
 *  
 */
int exec_add(t_PCB* pcb, t_instruction* instruction) {
    // Registro origen (parámetro 2)
    uint32_t* cpu_register_source = get_cpu_register_from_instruction_parameter(pcb->cpu_registers, instruction->param_2);
    // Registro destino (parámetro 1)
    uint32_t* cpu_register_destination = get_cpu_register_from_instruction_parameter(pcb->cpu_registers, instruction->param_1);

    // Set memory value to CPU register
    if (cpu_register_source == NULL || cpu_register_source == NULL) {
        log_error(cpu_logger, "Registro de CPU inválido");
        return EXIT_FAILURE;        
    }

    // ADD (Registro destino, Registro origen)
    *cpu_register_destination += *cpu_register_source;    

    log_cpu_registers_status(cpu_logger, pcb->pid, pcb->cpu_registers);

    exec_instruction_delay(pcb->pid);

    return EXIT_SUCCESS;

}


/*
 *  Execute MOV IN
 *
 *  MOV_IN (Registro, Dirección Lógica)
 *  Lee el valor de memoria del segmento de datos correspondiente a la <Dirección Lógica> y lo almacena en el <Registro>.
 * 
 */
int exec_mov_in(t_PCB* pcb, t_instruction* instruction) {
    uint8_t should_break_loop = 0;  

    // Get CPU register
    uint32_t* cpu_register = get_cpu_register_from_instruction_parameter(pcb->cpu_registers, instruction->param_1);
    if (cpu_register == NULL) {
        log_error(cpu_logger, "Registro de CPU inválido");
        return EXIT_FAILURE;
    }
    
    uint32_t memory_value = 0;
    uint32_t logical_address = atoi(instruction->param_2);

    if (read_from_memory(pcb, pcb->pid, logical_address, &memory_value)) {
    
        // Set memory value to CPU register
        *cpu_register = memory_value;
    }
    else {
        // Caso de page fault
        should_break_loop = 1;  
    }
    
    log_cpu_registers_status(cpu_logger, pcb->pid, pcb->cpu_registers);

    return should_break_loop;

}

/*
 *  Execute MOV OUT
 * 
 *  MOV_OUT (Dirección Lógica, Registro)
 *  Lee el valor del <Registro> y lo escribe en la dirección física de memoria del segmento de datos obtenido a partir de la <Dirección Lógica>.
 * 
 */
int exec_mov_out(t_PCB* pcb, t_instruction* instruction) {
    uint8_t should_break_loop = 0;  

    // Dirección lógica (parámetro 1)
    uint32_t logical_address = atoi(instruction->param_1);

    // Registro CPU (parámetro 2)
    uint32_t* cpu_register = get_cpu_register_from_instruction_parameter(pcb->cpu_registers, instruction->param_2);
    if (cpu_register == NULL) {
        log_error(cpu_logger, "Registro de CPU inválido");
        return EXIT_FAILURE;
    }
    
    if (write_into_memory(pcb, pcb->pid, logical_address, *cpu_register)) {
        log_debug(cpu_logger,"//TODO SACAR. LO METIMO EN MEMORIA");
    }
    else {
        // Caso de page fault
        should_break_loop = 1;
    }

    log_cpu_registers_status(cpu_logger, pcb->pid, pcb->cpu_registers);

    return should_break_loop;
}

// Execute I/O
int exec_io(t_PCB* pcb) {
    
    send_msg_cpu_kernel_io(pcb, fd_kernel_dispatch);
    sem_post(&SOCKET_KERNEL);


    return EXIT_SUCCESS;

}

// Execute EXIT
int exec_exit(t_PCB* pcb) {

    limpiar_proceso_TLB(pcb->pid);
    send_msg_cpu_kernel_exit(pcb, fd_kernel_dispatch);
    sem_post(&SOCKET_KERNEL);

    

    return EXIT_SUCCESS;

}

// FETCH instruction
t_instruction* fetch_instruction(t_PCB* pcb) {

    size_t size_list = list_size(pcb->instructions);

    if (pcb->pc >= size_list) {
        log_error(cpu_logger, "Program counter (%u) inválido", pcb->pc);
        return NULL;
    }

    return list_get(pcb->instructions, pcb->pc);

}

// EXECUTE instruction
int exec_instruction(t_PCB* pcb, t_instruction* instruction) {

    uint8_t should_break_loop = 0;

    // PID: <PID> - Ejecutando: <INSTRUCCIÓN> - <PARÁMETRO 1> - <PARÁMETRO 2>
    log_info(cpu_logger, "PID: %u - Ejecutando: %s - %s - %s", 
        pcb->pid,                                                           // <PID>
        get_string_from_instruction_code(instruction->instruction_code),    // <INSTRUCTION>
        instruction->param_1,                                               // <PARÁMETRO 1>
        instruction->param_2                                                // <PARÁMETRO 2>
    );

    // Update program counter
    pcb->pc++;

    switch(instruction->instruction_code) {

        case SET:
            exec_set(pcb, instruction);
            break;
        case ADD:
            exec_add(pcb, instruction);
            break;
        case MOV_IN:
            should_break_loop = exec_mov_in(pcb, instruction);
            break;
        case MOV_OUT:
            should_break_loop = exec_mov_out(pcb, instruction);
            break;
        case I_O:
            exec_io(pcb);
            break;
        case EXIT:
            exec_exit(pcb);
            break;
        default:
            log_error(cpu_logger, "Código de instrucción no esperado");
    }

    return should_break_loop;
}


void dispatch_server() {

    fd_kernel_dispatch = esperar_cliente(cpu_logger, SERVER_DISPATCH, fd_server_dispatch);

    for (;;) {

        // Package receive
        t_package* package = package_create(NULL_HEADER);

        sem_wait(&SOCKET_KERNEL);
        package_recv(package, fd_kernel_dispatch);
        
        // Check message header
        if (package->msg_header != MSG_KERNEL_CPU_EXEC) {
            log_error(cpu_logger, "Código de mensaje no esperado: %s",
                get_string_from_msg_header(package->msg_header)
            );
            package_destroy(package);
            break;
        }

        // PCB
        t_PCB* pcb = malloc(sizeof(t_PCB));        
        recv_msg_kernel_cpu_exec(package->buffer, pcb);

        // Execute instruccion list
        size_t size_list = list_size(pcb->instructions);        
        while (pcb->pc < size_list) {

            // Fetch instruction
            t_instruction* instruction = fetch_instruction(pcb);
            if (instruction == NULL) {
                break;
            }
            
            if(instruction->instruction_code == MOV_IN) {
                if(es_seg_fault(pcb->pid, pcb->segments,atoi(instruction->param_2))){
                    limpiar_proceso_TLB(pcb->pid);
                    send_msg_cpu_kernel_sigsegv(pcb,fd_kernel_dispatch);
                    sem_post(&SOCKET_KERNEL);
                    break;
                }
            }

            if(instruction->instruction_code == MOV_OUT) {
                if(es_seg_fault(pcb->pid, pcb->segments,atoi(instruction->param_1))){
                    limpiar_proceso_TLB(pcb->pid);
                    send_msg_cpu_kernel_sigsegv(pcb,fd_kernel_dispatch);
                    sem_post(&SOCKET_KERNEL);
                    break;
                }
            }

            // Exec instruction
            if(exec_instruction(pcb, instruction)){
                break;
            }
            

            // Return PCB to Kernel based on instruction code (I/O & EXIT)
            if (should_return_to_kernel(instruction->instruction_code)) {
                break;
            }

            // Check interrupt
            pthread_mutex_lock(&mutex_interrupt);
            if((interrupt) && (pcb->pid == pid_interrupted)) {
                interrupt = false;
                pthread_mutex_unlock(&mutex_interrupt);
                send_msg_cpu_kernel_interrupt(pcb, fd_kernel_dispatch);
                sem_post(&SOCKET_KERNEL);
                break;
            }
            else {
                pthread_mutex_unlock(&mutex_interrupt);
            }

        }

        pcb_destroy(pcb);
        package_destroy(package);

    }   
     
}