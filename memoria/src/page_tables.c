#include "page_tables.h"

// Module variables

t_list* lista_tablas_de_paginas_por_proceso;
pthread_mutex_t MUTEX_LISTA_TABLAS_PAGINAS;
bool is_structure_created = false;
bool *bitarray_marcos_de_memoria;
pthread_mutex_t MUTEX_BIT_ARRAY_MEMORY;

// Internal methods declaration

void algoritmo_clock(uint32_t numero_pid,t_entrada_tabla_de_paginas* pagina_a_reemplazar,uint32_t segment_number,uint32_t page_number);
void algoritmo_clock_modificado(uint32_t numero_pid,t_entrada_tabla_de_paginas* pagina_a_reemplazar,uint32_t segment_number,uint32_t page_number);
bool reemplazar_pagina(t_pid_tables* proceso_obtenido,uint32_t cant_segmentos,t_entrada_tabla_de_paginas* pagina_a_reemplazar, uint32_t bitUso, uint32_t bitModificado,uint32_t segment_number,uint32_t page_number);
int32_t _hay_marcos_libres_por_proceso(uint32_t numero_pid);
int32_t _frame_disponible();
t_pid_tables* _obtener_proceso(uint32_t numero_pid);
t_entrada_tabla_de_paginas* _create_default_page();
t_pid_tables* _find_pid(uint32_t pid, t_list* lista);
bool _get_bitarray_value_memory(uint32_t position);
void _set_bitarray_value_memory(uint32_t position, bool value);
void _initialize_bit_array_memory();
void _mover_de_swap_a_memoria(uint32_t frame_disponible, t_entrada_tabla_de_paginas* pagina_a_reemplazar);

// Exported methods

void create_page_tables_structure(){
    if(is_structure_created){
        log_error(memoria_logger, "Se está tratando de crear dos veces la estructura de tablas de pags. Aborting...");
        exit(EXIT_FAILURE);
    }
    lista_tablas_de_paginas_por_proceso = list_create();
    pthread_mutex_init(&MUTEX_LISTA_TABLAS_PAGINAS, NULL);
    _initialize_bit_array_memory();
    is_structure_created = true;

}

uint32_t techo_numero(uint32_t a, uint32_t b){
    uint32_t aditivo = (a % b == 0) ? 0 : 1;
    uint32_t division = a / b;
    return division + aditivo;
}

// Añade a la lista general una entrada de PID y lista de segmentos
// -- Por cada segmento va a guardar su tabla de paginas y un sem mutex
// -- -- Por cada tabla de páginas guardamos todas las columnas de t_entrada_tabla_de_paginas
void create_process(uint32_t pid, t_list* segments){
    if(!is_structure_created){
        log_error(memoria_logger, "Se debe crear primero la lista de tablas y luego crear un proceso. Aborting...");
        exit(EXIT_FAILURE);
    }

    uint32_t segments_qty = list_size(segments);
    t_pid_tables* pid_table = malloc(sizeof(t_pid_tables));
    pid_table->pid = pid;
    pid_table->segmentos = list_create();
    pid_table->ultima_referencia_pagina = 0;
    pid_table->ultima_referencia_segmento = 0;

    for(int i = 0; i < segments_qty; i++){
        uint32_t *tamano_segmento = (uint32_t*) list_get(segments, i);

        t_memory_segment* segmento = malloc(sizeof(t_memory_segment));
        segmento->tabla_de_paginas = list_create();
        pthread_mutex_init(&(segmento->mutex_tabla_de_paginas), NULL);

        int32_t cant_pags;
        if((*tamano_segmento / memoria_config->TAM_PAGINA) < 1){
            cant_pags = 1;
        } else {
            cant_pags = techo_numero(*tamano_segmento, memoria_config->TAM_PAGINA);
        }

        //Divido al segmento en x paginas
        for(int j = 0; j < cant_pags; j++){
            t_entrada_tabla_de_paginas* pagina = _create_default_page();

            uint32_t pos_swap = create_page_in_swap();
            pagina->pos_en_swap = pos_swap;

            pthread_mutex_lock(&(segmento->mutex_tabla_de_paginas));
                list_add(segmento->tabla_de_paginas, pagina);          // Agregamos una pagina a la tabla de paginas del segmento
            pthread_mutex_unlock(&(segmento->mutex_tabla_de_paginas));
        }
        list_add(pid_table->segmentos, segmento);
        log_info(memoria_logger, "PID: %d - Segmento: %d - TAMAÑO: %d paginas", pid, i, cant_pags);
    }
    pthread_mutex_lock(&MUTEX_LISTA_TABLAS_PAGINAS);
        list_add(lista_tablas_de_paginas_por_proceso, pid_table);          // Agregamos todos los segmentos asociados a un PID
    pthread_mutex_unlock(&MUTEX_LISTA_TABLAS_PAGINAS);
}

