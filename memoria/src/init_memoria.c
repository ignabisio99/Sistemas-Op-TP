#include "init_memoria.h"

// Declarar globals
t_log *memoria_logger;
t_config_memoria *memoria_config;
int fd_server_memoria;
int fd_kernel_init;
int fd_kernel_page_fault;
int fd_kernel_exit;
int fd_cpu;
// Sync
sem_t BLOQUEADOR;

int server_create(t_config_memoria *memoria_config, t_log *memoria_logger, int *fd_server, char *server_name);

void initializeSemaphores()
{
    // Main blocking
    sem_init(&BLOQUEADOR, 0, 0);
}

uint8_t cargar_configuracion(char *config_path)
{
    t_config *cfg_aux = config_create(config_path);

    if (cfg_aux == NULL)
    {
        log_error(memoria_logger, "La ruta de la config es erronea");
        return 0;
    }

    char *properties[] = {
        "PUERTO_ESCUCHA",
        "TAM_MEMORIA",
        "TAM_PAGINA",
        "ENTRADAS_POR_TABLA",
        "RETARDO_MEMORIA",
        "ALGORITMO_REEMPLAZO",
        "MARCOS_POR_PROCESO",
        "RETARDO_SWAP",
        "PATH_SWAP",
        "TAMANIO_SWAP",
        NULL};

    if (!config_has_all_properties(cfg_aux, properties))
    {
        log_error(memoria_logger, "Faltan props en la config");
        config_destroy(cfg_aux);
        return 0;
    }

    memoria_config->PUERTO_ESCUCHA = config_get_int_value(cfg_aux, "PUERTO_ESCUCHA");
    memoria_config->TAM_MEMORIA = config_get_int_value(cfg_aux, "TAM_MEMORIA");
    memoria_config->TAM_PAGINA = config_get_int_value(cfg_aux, "TAM_PAGINA");
    memoria_config->ENTRADAS_POR_TABLA = config_get_int_value(cfg_aux, "ENTRADAS_POR_TABLA");
    memoria_config->RETARDO_MEMORIA = config_get_int_value(cfg_aux, "RETARDO_MEMORIA");
    memoria_config->ALGORITMO_REEMPLAZO = strdup(config_get_string_value(cfg_aux, "ALGORITMO_REEMPLAZO"));
    memoria_config->MARCOS_POR_PROCESO = config_get_int_value(cfg_aux, "MARCOS_POR_PROCESO");
    memoria_config->RETARDO_SWAP = config_get_int_value(cfg_aux, "RETARDO_SWAP");
    memoria_config->PATH_SWAP = strdup(config_get_string_value(cfg_aux, "PATH_SWAP"));
    memoria_config->TAMANIO_SWAP = config_get_int_value(cfg_aux, "TAMANIO_SWAP");

    log_info(memoria_logger, "Archivo de configuracion cargado correctamente");

    config_destroy(cfg_aux);

    return 1;
}

void initializeMemoria(char *config_path)
{
    memoria_logger = log_create("./cfg/memoria.log", "MEMORIA", true, LOG_LEVEL_DEBUG);
    memoria_config = malloc(sizeof(t_config_memoria));
    if (!cargar_configuracion(config_path))
    {
        exit(EXIT_FAILURE);
    }
    initializeSemaphores();
    create_memory_space(memoria_config->TAM_MEMORIA, memoria_config->TAM_PAGINA);
    create_page_tables_structure();
    create_swap_file();
    
    // MEMORIA server
    if (!server_create(memoria_config, memoria_logger, &fd_server_memoria, SERVER_MEMORIA))
    {
        exit(EXIT_FAILURE);
    }
}

// Server
int server_create(t_config_memoria *memoria_config, t_log *memoria_logger, int *fd_server, char *server_name)
{

    char *server_port;

    server_port = string_itoa(memoria_config->PUERTO_ESCUCHA);

    *fd_server = iniciar_servidor(
        memoria_logger,
        server_name,
        "0.0.0.0",
        server_port);

    if (*fd_server != -1)
        log_info(memoria_logger, "%s server listo escuchando en puerto %s", server_name, server_port);
    else
        log_error(memoria_logger, "Error al iniciar %s server en puerto %s", server_name, server_port);

    free(server_port);

    return *fd_server != -1;
}

// Close program clean-up
void close_program()
{
    close_swap_file();
    if (fd_server_memoria > 0)
    {
        liberar_conexion(fd_server_memoria);
    }
    if (fd_cpu > 0)
    {
        liberar_conexion(fd_cpu);
    }
    if (fd_kernel_init > 0)
    {
        liberar_conexion(fd_kernel_init);
    }
    if (fd_kernel_page_fault > 0)
    {
        liberar_conexion(fd_kernel_page_fault);
    }
    if (fd_kernel_exit > 0)
    {
        liberar_conexion(fd_kernel_exit);
    }

    free(memoria_config->ALGORITMO_REEMPLAZO);
    free(memoria_config->PATH_SWAP);
    free(memoria_config);

    log_destroy(memoria_logger);

    rl_clear_history();
}

// Handle exit via CTRL+C
void handle_exit(int signal)
{
    log_info(memoria_logger, "FINALIZAMOS EL PROCESO MAIN");
    close_program();
    exit(EXIT_SUCCESS);
}