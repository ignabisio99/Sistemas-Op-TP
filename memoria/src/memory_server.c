#include "memory_server.h"

bool is_cpu_connected = false;
bool is_kernel_connected = false;

void cpu_server();
void kernel_init_server();
void kernel_page_fault_server();
void kernel_exit_server();

/*
 *      MEMORIA <-> CPU MESSAGES
 */

// MSG_CPU_MEMORIA_PAGE
int process_message_cpu_page(t_buffer* buffer) {

    uint32_t pid = 0;
    uint32_t page_table_id = 0;
    uint32_t page_number = 0;

    recv_msg_cpu_memoria_page(buffer, &pid, &page_table_id, &page_number);
    t_entrada_tabla_de_paginas* pagina = get_page_data(pid, page_table_id, page_number);

    if(pagina->presencia == 1){
        log_info(memoria_logger,"ACCESO A TABLA: PID: %d - Segmento %d - Pagina: %d - Marco: %d", 
                pid, page_table_id, page_number, pagina->frame);
        send_msg_memoria_cpu_frame(pagina->frame, fd_cpu);
    } else {
        log_debug(memoria_logger, "PAGE FAULT | PID: %u | Segmento : %u | Página: %u", 
                    pid, page_table_id, page_number);
        send_msg_memoria_cpu_page_fault(fd_cpu);
    }
    

	return 0;
}

// MSG_CPU_MEMORIA_DATA_READ
int process_message_cpu_data_read(t_buffer* buffer) {

    uint32_t pid = 0;
    uint32_t segment = 0;
    uint32_t page_number = 0;
    uint32_t frame = 0;
    uint32_t offset = 0;

    recv_msg_cpu_memoria_data_read(buffer, &pid, &segment, &page_number, &frame, &offset);
    t_entrada_tabla_de_paginas* pagina = get_page_data(pid, segment, page_number);

    pagina->uso = 1;
    uint32_t value = read_data(frame, offset);
    send_msg_memoria_cpu_data_read(value, fd_cpu);

    log_info(memoria_logger, "PID: %u - Accion: LEER <%u> - Direccion fisica: (segmento: %u | Page: %u | Marco: %u | Desplazamiento: %u", 
        pid, value, segment, page_number, frame, offset);
    return 0;
}

// MSG_CPU_MEMORIA_DATA_WRITE
int process_message_cpu_data_write(t_buffer* buffer) {

    uint32_t pid = 0;
    uint32_t segment = 0;
    uint32_t page_number = 0;
    uint32_t frame = 0;
    uint32_t offset = 0;
    uint32_t value = 0;

    recv_msg_cpu_memoria_data_write(buffer, &pid, &segment, &page_number, &frame, &offset, &value);
    t_entrada_tabla_de_paginas* pagina = get_page_data(pid, segment, page_number);
    pagina->uso = 1;
    pagina->modificado = 1;
    write_data(frame, offset, value);

    log_info(memoria_logger, "PID: %u - Accion: ESCRIBIR <%u> - Direccion fisica: (segmento: %u | Page: %u | Marco: %u | Desplazamiento: %u", 
        pid, value, segment, page_number, frame, offset);
    return 0;
}

/*
 *      MEMORIA <-> KERNEL MESSAGES
 */

// MSG_KERNEL_MEMORIA_INIT
int process_message_kernel_init(t_buffer* buffer) {
    uint32_t pid;
    t_list* segments = list_create();
    recv_msg_kernel_memoria_init(buffer, &pid, segments);

    log_debug(memoria_logger,"INIT | pid: %d con %d segmentos", pid, list_size(segments));

    create_process(pid, segments);
    
    segment_list_destroy(segments);
    send_msg_memoria_kernel_init(fd_kernel_init);
	return 0;
}

// MSG_KERNEL_MEMORIA_PAGE_FAULT
int process_message_kernel_page_fault(t_buffer* buffer) {
    uint32_t pid, segment, page;

    recv_msg_kernel_memoria_page_fault(buffer, &pid, &segment, &page);
    
    log_debug(memoria_logger,"PAGE FAULT HANDLER | pid: %d | segment: %d | page: %d", pid, segment, page);
    agregar_pagina_a_memoria(pid, segment, page);
    send_msg_memoria_kernel_page_fault_done(fd_kernel_page_fault);

	return 0;
}

// MSG_KERNEL_MEMORIA_EXIT
int process_message_kernel_exit(t_buffer* buffer) {
    uint32_t pid = 0;
    recv_msg_kernel_memoria_exit(buffer, &pid);

    finalizar_proceso(pid);
    send_msg_memoria_kernel_exit(fd_kernel_exit);
    
	return 0;
}