t_entrada_tabla_de_paginas* get_page_data(uint32_t pid, uint32_t segment_number, uint32_t page_number){
    pthread_mutex_lock(&MUTEX_LISTA_TABLAS_PAGINAS);
    t_pid_tables* pid_table = _find_pid(pid, lista_tablas_de_paginas_por_proceso);
    pthread_mutex_unlock(&MUTEX_LISTA_TABLAS_PAGINAS);

    t_memory_segment* segment = list_get(pid_table->segmentos, segment_number);

    pthread_mutex_lock(&(segment->mutex_tabla_de_paginas));
        t_entrada_tabla_de_paginas* pagina = list_get(segment->tabla_de_paginas, page_number);
    pthread_mutex_unlock(&(segment->mutex_tabla_de_paginas));

    return pagina;
}

// Función para obtener frame de una pagina en particular
// Si retorna 0 es pagefault
// Si retorna 1 salió todo ok y cargo en la variable frame_response
bool get_page_frame(uint32_t pid, uint32_t segment_number, uint32_t page_number,uint32_t* frame_response){
    t_entrada_tabla_de_paginas* pagina = get_page_data(pid, segment_number, page_number);
    if(pagina->presencia == 0){
        return 0;
    } else{
        *frame_response = pagina->frame;
        return 1;
    }
}

void agregar_pagina_a_memoria (uint32_t numero_pid, uint32_t segment_number, uint32_t page_number){

    t_entrada_tabla_de_paginas* pagina_a_reemplazar = get_page_data(numero_pid, segment_number, page_number);

    int32_t frame_disponible = _hay_marcos_libres_por_proceso(numero_pid);
    if(frame_disponible != -1){        
        _mover_de_swap_a_memoria(frame_disponible, pagina_a_reemplazar);
        log_info(memoria_logger,
            "SWAP IN - PID: %d - Marco: %d - Page In: %d | %d",
            numero_pid, frame_disponible, 
            segment_number,page_number);    
    } else {
        if (strcmp(memoria_config->ALGORITMO_REEMPLAZO,"CLOCK") == 0) algoritmo_clock(numero_pid,pagina_a_reemplazar,segment_number,page_number);
        if (strcmp(memoria_config->ALGORITMO_REEMPLAZO,"CLOCK-M") == 0) algoritmo_clock_modificado(numero_pid,pagina_a_reemplazar,segment_number,page_number);
    }
}


// Internal utils

void _mover_de_swap_a_memoria(uint32_t frame_disponible, t_entrada_tabla_de_paginas* pagina_a_reemplazar){
    _set_bitarray_value_memory(frame_disponible, true);
    void* data = read_swap_file(pagina_a_reemplazar->pos_en_swap);
    write_page(frame_disponible, data);
    
    free(data);

    pagina_a_reemplazar->presencia = 1;
    pagina_a_reemplazar->uso = 1;
    pagina_a_reemplazar->modificado = 0;
    pagina_a_reemplazar->frame = frame_disponible;
}

