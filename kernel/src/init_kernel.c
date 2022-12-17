#include "init_kernel.h"

// Declarar globals
t_log* kernel_logger;
t_config_kernel* kernel_config;
int fd_server;




uint8_t cargar_configuracion(char* config_path) {
    t_config* cfg_aux = config_create(config_path);

    if(cfg_aux == NULL) {
        log_error(kernel_logger, "La ruta de la config es erronea");
        return 0;
    }

    char* properties[] = {
        "IP_MEMORIA",    
        "PUERTO_MEMORIA",
        "IP_CPU",
        "PUERTO_CPU_DISPATCH",
        "PUERTO_CPU_INTERRUPT",
        "PUERTO_ESCUCHA",
        "ALGORITMO_PLANIFICACION",
        "GRADO_MAX_MULTIPROGRAMACION",
        "DISPOSITIVOS_IO",
        "TIEMPOS_IO",
        "QUANTUM_RR",
        NULL
    };

    if(!config_has_all_properties(cfg_aux, properties)) {
        log_error(kernel_logger, "Faltan props en la config");
        config_destroy(cfg_aux);
        return 0;
    }

    kernel_config->IP_MEMORIA = strdup(config_get_string_value(cfg_aux, "IP_MEMORIA"));
    kernel_config->PUERTO_MEMORIA = config_get_int_value(cfg_aux,"PUERTO_MEMORIA");
    kernel_config->IP_CPU = strdup(config_get_string_value(cfg_aux, "IP_CPU"));
    kernel_config->PUERTO_CPU_DISPATCH = config_get_int_value(cfg_aux,"PUERTO_CPU_DISPATCH");
    kernel_config->PUERTO_CPU_INTERRUPT = config_get_int_value(cfg_aux,"PUERTO_CPU_INTERRUPT");
    kernel_config->PUERTO_ESCUCHA = config_get_int_value(cfg_aux,"PUERTO_ESCUCHA");
    kernel_config->ALGORITMO_PLANIFICACION = strdup(config_get_string_value(cfg_aux, "ALGORITMO_PLANIFICACION"));
    kernel_config->GRADO_MAX_MULTIPROGRAMACION = config_get_int_value(cfg_aux,"GRADO_MAX_MULTIPROGRAMACION");
    kernel_config->DISPOSITIVOS_IO = config_get_array_value(cfg_aux,"DISPOSITIVOS_IO");
    char **tiempos_io = config_get_array_value(cfg_aux,"TIEMPOS_IO");
    kernel_config->TIEMPOS_IO = convert_string_array_to_int_array(tiempos_io);
    string_array_destroy(tiempos_io);
    kernel_config->QUANTUM_RR = config_get_int_value(cfg_aux,"QUANTUM_RR");

    log_info(kernel_logger, "Archivo de configuracion cargado correctamente");

    config_destroy(cfg_aux);

    return 1;
}




void inicializar_planificadores(){
    
    pthread_t THREAD_LARGO_PLAZO;
    if(!pthread_create(&THREAD_LARGO_PLAZO, NULL, (void*) planificadorLargoPlazo, NULL))
        pthread_detach(THREAD_LARGO_PLAZO);
    else {
        log_error(kernel_logger, "ERROR CRITICO INICIANDO EL PLANIFICADOR DE LARGO PLAZO. ABORTANDO.");
        exit(EXIT_FAILURE);
    }

    // Comienza planificador de corto plazo
    pthread_t THREAD_CORTO_PLAZO;
    if(!pthread_create(&THREAD_CORTO_PLAZO, NULL, (void*) handler_corto_plazo, NULL))
        pthread_detach(THREAD_CORTO_PLAZO);
    else {
        log_error(kernel_logger, "ERROR CRITICO INICIANDO EL PLANIFICADOR DE CORTO PLAZO. ABORTANDO.");
        exit(EXIT_FAILURE);
    }
    
    
}


t_queue* COLA_READY_FIFO;
t_queue* COLA_READY_RR;
t_queue* COLA_EXIT;
t_queue* COLA_NEW;
t_list* LISTA_CONSOLAS;
t_list* LISTA_COLAS_DISPOSITIVOS; 

void initializeLists(){
    COLA_READY_FIFO = queue_create();
    COLA_READY_RR = queue_create();
    COLA_EXIT = queue_create();
    COLA_NEW = queue_create();
    LISTA_CONSOLAS = list_create();
    LISTA_COLAS_DISPOSITIVOS = list_create();    
}


void inicializarDispositivos(){
    char* p;
    uint32_t i = 0; //cantidad de dispositivos
    //fijarse la condicion del while (ver si no es null)
    while(kernel_config->DISPOSITIVOS_IO[i] != NULL){
        p = kernel_config->DISPOSITIVOS_IO[i];
        t_dispositivo* disp = malloc(sizeof(t_dispositivo));
        disp->COLA_IO = queue_create();
        pthread_mutex_init(&disp->MUTEX_IO,NULL);
        sem_init(&disp->SEM_IO,0,0);
        disp->NOMBRE = p;
        disp->TIEMPO_IO = kernel_config->TIEMPOS_IO[i];
        i++;
        list_add(LISTA_COLAS_DISPOSITIVOS,disp);
        pthread_t hilo;
        pthread_create(&hilo,NULL,(void*) atenderCola,(void*) disp);
        pthread_detach(hilo);
    }
}


void initializeKernel(char* config_path){
    
    kernel_logger = log_create("./cfg/kernel.log", "KERNEL", true, LOG_LEVEL_DEBUG);
    kernel_config = malloc(sizeof(t_config_kernel));
    
    if (!cargar_configuracion(config_path)){
        exit(EXIT_FAILURE);
    }
    if (!crear_servidor(kernel_config, kernel_logger, &fd_server, SERVERNAME)) {
        exit(EXIT_FAILURE);
    }
    
    initializeSemaphores();
    initializeLists();
    inicializarDispositivos();
    
}


int crear_servidor(t_config_kernel* kernel_config, t_log* kernel_logger, int* fd_server, char* server_name) {
    
    char* server_port = string_itoa(kernel_config->PUERTO_ESCUCHA);
    
    *fd_server = iniciar_servidor(
        kernel_logger,
        server_name,
        "0.0.0.0",
        server_port
    );

    if (*fd_server != -1)
        log_info(kernel_logger, "%s server listo escuchando en puerto %s", server_name, server_port);
    else
        log_error(kernel_logger, "Error al iniciar %s server en puerto %s", server_name, server_port);

    free(server_port);

    return *fd_server != -1;

}

    