void memory_server() {

    if(!is_cpu_connected){
        fd_cpu = esperar_cliente(memoria_logger, SERVER_MEMORIA, fd_server_memoria);
        is_cpu_connected = true;

        pthread_t THREAD_CPU;

        if (!pthread_create(&THREAD_CPU, NULL, (void*) cpu_server, NULL))
            pthread_detach(THREAD_CPU);
        else {
            close_program();
        }
    }

    fd_kernel_init = esperar_cliente(memoria_logger,SERVER_MEMORIA, fd_server_memoria);
    is_kernel_connected = true;

    pthread_t THREAD_KERNEL_INIT;

    if (!pthread_create(&THREAD_KERNEL_INIT, NULL, (void*) kernel_init_server, NULL))
        pthread_detach(THREAD_KERNEL_INIT);
    else {
        close_program();
    }

    fd_kernel_page_fault = esperar_cliente(memoria_logger,SERVER_MEMORIA, fd_server_memoria);
    is_kernel_connected = true;

    pthread_t THREAD_KERNEL_PAGE_FAULT;

    if (!pthread_create(&THREAD_KERNEL_PAGE_FAULT, NULL, (void*) kernel_page_fault_server, NULL))
        pthread_detach(THREAD_KERNEL_PAGE_FAULT);
    else {
        close_program();
    }

    fd_kernel_exit = esperar_cliente(memoria_logger,SERVER_MEMORIA, fd_server_memoria);
    is_kernel_connected = true;

    pthread_t THREAD_KERNEL_EXIT;

    if (!pthread_create(&THREAD_KERNEL_EXIT, NULL, (void*) kernel_exit_server, NULL))
        pthread_detach(THREAD_KERNEL_EXIT);
    else {
        close_program();
    }

    log_debug(memoria_logger, "AMBAS CONEXIONES ESTABLECIDAS!!!");    
}

void cpu_server() {

    t_package* packageHandshake = package_create(NULL_HEADER);
    package_recv(packageHandshake, fd_cpu);
    if(packageHandshake->msg_header != MSG_CPU_MEMORIA_INIT){
        log_error(memoria_logger,"SE DEBE RECIBIR EL HANDSHAKE DE CPU ANTES QUE CUALQUIER MSJ");
        exit(EXIT_FAILURE);
    }
    send_msg_memoria_cpu_init(memoria_config->ENTRADAS_POR_TABLA, memoria_config->TAM_PAGINA, fd_cpu);
    package_destroy(packageHandshake);

    while(1) {

        t_package* package = package_create(NULL_HEADER);
        package_recv(package, fd_cpu);

        log_debug(memoria_logger, "CPU | Mensaje recibido: %s", 
            get_string_from_msg_header(package->msg_header)
        );

        switch(package->msg_header) {
            case MSG_CPU_MEMORIA_PAGE:
                process_message_cpu_page(package->buffer);
                break;

            case MSG_CPU_MEMORIA_DATA_READ:
                process_message_cpu_data_read(package->buffer);
                break;

            case MSG_CPU_MEMORIA_DATA_WRITE:
                process_message_cpu_data_write(package->buffer);
                break;

            default:
                log_error(memoria_logger, "%s | Código de mensaje no esperado: %s", 
                    MEMORIA_CPU_SERVER,
                    get_string_from_msg_header(package->msg_header)
                );
        }

        package_destroy(package);
    }

}

void kernel_init_server() {
 
    while(1) {

        t_package* package = package_create(NULL_HEADER);
        package_recv(package, fd_kernel_init);

        switch(package->msg_header) {

            case MSG_KERNEL_MEMORIA_INIT:
                process_message_kernel_init(package->buffer);
                break;
            default:
                log_error(memoria_logger, "%s INIT | Código de mensaje no esperado: %s", 
                    MEMORIA_KERNEL_SERVER,
                    get_string_from_msg_header(package->msg_header)
                );
        }
        
        package_destroy(package);
    }

}

void kernel_page_fault_server() {
 
    while(1) {

        t_package* package = package_create(NULL_HEADER);
        package_recv(package, fd_kernel_page_fault);

        

        switch(package->msg_header) {
            case MSG_KERNEL_MEMORIA_PAGE_FAULT:
                process_message_kernel_page_fault(package->buffer);
                break;
            
            default:
                log_error(memoria_logger, "%s PAGE FAULT | Código de mensaje no esperado: %s", 
                    MEMORIA_KERNEL_SERVER,
                    get_string_from_msg_header(package->msg_header)
                );
        }
        
        package_destroy(package);
    }

}

void kernel_exit_server() {
 
    while(1) {

        t_package* package = package_create(NULL_HEADER);
        package_recv(package, fd_kernel_exit);

        switch(package->msg_header) {

            case MSG_KERNEL_MEMORIA_EXIT:
                process_message_kernel_exit(package->buffer);
                break;
            
            default:
                log_error(memoria_logger, "%s EXIT | Código de mensaje no esperado: %s", 
                    MEMORIA_KERNEL_SERVER,
                    get_string_from_msg_header(package->msg_header)
                );
        }
        
        package_destroy(package);
    }

}