void algoritmo_clock(uint32_t numero_pid,t_entrada_tabla_de_paginas* pagina_a_reemplazar,uint32_t segment_number,uint32_t page_number){
    bool reemplazo_exitoso = false;
    t_pid_tables* proceso_obtenido = _obtener_proceso(numero_pid);
    uint32_t cant_segmentos = list_size(proceso_obtenido->segmentos);

    reemplazo_exitoso = reemplazar_pagina(proceso_obtenido,cant_segmentos,pagina_a_reemplazar,2,2,segment_number,page_number);

    // Ejecutamos de nuevo el algoritmo de busqueda en caso de que no se haya encontrado victima
    if(!reemplazo_exitoso){
        reemplazo_exitoso = reemplazar_pagina(proceso_obtenido,cant_segmentos,pagina_a_reemplazar,2,2,segment_number,page_number);
    }
}

void algoritmo_clock_modificado(uint32_t numero_pid,t_entrada_tabla_de_paginas* pagina_a_reemplazar,uint32_t segment_number,uint32_t page_number){
    bool reemplazo_exitoso = false;
    t_pid_tables* proceso_obtenido = _obtener_proceso(numero_pid);
    uint32_t cant_segmentos = list_size(proceso_obtenido->segmentos);


    while(!reemplazo_exitoso){  
        
        // Primero se busca por uso y modificado en 0
        reemplazo_exitoso = reemplazar_pagina(proceso_obtenido,cant_segmentos,pagina_a_reemplazar,0,0,segment_number,page_number);

        // Si no se encontro victima, se busca por U=0 Y M=1, bajando el bit de uso
        if(!reemplazo_exitoso){
            reemplazo_exitoso = reemplazar_pagina(proceso_obtenido,cant_segmentos,pagina_a_reemplazar,0,1,segment_number,page_number);
        }
    }
}

void _avanzar_puntero(t_pid_tables* proceso_obtenido, uint32_t cant_segmentos, uint32_t cant_paginas_segmento){
    if(proceso_obtenido->ultima_referencia_pagina == cant_paginas_segmento - 1){
        if(proceso_obtenido->ultima_referencia_segmento == cant_segmentos - 1){
            proceso_obtenido->ultima_referencia_segmento = 0;
            proceso_obtenido->ultima_referencia_pagina = 0;
        } else {
            proceso_obtenido->ultima_referencia_segmento++;
            proceso_obtenido->ultima_referencia_pagina = 0;
        }
    } else {
        proceso_obtenido->ultima_referencia_pagina ++;
    }
}

