#include "corto_plazo.h"

void create_page_fault_thread(t_PCB *pcb, uint32_t segment, uint32_t page);

void handler_corto_plazo(){
    log_info(kernel_logger, "Controlador de de corto plazo iniciado correctamente");

    // Crear conexión con CPU (dispatch)
    char* puerto_dispatch = string_itoa(kernel_config->PUERTO_CPU_DISPATCH);
    kernelDispatchFd = crear_conexion (
        kernel_logger, 
        "CPU", 
        kernel_config->IP_CPU, 
        puerto_dispatch
    );

     // Crear conexión con CPU (interrupciones)
    char* puerto_interrupt = string_itoa(kernel_config->PUERTO_CPU_INTERRUPT);
    kernelInterruptFd = crear_conexion (
        kernel_logger, 
        "CPU", 
        kernel_config->IP_CPU, 
        puerto_interrupt
    );

    while(1){

        sem_wait(&SEM_READY);

        t_PCB* pcb = get_next_pcb_to_exec();

        log_info(kernel_logger,"PID: %d - Estado Anterior: %s - Estado Actual: %s", pcb->pid, "READY", "EXEC");
        sem_wait(&SEM_CPU);
        send_msg_kernel_cpu_exec(pcb, kernelDispatchFd);
        pcb_destroy(pcb);
        
        t_package* paquete = package_create(NULL_HEADER);
        t_PCB* pcb_recibido = malloc(sizeof(t_PCB));

        package_recv(paquete, kernelDispatchFd);
        sem_post(&SEM_CPU);

        log_debug(kernel_logger,"EN CORTO PLAZO RECIBO: %s", get_string_from_msg_header(paquete->msg_header));
        switch(paquete->msg_header){
            case MSG_CPU_KERNEL_EXIT:
            case MSG_CPU_KERNEL_SIGSEGV:
                recv_msg_cpu_kernel_exit(paquete->buffer, pcb_recibido);
                log_info(kernel_logger,"PID: %d - Estado Anterior: %s - Estado Actual: %s", pcb_recibido->pid, "EXEC", "EXIT");
                agregar_a_cola_exit(pcb_recibido);
                package_destroy(paquete);
                break;
            case MSG_CPU_KERNEL_IO:
                recv_msg_cpu_kernel_io(paquete->buffer, pcb_recibido);
                log_debug(kernel_logger,"Ultimo que funciona: pid: %d", pcb_recibido->pid);
                create_blocked_thread(pcb_recibido, paquete);
                package_destroy(paquete);
                break;
            case MSG_CPU_KERNEL_INTERRUPT:
                recv_msg_cpu_kernel_interrupt(paquete->buffer, pcb_recibido);
                log_info(kernel_logger, "PID: %d - Desalojado por fin de Quantum",pcb_recibido->pid);
                log_info(kernel_logger,"PID: %d - Estado Anterior: %s - Estado Actual: %s", pcb_recibido->pid, "EXEC", "READY");
                agregar_a_ready_fin_quantum(pcb_recibido);
                package_destroy(paquete);
                break;
            case MSG_CPU_KERNEL_PAGE_FAULT:
                uint32_t segment, page;
                recv_msg_cpu_kernel_page_fault(paquete->buffer, pcb_recibido, &segment, &page);
                create_page_fault_thread(pcb_recibido, segment, page);
                log_info(kernel_logger,"PID: %d - Estado Anterior: %s - Estado Actual: %s", pcb_recibido->pid, "EXEC", "BLOCKED (PAGE FAULT)");
                package_destroy(paquete);
                break;
            default:
                log_error(kernel_logger, "ERROR");
                exit(EXIT_FAILURE);
        }
    }
}

// Recibe process id, espera tiempo por config y avisa a la cpu para que interrumpa
void page_fault_handler(void* args){
    t_arg_blocked_page_fault* parsed_args = (t_arg_blocked_page_fault*) args;

    t_PCB* pcb_recibido = parsed_args->pcb_recibido;
    uint32_t segment = parsed_args->segment;
    uint32_t page = parsed_args->page;
    free(parsed_args);

    log_info(kernel_logger, "Page fault iniciado");
    
    sem_wait(&SEM_PAGE_FAULT);

    send_msg_kernel_memoria_page_fault(pcb_recibido->pid, segment, page, fd_memoria_page_fault);

    t_package* paquete = package_create(NULL_HEADER);
    package_recv(paquete, fd_memoria_page_fault);
    if(paquete->msg_header != MSG_MEMORIA_KERNEL_PAGE_FAULT){
        log_debug(kernel_logger, "ERROR AL RECIBIR DESDE MEMORIA LA CONFIRMACION DE PAGE FAULT, received: %s", get_string_from_msg_header(paquete->msg_header));
        exit(EXIT_FAILURE);
    }
    sem_post(&SEM_PAGE_FAULT);

    log_info(kernel_logger,"PID: %d - Estado Anterior: %s - Estado Actual: %s", pcb_recibido->pid, "BLOCKED (PAGE FAULT)", "READY");
    package_destroy(paquete);
    agregar_de_blocked_a_ready(pcb_recibido);
}

void create_page_fault_thread(t_PCB *pcb, uint32_t segment, uint32_t page){
    t_arg_blocked_page_fault* args = malloc(sizeof(t_arg_blocked_page_fault));
    args->pcb_recibido = pcb;
    args->segment = segment;
    args->page = page;

    pthread_t THREAD_PAGE_FAULT;
    if(!pthread_create(&THREAD_PAGE_FAULT, NULL, (void*) page_fault_handler, (void*) args))
        pthread_detach(THREAD_PAGE_FAULT);
    else {
        log_error(kernel_logger, "ERROR CRITICO INICIANDO EL PLANIFICADOR DE CORTO PLAZO. ABORTANDO.");
        exit(EXIT_FAILURE);
    }
}
