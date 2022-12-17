#include "lists.h"

void mostrar_colas(char* algoritmo, t_queue* cola){
    uint32_t tam_cola = queue_size(cola);    
    char *string = string_new();

    for(int i = 0; i < tam_cola; i++){
        t_PCB* pcb = list_get(cola->elements, i);
        char* pid_string;
        if(i == 0){
            pid_string = string_from_format("%d", pcb->pid);
        } else {
            pid_string = string_from_format(", %d", pcb->pid);
        }

	    string_append(&string, pid_string);
        free(pid_string);
    }
    if(strlen(string) > 0)
        log_info(kernel_logger, "Cola Ready <%s>: [%s]",algoritmo, string);

    free(string);
}

void agregar_a_cola_exit(t_PCB* pcb){
    pthread_mutex_lock(&MUTEX_EXIT);
        queue_push(COLA_EXIT, pcb);
    pthread_mutex_unlock(&MUTEX_EXIT);
    sem_post(&SEM_EXIT);
}

void agregar_a_cola_ready_rr(t_PCB* pcb){
    pthread_mutex_lock(&MUTEX_READY_RR);
        queue_push(COLA_READY_RR, pcb);
        mostrar_colas("RR", COLA_READY_RR);
    pthread_mutex_unlock(&MUTEX_READY_RR);

    sem_post(&SEM_READY);
}

void agregar_a_cola_ready_fifo(t_PCB* pcb){
    pthread_mutex_lock(&MUTEX_READY_FIFO);
        queue_push(COLA_READY_FIFO, pcb);
        mostrar_colas("FIFO", COLA_READY_FIFO);
    pthread_mutex_unlock(&MUTEX_READY_FIFO);

    sem_post(&SEM_READY);
}

void agregar_de_new_a_ready(t_PCB* pcb){
    if(strcasecmp(kernel_config->ALGORITMO_PLANIFICACION, "FIFO") == 0){
        agregar_a_cola_ready_fifo(pcb);
    }
    if(strcasecmp(kernel_config->ALGORITMO_PLANIFICACION, "RR") == 0){
        agregar_a_cola_ready_rr(pcb);
    }
    if(strcasecmp(kernel_config->ALGORITMO_PLANIFICACION, "FEEDBACK") == 0){
        agregar_a_cola_ready_rr(pcb);
    }
}

void agregar_a_ready_fin_quantum(t_PCB* pcb){
    if(strcasecmp(kernel_config->ALGORITMO_PLANIFICACION, "FIFO") == 0){
        agregar_a_cola_ready_fifo(pcb);
    }
    if(strcasecmp(kernel_config->ALGORITMO_PLANIFICACION, "RR") == 0){
        agregar_a_cola_ready_rr(pcb);
    }
    if(strcasecmp(kernel_config->ALGORITMO_PLANIFICACION, "FEEDBACK") == 0){
        agregar_a_cola_ready_fifo(pcb);
    }
}

void agregar_de_blocked_a_ready(t_PCB* pcb){
    if(strcasecmp(kernel_config->ALGORITMO_PLANIFICACION, "FIFO") == 0){
        agregar_a_cola_ready_fifo(pcb);
    }
    if(strcasecmp(kernel_config->ALGORITMO_PLANIFICACION, "RR") == 0){
        agregar_a_cola_ready_rr(pcb);
    }
    if(strcasecmp(kernel_config->ALGORITMO_PLANIFICACION, "FEEDBACK") == 0){
        agregar_a_cola_ready_rr(pcb);
    }
}
