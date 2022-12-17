#include "init_cpu.h"


// Config & logger
t_config_cpu* cpu_config;
t_log* cpu_logger;


// Sync
sem_t BLOQUEADOR;
pthread_mutex_t mutex_interrupt;
sem_t SOCKET_KERNEL;

bool interrupt = false;

void memory_handshake();

// Sync
void initializeSemaphores(){
    
    // Main blocking
    sem_init(&BLOQUEADOR, 0, 0);
    sem_init(&SOCKET_KERNEL, 0, 1);

    // Mutex INTERRUPT
    pthread_mutex_init(&mutex_interrupt, NULL);

}

// Config
uint8_t cargar_configuracion(char* config_path) {
    t_config* cfg_aux = config_create(config_path);

    if(cfg_aux == NULL) {
        log_error(cpu_logger, "La ruta de la config es erronea");
        return 0;
    }

    char* properties[] = {
        "ENTRADAS_TLB",    
        "REEMPLAZO_TLB",
        "IP_MEMORIA",
        "PUERTO_MEMORIA",
        "PUERTO_ESCUCHA_DISPATCH",
        "PUERTO_ESCUCHA_INTERRUPT",
        NULL
    };

    if(!config_has_all_properties(cfg_aux, properties)) {
        log_error(cpu_logger, "Faltan props en la config");
        config_destroy(cfg_aux);
        return 0;
    }

    cpu_config->ENTRADAS_TLB = config_get_int_value(cfg_aux,"ENTRADAS_TLB");
    cpu_config->REEMPLAZO_TLB = strdup(config_get_string_value(cfg_aux, "REEMPLAZO_TLB"));
    cpu_config->RETARDO_INSTRUCCION = config_get_int_value(cfg_aux,"RETARDO_INSTRUCCION");
    cpu_config->IP_MEMORIA = strdup(config_get_string_value(cfg_aux, "IP_MEMORIA"));
    cpu_config->PUERTO_MEMORIA = config_get_int_value(cfg_aux,"PUERTO_MEMORIA");    
    cpu_config->PUERTO_ESCUCHA_DISPATCH = config_get_int_value(cfg_aux,"PUERTO_ESCUCHA_DISPATCH");    
    cpu_config->PUERTO_ESCUCHA_INTERRUPT = config_get_int_value(cfg_aux,"PUERTO_ESCUCHA_INTERRUPT");    

    log_info(cpu_logger, "Archivo de configuracion cargado correctamente");

    config_destroy(cfg_aux);

    return 1;
}

// Init Logger, Config & Servers Dispatch + Interrupt
void initializeCpu(char* config_path) {

    // Logger
    cpu_logger = log_create("./cfg/cpu.log", "CPU", true, LOG_LEVEL_DEBUG);

    // Config
    cpu_config = malloc(sizeof(t_config_cpu));
    if (!cargar_configuracion(config_path)){
        exit(EXIT_FAILURE);
    }

    // Semaphores
    initializeSemaphores();

    memory_handshake();

    init_mmu();

    // INTERRUPT server
    if (!server_create(cpu_config, cpu_logger, &fd_server_interrupt, SERVER_INTERRUPT)) {
        exit(EXIT_FAILURE);
    }

    // DISPATCH server
    if (!server_create(cpu_config, cpu_logger, &fd_server_dispatch, SERVER_DISPATCH)) {
        exit(EXIT_FAILURE);
    }

    // TLB init

    inicializar_TLB();
    
    
}

uint32_t page_table_entries_qty; 
uint32_t page_size;

void memory_handshake(){
    // Connect to memory
    char* puerto_memoria = string_itoa(cpu_config->PUERTO_MEMORIA);
    fd_memoria = crear_conexion (
        cpu_logger, 
        "MEMORIA", 
        cpu_config->IP_MEMORIA, 
        puerto_memoria
    );
    free(puerto_memoria);

    send_msg_cpu_memoria_init(fd_memoria);

    t_package* packageHandshake = package_create(NULL_HEADER);
    package_recv(packageHandshake, fd_memoria);
    if(packageHandshake->msg_header != MSG_MEMORIA_CPU_INIT){
        log_error(cpu_logger,"SE DEBE RECIBIR EL HANDSHAKE DESDE MEMORIA");
        exit(EXIT_FAILURE);
    }

    recv_msg_memoria_cpu_init(packageHandshake->buffer, &page_table_entries_qty, &page_size);
    log_debug(cpu_logger, "HANDSHAKE CON MEMORIA RECIBIDO -- ENTRADAS_TABLA: %d -- PAGE_SIZE: %d", page_table_entries_qty, page_size);
    package_destroy(packageHandshake);
}

// Server (Dispatch & Interrupt)
int server_create(t_config_cpu* cpu_config, t_log* cpu_logger, int* fd_server, char* server_name) {
    
    char* server_port;

    if (!strcmp(server_name, SERVER_DISPATCH)) {
        server_port = string_itoa(cpu_config->PUERTO_ESCUCHA_DISPATCH);
    }
    
    if (!strcmp(server_name, SERVER_INTERRUPT)) {
        server_port = string_itoa(cpu_config->PUERTO_ESCUCHA_INTERRUPT);
    }

    *fd_server = iniciar_servidor(
        cpu_logger,
        server_name,
        "0.0.0.0",
        server_port
    );

    if (*fd_server != -1)
        log_info(cpu_logger, "%s server listo escuchando en puerto %s", server_name, server_port);
    else
        log_error(cpu_logger, "Error al iniciar %s server en puerto %s", server_name, server_port);

    free(server_port);

    return *fd_server != -1;

}


// Close program clean-up
void close_program() {
    
    if (fd_server_dispatch > 0) {
        liberar_conexion(fd_server_dispatch);
    }
    if (fd_server_interrupt > 0) {
        liberar_conexion(fd_server_interrupt);
    }

    free(cpu_config->IP_MEMORIA);
    free(cpu_config->REEMPLAZO_TLB);
    free(cpu_config);

    log_destroy(cpu_logger);

    rl_clear_history();
}

// Handle exit via CTRL+C
void handle_exit(int signal){
    log_info(cpu_logger,"FINALIZAMOS EL PROCESO MAIN");
    close_program();
    exit(EXIT_SUCCESS);
}