bool reemplazar_pagina(t_pid_tables* proceso_obtenido,uint32_t cant_segmentos,t_entrada_tabla_de_paginas* pagina_a_reemplazar, uint32_t bitUso, uint32_t bitModificado,uint32_t segment_number,uint32_t page_number){
    uint32_t cant_paginas_del_proceso = 0;
    
    for(int i = 0; i < cant_segmentos; i++){
        t_memory_segment* seg = list_get(proceso_obtenido->segmentos, i);
        pthread_mutex_lock(&seg->mutex_tabla_de_paginas);
            uint32_t  cant_paginas_segmento = list_size(seg->tabla_de_paginas);
        pthread_mutex_unlock(&seg->mutex_tabla_de_paginas);
        cant_paginas_del_proceso += cant_paginas_segmento;
    }

    for(int i = 0; i < cant_paginas_del_proceso; i++){
        t_memory_segment* segmento_obtenido = list_get(proceso_obtenido->segmentos, proceso_obtenido->ultima_referencia_segmento); 

        pthread_mutex_lock(&segmento_obtenido->mutex_tabla_de_paginas);
            uint32_t  cant_paginas_segmento = list_size(segmento_obtenido->tabla_de_paginas);
        pthread_mutex_unlock(&segmento_obtenido->mutex_tabla_de_paginas);
        
            
        pthread_mutex_lock(&segmento_obtenido->mutex_tabla_de_paginas);
            t_entrada_tabla_de_paginas* pagina_obtenida = list_get(segmento_obtenido->tabla_de_paginas, proceso_obtenido->ultima_referencia_pagina);
        pthread_mutex_unlock(&segmento_obtenido->mutex_tabla_de_paginas);
        
        // clock
        if(bitUso == 2 && bitModificado == 2){
            if(pagina_obtenida->presencia == 1 && pagina_obtenida->uso == 1){
                pagina_obtenida->uso = 0;
                _avanzar_puntero(proceso_obtenido, cant_segmentos, cant_paginas_segmento);
                log_info(memoria_logger, "Pagina existente en memoria en el proceso: %d",proceso_obtenido->pid);
                continue;
            }
    
            if(pagina_obtenida->presencia == 1 && pagina_obtenida->uso == 0){
                pagina_obtenida->presencia = 0;

                if(pagina_obtenida->modificado == 1){
                    // Escribir en swap ya que estaba modificado
                    void* target_data = read_page(pagina_obtenida->frame);
                    write_swap_file(target_data, pagina_obtenida->pos_en_swap);
                    log_info(memoria_logger,
                        "SWAP OUT - PID: %d - Marco: %d - Page Out: %d | %d",
                            proceso_obtenido->pid, pagina_obtenida->frame, 
                            proceso_obtenido->ultima_referencia_segmento,
                            proceso_obtenido->ultima_referencia_pagina);
                        
                    free(target_data);
                }

                _mover_de_swap_a_memoria(pagina_obtenida->frame, pagina_a_reemplazar);
                log_info(memoria_logger,
                    "SWAP IN - PID: %d - Marco: %d - Page In: %d | %d",
                    proceso_obtenido->pid, pagina_obtenida->frame, 
                    segment_number,page_number); 

                log_info(memoria_logger,
                    "REEMPLAZO - PID: %d - Marco: %d - Page Out: %d | %d - Page In: %d | %d",
                        proceso_obtenido->pid, pagina_obtenida->frame, 
                        proceso_obtenido->ultima_referencia_segmento,
                        proceso_obtenido->ultima_referencia_pagina,
                        segment_number,page_number);

                _avanzar_puntero(proceso_obtenido, cant_segmentos, cant_paginas_segmento);
                return true;
            }
        }

        // clockM 1era iteracion
        if (bitUso == 0 && bitModificado == 0){
            if((pagina_a_reemplazar == pagina_obtenida) && (pagina_obtenida->presencia == 1)) return true;
            if((pagina_obtenida->presencia == 1) && (pagina_obtenida->uso == 0) && (pagina_obtenida->modificado == 0)){
                pagina_obtenida->presencia = 0; 
                
                _mover_de_swap_a_memoria(pagina_obtenida->frame, pagina_a_reemplazar);
                log_info(memoria_logger,
                    "SWAP IN - PID: %d - Marco: %d - Page In: %d | %d",
                    proceso_obtenido->pid, pagina_obtenida->frame, 
                    segment_number,page_number);

                log_info(memoria_logger,
                    "REEMPLAZO - PID: %d - Marco: %d - Page Out: %d | %d - Page In: %d | %d",
                        proceso_obtenido->pid, pagina_obtenida->frame, 
                        proceso_obtenido->ultima_referencia_segmento,
                        proceso_obtenido->ultima_referencia_pagina,
                        segment_number,page_number);

                _avanzar_puntero(proceso_obtenido, cant_segmentos, cant_paginas_segmento);
                return true;           
            }
        }       

        // clockM segunda iteracion
        if (bitUso == 0 && bitModificado == 1){
            if(pagina_obtenida->presencia == 1 && pagina_obtenida->uso == 1){
                pagina_obtenida->uso = 0;

                _avanzar_puntero(proceso_obtenido, cant_segmentos, cant_paginas_segmento);
                continue;
            }
            if(pagina_obtenida->presencia == 1 && pagina_obtenida->uso == 0 && pagina_obtenida->modificado == 1){
                pagina_obtenida->presencia = 0; 
                
                // Escribir en swap ya que estaba modificado
                void* target_data = read_page(pagina_obtenida->frame);
                write_swap_file(target_data, pagina_obtenida->pos_en_swap);
                log_info(memoria_logger,
                        "SWAP OUT - PID: %d - Marco: %d - Page Out: %d | %d",
                            proceso_obtenido->pid, pagina_obtenida->frame, 
                            proceso_obtenido->ultima_referencia_segmento,
                            proceso_obtenido->ultima_referencia_pagina);

                free(target_data);

                // Mover de swap a memoria
                _mover_de_swap_a_memoria(pagina_obtenida->frame, pagina_a_reemplazar);
                log_info(memoria_logger,
                    "SWAP IN - PID: %d - Marco: %d - Page In: %d | %d",
                    proceso_obtenido->pid, pagina_obtenida->frame, 
                    segment_number,page_number);
                    
                log_info(memoria_logger,
                    "REEMPLAZO - PID: %d - Marco: %d - Page Out: %d | %d - Page In: %d | %d",
                        proceso_obtenido->pid, pagina_obtenida->frame, 
                        proceso_obtenido->ultima_referencia_segmento,
                        proceso_obtenido->ultima_referencia_pagina,
                        segment_number,page_number);
                _avanzar_puntero(proceso_obtenido, cant_segmentos, cant_paginas_segmento);

                return true;
            }
        }
        _avanzar_puntero(proceso_obtenido, cant_segmentos, cant_paginas_segmento);
    }
    return false;
}

