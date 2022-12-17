#include "dispatcher.h"

void create_quantum_counter_thread(uint32_t pid);

t_PCB* get_next_pcb_to_exec(){
    t_PCB* pcb_a_tomar;
    if(strcasecmp(kernel_config->ALGORITMO_PLANIFICACION, "FIFO") == 0){
        pthread_mutex_lock(&MUTEX_READY_FIFO);
            pcb_a_tomar = queue_pop(COLA_READY_FIFO);
        pthread_mutex_unlock(&MUTEX_READY_FIFO);
        return pcb_a_tomar;
    }
    if(strcasecmp(kernel_config->ALGORITMO_PLANIFICACION, "RR") == 0){
        pthread_mutex_lock(&MUTEX_READY_RR);
            pcb_a_tomar = queue_pop(COLA_READY_RR);
        pthread_mutex_unlock(&MUTEX_READY_RR);
        create_quantum_counter_thread(pcb_a_tomar->pid);
        return pcb_a_tomar;
    }
    if(strcasecmp(kernel_config->ALGORITMO_PLANIFICACION, "FEEDBACK") == 0){
        if(!queue_is_empty(COLA_READY_RR)){
            pthread_mutex_lock(&MUTEX_READY_RR);
                pcb_a_tomar = queue_pop(COLA_READY_RR);
            pthread_mutex_unlock(&MUTEX_READY_RR);
            create_quantum_counter_thread(pcb_a_tomar->pid);
            return pcb_a_tomar;
        }
        pthread_mutex_lock(&MUTEX_READY_FIFO);
            pcb_a_tomar = queue_pop(COLA_READY_FIFO);
        pthread_mutex_unlock(&MUTEX_READY_FIFO);
        return pcb_a_tomar;
    }
    log_error(kernel_logger, "Invalid algoritmo_planificación");
    exit(EXIT_FAILURE);
}

// Recibe process id, espera tiempo por config y avisa a la cpu para que interrumpa
void quantum_counter(uint32_t *pid){
    log_info(kernel_logger, "Timer de quantum iniciado");
    
    usleep(kernel_config->QUANTUM_RR * 1000);

    send_msg_kernel_cpu_interrupt(*pid, kernelInterruptFd);
    free(pid);
}

void create_quantum_counter_thread(uint32_t pid){
    uint32_t *pid_to_send = malloc(sizeof(uint32_t));
    *pid_to_send = pid;

    pthread_t THREAD_QUANTUM_HANDLER;
    if(!pthread_create(&THREAD_QUANTUM_HANDLER, NULL, (void*) quantum_counter, pid_to_send))
        pthread_detach(THREAD_QUANTUM_HANDLER);
    else {
        log_error(kernel_logger, "ERROR CRITICO INICIANDO EL PLANIFICADOR DE CORTO PLAZO. ABORTANDO.");
        exit(EXIT_FAILURE);
    }
}
