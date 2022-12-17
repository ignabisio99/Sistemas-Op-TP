#include "consola.h"


// MSG_KERNEL_CONSOLA_INIT
int process_message_init(int fd_kernel) {

    log_info(consola_logger, "Conexión con Kernel establecida");

    return EXIT_SUCCESS;
}


// MSG_KERNEL_CONSOLA_PRINT
int process_message_print(t_buffer* buffer, int fd_kernel) {
    uint32_t value;

    recv_msg_kernel_consola_print(buffer, &value);
    log_info(consola_logger, "Valor recibido: %u", value);
    log_debug(consola_logger, "Retardo de pantalla: Inicio");
    usleep(consola_config->TIEMPO_PANTALLA * 1000); 
    log_debug(consola_logger, "Retardo de pantalla: Fin");
    send_msg_consola_kernel_print(fd_kernel);
    log_info(consola_logger, "Mensaje enviado a Kernel: MSG_CONSOLA_KERNEL_PRINT");

    return EXIT_SUCCESS;
}


// MSG_KERNEL_CONSOLA_INPUT
int process_message_input(int fd_kernel) {

    char* keyboard_input;
    keyboard_input = readline("Ingresar valor numérico: ");

    uint32_t value = atoi(keyboard_input);

    send_msg_consola_kernel_input(value, fd_kernel);
    log_info(consola_logger, "Valor numérico <%s> enviado a Kernel", keyboard_input);

    free(keyboard_input);

    return EXIT_SUCCESS;
}


// MSG_KERNEL_CONSOLA_EXIT
int process_message_exit(int fd_kernel) {

    log_info(consola_logger, "Finaliza ejecución");
    close_program(consola_config, consola_logger, fd_kernel);

    exit(EXIT_SUCCESS);
}


int receive_and_process_messages(int fd_kernel) {

    t_package* package = package_create(NULL_HEADER);

    package_recv(package, fd_kernel);

    log_info(consola_logger, "Mensaje recibido: %s", get_string_from_msg_header(package->msg_header));

    switch (package->msg_header) {

        case MSG_KERNEL_CONSOLA_INIT:
            process_message_init(fd_kernel);
            break;

        case MSG_KERNEL_CONSOLA_INPUT:
            process_message_input(fd_kernel);
            break;

        case MSG_KERNEL_CONSOLA_PRINT:
            process_message_print(package->buffer, fd_kernel);
            break;

        case MSG_KERNEL_CONSOLA_EXIT:
            process_message_exit(fd_kernel);
            break;

        default: 
            log_error(consola_logger, "Código de instrucción no esperado");
    }

    package_destroy(package);

    return EXIT_SUCCESS;

}

int main(int argc, char* argv[]){

    if(argc!=3) {
        printf("Adjunte correctamente los archivos.\n");
        exit(1);
    }
    FILE* pseudocodigo = fopen(argv[2],"r");

    if (!pseudocodigo){
        printf("Archivos no encontrados \n");
        exit(EXIT_FAILURE);
    }    

    initializeConsola(argv[1],argv[2]);
    fclose(pseudocodigo);
    log_info(consola_logger, "Inicia ejecución");

    // Establece conexión con KERNEL
    int fd_kernel;
    if (!generar_conexiones(consola_config, consola_logger, &fd_kernel)) {
        close_program(consola_config, consola_logger, fd_kernel);
        return EXIT_FAILURE;
    }

    // Genera estructura t_segment
    t_list* segments = list_create();
    for(int i=0;consola_config->SEGMENTOS[i]!=0;i++){
        t_segment* segment = segment_create();
        segment->segment = consola_config->SEGMENTOS[i];
        segment->page = i;
        list_add(segments, segment);        
    }

    
    send_msg_consola_kernel_init(listaDeInstrucciones,segments,fd_kernel);
    while(1){
        receive_and_process_messages(fd_kernel);
    }
    
    exit(EXIT_SUCCESS);
}