int32_t _hay_marcos_libres_por_proceso(uint32_t numero_pid){

    uint32_t cant_frames_ocupados = 0;
    t_pid_tables* proceso_obtenido = _obtener_proceso(numero_pid);    
    uint32_t cant_segmentos = list_size(proceso_obtenido->segmentos);
    for(int i = 0; i<cant_segmentos;i++){
        t_memory_segment* segmento_obtenido = list_get(proceso_obtenido->segmentos,i);
        uint32_t  cant_paginas_segmento = list_size(segmento_obtenido->tabla_de_paginas);
        for(int j = 0;j<cant_paginas_segmento;j++){
            t_entrada_tabla_de_paginas* pagina_obtenida = list_get(segmento_obtenido->tabla_de_paginas,j);
            if(pagina_obtenida->presencia == 1) cant_frames_ocupados++;
        }
        
    }
    if (cant_frames_ocupados < memoria_config->MARCOS_POR_PROCESO){
        int32_t frame_asignado = _frame_disponible();
        log_info(memoria_logger,"Frame disponible: %d",frame_asignado);
        return frame_asignado;
    }
    return -1;
}

int32_t _frame_disponible(){
    for(int i = 0; i < (memoria_config->TAM_MEMORIA / memoria_config->TAM_PAGINA); i++){
        if(!_get_bitarray_value_memory(i)) return i;
    }
    return -1;
}

t_pid_tables* _obtener_proceso(uint32_t numero_pid){
    pthread_mutex_lock(&MUTEX_LISTA_TABLAS_PAGINAS);
        uint32_t cant_procesos = list_size(lista_tablas_de_paginas_por_proceso);
    pthread_mutex_unlock(&MUTEX_LISTA_TABLAS_PAGINAS);

    for(int i=0; i < cant_procesos; i++){
        pthread_mutex_lock(&MUTEX_LISTA_TABLAS_PAGINAS);
            t_pid_tables* proceso_obtenido = list_get(lista_tablas_de_paginas_por_proceso,i);
        pthread_mutex_unlock(&MUTEX_LISTA_TABLAS_PAGINAS);
        if(proceso_obtenido->pid == numero_pid) return proceso_obtenido;
    }
    return NULL;
}

