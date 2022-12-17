#include "blocked.h"

void blocked_handler(void *args);
void procesarIO(t_instruction* instruccion, t_PCB* pcb);

void create_blocked_thread(t_PCB *pcb_recibido, t_package *paquete){
    t_arg_blocked_thread* args = malloc(sizeof(t_arg_blocked_thread));
    args->pcb_recibido = pcb_recibido;

    pthread_t THREAD_BLOCKED;
    if(!pthread_create(&THREAD_BLOCKED, NULL, (void*) blocked_handler, (void*) args))
        pthread_detach(THREAD_BLOCKED);
    else {
        log_error(kernel_logger, "ERROR CRITICO INICIANDO EL HANDLER DE BLOQUEADOS. ABORTANDO.");
        exit(EXIT_FAILURE);
    }

}

void blocked_handler(void *args){
    t_arg_blocked_thread* parsed_args = (t_arg_blocked_thread*) args;
    t_PCB* pcb_recibido = parsed_args->pcb_recibido;
    free(parsed_args);

    t_instruction *instruction = list_get(pcb_recibido->instructions, pcb_recibido->pc - 1);
    if(strcasecmp(instruction->param_1, "TECLADO") == 0 ){
        pthread_mutex_lock(&MUTEX_CONSOLAS);
            t_consola_pid* consola_pid = find_pid(pcb_recibido->pid, LISTA_CONSOLAS);
        pthread_mutex_unlock(&MUTEX_CONSOLAS);

        send_msg_kernel_consola_input(consola_pid->consola_socket);
        log_info(kernel_logger,"PID: %d - Estado Anterior: %s - Estado Actual: %s", pcb_recibido->pid, "EXEC", "BLOCKED (Teclado)");

        t_package* paquete_teclado = package_create(NULL_HEADER);
        package_recv(paquete_teclado, consola_pid->consola_socket);
        if(paquete_teclado->msg_header != MSG_CONSOLA_KERNEL_INPUT){
            log_error(kernel_logger, "Se espera recibir mensaje de MSG_CONSOLA_KERNEL_INPUT y se recibe: %s", get_string_from_msg_header(paquete_teclado->msg_header));
            exit(EXIT_FAILURE);
        }

        uint32_t register_value;
        recv_msg_consola_kernel_input(paquete_teclado->buffer, &register_value);

        edit_cpu_register_value(pcb_recibido->cpu_registers, instruction->param_2, register_value);
        log_info(kernel_logger,"PID: %d - Estado Anterior: %s - Estado Actual: %s", pcb_recibido->pid, "BLOCKED (Teclado)", "READY");
        agregar_de_blocked_a_ready(pcb_recibido);

        package_destroy(paquete_teclado);
        return;
    }
    if(strcasecmp(instruction->param_1, "PANTALLA") == 0 ){
        pthread_mutex_lock(&MUTEX_CONSOLAS);
            t_consola_pid* consola_pid = find_pid(pcb_recibido->pid, LISTA_CONSOLAS);
        pthread_mutex_unlock(&MUTEX_CONSOLAS);
        uint32_t* cpu_register = get_cpu_register_from_instruction_parameter(pcb_recibido->cpu_registers, instruction->param_2);
        send_msg_kernel_consola_print(*cpu_register, consola_pid->consola_socket);
        log_info(kernel_logger,"PID: %d - Estado Anterior: %s - Estado Actual: %s", pcb_recibido->pid, "EXEC", "BLOCKED (Pantalla)");

        t_package* paquete_pantalla = package_create(NULL_HEADER);
        
        package_recv(paquete_pantalla, consola_pid->consola_socket);
        if(paquete_pantalla->msg_header != MSG_CONSOLA_KERNEL_PRINT){
            log_error(kernel_logger, "Se espera recibir mensaje de CONSOLA_KERNEL_PRINT y se recibe: %s", get_string_from_msg_header(paquete_pantalla->msg_header));
            exit(EXIT_FAILURE);
        }

        log_info(kernel_logger,"PID: %d - Estado Anterior: %s - Estado Actual: %s", pcb_recibido->pid, "BLOCKED (Pantalla)", "READY");
        agregar_de_blocked_a_ready(pcb_recibido);
        package_destroy(paquete_pantalla);
        return;
    }
    procesarIO(instruction,pcb_recibido);
}


void atenderCola(void* args){
    t_dispositivo* disp = (t_dispositivo*) args;
    while(1){
        sem_wait(&disp->SEM_IO);
        
        pthread_mutex_lock(&disp->MUTEX_IO);
        t_PCB* pcb = queue_pop(disp->COLA_IO); 
        pthread_mutex_unlock(&disp->MUTEX_IO);

        t_instruction *instruction = list_get(pcb->instructions, pcb->pc - 1);
        uint32_t unidades_de_trabajo = atoi(instruction->param_2);
        usleep(disp->TIEMPO_IO*unidades_de_trabajo*1000); 
        log_info(kernel_logger,"PID: %d - Estado Anterior: BLOCKED(%s) - Estado Actual: %s", pcb->pid, disp->NOMBRE, "READY");
        agregar_de_blocked_a_ready(pcb); 
    } 
}

void procesarIO(t_instruction* instruccion, t_PCB* pcb){
    uint32_t i =0;
    t_dispositivo* disp = list_get(LISTA_COLAS_DISPOSITIVOS,i);
    while(strcasecmp(instruccion->param_1,disp->NOMBRE) != 0){
        i++;
        disp = list_get(LISTA_COLAS_DISPOSITIVOS,i);
    }
    log_info(kernel_logger,"PID: %d - Estado Anterior: %s - Estado Actual: BLOCKED(%s)", pcb->pid, "EXEC", disp->NOMBRE);
    pthread_mutex_lock(&disp->MUTEX_IO);
    queue_push(disp->COLA_IO, pcb);
    pthread_mutex_unlock(&disp->MUTEX_IO);
    sem_post(&disp->SEM_IO);
    
}

