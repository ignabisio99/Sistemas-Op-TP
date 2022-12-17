#include "swap.h"

FILE* swap_file;
uint32_t cantidad_de_marcos_de_swap = 0;
pthread_mutex_t MUTEX_FILE_SWAP;
pthread_mutex_t MUTEX_BIT_ARRAY;

//Con true podemos ver si un marco del swap está ocupado. Con false está libre
bool* bitarray_marcos_de_swap;

void _write(void* page_data, uint32_t posicion_swap);
void* _read(uint32_t posicion_swap);

void _initialize_file(){
    uint32_t* initial_page_values = malloc(memoria_config->TAM_PAGINA);
    for(int j = 0; j < memoria_config->TAM_PAGINA / sizeof(uint32_t); j++){
        initial_page_values[j] = 0; 
    }
    for(int i = 0; i < cantidad_de_marcos_de_swap; i++){
        _write((void*) initial_page_values, i * memoria_config->TAM_PAGINA);
    }
    free(initial_page_values);
}

void _initialize_bitarray_swap(){
    bitarray_marcos_de_swap = malloc(cantidad_de_marcos_de_swap * sizeof(bool));
    for(int i = 0; i < cantidad_de_marcos_de_swap; i++){
        bitarray_marcos_de_swap[i] = false;
    }
}

bool get_bitarray_value(uint32_t position){

    pthread_mutex_lock(&MUTEX_BIT_ARRAY);
        bool res = bitarray_marcos_de_swap[position];
    pthread_mutex_unlock(&MUTEX_BIT_ARRAY);

    return res;
}

void set_bitarray_value(uint32_t position, bool value){
    pthread_mutex_lock(&MUTEX_BIT_ARRAY);
        bitarray_marcos_de_swap[position] = value;
    pthread_mutex_unlock(&MUTEX_BIT_ARRAY);
}

void create_swap_file(){
    cantidad_de_marcos_de_swap = memoria_config->TAMANIO_SWAP / memoria_config->TAM_PAGINA;
    _initialize_bitarray_swap();
    pthread_mutex_init(&MUTEX_FILE_SWAP, NULL);
    pthread_mutex_init(&MUTEX_BIT_ARRAY, NULL);

    swap_file = fopen(memoria_config->PATH_SWAP,"wb+");
    if(swap_file == NULL){
        log_error(memoria_logger, "Error creando/abriendo el archivo.");
        exit(EXIT_FAILURE);
    }
    ftruncate(fileno(swap_file),memoria_config->TAMANIO_SWAP);
    _initialize_file();

    log_debug(memoria_logger,"Archivo swap creado");
}

void close_swap_file(){
    fclose(swap_file);
}

uint32_t create_page_in_swap(){
    int i = 0;
    
    // Buscamos el indice y cortamos en el primer false
    while(i < cantidad_de_marcos_de_swap && get_bitarray_value(i)){
        i++;
    }

    uint32_t pos_swap = i * memoria_config->TAM_PAGINA;
    set_bitarray_value(i, true);
    return pos_swap;
}

void remove_page_from_swap(uint32_t pos_en_swap){
    set_bitarray_value(pos_en_swap / memoria_config->TAM_PAGINA, false);
}

void _write(void* page_data, uint32_t posicion_swap){

    pthread_mutex_lock(&MUTEX_FILE_SWAP);
        fseek(swap_file, posicion_swap, SEEK_SET);
        fwrite(page_data, memoria_config->TAM_PAGINA, 1, swap_file);
    pthread_mutex_unlock(&MUTEX_FILE_SWAP);

}

void write_swap_file(void* page_data, uint32_t posicion_swap){

    usleep(memoria_config->RETARDO_SWAP * 1000);
    _write(page_data, posicion_swap);

}

void* _read(uint32_t posicion_swap){
    void* swap_page_data = malloc(memoria_config->TAM_PAGINA);

    pthread_mutex_lock(&MUTEX_FILE_SWAP);
        fseek(swap_file, posicion_swap, SEEK_SET);
        fread(swap_page_data, memoria_config->TAM_PAGINA, 1, swap_file);
    pthread_mutex_unlock(&MUTEX_FILE_SWAP);
    return swap_page_data;
}
void* read_swap_file(uint32_t posicion_swap){
    usleep(memoria_config->RETARDO_SWAP * 1000);
    void* swap_page_data = _read(posicion_swap);

    return swap_page_data;
}