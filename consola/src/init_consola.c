#include "init_consola.h"

// Declarar globals
t_log* consola_logger;
t_config_consola* consola_config;
t_list* listaDeInstrucciones;

uint8_t cargar_configuracion(char* config_path) {
    t_config* cfg_aux = config_create(config_path);

    if(cfg_aux == NULL) {
        log_error(consola_logger, "La ruta de la config es erronea");
        return 0;
    }

    char* properties[] = {
        "IP_KERNEL",    
        "PUERTO_KERNEL",
        "SEGMENTOS",
        "TIEMPO_PANTALLA",
        NULL
    };

    if(!config_has_all_properties(cfg_aux, properties)) {
        log_error(consola_logger, "Faltan props en la config");
        config_destroy(cfg_aux);
        return 0;
    }

    consola_config->IP_KERNEL = strdup(config_get_string_value(cfg_aux, "IP_KERNEL"));
    consola_config->PUERTO_KERNEL = config_get_int_value(cfg_aux,"PUERTO_KERNEL");
    consola_config->SEGMENTOS = convert_string_array_to_int_array(config_get_array_value(cfg_aux,"SEGMENTOS"));
    consola_config->TIEMPO_PANTALLA = config_get_int_value(cfg_aux,"TIEMPO_PANTALLA");;

    log_info(consola_logger, "Archivo de configuracion cargado correctamente");

    config_destroy(cfg_aux);

    return 1;
}

void initializeConsola(char* config_path,char* pseudocodigo_path){
    consola_logger = log_create("./cfg/consola.log", "CONSOLA", true, LOG_LEVEL_DEBUG);
    consola_config = malloc(sizeof(t_config_consola));
    if (!cargar_configuracion(config_path)){
        exit(EXIT_FAILURE);
    }
    listaDeInstrucciones = list_create();
    crearInstrucciones(pseudocodigo_path);
}

void crearInstrucciones(char* path_pseudocodigo){

    FILE* pseudocodigo = fopen(path_pseudocodigo,"r");
    char* puntero = 0;
    int tamanio = 0;
    int cantIdentificador = 0;
    char caracter = fgetc(pseudocodigo);
    bool isExit = false;
    int instruction_counter = 0;

    while (caracter != EOF && !isExit){
        t_instruction* instruccion = malloc(sizeof(t_instruction));
        while (cantIdentificador < 3 && caracter != EOF){
            while (caracter != ' ' && caracter != '\n' && caracter != EOF){
                puntero = (char*)realloc(puntero,tamanio + 1);
                puntero[tamanio] = (char)caracter;
                tamanio++;
                caracter = fgetc(pseudocodigo);
            }
            puntero[tamanio] = 0;
            if (cantIdentificador == 0){
                if(strcmp("EXIT",puntero) == 0) {
                    cantIdentificador = 3;
                    instruccion->param_1 = malloc(sizeof(puntero));
                    strcpy(instruccion->param_1,"");
                     instruccion->param_2 = malloc(sizeof(puntero));
                    strcpy(instruccion->param_2,"");
                    isExit = true;
                }
                instruccion->instruction_code = get_instruction_code_from_string(puntero);
            }
            if (cantIdentificador == 1){
                instruccion->param_1 = malloc(sizeof(puntero));
                strcpy(instruccion->param_1,puntero);
            }
            if (cantIdentificador == 2){
                instruccion->param_2 = malloc(sizeof(puntero));
                strcpy(instruccion->param_2,puntero);
            }    
            free(puntero);
            puntero = 0;
            tamanio = 0;
            cantIdentificador ++;
            caracter = fgetc(pseudocodigo);
        }

        list_add(listaDeInstrucciones,instruccion);
        cantIdentificador = 0; 

        log_debug(consola_logger, "Instrucción #%d:", ++instruction_counter);
        log_debug(consola_logger, "Código:\t\t%s", get_string_from_instruction_code(instruccion->instruction_code));
        log_debug(consola_logger, "Parámetro 1:\t%s", instruccion->param_1);
        log_debug(consola_logger, "Parámetro 2:\t%s", instruccion->param_2);

    }
    if(!isExit){
        log_error(consola_logger,"NO HAY INSTRUCCION DE EXIT ...abort");
        exit(EXIT_FAILURE);
    }
}


int generar_conexiones(t_config_consola* cfg, t_log* consola_logger, int* fd_kernel) {
    
    char* kernel_port = string_itoa(cfg->PUERTO_KERNEL);

    *fd_kernel = crear_conexion(
        consola_logger,
        "KERNEL",
        cfg->IP_KERNEL,
        kernel_port
    );

    free(kernel_port);

    return *fd_kernel != 0;
}


void close_program(t_config_consola* consola_config, t_log* consola_logger, int fd_kernel) {

    log_destroy(consola_logger);

    liberar_conexion(fd_kernel);
    free(consola_config->IP_KERNEL);
    free(consola_config);

    rl_clear_history();
}