t_pid_tables* _find_pid(uint32_t pid, t_list* lista) {
    
    int _is_the_one(t_pid_tables* p){
        return pid == p->pid;
    }

    return list_find(lista, (void*) _is_the_one);
}

t_entrada_tabla_de_paginas* _create_default_page(){
    t_entrada_tabla_de_paginas* pagina = malloc(sizeof(t_entrada_tabla_de_paginas));
    pagina->frame = UINT32_MAX; //Seteo como default el max ya que es un valor que nunca tendrá
    pagina->modificado = 0;
    pagina->presencia = 0;
    pagina->uso = 1;
    pagina->pos_en_swap = UINT32_MAX;  //Seteo como default el max ya que es un valor que nunca tendrá

    return pagina;
}

bool _get_bitarray_value_memory(uint32_t position)
{
    pthread_mutex_lock(&MUTEX_BIT_ARRAY_MEMORY);
    bool res = bitarray_marcos_de_memoria[position];
    pthread_mutex_unlock(&MUTEX_BIT_ARRAY_MEMORY);

    return res;
}

void _set_bitarray_value_memory(uint32_t position, bool value){
    pthread_mutex_lock(&MUTEX_BIT_ARRAY_MEMORY);
        bitarray_marcos_de_memoria[position] = value;
    pthread_mutex_unlock(&MUTEX_BIT_ARRAY_MEMORY);
}

void _initialize_bit_array_memory()
{
    pthread_mutex_init(&MUTEX_BIT_ARRAY_MEMORY,NULL);
    uint32_t cantidad_marcos = memoria_config->TAM_MEMORIA / memoria_config->TAM_PAGINA;
    
    bitarray_marcos_de_memoria = malloc((cantidad_marcos) * sizeof(bool));
    for (int i = 0; i < (cantidad_marcos); i++)
    {
        bitarray_marcos_de_memoria[i] = false;
    }
}

void finalizar_proceso(uint32_t pid){

    // buscamos en la lista de tablas de paginas de un proceso t_pid_table

    t_pid_tables* proceso_obtenido = _find_pid(pid, lista_tablas_de_paginas_por_proceso);

    // seteamos en todas las paginas de todos los segmentos su precencia en 0
    
    
    uint32_t cant_segmentos_proceso = list_size(proceso_obtenido->segmentos);
    

    for(int i = 0; i<cant_segmentos_proceso; i++){

        t_memory_segment* segmento_obtenido = list_get(proceso_obtenido->segmentos,i); 
         pthread_mutex_lock(&segmento_obtenido->mutex_tabla_de_paginas);
            uint32_t  cant_paginas_segmento = list_size(segmento_obtenido->tabla_de_paginas);
        pthread_mutex_unlock(&segmento_obtenido->mutex_tabla_de_paginas);

        
        for(int j = 0; j<cant_paginas_segmento; j++){
            pthread_mutex_lock(&segmento_obtenido->mutex_tabla_de_paginas);
            t_entrada_tabla_de_paginas* pagina_obtenida = list_get(segmento_obtenido->tabla_de_paginas, proceso_obtenido->ultima_referencia_pagina);
            pthread_mutex_unlock(&segmento_obtenido->mutex_tabla_de_paginas);

            if(pagina_obtenida->presencia == 1){
                pagina_obtenida->presencia = 0;
                _set_bitarray_value_memory(pagina_obtenida->frame,0);
                remove_page_from_swap(pagina_obtenida->pos_en_swap);
            }
        }
        log_info(memoria_logger,"FINALIZO PID: %d - Segmento: %d - TAMAÑO: %d paginas", 
                        pid,i,cant_paginas_segmento);
    }
    

}